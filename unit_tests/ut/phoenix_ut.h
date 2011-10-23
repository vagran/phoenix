/*
 * /phoenix/unit_tests/ut/phoenix_ut.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file phoenix_ut.h
 * Main header file for unit tests source files inclusions. This header should
 * not include any third party header (e.g. standard library headers) to avoid
 * conflicts with the code being tested.
 */

#ifndef PHOENIX_UT_H_
#define PHOENIX_UT_H_

#define __UT_CONCAT2(x, y)      x##y
/** Concatenate identifiers. */
#define __UT_CONCAT(x, y)       __UT_CONCAT2(x, y)

#define __UT_STR2(x)            # x
/** Stringify identifiers. */
#define __UT_STR(x)             __UT_STR2(x)

/** Get unique identifier. */
#define __UT_UID(str)           __UT_CONCAT(str, __COUNTER__)

#define __UT_TEST_DESC          __UT_CONCAT(UtTestDesc_, __LINE__)

/** Macro for defining a test. Usage example:
 * @code
 * UT_TEST("My test of some functionality")
 * {
 *      <test code here>
 * }
 * UT_TEST_END
 * @endcode
 *
 * @param name Arbitrary name of the test.
 */
#define UT_TEST(name) \
    namespace { \
    class __UT_TEST_DESC : public ut::TestDesc { \
    public: \
        __UT_TEST_DESC() : TestDesc(__FILE__, __LINE__, name) { } \
        \
        virtual void TestBody()

/** Macro for test definition termination. See @ref UT_TEST. */
#define UT_TEST_END \
    } __UT_UID(utTestDesc_); \
    } /* anonymous namespace */

/** Wrapper for all values which are participating in asserts. Usage example:
 * @code
 * UT(someValue) == UT(anotherValue);
 * UT(a) < UT(b);
 * UT(someString) != UT("string content");
 * @endcode
 *
 * @param value Value of any supported type to participate in assert condition.
 */
#define UT(value)   ut::TestValue<decltype(value)>(value, __UT_STR(value), __FILE__, __LINE__)

/** Wrapper for null pointer value. */
#define UT_NULL     ut::TestValue<void *>(0, "NULL", __FILE__, __LINE__)

/** User requested failure.
 * @param desc Description of the fault.
 */
#define UT_FAIL(desc) ut::__ut_user_fault(desc, __FILE__, __LINE__)

/** Unit tests related definitions reside in this namespace. */
namespace ut {

/** Increment tested values statistics. */
void __ut_hit_value();
/** Increment assertions statistics. */
void __ut_hit_assert();

int __ut_strcmp(const char *s1, const char *s2);

class UtString {
public:
    UtString();
    UtString(void *handle);
    ~UtString();

    inline void *GetHandle() { return _handle; }

    UtString &operator =(void *handle);
    UtString &operator =(const UtString &s);

    template <typename T>
    void ToString(T value)
    {
        _ToString(value);
    }

    template <typename T>
    void ToString(T *value)
    {
        _ToString(static_cast<void *>(value));
    }

    template <typename T>
    void ToString(const T *value)
    {
        _ToString(static_cast<void *>(const_cast<T *>(value)));
    }

    void ToString(char *value)
    {
        _ToString(value);
    }

    void ToString(const char *value)
    {
        _ToString(value);
    }

private:
    void *_handle;
    bool _allocated;

    template <typename T>
    void _ToString(T value);
};

/** Test descriptor. Used for registering tests. */
class TestDesc {
public:
    TestDesc(const char *file, int line, const char *name);
    virtual ~TestDesc();

    virtual void TestBody() = 0;

    inline const char *GetName() { return _name; }
    inline const char *GetFile() { return _file; }
    inline int GetLine() { return _line; }
private:
    const char *_file;
    int _line;
    const char *_name;
};

/** Base class for @ref TestValue. Should not be used directly. */
class TestValueBase {
public:
    TestValueBase() {
        _name = 0;
        _file = 0;
        _line = 0;
    }

