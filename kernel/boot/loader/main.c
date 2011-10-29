/*
 * /phoenix/kernel/boot/loader/main.c
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <defs.h>
#include <types.h>
#include <compat_types.h>
#include <boot.h>

#include <efi_sys.h>
#include <efilib.h>

#include <elf/libelf.h>

#include "loader.h"

/* Kernel command line converted to ASCII */
static char *kernelCmdLine;
/* Kernel file name */
static CHAR16 *kernelImage;
/* Boot parameters block passed to the kernel entry point. */
static BootParam bootParam;

/* Boot loader options */

static int optDebugger;

static struct Option {
    CHAR16 *name;
    int *flag;
} options[] = {
    { L"--debugger", &optDebugger }
};

static EFI_HANDLE imageHandle;
static EFI_LOADED_IMAGE *loadedImage;

static inline int
IsSpace(CHAR16 c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

static CHAR16 *
SkipSpaces(CHAR16 *str, UINT32 *size)
{
    while (*size && IsSpace(*str)) {
        str++;
        (*size)--;
    }
    return str;
}

static CHAR16 *
SkipWord(CHAR16 *str, UINT32 *size)
{
    while (*size && !IsSpace(*str)) {
        str++;
        (*size)--;
    }
    return str;
}

static int
WordCompare(CHAR16 *str, UINT32 *size, CHAR16 *word)
{
    UINT32 len = *size;
    while (len && *word && *str == *word) {
        len--;
        str++;
        word++;
    }
    if (*word || (*str && !IsSpace(*str))) {
        return -1;
    }
    return 0;
}

static void
WaitDebugger()
{
    /* Change this value to non-zero in a debugger when attached */
    volatile int isAttached = FALSE;
    Print(L"Image base: 0x%lx\n", loadedImage->ImageBase);
    Print(L"Image size: 0x%lx\n", loadedImage->ImageSize);
    Print(L"Waiting for debugger...");
    while (!isAttached) {
        __asm__ __volatile__("pause");
    }
    Print(L"Attached\n");
}

/*
 * Process command line options.
 * optionsSize: length of options string in characters. Actual string length
 *      may be shorter.
 */
static EFI_STATUS
ProcessOptions(CHAR16 *optionsStr, UINT32 optionsSize)
{
    struct Option *opt;
    UINT32 i;

    if (!optionsSize) {
        return EFI_INVALID_PARAMETER;
    }

    /* Correct options size if null terminator is before the buffer end. */
    for (i = 0; i < optionsSize; i++) {
        if (!optionsStr[i]) {
            optionsSize = i;
        }
    }

    /* Skip loader image path */
    optionsStr = SkipSpaces(optionsStr, &optionsSize);
    optionsStr = SkipWord(optionsStr, &optionsSize);

    do {
        size_t optIdx;
        opt = 0;
        optionsStr = SkipSpaces(optionsStr, &optionsSize);

        for (optIdx = 0; optIdx < sizeof(options) / sizeof(options[0]); optIdx++) {
            if (!WordCompare(optionsStr, &optionsSize, options[optIdx].name)) {
                opt = &options[optIdx];
                *opt->flag = 1;
                optionsStr = SkipWord(optionsStr, &optionsSize);
                break;
            }
        }
    } while (opt);

    if (!optionsSize) {
        Print(L"Kernel image not specified\n");
        return EFI_INVALID_PARAMETER;
    }

    /* Extract kernel image file name */
    i = optionsSize;
    CHAR16 *nextPtr = SkipWord(optionsStr, &i);
    kernelImage = (CHAR16 *)AllocatePool((nextPtr - optionsStr + 1) * sizeof(CHAR16));
    if (!kernelImage) {
        return EFI_OUT_OF_RESOURCES;
    }
    RtCopyMem(kernelImage, optionsStr, (nextPtr - optionsStr) * sizeof(CHAR16));
    kernelImage[nextPtr - optionsStr] = 0;

    /* Convert kernel command line to ASCII */
    kernelCmdLine = (char *)AllocatePool(optionsSize / sizeof(CHAR16) + 1);
    if (!kernelCmdLine) {
        FreePool(kernelImage);
        kernelImage = NULL;
        return EFI_OUT_OF_RESOURCES;
    }
    for (i = 0; i < optionsSize; i++) {
        kernelCmdLine[i] = (char)optionsStr[i];
    }
    kernelCmdLine[optionsSize] = 0;

    return EFI_SUCCESS;
}

static void *
_elf_malloc(size_t size)
{
    return AllocatePool(size);
}

static void
_elf_mfree(void *ptr)
{
    FreePool(ptr);
}

static void *
_elf_mrealloc(void *ptr, size_t size)
{
    FreePool(ptr);
    return AllocatePool(size);
}

typedef struct {
    UINTN size;
    INTN cur_offset;
    SIMPLE_READ_FILE file;
    EFI_FILE_HANDLE hFile;
} ImageFile;

static void
CloseImageFile(Elf_File *file)
{
    if (file->f_priv) {
        FreePool(file->f_priv);
    }
    FreePool(file);
}

static size_t
SeekImageFile(Elf_File *file, size_t offset, Elf_Seek_Whence whence)
{
    ImageFile *img = (ImageFile *)file->f_priv;
    switch (whence) {
    case ELF_SEEK_SET:
        img->cur_offset = offset;
        break;
    case ELF_SEEK_CUR:
        img->cur_offset += offset;
        break;
    case ELF_SEEK_END:
        img->cur_offset = img->size + offset;
        break;
    }
    if (img->cur_offset < 0) {
        img->cur_offset = 0;
    } else if (img->cur_offset > img->size) {
        img->cur_offset = img->size;
    }
    return img->cur_offset;
}

static size_t
ReadImageFileOff(Elf_File *file, u64 offset, char *buffer, size_t len)
{
    ImageFile *img = (ImageFile *)file->f_priv;
    UINTN read_len;
    if (len > img->size - offset) {
        read_len = img->size - offset;
    } else {
        read_len = len;
    }
    if (read_len && EFI_ERROR(ReadSimpleReadFile(img->file, offset,
                                                 &read_len, buffer))) {

        return 0;
    }
    return read_len;
}

static size_t
ReadImageFile(Elf_File *file, char *buffer, size_t len)
{
    ImageFile *img = (ImageFile *)file->f_priv;
    size_t read_len = ReadImageFileOff(file, img->cur_offset, buffer, len);
    img->cur_offset += read_len;
    return read_len;
}

static Elf_File *
OpenImageFile(SIMPLE_READ_FILE file)
{
    Elf_File *ef = (Elf_File *)AllocatePool(sizeof(*ef));
    if (!ef) {
        DbgPrint(D_INFO, "Memory allocation failed\n");
        return NULL;
    }

    RtZeroMem(ef, sizeof(*ef));
    ef->f_priv = AllocatePool(sizeof(ImageFile));
    if (!ef->f_priv) {
        DbgPrint(D_INFO, "Memory allocation failed\n");
        CloseImageFile(ef);
        return NULL;
    }
    ef->Close = CloseImageFile;
    ef->Seek = SeekImageFile;
    ef->Read = ReadImageFile;

    ImageFile *img = (ImageFile *)ef->f_priv;
    RtZeroMem(img, sizeof(*img));
    img->file = file;
    img->hFile = GetSimpleReadFileHandle(img->file);

    /* Get file size */
    EFI_FILE_INFO *info = LibFileInfo(img->hFile);
    if (!info) {
        DbgPrint(D_INFO, "Failed to get file info\n");
        CloseImageFile(ef);
        return NULL;
    }
    img->size = info->FileSize;
    FreePool(info);

    return ef;
}

void
LoaderPrintV(const WCHAR_T *fmt, va_list args)
{
    VPrint((WCHAR_T *)fmt, args);
}

void
LoaderPrint(const WCHAR_T *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    LoaderPrintV((WCHAR_T *)fmt, args);
    va_end(args);
}

WCHAR_T *
LoaderStrConvert(const char *str)
{
    size_t len, i;
    for (len = 0; str[len]; len++);
    CHAR16 *msgW = AllocatePool((len + 1) * sizeof(CHAR16));
    for (i = 0; i < len; i++) {
        msgW[i] = str[i];
    }
    msgW[len] = 0;
    return msgW;
}

int
LoaderGetMemory(vaddr_t address, u64 pages)
{
    EFI_STATUS rc;

    rc = uefi_call_wrapper(BS->AllocatePages, 4,
                           AllocateAddress,
                           EfiRuntimeServicesCode,
                           pages,
                           &address);
    return EFI_ERROR(rc) ? -1 : 0;
}

int
LoaderReadFile(Elf_File *file, u64 offset, u64 size, void *mem)
{
    while (size) {
        size_t num_read = ReadImageFileOff(file, offset, mem, size);
        if (!num_read) {
            return -1;
        }
        ASSERT(size >= num_read);
        size -= num_read;
        offset += num_read;
    }
    return 0;
}

typedef void (*KernelEntry)(BootParam *bootParam);

static EFI_STATUS
StartKernel(vaddr_t entry_addr)
{
    /* Prepare boot parameters */

    /* Kernel command line */
    bootParam.cmdLine = kernelCmdLine;
    for (bootParam.cmdLineSize = 0;
         bootParam.cmdLine[bootParam.cmdLineSize];
         bootParam.cmdLineSize++);
    bootParam.cmdLineSize++;

    /* Get memory map */
    UINTN mapKey, mapDescSize, numEntries;
    UINT32 mapDescVersion;
    EFI_MEMORY_DESCRIPTOR *map = LibMemoryMap(&numEntries, &mapKey,
                                              &mapDescSize, &mapDescVersion);
    if (!map) {
        Print(L"Failed to get memory map\n");
        return EFI_LOAD_ERROR;
    }
    bootParam.memMap = map;
    bootParam.memMapNumDesc = numEntries;
    bootParam.memMapDescSize = mapDescSize;
    bootParam.memMapDescVersion = mapDescVersion;

    /* Take control over the system */
    EFI_STATUS rc = uefi_call_wrapper(BS->ExitBootServices, 2,
                                      imageHandle, mapKey);
    if (EFI_ERROR(rc)) {
        Print(L"Failed to exit boot services (%r)\n", rc);
        return rc;
    }
    /* The system is ours */
    ST->ConsoleInHandle = NULL;
    ST->ConIn = NULL;
    ST->ConsoleOutHandle = NULL;
    ST->ConOut = NULL;
    ST->StandardErrorHandle = NULL;
    ST->StdErr = NULL;
    ST->BootServices = NULL;
    SetCrc(&ST->Hdr);
    bootParam.efiSystemTable = (paddr_t)ST;

    /* Pass control to the kernel */
    KernelEntry ke = (KernelEntry)entry_addr;
    ke(&bootParam);

    /* NOT REACHED */
    return EFI_SUCCESS;
}

static EFI_STATUS
LoadImage(SIMPLE_READ_FILE file)
{
    elf_malloc = _elf_malloc;
    elf_mfree = _elf_mfree;
    elf_mrealloc = _elf_mrealloc;

    Elf_File *ef = OpenImageFile(file);
    if (!ef) {
        Print(L"Failed to open image file\n");
        return EFI_LOAD_ERROR;
    }

    if (elf_version(EV_CURRENT) == EV_NONE) {
        CloseImageFile(ef);
        Print(L"ELF library initialization failed: %a", elf_errmsg(-1));
        return EFI_LOAD_ERROR;
    }

    Elf *elf = elf_begin(ef, ELF_C_READ, NULL);
    if (!elf) {
        CloseImageFile(ef);
        Print(L"Failed to open ELF file: %a\n", elf_errmsg(-1));
        return EFI_LOAD_ERROR;
    }

    vaddr_t entry_addr;
    EFI_STATUS rc = LoadElfImage(ef, elf, &entry_addr) ? EFI_LOAD_ERROR : EFI_SUCCESS;
    elf_end(elf);

    if (!EFI_ERROR(rc)) {
        rc = StartKernel(entry_addr);
    }
    return rc;
}

static EFI_STATUS
LoadKernel()
{
    DbgPrint(D_INFO, "Kernel image: '%s'\n", kernelImage);
    EFI_DEVICE_PATH *path;
    EFI_STATUS rc = EFI_SUCCESS;
    SIMPLE_READ_FILE readHandle = NULL;
    UINTN handleCount, handleIdx;
    EFI_HANDLE *handleBuffer;

    rc = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
                               ByProtocol,
                               &FileSystemProtocol,
                               NULL,
                               &handleCount,
                               &handleBuffer);
    if (EFI_ERROR(rc)) {
        return rc;
    }

    DbgPrint(D_INFO, "handleCount = %d\n", handleCount);

    for (handleIdx = 0; handleIdx < handleCount; handleIdx++) {
        EFI_HANDLE deviceHandle;

        path = FileDevicePath(handleBuffer[handleIdx], kernelImage);
        if (!path) {
            rc = EFI_NOT_FOUND;
            break;
        }

        rc = OpenSimpleReadFile(TRUE, NULL, 0, &path, &deviceHandle, &readHandle);
        if (!EFI_ERROR(rc)) {
            break;
        }

        FreePool(path);
        path = NULL;
    }

    if (!EFI_ERROR(rc)) {
        rc = LoadImage(readHandle);
    }

    if (readHandle) {
        CloseSimpleReadFile(readHandle);
    }
    if (path) {
        FreePool(path);
    }

    FreePool(handleBuffer);
    return rc;
}

EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status, rc;

    InitializeLib(image, systab);
    imageHandle = image;

    status = uefi_call_wrapper(BS->OpenProtocol,
                               6,
                               image,
                               &LoadedImageProtocol,
                               (void **)&loadedImage,
                               image,
                               NULL,
                               EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        Print(L"OpenProtocol: %r\n", status);
        return status;
    }

#   ifdef EFI_DEBUG
    DbgPrint(D_INFO, "Image base: 0x%lx\n", loadedImage->ImageBase);
#   endif

    rc = ProcessOptions(loadedImage->LoadOptions,
                        loadedImage->LoadOptionsSize / sizeof(CHAR16));

    if (!EFI_ERROR(rc)) {
        if (optDebugger) {
            WaitDebugger();
        }
        rc = LoadKernel();
    }

    status = uefi_call_wrapper(BS->CloseProtocol,
                               4,
                               image,
                               &LoadedImageProtocol,
                               image,
                               NULL);
    if (EFI_ERROR(status)) {
        Print(L"CloseProtocol: %r\n", status);
    }

    if (EFI_ERROR(rc)) {
        Print(L"Exit with error: %r\n", rc);
    }

    return rc;
}
