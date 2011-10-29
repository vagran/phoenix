/*
 * /phoenix/include/log.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file log.h
 * Centralized system logging definitions.
 */

#ifndef LOG_H_
#define LOG_H_

#ifdef KERNEL

namespace log {

class DbgSerialPort;
extern DbgSerialPort dbgSerialPort;
extern text_stream::OTextStream<DbgSerialPort> dbgStream;

} /* namespace log */

#define TRACE_FMT log::dbgStream.Format

#endif /* KERNEL */

/** Macro for printing debug messages into debug console. Has no effect in
 * production build.
 *
 * @param msg Message to output.
 */
#ifdef DEBUG
#define TRACE(msg, ...) \
    TRACE_FMT("[TRACE] %s:%d: " msg "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#else /* DEBUG */
#define TRACE(msg, ...)
#endif /* DEBUG */

/** Contains definitions related to centralized system logging. */
namespace log {

/** Global system log. Messages to the system log should be written by methods
 * provided by @ref OTextStream underlying class.
 */
class SysLogBase : public text_stream::OTextStream<SysLogBase> {
public:
    /** Importance levels for system log messages. */
    enum Level {
        LOG_ALERT,
        LOG_CRITICAL,
        LOG_ERROR,
        LOG_WARNING,
        LOG_NOTICE,
        LOG_INFO,
        LOG_DEBUG
    };

    SysLogBase() : text_stream::OTextStream<SysLogBase>(this) { }
    ~SysLogBase() { }
};

#ifdef KERNEL

/** Kernel implementation for system log. */
class KSysLog : public SysLog {

};

typedef KSysLog SysLog;

#else /* KERNEL */

// XXX user space log

#endif /* KERNEL */

/** Global system log object. It should be defined in each component. */
extern SysLog sysLog;

/** Macro for short reference to system log levels. Usage example:
 * @code
 * sysLog << LL(WARNING) << "Some warning message";
 * @endcode
 */
#define LL(level) __CONCAT(log::SysLog::LOG_, level)

} /* namespace log */

#endif /* LOG_H_ */
