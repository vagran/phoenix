/*
 * /phoenix/include/common/OTextStream.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file OTextStream.h
 * Universal output text stream.
 */

#ifndef OTEXTSTREAM_H_
#define OTEXTSTREAM_H_

#include <BitString.h>

/** Text streams manipulations. */
namespace text_stream {

/** Base class for output text stream objects. They should be derived from this
 * class.
 */
class OTextStreamBase {
public:
    /** Wrapper for options. Objects of this class can be applied to text
     * stream by the insertion operator.
     */
    class Opt {
    public:
        /** Converting options. */
        enum Option {
            /** Set radix for subsequent integer numbers conversions. Parameter
             * is a new radix value.
             */
            O_RADIX,
            /** Set width for subsequent formatting. Valid for one value only.
             * Parameter is width in characters.
             */
            O_WIDTH,
            /** Represent booleans as numbers instead of symbolic name. */
            O_NUM_BOOL,

            /** Maximal option value. */
            O_MAX
        };

        inline Opt(Option option, long param, bool _enable = true) {
            _option = option;
            _param = param;
            _enable = true;
        }

        inline Opt(Option option, bool enable = true) {
            _option = option;
            _param = 0;
            _enable = enable;
        }
    private:
        friend class OTextStreamBase;
        Option _option;
        long _param;
        bool _enable;
    };

    /** Output formatted string. It has more limited functionality than
     * @ref Format method because it is not types aware for format arguments.
     * So it cannot format user defined classes.
     *
     * @param fmt Format to output.
     * @param args List of variable arguments for a format.
     * @return Number of characters written.
     */
    size_t FormatV(const char *fmt, va_list args);

    template <typename... Args>
    inline size_t Format(const char *fmt, Args... args) {
        Context ctx;
        _Format(ctx, fmt, args...);
        return ctx;
    }

    /** This method handles edge case of previous template when only format
     * string is specified.
     *
     * @param fmt Format string. Should not contain any formatting
     *      operators.
     * @return Number of characters written.
     */
    inline size_t Format(const char *fmt) {
        Context ctx;
        _Format(ctx, fmt);
        return ctx;
    }

    /** Specify option for conversion.
     *
     * @param opt Option to switch.
     * @return Reference to itself.
     */
    OTextStreamBase &operator << (const Opt &&opt);

    /** Convert the provided value to string. This operator is overloaded for
     * all supported types. For the types which are not supported here, the
     * @a >> operator of the object provided as value will be called. The
     * operator should have the following prototype:
     * @code
     * void operator >> (OTextStreamBase &s);
     * @endcode
     *
     * @return Reference to itself.
     */
    OTextStreamBase &operator << (bool value);

    OTextStreamBase &operator << (char value);

    OTextStreamBase &operator << (short value);
    OTextStreamBase &operator << (unsigned short value);
    OTextStreamBase &operator << (int value);
    OTextStreamBase &operator << (unsigned int value);
    OTextStreamBase &operator << (long value);
    OTextStreamBase &operator << (unsigned long value);

    /** Default conversion operator. */
    template <class T>
    inline OTextStreamBase &operator << (T value) {
        /* If there are no << operator for a provided type in the base class
         * then use >> operator in the object being converted to string.
         */
        value >> *this;
        return *this;
    }

protected:

    /** Conversion context. */
    class Context {
    public:
        Context();

        /** Get option presence and value.
         *
         * @param opt Option to check.
         * @param value Optional pointer where the option value should be stored.
         *      Can be zero if value is not required.
         * @return @a true if the option is set, @a false otherwise.
         */
        inline bool Opt(Opt::Option opt, long *value = 0) {
            bool ret = _optMap.IsSet(opt);
            if (value) {
                *value = ret ? _optVal[opt] : 0;
            }
            return ret;
        }

        /** Set specified option.
         *
         * @param opt Option to set.
         * @param value Optional associated value.
         */
        inline void SetOpt(Opt::Option opt, long value = 0) {
            _optMap.Set(opt);
            _optVal[opt] = value;
        }

        /** Clear specified option.
         *
         * @param opt Option to clear.
         */
        inline void ClearOpt(Opt::Option opt) {
            _optMap.Clear(opt);
        }

        /** Indicate end of stream status. */
        inline void End() { _endOfStream = true; }

        /** Increment counter of written characters.
         * @return New counter value.
         */
        inline size_t operator++(int) { return ++_size; }

        /** Add specified value to counter for written characters.
         * @param addend Value to add.
         * @return New counter value.
         */
        inline size_t operator+=(size_t addend) { return _size += addend; }

        /** Get number of written characters. */
        inline operator size_t() { return _size; }

        /** Get end of stream status.
         * @return @a true if end of stream reached.
         */
        inline operator bool() { return _endOfStream; }

    private:
        /** Bitmap of set options. */
        BitString<Opt::O_MAX> _optMap;
        /** Values of options. */
        long _optVal[Opt::O_MAX];
        /** Number of characters written. */
        size_t _size;
        /** @a true if end of stream reached. */
        bool _endOfStream;
    };

    Context _globalCtx; /**< Global context for insertion operators. */

    OTextStreamBase();

    /** This method must be overridden in a derived class. All formatting
     * methods call it to output next text character.
     *
     * @param c Character to output.
     * @return @a true if the character was output. @a false if end of stream
     *      reached, the rest characters in current string will be dropped in
     *      such case.
     */
    virtual bool _Putc(char c) = 0;

