/*
 * /phoenix/unit_tests/ut/phoenix_stubs.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file phoenix_stubs.cpp
 * Stubs for global Phoenix functions.
 */

#include <phoenix_ut.h>

#include <sys.h>

void
__Fault(const char *file, int line, const char *msg, ...)
{
    char buf[1024];
    __ut_va_list args;
    __ut_va_start(args, msg);
    int size = ut::__ut_snprintf(buf, sizeof(buf), "Fault occurred at %s:%d: ", file, line);
    ut::__ut_vsnprintf(&buf[size], sizeof(buf) - size, msg, args);
    __ut_va_end(args);
    UT_FAIL("%s", buf);
    while (true);
}

void
__ut_trace(const char *file, int line, const char *msg, ...)
{
    __ut_va_list args;
    __ut_va_start(args, msg);
    ut::__ut_vtrace(file, line, msg, args);
    __ut_va_end(args);
}

/* Memory allocation interface. */

void *
operator new(size_t size)
{
    /* Do not track allocations for default operators. */
    return ut::__ut_malloc(0, 0, size);
}

void *
operator new[](size_t size)
{
    /* Do not track allocations for default operators. */
    return ut::__ut_malloc(0, 0, size);
}

void *
operator new(size_t size, size_t align, bool nonRec)
{
    void *p = ut::__ut_malloc(__FILE__, __LINE__, size, align);
    if (!p && nonRec) {
        UT_FAIL("Unrecoverable memory allocation failed.");
    }
    return p;
}

void *
operator new[](size_t size, size_t align, bool nonRec)
{
    void *p = ut::__ut_malloc(__FILE__, __LINE__, size, align);
    if (!p && nonRec) {
        UT_FAIL("Unrecoverable memory allocation failed.");
    }
    return p;
}

void *
operator new(size_t size, const char *file, int line, size_t align, bool nonRec)
{
    void *p = ut::__ut_malloc(file, line, size, align);
    if (!p && nonRec) {
        UT_FAIL("Unrecoverable memory allocation failed: %s:%d", file, line);
    }
    return p;
}

void *
operator new[](size_t size, const char *file, int line, size_t align, bool nonRec)
{
    void *p = ut::__ut_malloc(file, line, size, align);
    if (!p && nonRec) {
        UT_FAIL("Unrecoverable memory allocation failed: %s:%d", file, line);
    }
    return p;
}

void
operator delete(void *ptr)
{
    ut::__ut_mfree(ptr);
}

void
operator delete[](void *ptr)
{
    ut::__ut_mfree(ptr);
}

#ifdef KERNEL

/* Kernel logging interface. */

log::SysLog *log::sysLog;

log::KSysLog::KSysLog()
{
    lastNewLine = true;
}

log::SysLogBase &
log::KSysLog::operator << (Level level)
{
    /* Terminate previous message if necessary. */
    if (!lastNewLine) {
        Putc('\n');
        lastNewLine = true;
    }
    ClearOptions();

    _curLevel = level;
    if (_curLevel > _maxLevel) {
        return *this;
    }

    const char *name;
    switch (level) {
    case LOG_ALERT:
        name = "ALERT";
        break;
    case LOG_CRITICAL:
        name = "CRITICAL";
        break;
    case LOG_ERROR:
        name = "ERROR";
        break;
    case LOG_WARNING:
        name = "WARNING";
        break;
    case LOG_NOTICE:
        name = "NOTICE";
        break;
    case LOG_INFO:
        name = "INFO";
        break;
    case LOG_DEBUG:
        name = "DEBUG";
        break;
    default:
        FAULT("Invalid log level specified: %d", static_cast<int>(level));
        break;
    }
    Format("[%s] ", name);
    return *this;
}

bool
log::KSysLog::Putc(char c, void *)
{
    lastNewLine = c == '\n';
    ut::__ut_putc(c);
    return true;
}

#endif /* KERNEL */

/* Initialize stubs module. */
bool
ut::__ut_InitStubs()
{
#   ifdef KERNEL
    log::sysLog = new log::SysLog;
#   endif

    return true;
}
