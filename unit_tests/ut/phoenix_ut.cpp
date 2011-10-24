/*
 * /phoenix/unit_tests/ut/phoenix_ut.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file phoenix_ut.cpp
 * Phoenix unit testing framework implementation.
 */

#include <phoenix_ut.h>

#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <list>

using namespace ut;

#define UT_HDL2STR(str_hdl) (*static_cast<std::string *>(str_hdl))
#define UT_STR2HDL(str)     (static_cast<void *>(&str))

namespace {

std::string &
GetUtString(UtString &s)
{
    return UT_HDL2STR(s.GetHandle());
}

void
TestValueBase_Describe(TestValueBase &value, std::string &s)
{
    UtString str(UT_STR2HDL(s));
    value.Describe(str);
}

void
TestException_Describe(TestException &e, std::string &s)
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
    std::list<TestDesc *> _tests;
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
            std::string desc;
            TestException_Describe(e, desc);
            printf("%s\n", desc.c_str());
            failed = true;
        }

        printf("Test %s\n", failed ? "FAILED" : "PASSED");
        PrintStat();
        testIdx++;
        if (!failed ) {
            numPassed++;
        }
    }

    printf("======== Testing complete ========\n"
           "%lu of %lu tests passed\n", numPassed, numTests);
    PrintStat(true);
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
    std::string &s = GetUtString(_s);
    std::stringstream ss;
    ss << "Value: " << _name;
    if (!GetUtString(_value).empty()) {
        ss << " [" << GetUtString(_value) << "]";
    }
    ss << " (defined at " << _file << ":" << _line << ")";
    s = ss.str();
}

void
TestException::Describe(UtString &_s)
{
    std::string &s = GetUtString(_s);
    std::stringstream ss;
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
    std::string value;
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
ut::__ut_user_fault(const char *desc, const char *file, int line)
{
    throw TestException(desc, file, line);
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

/* UtString class */

UtString::UtString()
{
    _handle = UT_STR2HDL(*new std::string);
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
        delete &UT_HDL2STR(_handle);
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
    std::stringstream ss;
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
