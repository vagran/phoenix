/*
 * /phoenix/include/common/OTextStream.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
 * class. Output text streams are capable of converting user defined classes
 * to strings. In order to support such conversion the user defined class
 * should have the following methods defined:
 *
 * @code
 * bool CheckFmtChar(char fmtChar);
 * @endcode
 * This method should check is the provided format character applicable for the
 * class being stringified.
 *
 * @code
 * bool ToString(OTextStreamBase &stream, OTextStreamBase::Context &ctx, char fmtChar = 0);
 * @endcode
 * This method actually converts user defined class object to string.
 * @a fmtChar has value of format character specified for this object if it was
 * converted by @a Format method, or can be zero if it was converted by @a <<
 * operator. @a ctx is a conversion context which can be used to get formatting
 * options. @a Format or FormatValue methods should be used by the object to
 * output all string data. Number of printed characters via @a Format method
 * should be accounted in @a ctx. The method should return @a true if all
 * @a Format (or @a FormatValue) calls returned @a true, and @a false otherwise.
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
            /** Precision -
             * @li Number of digits to appear after the radix
             * character for floating point numbers.
             * @li Number of characters to take from string argument.
             */
            O_PREC,
            /** Represent booleans as numbers instead of symbolic name. */
            O_NUM_BOOL,
            /** Prepend octal numbers with 0 and hexadecimal with '0x'. */
            O_SHARP,
            /** Leave space before positive integers. */
            O_SPACE,
            /** Always prepend integers by sign. */
            O_SIGN,
            /** Pad integer numbers with zero when right justified. */
            O_ZERO,
            /** Left adjust the result. */
            O_LEFT_ADJ,
            /** Padding character. */
            O_PAD_CHAR,
            /** Long modifier. */
            O_LONG,
            /** Short modifier. */
            O_SHORT,

            /** Internal option, indicates that width should be taken from
             * format arguments. Value indicates position relatively to
             * @a O_PREC_REQUIRED if such exists.
             */
            O_WIDTH_REQUIRED,
            /** Internal option, indicates that precision should be taken from
             * format arguments. Value indicates position relatively to
             * @a O_WIDTH_REQUIRED if such exists.
             */
            O_PREC_REQUIRED,
            /** Internal option, indicates that current format specifier was
             * already parsed. Value is a format character.
             */
            O_FMT_PARSED,
            /** Internal option, indicates that it is signed conversion of
             * integer value.
             */
            O_SIGNED,

            /** Maximal option value. */
            O_MAX
        };

        inline Opt(Option option, long param, bool enable = true) {
            _option = option;
            _param = param;
            _enable = enable;
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

        /** Clear all options. */
        inline void ClearAllOpts() {
            _optMap.ClearAll();
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

        /** Apply provided sub-context state to the given context. */
        inline Context &operator+=(const Context &ctx) {
            _size += ctx._size;
            if (ctx._endOfStream) {
                _endOfStream = true;
            }
            return *this;
        }

        /** Get number of written characters. */
        inline operator size_t() { return _size; }

        /** Get end of stream status.
         * @return @a true if end of stream not yet reached.
         */
        inline operator bool() { return !_endOfStream; }

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

    /** Output formated string. */
    template <typename... Args>
    inline size_t Format(const char *fmt, Args... args) {
        Context ctx;
        Format(ctx, fmt, args...);
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
        Format(ctx, fmt);
        return ctx;
    }

    /** Output formatted string. This method should be used by user defined
     * classes in @a ToString method to output string data into provided stream.
     *
     * @param ctx Conversion context.
     * @param fmt Format string.
     * @param value Value to format.
     * @param args Format arguments.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    template <typename T, typename... Args>
    bool Format(Context &ctx, const char *fmt, T &value, Args... args) {
        char fmtChar;
        long _fmtChar;
        Context __ctx, *pCtx;

        if (ctx.Opt(Opt::O_WIDTH_REQUIRED) || ctx.Opt(Opt::O_PREC_REQUIRED)) {

            long widthOrder = 0, precOrder = 0;

            ctx.Opt(Opt::O_WIDTH_REQUIRED, &widthOrder);
            ctx.Opt(Opt::O_PREC_REQUIRED, &precOrder);

            if ((widthOrder < precOrder || !ctx.Opt(Opt::O_PREC_REQUIRED)) &&
                ctx.Opt(Opt::O_WIDTH_REQUIRED)) {

                ctx.ClearOpt(Opt::O_WIDTH_REQUIRED);
                _SetOpt(ctx, Opt::O_WIDTH, value);
            } else if (ctx.Opt(Opt::O_PREC_REQUIRED)) {
                ctx.ClearOpt(Opt::O_PREC_REQUIRED);
                _SetOpt(ctx, Opt::O_PREC, value);
            }
            return Format(ctx, fmt, args...);

        } else if (ctx.Opt(Opt::O_FMT_PARSED, &_fmtChar)) {
            fmtChar = _fmtChar;
            pCtx = &ctx;
            ctx.ClearOpt(Opt::O_FMT_PARSED);

        } else {
            if (!_ParseFormat(__ctx, &fmt, &fmtChar)) {
                return ctx += __ctx;
            }
            if (__ctx.Opt(Opt::O_WIDTH_REQUIRED) || __ctx.Opt(Opt::O_PREC_REQUIRED)) {
                Format(__ctx, fmt, value, args...);
                return ctx += __ctx;
            }
            if (!fmtChar) {
                FAULT("Format arguments without format operator");
                return ctx += __ctx;
            }
            pCtx = &__ctx;
        }

        if (!_CheckFmtChar(fmtChar, value)) {
            FAULT("Format operator ('%c') does not match format argument", fmtChar);
            return ctx += *pCtx;
        }

        /* Output value. */
        bool ret = FormatValue(*pCtx, value, fmtChar);
        if (pCtx != &ctx) {
            ctx += *pCtx;
        }
        if (ret) {
            ret = Format(ctx, fmt, args...);
        }
        return ret;
    }

    /** This method handles the last recursive iteration (or format string
     * without arguments) of the previous template.
     *
     * @param ctx Conversion context.
     * @param fmt Format string. Should not contain any formatting operators.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    bool Format(Context &ctx, const char *fmt);

    /** Output formatted string. It has more limited functionality than
     * @a Format method because it is not types aware for format arguments.
     * So it cannot format user defined classes. For the same reason it is not
     * safe and should not be used unless absolutely necessary.
     *
     * @param fmt Format to output.
     * @param args List of variable arguments for a format.
     * @return Number of characters written.
     */
    inline size_t FormatV(const char *fmt, va_list args) {
        Context ctx;
        FormatV(ctx, fmt, args);
        return ctx;
    }

    /** Output formatted string.
     *
     * @param ctx Conversion context.
     * @param fmt Format to output.
     * @param args List of variable arguments for a format.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    bool FormatV(Context &ctx, const char *fmt, va_list args);

    /** Specify option for conversion.
     *
     * @param opt Option to switch.
     * @return Reference to itself.
     */
    OTextStreamBase &operator << (const Opt &opt);

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

    inline OTextStreamBase &operator << (char *value) {
        _FormatString(_globalCtx, const_cast<const char *>(value));
        return *this;
    }
    inline OTextStreamBase &operator << (const char *value) {
        _FormatString(_globalCtx, value);
        return *this;
    }

    OTextStreamBase &operator << (short value);
    OTextStreamBase &operator << (unsigned short value);
    OTextStreamBase &operator << (int value);
    OTextStreamBase &operator << (unsigned int value);
    OTextStreamBase &operator << (long value);
    OTextStreamBase &operator << (unsigned long value);

    /** Default conversion operator for user defined classes. */
    template <class T>
    inline OTextStreamBase &operator << (T &value) {
        value.ToString(*this, _globalCtx, '\0');
        return *this;
    }

    /** Clear all global options for the stream. The stream will have default
     * conversion behavior after this operation.
     */
    inline void ClearOptions() {
        _globalCtx.ClearAllOpts();
    }

    /** @a FormatValue methods family converts value of specific type into string.
     * @param ctx Conversion context.
     * @param value Value of specific type to convert to string. @a FormatValue
     *      method should call @ref _Putc method for each character it wants to
     *      output.
     * @param fmt Format letter. When format letter is searched the first
     *      alphabetical symbol except 'l', 'L', 'h' and 'H' is considered
     *      to be format letter. All preceding symbols are considered to be
     *      options.
     * @return @a true if end of stream is not yet reached and @a false
     *      otherwise.
     */
    inline bool FormatValue(Context &ctx, short value, char fmt = 0) {
        ctx.SetOpt(Opt::O_SIGNED);
        return _FormatIntValue(ctx, value, fmt);
    }
    inline bool FormatValue(Context &ctx, unsigned short value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    inline bool FormatValue(Context &ctx, int value, char fmt = 0) {
        ctx.SetOpt(Opt::O_SIGNED);
        return _FormatIntValue(ctx, value, fmt);
    }
    inline bool FormatValue(Context &ctx, unsigned int value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }
    inline bool FormatValue(Context &ctx, long value, char fmt = 0) {
        ctx.SetOpt(Opt::O_SIGNED);
        return _FormatIntValue(ctx, value, fmt);
    }
    inline bool FormatValue(Context &ctx, unsigned long value, char fmt = 0) {
        return _FormatIntValue(ctx, value, fmt);
    }

    bool FormatValue(Context &ctx, bool value, char fmt = 0);
    bool FormatValue(Context &ctx, char value, char fmt = 0);

    inline bool FormatValue(Context &ctx, char *value, char fmt UNUSED) {
        return _FormatString(ctx, value);
    }
    inline bool FormatValue(Context &ctx, const char *value, char fmt UNUSED) {
        return _FormatString(ctx, value);
    }

    template <typename T>
    inline bool FormatValue(Context &ctx, T *value, char fmt = 0) {
        ctx.SetOpt(Opt::O_RADIX, 16);
        ctx.SetOpt(Opt::O_SHARP);
        return _FormatIntValue(ctx, reinterpret_cast<uintptr_t>(value), fmt);
    }

    /** Format user defined class object. */
    template <class T>
    inline bool FormatValue(Context &ctx, T &value, char fmt = 0) {
        return value.ToString(*this, ctx, fmt);
    }

protected:

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

    /** These methods validate type against format character.
     *
     * @param fmtChar Format character which was specified for @a value.
     * @return @a true if the specified format character is valid for the
     *      specified type.
     */
    inline bool _CheckFmtChar(char fmtChar, short) {
        return fmtChar == 'd' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X';
    }
    inline bool _CheckFmtChar(char fmtChar, unsigned short) {
        return fmtChar == 'u' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X';
    }
    inline bool _CheckFmtChar(char fmtChar, int) {
        return fmtChar == 'd' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X';
    }
    inline bool _CheckFmtChar(char fmtChar, unsigned) {
        return fmtChar == 'u' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X';
    }
    inline bool _CheckFmtChar(char fmtChar, long) {
        return fmtChar == 'd' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X';
    }
    inline bool _CheckFmtChar(char fmtChar, unsigned long) {
        return fmtChar == 'd' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X' ||
               fmtChar == 'z';
    }

    inline bool _CheckFmtChar(char fmtChar, char) {
        return fmtChar == 'c';
    }
    inline bool _CheckFmtChar(char fmtChar, char *) {
        return fmtChar == 's';
    }
    inline bool _CheckFmtChar(char fmtChar, const char *) {
        return fmtChar == 's';
    }

    template <typename T>
    inline bool _CheckFmtChar(char fmtChar, T *) {
        return fmtChar == 'p';
    }

    template <class T>
    inline bool _CheckFmtChar(char fmtChar, T &value) {
        return value.CheckFmtChar(fmtChar);
    }

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
     * @return @a true if end of stream is not yet reached during plain strings
     *      output, @a false otherwise.
     */
    bool _ParseFormat(Context &ctx, const char **fmt, char *fmtChar);

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

    inline bool _FormatIntValue(Context &ctx, unsigned short value, char fmt = 0) {
        return _FormatInt(ctx, value, false, fmt);
    }
    inline bool _FormatIntValue(Context &ctx, unsigned int value, char fmt = 0) {
        return _FormatInt(ctx, value, false, fmt);
    }
    inline bool _FormatIntValue(Context &ctx, unsigned long value, char fmt = 0) {
        return _FormatInt(ctx, value, false, fmt);
    }

    bool _FormatInt(Context &ctx, unsigned long value, bool neg = 0, char fmt = 0);

    bool _FormatString(Context &ctx, const char *value);

    /** Wrappers for setting option value from format arguments. They should
     * ensure that argument will have correct type for corresponding option.
     */
    inline void _SetOpt(Context &ctx, Opt::Option opt, long value) { ctx.SetOpt(opt, value); }
    inline void _SetOpt(Context &ctx, Opt::Option opt, unsigned long value) { ctx.SetOpt(opt, value); }
    inline void _SetOpt(Context &ctx, Opt::Option opt, int value) { ctx.SetOpt(opt, value); }
    inline void _SetOpt(Context &ctx, Opt::Option opt, unsigned int value) { ctx.SetOpt(opt, value); }
    inline void _SetOpt(Context &ctx, Opt::Option opt, short value) { ctx.SetOpt(opt, value); }
    inline void _SetOpt(Context &ctx, Opt::Option opt, unsigned short value) { ctx.SetOpt(opt, value); }

    template <typename T>
    inline void _SetOpt(Context &ctx UNUSED, Opt::Option opt UNUSED, T &value UNUSED) {
        FAULT("Invalid argument type used for initializing format option");
    }

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

    /** Output field representation.
     *
     * @param ctx Conversion context. Width and adjustment options are taken
     *      from it.
     * @param value Field string representation.
     * @param numChars Number of characters to take from @a value.
     * @param padChar Padding character. Use default if zero. Default may come
     *      from option O_PAD_CHAR.
     * @return @a true if end of stream is not yet reached, @a false otherwise.
     */
    bool _FormatField(Context &ctx, const char *value, size_t numChars,
                      char padChar = 0);
};

/** Shortcut type for output text stream options. */
typedef OTextStreamBase::Opt OtsOpt;

/** Implementation class for output text stream.
 *
 * @param T_backend Back-end class which must implement @a Putc method which
 *      is called for each character of stream. The method must have the
 *      following prototype:
 *      @code
 *      bool Putc(char c, T_arg *arg = 0);
 *      @endcode
 *      It should have optional argument which is pointer to type @a T_arg in
 *      this template.
 * @param T_arg Type of optional argument pointer to which is passed to the
 *      @a Putc method of a back-end class.
 *
 * @see OTextStreamBase
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
