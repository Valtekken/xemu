/*
 * xemu RetroAchievements plugin — public API (proof of concept)
 *
 * Scope of this PoC, deliberately narrow:
 *   There is no RetroAchievements account, login, game hash, or
 *   achievement set for the original Xbox yet (see ../../README.md).
 *   So this does NOT talk to retroachievements.org. What it proves,
 *   for real, against live Xbox guest RAM, is the actual pipeline
 *   the whole feature depends on:
 *
 *     xemu frame loop -> memory read -> rcheevos condition
 *     evaluation -> unlock event -> UI -> savestate round-trip
 *
 *   using rc_runtime (the low-level, server-independent half of
 *   rcheevos) with a hand-entered test condition instead of a
 *   downloaded one. See ra_plugin_set_poc_trigger().
 *
 * AGENT NOTE: this file was drafted with AI assistance (see
 * AGENT_NOTES.md at the repo root of this scaffold). It has not been
 * compiled against a real xemu tree — see README.md "What's actually
 * verified vs. what needs checking" before relying on it.
 */
#ifndef XEMU_RA_PLUGIN_H
#define XEMU_RA_PLUGIN_H

#include <stdint.h>
#include <stdbool.h>

#include "rc_runtime.h" /* rc_runtime_t — see ra_plugin_get_runtime() */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Lifecycle --------------------------------------------------
 * See INTEGRATION_NOTES.md for exactly where these should be called
 * from in xemu's real source (this scaffold can't see that source).
 */

/* Call once, after xemu's own subsystems (in particular whatever
 * owns guest physical memory) are up. Safe to call unconditionally;
 * a future XEMU_RA_ENABLED build flag can just no-op this out. */
void ra_plugin_init(void);

void ra_plugin_shutdown(void);

/* Call once per emulated frame — the same place xemu already does
 * its own per-frame housekeeping. Must be called even on frames that
 * get skipped/fast-forwarded, per rcheevos' own requirement that
 * rc_runtime_do_frame() runs every emulated frame. */
void ra_plugin_frame(void);

/* Call on guest reset / new title boot. */
void ra_plugin_reset(void);

/* ---- Proof-of-concept local trigger ------------------------------
 * No server round-trip. address is a guest-physical offset into Xbox
 * RAM (0..0x03FFFFFF); is_byte selects an 8-bit vs 16-bit read. Lets
 * you arm a real rcheevos condition against ANY known address in ANY
 * game you're running today, from xemu's own debug UI, and watch it
 * fire for real.
 */
void ra_plugin_set_poc_trigger(uint32_t address, uint8_t is_byte, int32_t value);
bool ra_plugin_poc_triggered(void);
void ra_plugin_poc_reset(void);

/* ---- Savestate hook -----------------------------------------------
 * Internal accessor so ra_savestate.c can serialize/deserialize the
 * same runtime instance via QEMU's vmstate machinery. Not something a
 * UI caller should need. */
rc_runtime_t *ra_plugin_get_runtime(void);

/* ---- Debug UI -------------------------------------------------------
 * Draws the plugin's ImGui debug window. See ra_ui.cc. Not styled to
 * match xemu's real ui/xui/ look — a self-contained proof, not a
 * finished feature. */
void ra_plugin_draw_debug_ui(bool *p_open);

#ifdef __cplusplus
}
#endif

#endif /* XEMU_RA_PLUGIN_H */
