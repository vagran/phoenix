/*
 * /phoenix/include/log.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file log.h
 * Centralized system logging definitions.
 */

#ifndef LOG_H_
#define LOG_H_

#ifdef UNITTEST

void __ut_trace(const char *file, int line, const char *msg, ...);

#define TRACE_FMT(...) __ut_trace(__FILE__, __LINE__, __VA_ARGS__)

#elif defined(KERNEL)

namespace log {

class DbgSerialPort;
extern DbgSerialPort *dbgSerialPort;
extern text_stream::OTextStream<DbgSerialPort> *dbgStream;

} /* namespace log */

#define TRACE_FMT(...) log::dbgStream->Format(__VA_ARGS__)

#elif defined(EFI_APP)

#define TRACE_FMT(...)

#endif

/** Macro for printing debug messages into debug console. Has no effect in
 * production build.
 *
 * @param msg Message to output.
 */
#ifdef DEBUG
#define TRACE(msg, ...) \
    TRACE_FMT("[TRACE] %s:%d <%s>: " msg "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else /* DEBUG */
#define TRACE(msg, ...)
#endif /* DEBUG */

/** Contains definitions related to centralized system logging. */
namespace log {

/** Global system log. Messages to the system log should be written by methods
 * provided by @a OTextStream underlying class.
 */
class SysLogBase : public text_stream::OTextStream<SysLogBase> {
public:
    /** Importance levels for system log messages. */
    enum Level {
        LOG_ALERT, /**< Fatal or unrecoverable fault occurred. */
        LOG_CRITICAL, /**< Critical conditions, action must be taken immediately. */
        LOG_ERROR, /**< Error conditions. */
        LOG_WARNING, /**< Warning conditions. */
        LOG_NOTICE, /**< Normal, but significant, condition. */
        LOG_INFO, /**< Informational message. */
        LOG_DEBUG /**< Debug-level message. */
    };

    inline SysLogBase() : text_stream::OTextStream<SysLogBase>(this) {
        _curLevel = LOG_DEBUG;
#ifdef DEBUG
        _maxLevel = LOG_DEBUG;
#else /* DEBUG */
        _maxLevel = LOG_NOTICE;
#endif /* DEBUG */
    }

    /** This operator must be overloaded in back-end derived class. This
     * operator should trigger new message printing transaction. It must at
     * least set @ref _curLevel member.
     *
     * @param level Level of the message.
     * @return Reference to itself.
     */
    virtual SysLogBase &operator <<(log::SysLogBase::Level level) = 0;

    template <typename T>
    inline SysLogBase &operator <<(T value) {
        if (UNLIKELY(_curLevel <= _maxLevel)) {
            text_stream::OTextStream<SysLogBase>::operator <<(value);
        }
        return *this;
    }

    /** Output formated string. */
    template <typename... Args>
    inline SysLogBase &Format(const char *fmt, Args... args) {
        if (UNLIKELY(_curLevel <= _maxLevel)) {
            text_stream::OTextStream<SysLogBase>::Format(fmt, args...);
        }
        return *this;
    }

    /** This method handles edge case of previous template when only format
     * string is specified.
     *
     * @param fmt Format string. Should not contain any formatting
     *      operators.
     */
    inline SysLogBase &Format(const char *fmt) {
        if (UNLIKELY(_curLevel <= _maxLevel)) {
            text_stream::OTextStream<SysLogBase>::Format(fmt);
        }
        return *this;
    }

    inline SysLogBase &FormatV(const char *fmt, va_list args) {
        if (UNLIKELY(_curLevel <= _maxLevel)) {
            text_stream::OTextStream<SysLogBase>::FormatV(fmt, args);
        }
        return *this;
    }

    /** Output log message with @ref LOG_ALERT level. */
    template <typename... Args>
    SysLogBase &Alert(const char *fmt, Args... args)
    {
        *this << LOG_ALERT;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_CRITICAL level. */
    template <typename... Args>
    SysLogBase &Critical(const char *fmt, Args... args)
    {
        *this << LOG_CRITICAL;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_ERROR level. */
    template <typename... Args>
    SysLogBase &Error(const char *fmt, Args... args)
    {
        *this << LOG_ERROR;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_WARNING level. */
    template <typename... Args>
    SysLogBase &Warning(const char *fmt, Args... args)
    {
        *this << LOG_WARNING;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_NOTICE level. */
    template <typename... Args>
    SysLogBase &Notice(const char *fmt, Args... args)
    {
        *this << LOG_NOTICE;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_INFO level. */
    template <typename... Args>
    SysLogBase &Info(const char *fmt, Args... args)
    {
        *this << LOG_INFO;
        Format(fmt, args...);
        return *this;
    }

    /** Output log message with @ref LOG_DEBUG level. */
    template <typename... Args>
    SysLogBase &Debug(const char *fmt, Args... args)
    {
        *this << LOG_DEBUG;
        Format(fmt, args...);
        return *this;
    }

    /** Output character to the log. Must be overloaded in derived back-end
     * class.
     *
     * @param c Character to output.
     * @param arg Optional argument.
     * @return @a true if end of back-end stream is not yet reached, @a false
     *      otherwise.
     */
    virtual bool Putc(char c, void *arg) = 0;
protected:
    /** Level for the message currently being printed. */
    Level _curLevel;
    /** Ignore messages above this level. */
    Level _maxLevel;
};

#ifdef KERNEL

/** Kernel implementation for system log. */
class KSysLog : public SysLogBase {
public:
    KSysLog();

    virtual SysLogBase &operator <<(log::SysLogBase::Level level);

    virtual bool Putc(char c, void *arg = 0);
private:
    bool lastNewLine;
};

/** Global system log class. */
typedef KSysLog SysLog;

extern SysLog *sysLog;

/** Global system log object. It should be defined in each component. It has
 * @ref text_stream::OTextStream interface so it can be used as in this example:
 * @code
 * LOG << LL(WARNING) << "Some warning message\n";
 * LOG.Format("Some additional info on warning %d", 100);
 * LOG.Notice("Some notice message here %d", 1000);
 * @endcode
 */
#define LOG     (*log::sysLog)

#elif defined(EFI_APP) /* KERNEL */

/* Stub for EFI environment. */
typedef void *SysLog;

#else /* EFI_APP */

// XXX user space log

#endif /* KERNEL/EFI_APP */

/** Initialize logging functionality. */
void InitLog();

/** Macro for short reference to system log levels.
 *
 * Usage example:
 * @code
 * sysLog << LL(WARNING) << "Some warning message";
 * @endcode
 *
 * @see log::SysLogBase::Level
 */
#define LL(level) __CONCAT(log::SysLogBase::LOG_, level)

} /* namespace log */

#endif /* LOG_H_ */
