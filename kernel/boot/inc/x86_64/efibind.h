/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    efefind.h

Abstract:

    EFI to compile bindings

--*/

#ifndef __GNUC__
#pragma pack()
#endif

//
// Basic integer types of various widths
//
#include <types.h>

//
// Basic EFI types of various widths
//

#ifndef __WCHAR_TYPE__
# define __WCHAR_TYPE__ short
#endif

typedef uint64_t   UINT64;
typedef int64_t    INT64;

#ifndef _BASETSD_H_
    typedef uint32_t   UINT32;
    typedef int32_t    INT32;
#endif

typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;
typedef __WCHAR_TYPE__ WCHAR;

#undef VOID
#define VOID    void


typedef int64_t    INTN;
typedef uint64_t   UINTN;

#ifdef EFI_NT_EMULATOR
    #define POST_CODE(_Data)
#else    
    #ifdef EFI_DEBUG
#define POST_CODE(_Data)    __asm mov eax,(_Data) __asm out 0x80,al
    #else
        #define POST_CODE(_Data)
    #endif  
#endif

#define EFIERR(a)           (0x8000000000000000 | a)
#define EFI_ERROR_MASK      0x8000000000000000
#define EFIERR_OEM(a)       (0xc000000000000000 | a)      


#define BAD_POINTER         0xFBFBFBFBFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFFFFFFFFFF

#ifdef EFI_NT_EMULATOR
    #define BREAKPOINT()        __asm { int 3 }
#else
    #define BREAKPOINT()        while (TRUE);    // Make it hang on Bios[Dbg]32
#endif

//
// Pointers must be aligned to these address to function
//

#define MIN_ALIGNMENT_SIZE  4

#define ALIGN_VARIABLE(Value ,Adjustment) \
            (UINTN)Adjustment = 0; \
            if((UINTN)Value % MIN_ALIGNMENT_SIZE) \
                (UINTN)Adjustment = MIN_ALIGNMENT_SIZE - ((UINTN)Value % MIN_ALIGNMENT_SIZE); \
            Value = (UINTN)Value + (UINTN)Adjustment


//
// Define macros to build data structure signatures from characters.
//

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((UINT64)(EFI_SIGNATURE_32(E,F,G,H)) << 32))
//
// To export & import functions in the EFI emulator environment
//

#ifdef EFI_NT_EMULATOR
    #define EXPORTAPI           __declspec( dllexport )
#else
    #define EXPORTAPI
#endif


//
// EFIAPI - prototype calling convention for EFI function pointers
// BOOTSERVICE - prototype for implementation of a boot service interface
// RUNTIMESERVICE - prototype for implementation of a runtime service interface
// RUNTIMEFUNCTION - prototype for implementation of a runtime function that is not a service
// RUNTIME_CODE - pragma macro for declaring runtime code    
//

#ifndef EFIAPI                  // Forces EFI calling conventions reguardless of compiler options 
    #ifdef _MSC_EXTENSIONS
        #define EFIAPI __cdecl  // Force C calling convention for Microsoft C compiler 
    #else
        #define EFIAPI          // Substitute expresion to force C calling convention 
    #endif
#endif

#define BOOTSERVICE
//#define RUNTIMESERVICE(proto,a)    alloc_text("rtcode",a); proto a
//#define RUNTIMEFUNCTION(proto,a)   alloc_text("rtcode",a); proto a
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION


#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE    volatile

#define MEMORY_FENCE()    

#ifdef EFI_NT_EMULATOR

//
// To help ensure proper coding of integrated drivers, they are
// compiled as DLLs.  In NT they require a dll init entry pointer.
// The macro puts a stub entry point into the DLL so it will load.
//

#define EFI_DRIVER_ENTRY_POINT(InitFunction)    \
    UINTN                                       \
    __stdcall                                   \
    _DllMainCRTStartup (                        \
        UINTN    Inst,                          \
        UINTN    reason_for_call,               \
        VOID    *rserved                        \
        )                                       \
    {                                           \
        return 1;                               \
    }                                           \
                                                \
    int                                         \
    EXPORTAPI                                   \
    __cdecl                                     \
    InitializeDriver (                          \
        void *ImageHandle,                      \
        void *SystemTable                       \
        )                                       \
    {                                           \
        return InitFunction(ImageHandle, SystemTable);       \
    }


    #define LOAD_INTERNAL_DRIVER(_if, type, name, entry)      \
        (_if)->LoadInternal(type, name, NULL)             

#else // EFI_NT_EMULATOR 

//
// When build similiar to FW, then link everything together as
// one big module.
//

    #define EFI_DRIVER_ENTRY_POINT(InitFunction)

    #define LOAD_INTERNAL_DRIVER(_if, type, name, entry)    \
            (_if)->LoadInternal(type, name, entry)

#endif // EFI_FW_NT 

//
// Some compilers don't support the forward reference construct:
//  typedef struct XXXXX
//
// The following macro provide a workaround for such cases.
//
#ifdef NO_INTERFACE_DECL
#define INTERFACE_DECL(x)
#else
#ifdef __GNUC__
#define INTERFACE_DECL(x) struct x
#else
#define INTERFACE_DECL(x) typedef struct x
#endif
#endif

/* for x86_64, EFI_FUNCTION_WRAPPER must be defined */
#ifdef  EFI_FUNCTION_WRAPPER
UINTN uefi_call_wrapper(void *func, unsigned long va_num, ...);
#else
#error "EFI_FUNCTION_WRAPPER must be defined for x86_64 architecture"
#endif

#ifdef _MSC_EXTENSIONS
#pragma warning ( disable : 4731 )  // Suppress warnings about modification of EBP
#endif

