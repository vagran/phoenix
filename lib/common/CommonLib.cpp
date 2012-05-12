/*
 * /phoenix/lib/common/CommonLib.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file CommonLib.cpp
 * Run-time mid-level support functions.
 *
 * This file contains definitions for C run-time support functions.
 */

#include <sys.h>

/** Fill block of memory.
 *
 * Sets the first @a size bytes of the block of memory pointed by @a dst
 * to the specified @a value (interpreted as an unsigned char).
 * @param dst Pointer to the block of memory to fill.
 * @param value Value to be set. The value is passed as an int, but the function
 *      fills the block of memory using the unsigned char conversion of this
 *      value.
 * @param size Number of bytes to be set to the value.
 */
#ifdef memset
#undef memset
#endif /* memset */
ASMCALL void *
memset(void *dst, u8 value, size_t size)
{
    void *ret = dst;
    while (size) {
        *static_cast<u8 *>(dst) = value;
        dst = static_cast<u8 *>(dst) + 1;
        size--;
    }
    return ret;
}

/** Copy block of memory.
 *
 * Copies the values of @a size bytes from the location pointed by @a src
 * directly to the memory block pointed by @a dst.
 * @par
 * The underlying type of the objects pointed by both the source and destination
 * pointers are irrelevant for this function; The result is a binary copy of the
 * data.
 * @par
 * The function does not check for any terminating null character in source -
 * it always copies exactly @a size bytes.
 * @par
 * To avoid overflows, the size of the arrays pointed by both the destination
 * and source parameters, shall be at least @a size bytes, and should not
 * overlap (for overlapping memory blocks, @link memmove @endlink is a safer
 * approach).
*
 * @param dst Pointer to the destination array where the content is to be
 *      copied, type-casted to a pointer of type @c void*.
 * @param src Pointer to the source of data to be copied, type-casted to a
 *      pointer of type @c void*.
 * @param size Number of bytes to copy.
 *
 * @return @a dst is returned.
 */
#ifdef memcpy
#undef memcpy
#endif /* memcpy */
ASMCALL void *
memcpy(void *dst, const void *src, size_t size)
{
    void *ret = dst;
    while (size) {
        *static_cast<u8 *>(dst) = *(u8 *)src;
        dst = static_cast<u8 *>(dst) + 1;
        src = static_cast<const u8 *>(src) + 1;
        size--;
    }
    return ret;
}

/** Move block of memory
 *
 * Copies the values of @a size bytes from the location pointed by @a src to the
 * memory block pointed by @a dst. Copying takes place as if an intermediate
 * buffer were used, allowing the destination and source to overlap.
 * @par
 * The underlying type of the objects pointed by both the source and destination
 * pointers are irrelevant for this function; The result is a binary copy of the
 * data.
 * @par
 * The function does not check for any terminating null character in source -
 * it always copies exactly @a size bytes.
 * @par
 * To avoid overflows, the size of the arrays pointed by both the destination
 * and source parameters, shall be at least @a size bytes.
 *
 * @param dst Pointer to the destination array where the content is to be
 *      copied, type-casted to a pointer of type @c void*.
 * @param src Pointer to the source of data to be copied, type-casted to a
 *      pointer of type @c const @c void*.
 * @param size Number of bytes to copy.
 *
 * @return @a dst is returned.
 */
#ifdef memmove
#undef memmove
#endif /* memmove */
ASMCALL void *
memmove(void *dst, const void *src, size_t size)
{
    void *ret = dst;
    if (src > dst) {
        while (size) {
            *static_cast<u8 *>(dst) = *static_cast<const u8 *>(src);
            dst = static_cast<u8 *>(dst) + 1;
            src = static_cast<const u8 *>(src) + 1;
            size--;
        }
    } else {
        dst = static_cast<u8 *>(dst) + size - 1;
        src = static_cast<const u8 *>(src) + size - 1;
        while (size) {
            *static_cast<u8 *>(dst) = *static_cast<const u8 *>(src);
            dst = static_cast<u8 *>(dst) - 1;
            src = static_cast<const u8 *>(src) - 1;
            size--;
        }
    }
    return ret;
}

