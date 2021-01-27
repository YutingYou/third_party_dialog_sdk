#
# Copyright (C) 2018-2020 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#

MTB_SIZE = 0x2000
MTB_ENTRY_SIZE = 0x08
ENTRY_ADRR_MASK = 0xFFFFFFFE
ENTRY_AS_BIT = 0x00000001

MTB_REGISTER_ADDRESS = 'ADDRESS'
MTB_REGISTER_FIELDS = 'FIELDS'

MTB_POSITION_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043000,
    MTB_REGISTER_FIELDS: {
        'WRAP': 0x00000004,
        'POINTER': 0xFFFFFFF8
    }
}

MTB_MASTER_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043004,
    MTB_REGISTER_FIELDS: {
        'MASK': 0x0000001F,
        'TSTARTEN': 0x00000020,
        'TSTOPEN': 0x00000040,
        'RAMPRIV': 0x00000100,
        'HALTREQ': 0x00000200,
        'NSEN': 0x40000000,
        'EN': 0x80000000,
    }
}

MTB_FLOW_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043008,
    MTB_REGISTER_FIELDS: {
        'AUTOSTOP': 0x00000001,
        'AUTOHALT': 0x00000002,
        'WATERMARK': 0xFFFFFFF8,
    }
}

MTB_BASE_REG = {
    MTB_REGISTER_ADDRESS: 0xE004300C,
    MTB_REGISTER_FIELDS: {
        'BASE': 0xFFFFFFE0,
    }
}

MTB_TSTART_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043010,
    MTB_REGISTER_FIELDS: {
        'CMPMATCH[1:0]': 0x00000003,
        'CMPMATCH[2:3]': 0x0000000C,
    }
}

MTB_TSTOP_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043014,
    MTB_REGISTER_FIELDS: {
        'CMPMATCH[1:0]': 0x00000003,
        'CMPMATCH[2:3]': 0x0000000C,
    }
}

MTB_SECURE_REG = {
    MTB_REGISTER_ADDRESS: 0xE0043018,
    MTB_REGISTER_FIELDS: {
        'THRSEN': 0x00000001,
        'NS': 0x00000002,
        'THRESHOLD': 0xFFFFFFE0,
    }
}

MTB_REGISTERS = {
    'MTB_POSITION': MTB_POSITION_REG,
    'MTB_MASTER': MTB_MASTER_REG,
    'MTB_FLOW': MTB_FLOW_REG,
    'MTB_BASE': MTB_BASE_REG,
    'MTB_TSTART': MTB_TSTART_REG,
    'MTB_TSTOP': MTB_TSTOP_REG,
    'MTB_SECURE': MTB_SECURE_REG,
}


def read_word(addr):
    return int(gdb.execute("x /1xw 0x{:08x}".format(addr), to_string=True).split(':', 1)[1].strip(), 0)


def read_mtb_entry(entry_addr):
    # Read even word address
    source = read_word(entry_addr)
    # Read odd word address
    destination = read_word(entry_addr + MTB_ENTRY_SIZE / 2)

    return (source & ENTRY_ADRR_MASK, source & ENTRY_AS_BIT),\
           (destination & ENTRY_ADRR_MASK, destination & ENTRY_AS_BIT)


def print_line_info(address):
    sym_line = gdb.find_pc_line(address)
    if sym_line.symtab:
        gdb.write('\t\t{}:{}\n'.format(sym_line.symtab.filename, sym_line.line))
        gdb.write('\t\t\t{}\n'.format(gdb.execute("x/i 0x{:08x}".format(address), to_string=True).strip('\n')))
    else:
        gdb.write('\t\tFailed to find pc_line\n')


def mtb_gen(start, size, pos_value):
    pos_addr = start + (pos_value & MTB_POSITION_REG[MTB_REGISTER_FIELDS]['POINTER'])

    if pos_value & MTB_POSITION_REG[MTB_REGISTER_FIELDS]['WRAP']:
        mtb_addresses = range(pos_addr, start + size, MTB_ENTRY_SIZE)
    else:
        mtb_addresses = []

    mtb_addresses.extend(range(start, pos_addr, MTB_ENTRY_SIZE))
    mtb_addresses.reverse()

    for addr in mtb_addresses:
        yield addr

def is_mtb_en():
    if read_word(MTB_MASTER_REG[MTB_REGISTER_ADDRESS]) & MTB_MASTER_REG[MTB_REGISTER_FIELDS]['EN']:
        return True
    else:
        return False

class MTBReadRegisters(gdb.Command):
    def __init__(self):
        super(MTBReadRegisters, self).__init__("mtb-reg", gdb.COMMAND_STATUS, gdb.COMPLETE_NONE)

    # noinspection PyUnusedLocal, PyMethodMayBeStatic
    def invoke(self, arg, from_tty):
        if is_mtb_en():
            for name, reg in MTB_REGISTERS.iteritems():
                reg_value = read_word(reg[MTB_REGISTER_ADDRESS])
                gdb.write('{:16}'.format(name) + 'hex: 0x{:08x} '.format(reg_value) + ' bin: {0:032b}\n'.format(reg_value))
        else:
            gdb.write("\t\tMTB not enabled\n")


MTBReadRegisters()


class MTBDownloader(gdb.Command):
    def __init__(self):
        super(MTBDownloader, self).__init__("mtb-fetch", gdb.COMMAND_STATUS, gdb.COMPLETE_NONE)

    # noinspection PyUnusedLocal, PyMethodMayBeStatic
    def invoke(self, arg, from_tty):
        if is_mtb_en():
            # Read position register
            pos_value = read_word(MTB_POSITION_REG[MTB_REGISTER_ADDRESS])
            start = read_word(MTB_BASE_REG[MTB_REGISTER_ADDRESS]) & MTB_BASE_REG[MTB_REGISTER_FIELDS]['BASE']

            for addr in mtb_gen(start, MTB_SIZE, pos_value):
                ((source, a_bit), (dest, s_bit)) = read_mtb_entry(addr)
                gdb.write('{:*^80}\n'.format('0x{:08x}'.format(addr)))
                gdb.write('\tSOURCE ADDRESS = 0x{:08x} A_BIT = 0x{:01x}\n'.format(source, a_bit))
                print_line_info(source)
                gdb.write('\tDEST ADDRESS   = 0x{:08x} S_BIT = 0x{:01x}\n'.format(dest, s_bit))
                print_line_info(dest)
                gdb.write('\n')


MTBDownloader()