    TestValueBase(const TestValueBase& value) {
        _name = value._name;
        _file = value._file;
        _line = value._line;
        _value = value._value;
    }

    TestValueBase(const char *name, const char *file, int line) {
        _name = name;
        _file = file;
        _line = line;
    }

    template <typename T>
    void SetValue(T value) {
        _value.ToString(value);
    }

    const char *GetName() { return _name; }

    void Describe(UtString &_s);

protected:
    const char *_name, *_file;
    int _line;
    UtString _value;
};

/** Exceptions during a test (e.g. failed assertion) are represented by this
 * class.
 */
class TestException {
public:
    enum Type {
        BINARY_ASSERT,
        UNARY_ASSERT,
        USER_FAILURE
    };

    /** Failed assertion with two values.
     * @param op String representation of binary operator.
     * @param value1 First value of failed assertion.
     * @param value2 Second value of failed assertion.
     */
    TestException(const char *op, const TestValueBase &value1,
                  const TestValueBase &value2) :
        _value1(value1), _value2(value2)
    {
        _op = op;
        _type = BINARY_ASSERT;
    }

    /** Failed assertion with one value (unary operator).
     *
     * @param op String representation of unary operator.
     * @param value Failed assertion operator argument.
     */
    TestException(const char *op, const TestValueBase &value) :
        _value1(value)
    {
        _op = op;
        _type = UNARY_ASSERT;
    }

    /** User requested failure.
     * @param desc Description of the failure.
     */
    TestException(const char *desc, const char *file, int line) :
        _value1(desc, file, line)
    {
        _op = desc;
        _type = USER_FAILURE;
    }

    ~TestException()
    {

    }

    void Describe(UtString &s);

private:
    TestValueBase _value1, _value2;
    const char *_op;
    Type _type;
};

/** Class for wrapping all values being tested in tests. Use @ref UT macro for
 * this class objects creation.
 */
template <typename T>
class TestValue : public TestValueBase {
public:
    T value;

    TestValue(T value, const char *name, const char *file, int line) :
        TestValueBase(name, file, line)
    {
        this->value = value;
        SetValue(value);
        __ut_hit_value();
    }

    ~TestValue() {

    }

    /* Binary operators */
#   define __UT_B_OPERATOR(__op) \
    template <typename T2> \
    bool operator __op(const TestValue<T2> &value2) { \
        __ut_hit_assert(); \
        if (value __op value2.value) { \
            return true; \
        } \
        throw TestException(__UT_STR(__op), *this, value2); \
    }

    __UT_B_OPERATOR(==)
    __UT_B_OPERATOR(!=)
    __UT_B_OPERATOR(<)
    __UT_B_OPERATOR(<=)
    __UT_B_OPERATOR(>)
    __UT_B_OPERATOR(>=)

    bool operator ==(const TestValue<const char *> &value2) {
        __ut_hit_assert();
        if (!strcmp(value, value2.value)) {
            return true;
        }
        throw TestException("==", *this, value2);
    }

    bool operator !=(const TestValue<const char *> &value2) {
        __ut_hit_assert();
        if (strcmp(value, value2.value)) {
            return true;
        }
        throw TestException("!=", *this, value2);
    }

    bool operator ==(const TestValue<char *> &value2) {
        __ut_hit_assert();
        if (!strcmp(value, value2.value)) {
            return true;
        }
        throw TestException("==", *this, value2);
    }

    bool operator !=(const TestValue<char *> &value2) {
        __ut_hit_assert();
        if (strcmp(value, value2.value)) {
            return true;
        }
        throw TestException("!=", *this, value2);
    }

    /* Unary operators */
#   define __UT_U_OPERATOR(__op) \
    bool operator __op() { \
        __ut_hit_assert(); \
        if (__op value) { \
            return true; \
        } \
        throw TestException(__UT_STR(__op), *this); \
    }

    __UT_U_OPERATOR(!)
};

/** Throw user requested fault. Use @ref UT_FAIL macro to call this function. */
void __ut_user_fault(const char *desc, const char *file, int line);

}

#endif /* PHOENIX_UT_H_ */