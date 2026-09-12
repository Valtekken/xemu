/*
 * Bridges rc_runtime's memory reads to QEMU guest physical RAM.
 */
#ifndef XEMU_RA_MEMORY_H
#define XEMU_RA_MEMORY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The original Xbox has 64 MiB of unified system RAM (128 MiB on
 * debug kits — NOT handled here, see README "Known unknowns").
 * RetroAchievements addresses for a system conventionally map 1:1
 * onto the start of main RAM, and Xbox RAM starts at guest physical
 * address 0 and is fully preallocated and contiguous (no lazy paging
 * the way e.g. PS3's user memory pools work — see README for why
 * that matters), so this should just be a direct, bounds-checked
 * passthrough.
 *
 * TODO(verify): confirm 0x04000000 against xemu's actual hw/xbox RAM
 * size constant rather than trusting this comment.
 */
#define RA_XBOX_RAM_SIZE 0x04000000u /* 64 MiB */

/* Matches rc_runtime_peek_t exactly (include/rc_runtime.h):
 *   typedef uint32_t (*rc_runtime_peek_t)(uint32_t address,
 *                                          uint32_t num_bytes,
 *                                          void *ud);
 * Reads num_bytes (1-4) starting at address and returns them
 * combined little-endian, per rc_runtime's documented contract — this
 * is a peek-and-return-the-value callback, NOT a buffer-fill one
 * (that's rc_client's read_memory, which has a different signature —
 * see future_rc_client_reference.c).
 *
 * Returns 0 for any read that falls outside RA_XBOX_RAM_SIZE. Note
 * for later: in rc_client (not used by this PoC), a memory read that
 * comes back short can disable the achievement that requested it
 * unless the read was part of a pointer chain being resolved — see
 * the RPCS3/PS3 PR discussion linked in FUTURE_RCHEEVOS_UPSTREAM.md.
 * TODO(verify): confirm whether rc_runtime's own condition/memref
 * handling has equivalent short-read semantics before relying on
 * this function's out-of-range behavior for anything beyond this
 * PoC's single hand-armed trigger, since I haven't traced that path
 * in rc_runtime's .c files myself.
 */
uint32_t ra_peek_guest_memory(uint32_t address, uint32_t num_bytes, void *ud);

#ifdef __cplusplus
}
#endif

#endif /* XEMU_RA_MEMORY_H */
