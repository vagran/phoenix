# -*- coding: utf-8 -*-
# /phoenix/tools/gdb/phoenix_gdb.py
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

import gdb
import gdb.printing

'''
This module is used as GDB extension to provide more convenient and effective
system debugging and troubleshooting technique.
'''

# Built-in types

short_t = gdb.lookup_type('short')
ushort_t = gdb.lookup_type('unsigned short')
int_t = gdb.lookup_type('int')
uint_t = gdb.lookup_type('unsigned int')
quad_t = gdb.lookup_type('long long')
uquad_t = gdb.lookup_type('unsigned long long')
byte_t = gdb.lookup_type('unsigned char')
byteptr_t = byte_t.pointer()

###############################################################################
# Internal helpers. Can be used by other modules.

def cast(value, type_name):
    '''
    Cast provided value to specified type.
    '''
    
    type = gdb.lookup_type(type_name)
    return value.cast(type)
    
def gdb_eval(expression, cast_type = None):
    '''
    Parse and evaluate provided expression. Optionally cast it
    to specified type.
    '''
    
    if type(expression).__name__ != 'str':
        expression = str(expression)
    value = gdb.parse_and_eval(expression)
    
    if cast_type is not None:
        if type(cast_type).__name__ != 'Type':
            cast_type = gdb.lookup_type(cast_type)
        value = value.cast(cast_type)
    
    return value

def add_pretty_printer(printer):
    printer.lookup.name = printer.name
    printer.lookup.enabled = True
    gdb.printing.register_pretty_printer(gdb.current_objfile(), printer.lookup)
    
def get_string(str_ptr, len = -1):
    '''
    Convert null terminated string in target memory to Python string.
    Optionally length limit can be specified.
    '''
    
    ptr = str_ptr.cast(byteptr_t)
    s = ''
    while len == -1 or len > 0:
        byte = ptr.dereference()
        if byte == 0:
            break
        s += chr(byte)
        ptr += 1
        if len > 0:
            len -= 1
    return s

def get_data(ptr, size):
    '''Get data from target memory in Python object'''
    
    ptr = ptr.cast(byteptr_t)
    data = ''
    
    while size > 0:
        data += chr(ptr.dereference())
        ptr += 1
        size -= 1
        
    return data

def str_enum(val, enum):
    '''
    Covert enumeration value to string. This helper function is intended for
    enumerations defined by preprocessor macros instead of 'enum' keyword. Those
    which are defined by 'enum' keyword can be directly converted to string
    without helpers. 'enum' parameter is a dictionary with enumeration values.
    '''
    
    val = int(val)
    if val in enum:
        return enum[val]
    return str(val)

def str_flags(val, flags):
    '''
    Convert integer value with flags into string representation. 'flags'
    parameter is a dictionary with all flags bits defined.
    '''
    
    val = int(val)
    
    if val == 0:
        return ' <NONE>'
    if val <= 0xff:
        count = 8
    elif val <= 0xffff:
        count = 16
    elif val <= 0xffffffff:
        count = 32
    else:
        count = 64
    
    s = ''
    for bit_pos in range(0, count):
        bit = 1 << bit_pos
        if val & bit != 0:
            if bit in flags:
                s += ' %s' % flags[bit]
            else:
                s += ' <BIT%d>' % bit_pos
    return s

def str_bitfield_flags(val, flags):
    '''Convert structure value with flags into string representation. 'flags'
    parameter is a dictionary with all flags members defined.
    '''
    s = ''
    for flag in flags.keys():
        if int(val[flag]) != 0:
            s += '%s ' % flags[flag]
    return s;

def hex_dump(address_str, size = 16):
    '''Output memory region in hex.'''
    
    if size <= 0:
        raise gdb.error('Size must be positive')
    
    address = gdb_eval(address_str, uint_t)
    start_address = address & ~0xf
    end_address = (address + size + 0xf) & ~0xf
    
    chars = ''
    
    for cur_address in range(start_address, end_address):
        
        if cur_address & 0xf == 0:
            gdb.write('0x%08x: ' % cur_address)
        elif cur_address & 0xf == 0x4 or cur_address & 0xf == 0xc:
            gdb.write(' ')
        elif cur_address & 0xf == 0x8:
            gdb.write('- ')
        
        if cur_address < int(address) or cur_address >= int(address + size):
            gdb.write('** ')
            chars += ' '
        else:
            byte = int(gdb.Value(cur_address).cast(byteptr_t).dereference())
            gdb.write('%02x ' % byte)
            if byte >= 32 and byte < 128:
                chars += chr(byte)
            else:
                chars += '.'

        if cur_address & 0xf == 0xf:
            gdb.write(' | %s\n' % chars)
            chars = ''

