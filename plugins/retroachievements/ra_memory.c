/*
 * TODO(verify against your local xemu checkout) — this is the single
 * biggest "I'm confident about the API, not about the exact xemu
 * wiring" spot in this scaffold:
 *
 *  - QEMU convention requires "qemu/osdep.h" to be the FIRST include
 *    in any translation unit that touches QEMU internals.
 *  - cpu_physical_memory_read(hwaddr addr, void *buf, hwaddr len) is
 *    long-standing, stable QEMU API for reading guest physical RAM —
 *    the same mechanism the QEMU monitor/gdbstub use. Depending on
 *    xemu's exact QEMU base version it's declared in
 *    "exec/cpu-common.h" or "system/memory.h" — check both, and
 *    check whether xemu already has a project-local helper for this
 *    (many QEMU forks add a thin wrapper) that should be reused
 *    instead of calling the QEMU-core symbol directly.
 */
#include "qemu/osdep.h"
#include "exec/cpu-common.h"

#include "ra_memory.h"

uint32_t ra_peek_guest_memory(uint32_t address, uint32_t num_bytes, void *ud)
{
    (void)ud;

    if (num_bytes == 0 || num_bytes > 4) {
        return 0;
    }
    if ((uint64_t)address + num_bytes > RA_XBOX_RAM_SIZE) {
        return 0;
    }

    uint8_t bytes[4] = {0, 0, 0, 0};
    cpu_physical_memory_read((hwaddr)address, bytes, (hwaddr)num_bytes);

    uint32_t value = 0;
    for (uint32_t i = 0; i < num_bytes; i++) {
        value |= ((uint32_t)bytes[i]) << (8 * i);
    }
    return value;
}