/** Compare two blocks of memory.
 *
 * Compares the first @a size bytes of the block of memory pointed by @a ptr1 to
 * the first @a size bytes pointed by @a ptr2, returning zero if they all match
 * or a value different from zero representing which is greater if they do not.
 *
 * @param ptr1 Pointer to block of memory.
 * @param ptr2 Pointer to block of memory.
 * @param size Number of bytes to compare.
 *
 * @return Returns an integral value indicating the relationship between the
 * content of the memory blocks: @n
 * @li A zero value indicates that the contents of both memory blocks are equal.
 * @li A value greater than zero indicates that the first byte that does not
 * match in both memory blocks has a greater value in @a ptr1 than in @a ptr2 as
 * if evaluated as unsigned char values; And a value less than zero indicates
 * the opposite.
 */
#ifdef memcmp
#undef memcmp
#endif /* memcmp */
ASMCALL int
memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    while (size) {
        if (*static_cast<const u8 *>(ptr1) != *static_cast<const u8 *>(ptr2)) {
            return *static_cast<const u8 *>(ptr2) - *static_cast<const u8 *>(ptr1);
        }
        size--;
    }
    return 0;
}

/** Locate character in block of memory.
 *
 * Searches within the first @a size bytes of the block of memory pointed by
 * @a ptr for the first occurrence of value (interpreted as an @c unsigned
 * @c char),
 * and returns a pointer to it.
 *
 * @param ptr Pointer to the block of memory where the search is performed.
 * @param value Value to be located. The value is passed as an @c int, but the
 * function performs a byte per byte search using the @c unsigned @c char
 * conversion of this value.
 * @param size Number of bytes to be analyzed.
 *
 * @return A pointer to the first occurrence of @a value in the block of memory
 * pointed by @a ptr. If the value is not found, the function returns 0.
 */
#ifdef memchr
#undef memchr
#endif /* memchr */
ASMCALL void *
memchr(void *ptr, int value, size_t size)
{
    u8 *p = static_cast<u8 *>(ptr);
    while (size) {
        if (static_cast<u8>(value) == *p++) {
            return static_cast<void *>(p - 1);
        }
        size--;
    }
    return 0;
}

/** Convert ASCII character to upper case. */
ASMCALL int
toupper(int c)
{
    if (c >= 'a' && c <= 'z') {
        c -= 'a' - 'A';
    }
    return c;
}

/** Convert ASCII character to lower case. */
ASMCALL int
tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
        c += 'a' - 'A';
    }
    return c;
}

#ifdef strlen
#undef strlen
#endif /* strlen */
ASMCALL size_t
strlen(const char *str)
{
    register const char *s;

    for (s = str; *s; ++s);
    return s - str;
}

#ifdef strcpy
#undef strcpy
#endif /* strcpy */
ASMCALL char *
strcpy(char *dst, const char *src)
{
    char *ret = dst;
    while (*src) {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = 0;
    return ret;
}

ASMCALL char *
strncpy(char *dst, const char *src, size_t len)
{
    char *ret = dst;
    size_t numLeft = len;
    while (*src && numLeft) {
        *dst = *src;
        dst++;
        src++;
        numLeft--;
    }
    if (numLeft) {
        *dst = 0;
    } else if (len) {
        *(dst - 1) = 0;
    }
    return ret;
}

ASMCALL int
strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2) {
        if (!*s1) {
            return 0;
        }
        s1++;
        s2++;
    }
    return *s2 - *s1;
}

ASMCALL int
strncmp(const char *s1, const char *s2, size_t len)
{
    while (len && *s1 == *s2) {
        if (!*s1) {
            return 0;
        }
        s1++;
        s2++;
        len--;
    }
    if (!len) {
        return 0;
    }
    return *s2 - *s1;
}

