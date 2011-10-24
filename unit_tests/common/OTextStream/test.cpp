/*
 * /phoenix/unit_tests/common/OTextStream/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <phoenix_ut.h>

#include <sys.h>

#include <string>

class utStringStream : public text_stream::OTextStream<utStringStream, std::string> {
public:
    utStringStream (std::string *s) :
        OTextStream<utStringStream, std::string>(this, s)
    {

    }

    bool Putc(u8 c, std::string *arg) {
        (*arg) += c;
        return true;
    }
};

#define CHECK_STR(value) \
    do {\
        UT(s.c_str()) == UT(value); \
        s.erase(); \
    } while (0)

UT_TEST("Stringifying boolean values")
{
    std::string s;
    utStringStream stream(&s);

    stream << true;
    CHECK_STR("true");

    stream << false;
    CHECK_STR("false");
}
UT_TEST_END

UT_TEST("Stringifying integer values")
{

}
UT_TEST_END
