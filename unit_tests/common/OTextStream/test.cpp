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

using namespace text_stream;

class utStringStream : public OTextStream<utStringStream> {
public:
    utStringStream (char *buf, size_t bufSize) : OTextStream<utStringStream>(this)
    {
        _buf = buf;
        _buf[0] = 0;
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

    stream << OtsOpt(OtsOpt::O_NUM_BOOL);

    stream << true;
    CHECK_STR("1");

    stream << false;
    CHECK_STR("0");

    stream << OtsOpt(OtsOpt::O_NUM_BOOL, false);

    stream << true;
    CHECK_STR("true");

    stream << false;
    CHECK_STR("false");
}
UT_TEST_END

UT_TEST("Stringifying integer values")
{
    char buf[1024];
    utStringStream stream(buf, sizeof(buf));

    stream << 12345678;
    CHECK_STR("12345678");

    size_t size = stream.Format(static_cast<const char *>("Value %d tail"), 12345678);
    UT(size) == UT(sizeof("Value 12345678 tail") - 1);
    CHECK_STR("Value 12345678 tail");
}
UT_TEST_END

class utPrintable {
public:
    int x;

    utPrintable(int x) { this->x = x; }

    bool CheckFmtChar(char fmtChar) {
        return fmtChar == 'a' || fmtChar == 'b' || fmtChar == 'c';
    }

    bool ToString(OTextStreamBase &stream, OTextStreamBase::Context &ctx,
                  char fmtChar = 0)
    {
        if (fmtChar) {
            stream.Format(ctx, "fmt '%c': %d", fmtChar, x);
        } else {
            stream.Format(ctx, "nofmt: %d", x);
        }
        return ctx;
    }
};

UT_TEST("Stringifying user defined classes")
{
    char buf[1024];
    utStringStream stream(buf, sizeof(buf));
    utPrintable p(12345678);

    stream << p;
    CHECK_STR("nofmt: 12345678");

    size_t size = stream.Format("Object: %a tail", p);
    UT(size) == UT(sizeof("Object: fmt 'a: 12345678 tail"));
    CHECK_STR("Object: fmt 'a: 12345678 tail");
}
UT_TEST_END