ASMCALL const char *
strchr(const char *str, int c)
{
    while (*str) {
        if (*str == c) {
            return str;
        }
        str++;
    }
    return 0;
}

ASMCALL const char *
strstr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++)) {
        len = strlen(find);
        do {
            do {
                if (!(sc = *s++)) {
                    return 0;
                }
            } while (sc != c);
        } while (strncmp(s, find, len));
        s--;
    }
    return s;
}

/** Check if ASCII character belongs to alphanumeric class. */
ASMCALL bool
isalnum(int c)
{
    if (c >= '0' && c <= '9') {
        return true;
    }
    if (c >= 'a' && c <= 'z') {
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        return true;
    }
    return false;
}

/** Check if ASCII character is alphabetic. */
ASMCALL bool
isalpha(int c)
{
    if (c >= 'a' && c <= 'z') {
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        return true;
    }
    return false;
}

/** Check is ASCII character is control character. */
ASMCALL bool
iscntrl(int c)
{
    return c < 32;
}

/** Check if ASCII character is digit. */
ASMCALL bool
isdigit(int c)
{
    return c >= '0' && c <= '9';
}

/** Check if ASCII character is pseudo-graphical character. */
ASMCALL bool
isgraph(int c)
{
    return isalnum(c) || ispunct(c);
}

/** Check if ASCII character is lower case alphabetical character. */
ASMCALL bool
islower(int c)
{
    return c >= 'a' && c <= 'z';
}

/** Check if ASCII character is printable. */
ASMCALL bool
isprint(int c)
{
    return isalnum(c) || ispunct(c) || c == ' ';
}

/** Check if ASCII character is punctuation character. */
ASMCALL bool
ispunct(int c)
{
    return !(iscntrl(c) || isalnum(c) || c == ' ');
}

/** Check if ASCII character is space character. */
ASMCALL bool
isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

/** Check if ASCII character is upper case alphabetical character. */
ASMCALL bool
isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

/** Check if ASCII character is hexadecimal digit. */
ASMCALL bool
isxdigit(int c)
{
    if (c >= '0' && c <= '9') {
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        return true;
    }
    return false;
}

/** Check if ASCII character belongs to low part half of ASCII table. */
ASMCALL bool
isascii(int c)
{
    return c >= 0 && c <= 127;
}

ASMCALL long
strtol(const char *nptr, const char **endptr, unsigned base)
{
    const char *s = nptr;
    unsigned long acc;
    u8 c;
    unsigned long cutoff;
    bool neg = false;
    int any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = true;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(LONG_MIN + 1) : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c)) {
            break;
        }
        if (isdigit(c)) {
            c -= '0';
        } else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr) {
        *endptr = any ? s - 1 : nptr;
    }
    return (acc);
}

/*
 * Convert a string to an unsigned long integer.
 */