    /** Output provided character.
     *
     * @param ctx Conversion context.
     * @param c Character to output.
     * @return @a true if character was written, @a false otherwise.
     */
    inline bool _Putc(Context &ctx, char c) {
        if (_Putc(c)) {
            ctx++;
            return true;
        }
        ctx.End();
        return false;
    }

    /** Output provided string.
     *
     * @param ctx Conversion context.
     * @param str String to output.
     * @return @a true if end of stream is not yet reached, @a false otherwise
     *      (end of stream reached).
     */
    bool _Puts(Context &ctx, const char *str);

    /** Output formatted string.
     *
     * @param ctx Conversion context.
     * @param fmt Format string.
     * @param args Format arguments.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    template <typename T, typename... Args>
    inline bool _Format(Context &ctx, const char *fmt, T value, Args... args) {
        char fmtChar;
        Context _ctx;
        if (!_ParseFormat(_ctx, &fmt, &fmtChar)) {
            ctx += _ctx;
            return false;
        }

        if (!fmtChar) {
            FAULT("Format arguments without format operator");
            ctx += _ctx;
            return false;
        }

#if 0 //XXX will user defined class be supported?
        if (!_CheckFmtChar(fmtChar, value)) {
            FAULT("Format operator ('%c') does not match format argument", fmtChar);
            ctx += _ctx;
            return false;
        }
#endif
        /* Output value. */
        bool ret = _FormatValue(_ctx, value, fmtChar);
        ctx += _ctx;
        if (ret) {
            ret = _Format(ctx, fmt, args...);
        }

        return ret;
    }

    /** This method handles the last recursive iteration (or format string
     * without arguments) of the previous template.
     * @param ctx Conversion context.
     * @param fmt Format string. Should not contain any formatting operators.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    bool _Format(Context &ctx, const char *fmt);

    /** Parse format string. All encountered options should be applied to the
     * provided conversion context. Plain string preceding format operators
     * should be output using provided context.
     *
     * @param ctx Conversion context.
     * @param fmt Format string. Pointer is advanced during string parsing.
     *      When the functions returns it point to the character next to a
     *      format operator found.
     * @param fmtChar Pointer to location where format character will be stored.
     *      Will store zero if no format operators were found.
     * @return @true if end of stream is not yet reached during plain strings
     *      output, @a false otherwise.
     */
    bool _ParseFormat(Context &ctx, const char **fmt, char *fmtChar);

    /** @a _FormatValue methods family converts value of specific type into string.
     * @param ctx Conversion context.
     * @param value Value of specific type to convert to string. @a _FormatValue
     *      method should call @ref _Putc method for each character it wants to
     *      output.
     * @param fmt Format letter. When format letter is searched the first
     *      alphabetical symbol except 'l', 'L', 'h' and 'H' is considered
     *      to be format letter. All preceding symbols are considered to be
     *      options.
     * @return @a true if end of stream is not yet reached and @a false
     *      otherwise.
     */
    bool _FormatValue(Context &ctx, short value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    bool _FormatValue(Context &ctx, unsigned short value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    bool _FormatValue(Context &ctx, int value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    bool _FormatValue(Context &ctx, unsigned int value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    bool _FormatValue(Context &ctx, long value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    bool _FormatValue(Context &ctx, unsigned long value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }

    bool _FormatValue(Context &ctx, bool value, char fmt = 0);

    template <typename T>
    bool _FormatIntValue(Context &ctx, T value, char fmt = 0) {
        bool neg;
        unsigned long v;

        if (value >= 0) {
            v = value;
            neg = false;
        } else {
            v = -value;
            neg = true;
        }
        return _FormatInt(ctx, v, neg, fmt);
    }

    bool _FormatInt(Context &ctx, unsigned long value, bool neg = 0, char fmt = 0);

    /** Convert integer value to string. String is filled in reverse order.
     *
     * @param value Value to convert.
     * @param buf Buffer where to store the result.
     * @param radix Radix for integer representation (2..36).
     * @param upperCase Use upper case letter if @a true.
     * @return Number of characters stored in output buffer.
     */
    size_t _IntToString(unsigned long value, char *buf, unsigned long radix = 10,
                       bool upperCase = false);

};

typedef OTextStreamBase::Opt OtsOpt;

/** Implementation class for output text stream.
 *
 * @param T_backend Back-end class which must implement @a Putc method which
 *      is called for each character of stream. The method must have the
 *      following prototype:
 *      @code
 *      bool Putc(char c, T_arg *arg = 0);
 *      @endcode
 *      It should have optional argument which is of type @a T_arg in this
 *      template.
 * @param T_arg Type of optional argument which is passed to the @a Putc method
 *      of a back-end class.
 */
template <class T_backend, typename T_arg = void>
class OTextStream : public OTextStreamBase {
public:
    inline OTextStream(T_backend *backend, T_arg *arg = 0) {
        _backend = backend;
        _arg = arg;
    }

private:
    T_backend *_backend;
    T_arg *_arg;

    virtual bool _Putc(char c) {
        return _backend->Putc(c, _arg);
    }
};

} /* namespace text_stream */

#endif /* OTEXTSTREAM_H_ */
