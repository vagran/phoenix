/*
 * /phoenix/kernel/kern/debug.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file debug.cpp
 * Kernel debugging facilities implementation.
 */

#include <sys.h>

/** Serial console driver implementation in the kernel for debugging purposes. */
class DbgSerialPort {
public:
    enum UART {
        /* The offsets of UART registers.  */
        UART_TX =       0,
        UART_RX =       0,
        UART_DLL =      0,
        UART_IER =      1,
        UART_DLH =      1,
        UART_IIR =      2,
        UART_FCR =      2,
        UART_LCR =      3,
        UART_MCR =      4,
        UART_LSR =      5,
        UART_MSR =      6,
        UART_SR =       7,

        /* For LSR bits.  */
        UART_DATA_READY =           0x01,
        UART_EMPTY_TRANSMITTER =    0x20,

        /* The type of parity.  */
        UART_NO_PARITY =            0x00,
        UART_ODD_PARITY =           0x08,
        UART_EVEN_PARITY =          0x18,

        /* The type of word length.  */
        UART_5BITS_WORD =           0x00,
        UART_6BITS_WORD =           0x01,
        UART_7BITS_WORD =           0x02,
        UART_8BITS_WORD =           0x03,

        /* The type of the length of stop bit.  */
        UART_1_STOP_BIT =           0x00,
        UART_2_STOP_BITS =          0x04,

        /* the switch of DLAB.  */
        UART_DLAB =                 0x80,

        /* Enable the FIFO.  */
        UART_ENABLE_FIFO =          0xC7,

        /* Turn on DTR, RTS, and OUT2.  */
        UART_ENABLE_MODEM =         0x0B,
    };
private:
    enum {
        BASE_SPEED =        115200,
        DEFAULT_SPEED =     BASE_SPEED,
    };

    u16 iobase;
    u16 divisor;
    SpinLock lock;

    void Initialize();
    void SetSpeed(int speed);
public:
    DbgSerialPort();
    /** Get character from the port.
     *
     * @param c Pointer to location where to store the character.
     * @param arg Unused argument for compatibility with upper layers.
     * @return @a true if character read, @a false otherwise.
     */
    bool Getc(u8 *c, void *arg = 0);
    /** Output character to the port.
     *
     * @param c Character to output
     * @param arg Unused argument for compatibility with upper layers.
     * @return @a true if character written, @a false otherwise.
     */
    bool Putc(u8 c, void *arg = 0);
};

DbgSerialPort::DbgSerialPort()
{
    iobase = 0x3f8;
    SetSpeed(DEFAULT_SPEED);
    Initialize();
}

void
DbgSerialPort::SetSpeed(int speed)
{
    static int speedTab[] = {
        2400,
        4800,
        9600,
        19200,
        38400,
        57600,
        115200
    };

    for (auto s: speedTab) {
        if (speed == s) {
            divisor = BASE_SPEED / speed;
            return;
        }
    }
    divisor = BASE_SPEED / speedTab[0];
}

void
DbgSerialPort::Initialize()
{
    /* Turn off the interrupt */
    cpu::outb(iobase + UART_IER, 0);

    /* Set DLAB.  */
    cpu::outb(iobase + UART_LCR, UART_DLAB);

    /* Set the baud rate */
    cpu::outb(iobase + UART_DLL, divisor & 0xFF);
    cpu::outb(iobase + UART_DLH, divisor >> 8);

    /* Set the line status */
    u8 status = UART_8BITS_WORD | UART_NO_PARITY | UART_1_STOP_BIT;
    cpu::outb(iobase + UART_LCR, status);

    /* Enable the FIFO */
    cpu::outb(iobase + UART_FCR, UART_ENABLE_FIFO);

    /* Turn on DTR, RTS, and OUT2 */
    cpu::outb(iobase + UART_MCR, UART_ENABLE_MODEM);

    /* Drain the input buffer */
    u8 c;
    while (Getc(&c));

    /* Reset console attributes. */
    Putc(0x1b);
    Putc('[');
    Putc('m');
    /* Start new line. */
    Putc('\r');
    Putc('\n');
}

bool
DbgSerialPort::Getc(u8 *c, void *arg UNUSED)
{
    bool intr = cpu::DisableInterrupts();
    lock.Lock();
    if (cpu::inb(iobase + UART_LSR) & UART_DATA_READY) {
        *c = cpu::inb(iobase + UART_RX);
        lock.Unlock();
        if (intr) {
            cpu::EnableInterrupts();
        }
        return true;
    }
    lock.Unlock();
    if (intr) {
        cpu::EnableInterrupts();
    }
    return false;
}

bool
DbgSerialPort::Putc(u8 c, void *arg UNUSED)
{
    if (c == '\n') {
        bool status = Putc('\r');
        if (!status) {
            return status;
        }
    }
    u32 timeout = 100000;
    /* Wait until the transmitter holding register is empty */
    bool intr = cpu::DisableInterrupts();
    lock.Lock();
    while (!cpu::inb(iobase + UART_LSR) & UART_EMPTY_TRANSMITTER) {
        if (!--timeout) {
            /* There is something wrong. But what can I do? */
            lock.Unlock();
            if (intr) {
                cpu::EnableInterrupts();
            }
            return false;
        }
        cpu::pause();
    }
    cpu::outb(iobase + UART_TX, c);
    lock.Unlock();
    if (intr) {
        cpu::EnableInterrupts();
    }
    return true;
}

static DbgSerialPort dbgSerialPort;
static text_stream::OTextStream<DbgSerialPort> dbgStream(&dbgSerialPort);

class A {
public:
    void operator >> (text_stream::OTextStreamBase &s) {
        s << 'Q';
    }
};

void
__Trace(const char *file UNUSED, int line UNUSED, const char *msg UNUSED, ...)
{
    dbgSerialPort.Putc('Z');

    bool t = true;
    A a;

    dbgStream << a << '\n' << t << '\n';

    int i = -237;
    dbgStream << i << '\n';

}
