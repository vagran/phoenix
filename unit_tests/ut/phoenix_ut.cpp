/*
 * /phoenix/unit_tests/ut/phoenix_ut.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file phoenix_ut.cpp
 * Phoenix unit testing framework implementation.
 */

#include <phoenix_ut.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <list>

using namespace ut;

#define UT_HDL2STR(str_hdl) (*static_cast<ut_string *>(str_hdl))
#define UT_STR2HDL(str)     (static_cast<void *>(&str))

namespace ut {

bool __ut_mdump();

bool __ut_mtrack_enabled = true;

}

namespace {

template <class T>
class UtAllocator : public std::allocator<T> {
public:
    typedef size_t      size_type;
    typedef intptr_t    difference_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T           value_type;

    template<typename _Tp1>
    struct rebind {
        typedef UtAllocator<_Tp1> other;
    };

    pointer address(reference x) const { return &x; }

    const_pointer *address(const_reference x) const { return &x; }

    pointer allocate(size_t n, const void *hint = 0) {
        T *ptr = static_cast<T *>(malloc(n * sizeof(T)));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(pointer p, size_t n) {
        free(p);
    }

    size_type max_size() const throw() { return 1024 * 1024 / sizeof(T) + 1; }

    void construct(pointer p, const_reference val) {
        new(static_cast<void *>(p)) T(val);
    }

    void destroy(pointer p) {
        p->~T();
    }
};

typedef std::basic_string<char, std::char_traits<char>, UtAllocator<char>> ut_string;
typedef std::basic_stringstream<char, std::char_traits<char>, UtAllocator<char>> ut_stringstream;

ut_string &
GetUtString(UtString &s)
{
    return UT_HDL2STR(s.GetHandle());
}

void
TestValueBase_Describe(TestValueBase &value, ut_string &s)
{
    UtString str(UT_STR2HDL(s));
    value.Describe(str);
}

void
TestException_Describe(TestException &e, ut_string &s)
{
    UtString str(UT_STR2HDL(s));
    e.Describe(str);
}

/** Tests manager. */
class TestMan {
public:
    TestMan();
    ~TestMan();

