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

/** Base class for output text stream objects. They should be derived from this
 * class.
 */
class OTextStreamBase {
public:

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
        O_NUM_BOOl,

        /** Maximal option value. */
        O_MAX
    };

    class Opt {
    public:
        inline Opt(Option option, long param = 0) {
            _option = option;
            _param = param;
        }
    private:
        long _option, _param;
    };

    /** This method must be overridden in a derived class. All formatting
     * methods call it to output next text character.
     *
     * @param c Character to output.
     * @return @a true if the character was output. @a false if end of stream
     *      reached, the rest characters in current string will be dropped in
     *      such case.
     */
    virtual bool Putc(char c) = 0;

    /** Output formatted string.
     *
     * @param fmt Format to output.
     * @param args List of variable arguments for a format.
     * @return Number of characters written.
     */
    size_t FormatV(const char *fmt, va_list args);

    /** Specify option for conversion.
     *
     * @param opt Option to switch.
     * @return Reference to itself.
     */
    OTextStreamBase &operator << (const Opt &&opt);

    /** Convert a value to string. This operator is overloaded for all
     * supported types. For the types which are not supported here, the derived
     * class will call >> operator of the object provided as value.
     *
     * @param value Value to convert.
     * @return Reference to itself.
     */
    OTextStreamBase &operator << (bool value);
    OTextStreamBase &operator << (short value);

protected:

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
        bool Opt(Option opt, long *value = 0);
        /** Increment counter of written characters. */
        inline size_t operator++(int) { return ++_size; }
        /** Get number of written characters. */
        inline operator size_t() { return _size; }
    private:
        /** Bitmap of set options. */
        u8 _optMap[(O_MAX + NBBY - 1) / NBBY];
        /** Values of options. */
        long _optVal[O_MAX];
        /** Number of characters written. */
        size_t _size;
    };

    Context _globalCtx;

    OTextStreamBase();



    /** Output provided string.
     *
     * @param size Incremented by number of characters written.
     * @param str String to output.
     * @return @a true if all @a Putc calls returned @a true, @a false otherwise.
     */
    bool _Puts(size_t &size, const char *str);

    /** @a _FormatValue methods family converts value of specific type into string.
     * @param size Incremented by number of characters written.
     * @param opt Points to optional characters preceding format letter @a fmt
     *      for a specific type.
     * @param fmt Format letter. When format letter is searched the first
     *      alphabetical symbol except 'l', 'L', 'h' and 'H' is considered
     *      to be format letter. All preceding symbols are considered to be
     *      options.
     * @param optSize Number of characters in option string
     * @param value Value of specific type to convert to string. @a _FormatValue
     * method should call @a Putc method for each character it wants to output.
     * @return @a true if all @a Putc calls returned @a true, and @a false
     *      otherwise.
     */
    bool _FormatValue(size_t &size, char fmt, const char *opt, size_t optSize, bool value);
    bool _FormatValue(size_t &size, char fmt, const char *opt, size_t optSize, short value);
};

/** Implementation class for output text stream.
 *
 * @param T_backend Back-end class which must implement @a Putc method which
 *      is called for each character of stream stream. The method must have the
 *      following prototype: @n
 *      bool Putc(char c, T_arg *arg = 0); @n
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

    virtual bool Putc(char c) {
        return _backend->Putc(c, _arg);
    }

    template <typename T>
    OTextStream &operator << (T value) {
        OTextStreamBase::operator <<(value);
        return *this;
    }

private:
    T_backend *_backend;
    T_arg *_arg;
};

#endif /* OTEXTSTREAM_H_ */
