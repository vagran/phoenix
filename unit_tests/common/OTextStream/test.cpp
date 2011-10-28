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

/* Verify string. */
#define CHECK_STR(value) \
    do {\
        UT(stream.Get()) == UT_CSTR(value); \
        stream.Erase(); \
    } while (0)

/* Verify string and size. */
#define CHECK_STR_SZ(value) \
do {\
    UT(size) == UT(sizeof(value) - 1); \
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
    size_t size;

    stream << 12345678;
    CHECK_STR("12345678");

    stream << -12345678;
    CHECK_STR("-12345678");

    stream << 12345678 << " in the middle " << 87654321;
    CHECK_STR("12345678 in the middle 87654321");

    stream << OtsOpt(OtsOpt::O_RADIX, 2l) << BIN(0x11001101);
    CHECK_STR("11001101");
    stream.ClearOptions();

    size = stream.Format("Value %d tail", 12345678);
    CHECK_STR_SZ("Value 12345678 tail");

    size = stream.Format("Value %d tail", -12345678);
    CHECK_STR_SZ("Value -12345678 tail");

    size = stream.Format("Value % d tail", 12345678);
    CHECK_STR_SZ("Value  12345678 tail");

    size = stream.Format("Value %+d tail", 12345678);
    CHECK_STR_SZ("Value +12345678 tail");

    size = stream.Format("Value % +d tail", 12345678);
    CHECK_STR_SZ("Value +12345678 tail");

    size = stream.Format("Value %d in the middle %d tail", 12345678, 87654321);
    CHECK_STR_SZ("Value 12345678 in the middle 87654321 tail");

    size = stream.Format("Value %o tail", 01234567);
    CHECK_STR_SZ("Value 1234567 tail");

    size = stream.Format("Value %x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 1234abcd tail");

    size = stream.Format("Value %X tail", 0x1234abcd);
    CHECK_STR_SZ("Value 1234ABCD tail");

    size = stream.Format("Value %#o tail", 01234567);
    CHECK_STR_SZ("Value 01234567 tail");

    size = stream.Format("Value %#x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 0x1234abcd tail");

    size = stream.Format("Value %#X tail", 0x1234abcd);
    CHECK_STR_SZ("Value 0X1234ABCD tail");

    size = stream.Format("Value %12d tail", 12345678);
    CHECK_STR_SZ("Value     12345678 tail");

    size = stream.Format("Value %12d tail", -12345678);
    CHECK_STR_SZ("Value    -12345678 tail");

    size = stream.Format("Value %012d tail", 12345678);
    CHECK_STR_SZ("Value 000012345678 tail");

    size = stream.Format("Value %012d tail", -12345678);
    CHECK_STR_SZ("Value -00012345678 tail");

    size = stream.Format("Value %-12d tail", 12345678);
    CHECK_STR_SZ("Value 12345678     tail");

    size = stream.Format("Value %-12d tail", -12345678);
    CHECK_STR_SZ("Value -12345678    tail");

    size = stream.Format("Value %#-12x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 0x1234abcd   tail");

    size = stream.Format("Value %012x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 00001234abcd tail");

    size = stream.Format("Value %#012x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 0x001234abcd tail");

    size = stream.Format("Value %#-12x tail", 0x1234abcd);
    CHECK_STR_SZ("Value 0x1234abcd   tail");

    size = stream.Format("Value %s tail", "12345678");
    CHECK_STR_SZ("Value 12345678 tail");

    size = stream.Format("Value %12s tail", "12345678");
    CHECK_STR_SZ("Value     12345678 tail");

    size = stream.Format("Value %-12s tail", "12345678");
    CHECK_STR_SZ("Value 12345678     tail");

    size = stream.Format("Value %.4s tail", "12345678");
    CHECK_STR_SZ("Value 1234 tail");

    size = stream.Format("Value %8.4s tail", "12345678");
    CHECK_STR_SZ("Value     1234 tail");

    size = stream.Format("Value %-8.4s tail", "12345678");
    CHECK_STR_SZ("Value 1234     tail");

    size = stream.Format("Value %.4-8s tail", "12345678");
    CHECK_STR_SZ("Value 1234     tail");
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
        OTextStreamBase::Context _ctx;
        if (fmtChar) {
            stream.Format(_ctx, "fmt '%c': %d", fmtChar, x);
        } else {
            stream.Format(_ctx, "nofmt: %d", x);
        }
        return ctx += _ctx;
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
    CHECK_STR_SZ("Object: fmt 'a': 12345678 tail");
}
UT_TEST_END