###############################################################################
# System-specific helpers

def parse_va(va):
    pte = (va >> 12) & 0x1ff
    pde = (va >> (12 + 9)) & 0x1ff
    pdpte = (va >> (12 + 9 + 9)) & 0x1ff
    pml4e = (va >> (12 + 9 + 9 + 9)) & 0x1ff
    print("PML4E=%d PDPTE=%d PDE=%d PTE=%d" % (pml4e, pdpte, pde, pte))
            
###############################################################################
# Module startup code which outputs some useful information

print('################### phoenix_gdb module ###################')

###############################################################################
# Custom commands

# XXX replace this by some useful command soon
class PhoenixtestCommand(gdb.Command):
    '''Test command'''

    def __init__(self):
        super(PhoenixtestCommand, self).__init__('phoenixtest', gdb.COMMAND_RUNNING,
                                                 gdb.COMPLETE_FILENAME)

    def invoke(self, arg, from_tty):
        print('phoenixtest command')

PhoenixtestCommand()

###############################################################################
# Pretty printers for Phoenix data structures

class c_vaddr_printer:
    name = 'class Vaddr'
    
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return 'Vaddr: 0x%016x' % self.val['_addr']['addr']
    
    def display_hint(self):
        return c_vaddr_printer.name
    
    @staticmethod
    def lookup(val):
        if val.type.code != gdb.TYPE_CODE_STRUCT or \
            (val.type.tag != '(anonymous namespace)::vm::Vaddr' and val.type.tag != 'vm::Vaddr'):
            return None
        return c_vaddr_printer(val)
    
add_pretty_printer(c_vaddr_printer)

class c_paddr_printer:
    name = 'class Paddr'
    
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return 'Paddr: 0x%016x' % self.val['_addr']['addr']
    
    def display_hint(self):
        return c_paddr_printer.name
    
    @staticmethod
    def lookup(val):
        if val.type.code != gdb.TYPE_CODE_STRUCT or \
            (val.type.tag != '(anonymous namespace)::vm::Paddr' and val.type.tag != 'vm::Paddr'):
            return None
        return c_paddr_printer(val)
    
add_pretty_printer(c_paddr_printer)

class t_vaddr_printer:
    name = 'vaddr_t'
    
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return 'vaddr_t: 0x%016x' % self.val
    
    def display_hint(self):
        return t_vaddr_printer.name
    
    @staticmethod
    def lookup(val):
        if val.type.code != gdb.TYPE_CODE_TYPEDEF or str(val.type) != 'vaddr_t':
            return None
        return t_vaddr_printer(val)
    
add_pretty_printer(t_vaddr_printer)

class t_paddr_printer:
    name = 'paddr_t'
    
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return 'paddr_t: 0x%016x' % self.val
    
    def display_hint(self):
        return t_paddr_printer.name
    
    @staticmethod
    def lookup(val):
        if val.type.code != gdb.TYPE_CODE_TYPEDEF or str(val.type) != 'paddr_t':
            return None
        return t_paddr_printer(val)
    
add_pretty_printer(t_paddr_printer)

class c_LatEntry_printer:
    name = 'class LatEntry'
    flags = { 'present':    'PRESENT',
              'write':      'WRITE',
              'user':       'USER',
              'writeThrough': 'WT',
              'cacheDisable': 'CD',
              'accessed':   'ACCESSED',
              'dirty':      'DIRTY',
              'pat':        'PS',
              'global':     'GLOBAL',
              'executeDisable': 'XD'
    }
    
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        e = self.val['_ptr']['entryPage'].dereference();
        s = 'vm::LatEntry: pa = 0x%016x; ' % (e['pa'] << 12)
        s += str_bitfield_flags(e, c_LatEntry_printer.flags)
        return s
    
    def display_hint(self):
        return c_LatEntry_printer.name
    
    @staticmethod
    def lookup(val):
        if val.type.code != gdb.TYPE_CODE_STRUCT or \
            (val.type.tag != '(anonymous namespace)::vm::LatEntry' and \
             val.type.tag != 'vm::LatEntry'):
            return None
        return c_LatEntry_printer(val)
    
add_pretty_printer(c_LatEntry_printer)
