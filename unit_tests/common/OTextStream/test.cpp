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

class utStringStream : public text_stream::OTextStream<utStringStream> {
public:
    utStringStream (char *buf, size_t bufSize) : OTextStream<utStringStream>(this)
    {
        _buf = buf;
        _bufSize = bufSize;
        _curPos = 0;
    }

    bool Putc(char c, void *arg UNUSED) {
        if (_curPos >= _bufSize - 1) {
            return false;
        }
        _buf[_curPos] = c;
        _curPos++;
        _buf[_curPos] = 0;
        return true;
    }

    char *Get() { return _buf; }

    void Erase() {
        _curPos = 0;
        _buf[0] = 0;
    }
private:
    size_t _bufSize, _curPos;
    char *_buf;
};

#define CHECK_STR(value) \
    do {\
        UT(stream.Get()) == UT_CSTR(value); \
        stream.Erase(); \
    } while (0)

UT_TEST("Stringifying boolean values")
{
    char buf[1024];
    utStringStream stream(buf, sizeof(buf));

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
