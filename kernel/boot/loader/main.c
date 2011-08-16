/*
 * /phoenix/kernel/boot/loader/main.c
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <efi.h>
#include <efilib.h>

/* Kernel command line converted to ASCII */
char *kernelCmdLine;
/* Kernel file name */
CHAR16 *kernelImage;

/* Boot loader options */

static int optDebugger;

static struct Option {
    CHAR16 *name;
    int *flag;
} options[] = {
    { L"--debugger", &optDebugger }
};

static EFI_LOADED_IMAGE *loaded_image;

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
    int isAttached = 0;
    Print(L"Image base: 0x%lx\n", loaded_image->ImageBase);
    Print(L"Image size: 0x%lx\n", loaded_image->ImageSize);
    Print(L"Waiting for debugger...");
    while (!isAttached) {
        __asm__ __volatile__("pause");
    }
    Print(L"Attached\n");
}

/* Process command line options.
 * optionsSize: length of options string in characters.
 */
static EFI_STATUS
ProcessOptions(CHAR16 *optionsStr, UINT32 optionsSize)
{
    struct Option *opt;

    if (!optionsSize) {
        return EFI_INVALID_PARAMETER;
    }
    /* Do not account null terminator */
    if (!optionsStr[optionsSize - 1]) {
        optionsSize--;
    }
    WaitDebugger();//temp

    /* Skip loader image path */
    optionsStr = SkipSpaces(optionsStr, &optionsSize);
    optionsStr = SkipWord(optionsStr, &optionsSize);

    do {
        int optIdx;
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
    UINT32 i = optionsSize;
    CHAR16 *nextPtr = SkipWord(optionsStr, &i);
    kernelImage = (CHAR16 *)AllocatePool((nextPtr - optionsStr + 1) * sizeof(CHAR16));
    if (!kernelImage) {
        return EFI_OUT_OF_RESOURCES;
    }
    RtCopyMem(kernelImage, optionsStr, (nextPtr - optionsStr) * sizeof(CHAR16));
    kernelImage[nextPtr - optionsStr] = 0;

    /* Convert kernel command line to ACII */
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

static EFI_STATUS
LoadKernel()
{

    return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status, rc;

    InitializeLib(image, systab);

    status = uefi_call_wrapper(systab->BootServices->OpenProtocol,
                               6,
                               image,
                               &LoadedImageProtocol,
                               (void **)&loaded_image,
                               image,
                               NULL,
                               EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        Print(L"OpenProtocol: %r\n", status);
        return status;
    }

    rc = ProcessOptions(loaded_image->LoadOptions,
                        loaded_image->LoadOptionsSize / sizeof(CHAR16));

    if (!EFI_ERROR(rc)) {
        if (optDebugger) {
            WaitDebugger();
        }
        rc = LoadKernel();
    }

    status = uefi_call_wrapper(systab->BootServices->CloseProtocol,
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
