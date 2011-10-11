/*
 * /phoenix/kernel/sys/efi.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** Machine independent definitions for EFI (Extensible Firmware Interface)
 * functionality. Implemented according to UEFI specification rev2.3.
 */

#ifndef EFI_H_
#define EFI_H_

/** This namespace contains definitions for EFI (Extensible Firmware Interface)
 * functionality.
 */
namespace efi
{

/** EFI system table contains pointers to the runtime services and hardware
 * configuration tables.
 */
class EfiSystemTable {
public:
    /** Construct object from identity-map pointer to the system table.
     * @param ptr Identity-map pointer (treated as physical address).
     */
    EfiSystemTable(void *ptr) { _sysTable = ptr; }

private:
    vm::Paddr _sysTable;
};

} /* namespace efi */

#endif /* EFI_H_ */