ASMCALL unsigned long
strtoul(const char *nptr, const char **endptr, unsigned base)
{
    const char *s = nptr;
    unsigned long acc;
    unsigned char c;
    unsigned long cutoff;
    bool neg = false;
    int any, cutlim;

    /*
     * See strtol for comments as to the logic used.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = true;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c)) {
            break;
        }
        if (isdigit(c)) {
            c -= '0';
        } else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = 0xffffffff;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr != 0) {
        *((const char **)endptr) = any ? s - 1 : nptr;
    }
    return (acc);
}

ASMCALL i64
strtoq(const char *nptr, const char **endptr, unsigned base)
{
    const char *s;
    u64 acc;
    unsigned char c;
    u64 qbase, cutoff;
    bool neg;
    int any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = true;
        c = *s++;
    } else {
        neg = false;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for quads is
     * [-9223372036854775808..9223372036854775807] and the input base
     * is 10, cutoff will be set to 922337203685477580 and cutlim to
     * either 7 (neg==0) or 8 (neg==1), meaning that if we have
     * accumulated a value > 922337203685477580, or equal but the
     * next digit is > 7 (or 8), the number is too big, and we will
     * return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    qbase = base;
    if (neg) {
        cutoff = -(QUAD_MIN + QUAD_MAX);
        cutoff += QUAD_MAX;
    } else {
        cutoff = QUAD_MAX;
    }
    cutlim = cutoff % qbase;
    cutoff /= qbase;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c)) {
            break;
        }
        if (isdigit(c)) {
            c -= '0';
        } else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? QUAD_MIN : QUAD_MAX;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr != 0) {
        *((const char **)endptr) = any ? s - 1 : nptr;
    }
    return (acc);
}

ASMCALL u64
strtouq(const char *nptr, const char **endptr, unsigned base)
{
    const char *s = nptr;
    u64 acc;
    unsigned char c;
    u64 qbase, cutoff;
    bool neg;
    int any, cutlim;

    /*
     * See strtoq for comments as to the logic used.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = true;
        c = *s++;
    } else {
        neg = false;
        if (c == '+') {
            c = *s++;
        }
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    qbase = base;
    cutoff = UQUAD_MAX / qbase;
    cutlim = UQUAD_MAX % qbase;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c)) {
            break;
        }
        if (isdigit(c)) {
            c -= '0';
        } else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }
    if (any < 0) {
        acc = UQUAD_MAX;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr != 0) {
        *endptr = any ? s - 1 : nptr;
    }
    return (acc);
}

/* sscanf family, borrowed from FreeBSD */
#define BUF     32  /* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define LONG        0x01    /* l: long or double */
#define SHORT       0x04    /* h: short */
#define SUPPRESS    0x08    /* suppress assignment */
#define POINTER     0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP      0x20    /* do not skip blanks */
#define QUAD        0x400

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define SIGNOK      0x40    /* +/- is (still) legal */
#define NDIGITS     0x80    /* no digits detected */

#define DPTOK       0x100   /* (float) decimal point is still legal */
#define EXPOK       0x200   /* (float) exponent (e+3, etc) still legal */

#define PFXOK       0x100   /* 0x prefix is (still) legal */
#define NZDIGITS    0x200   /* no zero digits detected */

/*
 * Conversion types.
 */
#define CT_CHAR     0   /* %c conversion */
#define CT_CCL      1   /* %[...] conversion */
#define CT_STRING   2   /* %s conversion */
#define CT_INT      3   /* integer, i.e., strtoq or strtouq */

static const char *__sccl(char *, const char *);

/** Parse string into provided variables.
 *
 * @param str Null terminated input string.
 * @param fmt Format specifier for input string parsing.
 * @return Number of variables assigned.
 */
int
sscanf(const char *str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsscanf(str, fmt, ap);
    va_end(ap);
    return ret;
}

/** Parse string into provided variables.
 *
 * @param str Null terminated input string.
 * @param fmt Format specifier for input string parsing.
 * @param ap Arguments list with pointer to variables to assign values to.
 * @return Number of variables assigned.
 */
int
vsscanf(const char *str, char const *fmt, va_list ap)
{
    size_t inr;
    int c;          /* character from format, or conversion */
    size_t width;       /* field width, or 0 */
    char *p;        /* points into all kinds of strings */
    size_t n;          /* handy integer */
    int flags;      /* flags as defined above */
    char *p0;       /* saves original value of p when necessary */
    int nassigned;      /* number of fields assigned */
    int nconversions;   /* number of conversions */
    int nread;      /* number of characters consumed from fp */
    int base;       /* base argument to strtoq/strtouq */
    enum ccfntype {
        ccfn_none,
        ccfn_strtoq,
        ccfn_strtouq
    };
    ccfntype ccfn;      /* conversion function (strtoq/strtouq) */
    char ccltab[256];   /* character class table for %[...] */
    char buf[BUF];      /* buffer for numeric conversions */

    /* `basefix' is used to avoid `if' tests in the integer scanner */
    static short basefix[17] =
        { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    inr = strlen(str);

    nassigned = 0;
    nconversions = 0;
    nread = 0;
    base = 0;
    ccfn = ccfn_none;
    for (;;) {
        c = *fmt++;
        if (c == 0) {
            return nassigned;
        }
        if (isspace(c)) {
            while (inr > 0 && isspace(*str)) {
                nread++, inr--, str++;
            }
            continue;
        }
        if (c != '%') {
            goto literal;
        }
        width = 0;
        flags = 0;
        /*
         * switch on the format.  continue if done;
         * break once format type is derived.
         */
again:  c = *fmt++;
        switch (c) {
        case '%':
literal:
            if (inr <= 0) {
                goto input_failure;
            }
            if (*str != c) {
                goto match_failure;
            }
            inr--, str++;
            nread++;
            continue;

        case '*':
            flags |= SUPPRESS;
            goto again;
        case 'l':
            flags |= LONG;
            goto again;
        case 'q':
            flags |= QUAD;
            goto again;
        case 'h':
            flags |= SHORT;
            goto again;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            width = width * 10 + c - '0';
            goto again;

        /*
         * Conversions.
         *
         */
        case 'd':
            c = CT_INT;
            ccfn = ccfn_strtoq;
            base = 10;
            break;

        case 'i':
            c = CT_INT;
            ccfn = ccfn_strtoq;
            base = 0;
            break;

        case 'o':
            c = CT_INT;
            ccfn = ccfn_strtouq;
            base = 8;
            break;

        case 'u':
            c = CT_INT;
            ccfn = ccfn_strtouq;
            base = 10;
            break;

        case 'x':
            flags |= PFXOK; /* enable 0x prefixing */
            c = CT_INT;
            ccfn = ccfn_strtouq;
            base = 16;
            break;

        case 's':
            c = CT_STRING;
            break;

        case '[':
            fmt = __sccl(ccltab, fmt);
            flags |= NOSKIP;
            c = CT_CCL;
            break;

        case 'c':
            flags |= NOSKIP;
            c = CT_CHAR;
            break;

        case 'p':   /* pointer format is like hex */
            flags |= POINTER | PFXOK;
            c = CT_INT;
            ccfn = ccfn_strtouq;
            base = 16;
            break;

        case 'n':
            nconversions++;
            if (flags & SUPPRESS) {  /* ??? */
                continue;
            }
            if (flags & SHORT) {
                *va_arg(ap, short *) = nread;
            } else if (flags & LONG) {
                *va_arg(ap, long *) = nread;
            } else if (flags & QUAD) {
                *va_arg(ap, i64 *) = nread;
            } else {
                *va_arg(ap, int *) = nread;
            }
            continue;
        }

        /*
         * We have a conversion that requires input.
         */
        if (inr <= 0) {
            goto input_failure;
        }

        /*
         * Consume leading white space, except for formats
         * that suppress this.
         */
        if ((flags & NOSKIP) == 0) {
            while (isspace(*str)) {
                nread++;
                if (--inr > 0) {
                    str++;
                } else {
                    goto input_failure;
                }
            }
            /*
             * Note that there is at least one character in
             * the buffer, so conversions that do not set NOSKIP
             * can no longer result in an input failure.
             */
        }

        /*
         * Do the conversion.
         */
        switch (c) {

        case CT_CHAR:
            /* scan arbitrary characters (sets NOSKIP) */
            if (width == 0) {
                width = 1;
            }
            if (flags & SUPPRESS) {
                size_t sum = 0;
                for (;;) {
                    if ((n = inr) < width) {
                        sum += n;
                        width -= n;
                        str += n;
                        if (sum == 0) {
                            goto input_failure;
                        }
                        break;
                    } else {
                        sum += width;
                        inr -= width;
                        str += width;
                        break;
                    }
                }
                nread += sum;
            } else {
                memcpy(va_arg(ap, char *), str, width);
                inr -= width;
                str += width;
                nread += width;
                nassigned++;
            }
            nconversions++;
            break;

        case CT_CCL:
            /* scan a (nonempty) character class (sets NOSKIP) */
            if (width == 0) {
                width = ~0; /* `infinity' */
            }
            /* take only those things in the class */
            if (flags & SUPPRESS) {
                n = 0;
                while (ccltab[static_cast<u8>(*str)]) {
                    n++, inr--, str++;
                    if (--width == 0) {
                        break;
                    }
                    if (inr <= 0) {
                        if (n == 0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                if (n == 0)
                    goto match_failure;
            } else {
                p0 = p = va_arg(ap, char *);
                while (ccltab[static_cast<u8>(*str)]) {
                    inr--;
                    *p++ = *str++;
                    if (--width == 0) {
                        break;
                    }
                    if (inr <= 0) {
                        if (p == p0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                n = p - p0;
                if (n == 0) {
                    goto match_failure;
                }
                *p = 0;
                nassigned++;
            }
            nread += n;
            nconversions++;
            break;

        case CT_STRING:
            /* like CCL, but zero-length string OK, & no NOSKIP */
            if (width == 0)
                width = ~0;
            if (flags & SUPPRESS) {
                n = 0;
                while (!isspace(*str)) {
                    n++, inr--, str++;
                    if (--width == 0) {
                        break;
                    }
                    if (inr <= 0) {
                        break;
                    }
                }
                nread += n;
            } else {
                p0 = p = va_arg(ap, char *);
                while (!isspace(*str)) {
                    inr--;
                    *p++ = *str++;
                    if (--width == 0)
                        break;
                    if (inr <= 0)
                        break;
                }
                *p = 0;
                nread += p - p0;
                nassigned++;
            }
            nconversions++;
            continue;

        case CT_INT:
            /* scan an integer as if by strtoq/strtouq */
#ifdef hardway
            if (width == 0 || width > sizeof(buf) - 1)
                width = sizeof(buf) - 1;
#else
            /* size_t is unsigned, hence this optimisation */
            if (--width > sizeof(buf) - 2)
                width = sizeof(buf) - 2;
            width++;
#endif
            flags |= SIGNOK | NDIGITS | NZDIGITS;
            for (p = buf; width; width--) {
                c = *str;
                /*
                 * Switch on the character; `goto ok'
                 * if we accept it as a part of number.
                 */
                switch (c) {

                /*
                 * The digit 0 is always legal, but is
                 * special.  For %i conversions, if no
                 * digits (zero or nonzero) have been
                 * scanned (only signs), we will have
                 * base==0.  In that case, we should set
                 * it to 8 and enable 0x prefixing.
                 * Also, if we have not scanned zero digits
                 * before this, do not turn off prefixing
                 * (someone else will turn it off if we
                 * have scanned any nonzero digits).
                 */
                case '0':
                    if (base == 0) {
                        base = 8;
                        flags |= PFXOK;
                    }
                    if (flags & NZDIGITS)
                        flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
                    else
                        flags &= ~(SIGNOK|PFXOK|NDIGITS);
                    goto ok;

                /* 1 through 7 always legal */
                case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    base = basefix[base];
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* digits 8 and 9 ok if decimal or hex */
                case '8': case '9':
                    base = basefix[base];
                    if (base <= 8)
                        break;  /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* letters ok if hex */
                case 'A': case 'B': case 'C':
                case 'D': case 'E': case 'F':
                case 'a': case 'b': case 'c':
                case 'd': case 'e': case 'f':
                    /* no need to fix base here */
                    if (base <= 10)
                        break;  /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* sign ok only as first character */
                case '+': case '-':
                    if (flags & SIGNOK) {
                        flags &= ~SIGNOK;
                        goto ok;
                    }
                    break;

                /* x ok if flag still set and 2nd char */
                case 'x': case 'X':
                    if ((flags & PFXOK) && p == buf + 1) {
                        base = 16;  /* if %i */
                        flags &= ~PFXOK;
                        goto ok;
                    }
                    break;
                }

                /*
                 * If we got here, c is not a legal character
                 * for a number.  Stop accumulating digits.
                 */
                break;
        ok:
                /*
                 * c is legal: store it and look at the next.
                 */
                *p++ = c;
                if (--inr > 0) {
                    str++;
                } else {
                    break;      /* end of input */
                }
            }
            /*
             * If we had only a sign, it is no good; push
             * back the sign.  If the number ends in `x',
             * it was [sign] '0' 'x', so push back the x
             * and treat it as [sign] '0'.
             */
            if (flags & NDIGITS) {
                if (p > buf) {
                    str--;
                    inr++;
                }
                goto match_failure;
            }
            c = p[-1];
            if (c == 'x' || c == 'X') {
                --p;
                str--;
                inr++;
            }
            if ((flags & SUPPRESS) == 0) {
                u64 res;

                *p = 0;
                ASSERT(ccfn != ccfn_none);
                if (ccfn == ccfn_strtoq) {
                    res = strtoq(buf, 0, base);
                } else {
                    res = strtouq(buf, 0, base);
                }
                if (flags & POINTER) {
                    vm::Vaddr va(res);
                    *va_arg(ap, void **) = va;
                } else if (flags & SHORT) {
                    *va_arg(ap, short *) = res;
                } else if (flags & LONG) {
                    *va_arg(ap, long *) = res;
                } else if (flags & QUAD) {
                    *va_arg(ap, i64 *) = res;
                } else {
                    *va_arg(ap, int *) = res;
                }
                nassigned++;
            }
            nread += p - buf;
            nconversions++;
            break;
        }
    }
input_failure:
    return nconversions != 0 ? nassigned : -1;
match_failure:
    return nassigned;
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static const char *
__sccl(char *tab, const char *fmt)
{
    int c, n, v;

    /* first `clear' the whole table */
    c = *fmt++;     /* first char hat => negated scanset */
    if (c == '^') {
        v = 1;      /* default => accept */
        c = *fmt++; /* get new first char */
    } else {
        v = 0;      /* default => reject */
    }

    for (n = 0; n < 256; n++) {
        tab[n] = v;
    }

    if (c == 0) {
        return fmt - 1; /* format ended before closing ] */
    }

    /*
     * Now set the entries corresponding to the actual scanset
     * to the opposite of the above.
     *
     * The first character may be ']' (or '-') without being special;
     * the last character may be '-'.
     */
    v = 1 - v;
    for (;;) {
        tab[c] = v;     /* take character c */
doswitch:
        n = *fmt++;     /* and examine the next */
        switch (n) {

        case 0:         /* format ended too soon */
            return fmt - 1;

        case '-':
            /*
             * A scanset of the form
             *  [01+-]
             * is defined as `the digit 0, the digit 1,
             * the character +, the character -', but
             * the effect of a scanset such as
             *  [a-zA-Z0-9]
             * is implementation defined.  The V7 Unix
             * scanf treats `a-z' as `the letters a through
             * z', but treats `a-a' as `the letter a, the
             * character -, and the letter a'.
             *
             * For compatibility, the `-' is not considered
             * to define a range if the character following
             * it is either a close bracket (required by ANSI)
             * or is not numerically greater than the character
             * we just stored in the table (c).
             */
            n = *fmt;
            if (n == ']' || n < c) {
                c = '-';
                break;  /* resume the for(;;) */
            }
            fmt++;
            /* fill in the range */
            do {
                tab[++c] = v;
            } while (c < n);
            c = n;
            /*
             * Alas, the V7 Unix scanf also treats formats
             * such as [a-c-e] as `the letters a through e'.
             * This too is permitted by the standard....
             */
            goto doswitch;

        case ']':       /* end of scanset */
            return fmt;

        default:        /* just another character */
            c = n;
            break;
        }
    }
    NOT_REACHED();
    return 0;
}
