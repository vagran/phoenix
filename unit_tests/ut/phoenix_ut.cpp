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

#include <stdio.h>
#include <list>

using namespace ut;

namespace {

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

    size_t numTests = _tests.size();
    size_t testIdx = 0;
    size_t numPassed = 0;

    for (TestDesc *t: _tests) {
        printf("==== Running test '%s' (%lu of %lu) ====\n(defined at %s:%d)\n\n",
               t->GetName(), testIdx + 1, numTests, t->GetFile(), t->GetLine());

        try {
            t->TestBody();
        } catch (TestException &e) {
            std::string desc;
            e.Describe(desc);
            printf("%s\n", desc.c_str());
            printf("Test FAILED\n");
            PrintStat();
            break;
        }

        printf("Test PASSED\n");
        PrintStat();
        testIdx++;
        numPassed++;
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
TestValueBase::Describe(std::string &s)
{
    std::stringstream ss;
    ss << "Value: " << _name;
    if (!_value.empty()) {
        ss << " [" << _value << "]";
    }
    ss << " (defined at " << _file << ":" << _line << ")";
    s = ss.str();
}

void
TestException::Describe(std::string &s)
{
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
        _value1.Describe(value);
        ss << value << "\n";
        _value2.Describe(value);
        ss << value;
        break;
    case UNARY_ASSERT:
    case USER_FAILURE:
        _value1.Describe(value);
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

/* Main function. It will run all tests. Returns zero if all tests succeeded,
 * non-zero if any failures occurred.
 */
int
main()
{
    return !::testMan.Run();
}