    /** Run all registered tests.
     *
     * @return @a true if all tests succeeded, @a false otherwise.
     */
    bool Run();
    void RegisterTest(TestDesc *desc);
    inline void HitValue() { _numValues++; _totNumValues++; }
    inline void HitAssert() { _numAsserts++; _totNumAsserts++; }
    void PrintStat(bool total = false);
private:
    std::list<TestDesc *, UtAllocator<TestDesc *>> _tests;
    size_t _numValues, _numAsserts;
    size_t _totNumValues, _totNumAsserts;
};

TestMan::TestMan()
{
    _numValues = 0;
    _numAsserts = 0;
    _totNumValues = 0;
    _totNumAsserts = 0;
}

TestMan::~TestMan()
{

}

void
TestMan::PrintStat(bool total)
{
    if (!total) {
        printf("Values checked: %lu\nAsserts executed: %lu\n",
               _numValues,_numAsserts);
        _numValues = 0;
        _numAsserts = 0;
    } else {
        printf("Total values checked: %lu\nTotal asserts executed: %lu\n",
               _totNumValues,_totNumAsserts);
    }
}

bool
TestMan::Run()
{
    printf("========= Phoenix unit testing framework =========\n");
    if (*__ut_test_description) {
        printf("Test suite: %s\n", __ut_test_description);
    }

    if (!__ut_InitStubs()) {
        printf("Stubs initialization failed.");
        return false;
    }

    size_t numTests = _tests.size();
    size_t testIdx = 0;
    size_t numPassed = 0;

    for (TestDesc *t: _tests) {
        printf("==== Running test '%s' (%lu of %lu) ====\n(defined at %s:%d)\n\n",
               t->GetName(), testIdx + 1, numTests, t->GetFile(), t->GetLine());

        bool failed = false;
        try {
            t->TestBody();
        } catch (TestException &e) {
            printf("\n");
            ut_string desc;
            TestException_Describe(e, desc);
            printf("%s\n", desc.c_str());
            failed = true;
        }

        printf("\nTest %s\n", failed ? "FAILED" : "PASSED");
        PrintStat();
        failed |= ::__ut_mdump();
        testIdx++;
        if (!failed ) {
            numPassed++;
        }
    }

    printf("======== Testing complete [%s] ========\n"
           "%lu of %lu tests passed\n",
           numPassed == numTests ? "SUCCEED" : "FAILED",
           numPassed, numTests);
    PrintStat(true);
    ::__ut_mtrack_enabled = false;
    return numPassed == numTests;
}

void
TestMan::RegisterTest(TestDesc *desc)
{
    _tests.push_back(desc);
}

/** Global test manager object. */
TestMan testMan;

} /* anonymous namespace */


/* Test descriptor. */

TestDesc::TestDesc(const char *file, int line, const char *name)
{
    _file = file;
    _line = line;
    _name = name;
    ::testMan.RegisterTest(this);
}

TestDesc::~TestDesc()
{

}

void
TestValueBase::Describe(UtString &_s)
{
    ut_string &s = GetUtString(_s);
    ut_stringstream ss;
    ss << "Value: " << _name << " [" << GetUtString(_value) << "]" <<
        " (defined at " << _file << ":" << _line << ")";
    s = ss.str();
}

void
TestException::Describe(UtString &_s)
{
    ut_string &s = GetUtString(_s);
    ut_stringstream ss;
    switch (_type) {
    case BINARY_ASSERT:
        ss << "Assertion failed: " << _value1.GetName() << " " << _op <<
            " " << _value2.GetName();
        break;
    case UNARY_ASSERT:
        ss << "Assertion failed: " << _op << " " << _value1.GetName();
        break;
    case USER_FAILURE:
        ss << "Fault occurred: " << _op;
        break;
    }
    ss << "\n";
    ut_string value;
    switch (_type) {
    case BINARY_ASSERT:
        TestValueBase_Describe(_value1, value);
        ss << value << "\n";
        TestValueBase_Describe(_value2, value);
        ss << value;
        break;
    case UNARY_ASSERT:
    case USER_FAILURE:
        TestValueBase_Describe(_value1, value);
        ss << value;
        break;
    }
    s = ss.str();
}

void
ut::__ut_user_fault(const char *file, int line, const char *desc, ...)
{
    static char buf[2048];
    va_list args;
    va_start(args, desc);
    vsnprintf(buf, sizeof(buf), desc, args);
    va_end(args);
    throw TestException(buf, file, line);
}

void
ut::__ut_hit_value()
{
    ::testMan.HitValue();
}

void
ut::__ut_hit_assert()
{
    ::testMan.HitAssert();
}

int
ut::__ut_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

unsigned
ut::__ut_strlen(const char *s)
{
    return strlen(s);
}

void
ut::__ut_putc(char c)
{
    printf("%c", c);
}

void
ut::__ut_trace(const char *file, int line, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    __ut_vtrace(file, line, msg, args);
    va_end(args);
}

void
ut::__ut_vtrace(const char *file, int line, const char *msg, __ut_va_list args)
{
    printf("[%s:%d] ", file, line);
    vprintf(msg, args);
    printf("\n");
}

int
ut::__ut_snprintf(char *str, unsigned long size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(str, size, format, args);
    va_end(args);
    return ret;
}

int
ut::__ut_vsnprintf(char *str, unsigned long size, const char *format,
                   __builtin_va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

#ifndef IS_POWER_OF_2
#define IS_POWER_OF_2(value)       ((((value) - 1) & (value)) == 0)
#endif

#ifndef ROUND_UP2
#define ROUND_UP2(size, align)     (((size) + (align) - 1) & (~((align) - 1)))
#endif

struct ut_mblock_hdr {
    enum {
        MAGIC = 0x3e781b0a
    };
    unsigned magic;
    const char *file;
    int line;
    /* Requested size. */
    unsigned long size;
    /* Requested alignment. */
    unsigned long align;
    /* Start of the memory block allocated. */
    void *memStart;
};

static std::list<ut_mblock_hdr *, UtAllocator<ut_mblock_hdr *>> allocatedBlocks;

void *
ut::__ut_malloc(const char *file, int line, unsigned long size, unsigned long align)
{
    void *mem, *block;
    if (align) {
        if (!IS_POWER_OF_2(align)) {
            UT_FAIL("Invalid alignment: %lu bytes at %s:%d", align, file, line);
        }
        mem = malloc(sizeof(ut_mblock_hdr) + size * 2);
    } else {
        mem = malloc(sizeof(ut_mblock_hdr) + size);
    }
    if (!mem) {
        UT_FAIL("Memory allocation failed: %lu bytes at %s:%d", size, file, line);
    }
    if (align) {
        block = reinterpret_cast<void *>(ROUND_UP2(reinterpret_cast<uintptr_t>(mem) + sizeof(ut_mblock_hdr), size));
    } else {
        block = static_cast<char *>(mem) + sizeof(ut_mblock_hdr);
    }
    ut_mblock_hdr *hdr = static_cast<ut_mblock_hdr *>(block) - 1;

    hdr->magic = ut_mblock_hdr::MAGIC;
    hdr->file = file;
    hdr->line = line;
    hdr->size = size;
    hdr->align = align;
    hdr->memStart = mem;

    if (::__ut_mtrack_enabled) {
        ::allocatedBlocks.push_front(hdr);
    }

    memset(block, 0xcc, size);

    return block;
}

void
ut::__ut_mfree(void *ptr)
{
    ut_mblock_hdr *hdr = static_cast<ut_mblock_hdr *>(ptr) - 1;
    if (hdr->magic != ut_mblock_hdr::MAGIC) {
        UT_FAIL("Trying to free non-managed or already freed block");
    }

    memset(ptr, 0xfe, hdr->size);

    free(hdr->memStart);

    if (::__ut_mtrack_enabled) {
        ::allocatedBlocks.remove(hdr);
    }
}

bool
ut::__ut_mdump()
{
    bool found = false;
    for (ut_mblock_hdr *hdr: ::allocatedBlocks) {
        if (!hdr->file) {
            /* Do not account untracked allocations. */
            continue;
        }
        if (!found) {
            printf("Warning: Non freed memory blocks:\n");
            found = true;
        }
        printf("%lu bytes at %p (%s:%d)\n", hdr->size, hdr + 1, hdr->file, hdr->line);
    }
    return found;
}

/* UtString class */

UtString::UtString()
{
    _handle = UT_STR2HDL(*new ut_string);
    _allocated = true;
}

UtString::UtString(void *handle)
{
    _handle = handle;
    _allocated = false;
}

UtString::~UtString()
{
    if (_allocated) {
        //delete &UT_HDL2STR(_handle);
    }
}

UtString &
UtString::operator =(void *handle)
{
    UT_HDL2STR(_handle) = UT_HDL2STR(handle);
    return *this;
}

UtString &
UtString::operator =(const UtString &s)
{
    UT_HDL2STR(_handle) = UT_HDL2STR(s._handle);
    return *this;
}

template <typename T>
void
UtString::_ToString(T value)
{
    ut_stringstream ss;
    ss << value;
    UT_HDL2STR(_handle) = ss.str();
}

#define UT_STR_INSTANTIATE(__type) \
    template void UtString::_ToString<__type>(__type value)

UT_STR_INSTANTIATE(bool);
UT_STR_INSTANTIATE(char);
UT_STR_INSTANTIATE(unsigned char);
UT_STR_INSTANTIATE(short);
UT_STR_INSTANTIATE(unsigned short);
UT_STR_INSTANTIATE(int);
UT_STR_INSTANTIATE(unsigned int);
UT_STR_INSTANTIATE(long);
UT_STR_INSTANTIATE(unsigned long);
UT_STR_INSTANTIATE(float);
UT_STR_INSTANTIATE(double);
UT_STR_INSTANTIATE(char *);
UT_STR_INSTANTIATE(const char *);
UT_STR_INSTANTIATE(void *);

/* Main function. It will run all tests. Returns zero if all tests succeeded,
 * non-zero if any failures occurred.
 */
int
main()
{
    return !::testMan.Run();
}
