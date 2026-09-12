#ifndef XEMU_RA_SAVESTATE_H
#define XEMU_RA_SAVESTATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Registers rc_runtime's progress with QEMU's own savevm/loadvm
 * snapshot machinery, so achievement progress rides along in
 * existing xemu snapshots automatically instead of needing a second,
 * bespoke save format (contrast with non-QEMU-based emulators, which
 * generally have had to invent one). Call once, from ra_plugin_init(). */
void ra_savestate_register(void);

#ifdef __cplusplus
}
#endif

#endif /* XEMU_RA_SAVESTATE_H */
