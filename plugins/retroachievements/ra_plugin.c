/*
 * xemu RetroAchievements plugin — core (proof of concept)
 *
 * Uses rc_runtime directly (NOT rc_client). rc_client wraps
 * rc_runtime with login, game hashing, and HTTP session management —
 * none of which can be meaningfully tested yet, since RetroAchievements
 * doesn't support the original Xbox as a console. rc_runtime is the
 * part of rcheevos that doesn't know or care about any of that: give
 * it a memory-read callback and a condition string, and it will
 * evaluate that condition against live memory every frame. That's
 * the actual thing this PoC needs to prove works.
 *
 * Exact rc_runtime signatures below were taken directly from
 * rcheevos' real include/rc_runtime.h (verified against a vendored
 * copy in another project's tree), not reconstructed from memory —
 * see README.md for how confident to be about which parts of this
 * scaffold.
 */
#include "ra_plugin.h"
#include "ra_memory.h"
#include "ra_http.h"
#include "ra_savestate.h"

#include "rc_runtime.h"
#include "rc_error.h"

#include <stdio.h>
#include <string.h>

static rc_runtime_t g_ra_runtime;
static bool g_ra_initialized = false;
static bool g_ra_poc_triggered = false;

/* id=1 is reserved for the single hand-armed PoC trigger. A real
 * rc_client-based implementation would use RA's actual achievement
 * IDs here instead. */
#define RA_POC_ACHIEVEMENT_ID 1u

static void ra_event_handler(const rc_runtime_event_t *event)
{
    if (event->id != RA_POC_ACHIEVEMENT_ID) {
        return;
    }

    switch (event->type) {
    case RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED:
        g_ra_poc_triggered = true;
        fprintf(stderr, "[retroachievements-poc] trigger fired (id=%u)\n", event->id);
        break;
    case RC_RUNTIME_EVENT_ACHIEVEMENT_ACTIVATED:
    case RC_RUNTIME_EVENT_ACHIEVEMENT_RESET:
        g_ra_poc_triggered = false;
        break;
    default:
        break;
    }
}

void ra_plugin_init(void)
{
    if (g_ra_initialized) {
        return;
    }
    rc_runtime_init(&g_ra_runtime);
    ra_http_init();
    ra_savestate_register();
    g_ra_initialized = true;
    fprintf(stderr, "[retroachievements-poc] initialized\n");
}

void ra_plugin_shutdown(void)
{
    if (!g_ra_initialized) {
        return;
    }
    rc_runtime_destroy(&g_ra_runtime);
    ra_http_shutdown();
    g_ra_initialized = false;
}

void ra_plugin_reset(void)
{
    if (g_ra_initialized) {
        rc_runtime_reset(&g_ra_runtime);
        g_ra_poc_triggered = false;
    }
}

void ra_plugin_frame(void)
{
    if (!g_ra_initialized) {
        return;
    }

    /* unused_L is a Lua interpreter parameter left over from an old
     * scripting integration; every current rcheevos consumer passes
     * NULL. See rc_runtime.h. */
    rc_runtime_do_frame(&g_ra_runtime, ra_event_handler, ra_peek_guest_memory, NULL, NULL);

    /* No-op until something actually calls ra_http_get/post (i.e.
     * until a future rc_client path is wired in — see
     * future_rc_client_reference.c). Kept here so the real xemu
     * frame-loop hook only needs adding once, ever. */
    ra_http_pump();
}

void ra_plugin_set_poc_trigger(uint32_t address, uint8_t is_byte, int32_t value)
{
    if (!g_ra_initialized) {
        return;
    }

    rc_runtime_deactivate_achievement(&g_ra_runtime, RA_POC_ACHIEVEMENT_ID);
    g_ra_poc_triggered = false;

    /* Condition syntax per docs.retroachievements.org/developer-docs/condition-syntax.html:
     *   0xH<addr> = 8-bit read, 0x<space><addr> = 16-bit read.
     * Example produced: "0xH001234=10" */
    char trigger[64];
    snprintf(trigger, sizeof(trigger), "0x%c%06X=%d",
             is_byte ? 'H' : ' ', address, value);

    int result = rc_runtime_activate_achievement(&g_ra_runtime, RA_POC_ACHIEVEMENT_ID,
                                                   trigger, NULL, 0);
    if (result != RC_OK) {
        fprintf(stderr, "[retroachievements-poc] failed to activate trigger '%s': %s\n",
                trigger, rc_error_str(result));
    } else {
        fprintf(stderr, "[retroachievements-poc] armed trigger '%s'\n", trigger);
    }
}

bool ra_plugin_poc_triggered(void)
{
    return g_ra_poc_triggered;
}

void ra_plugin_poc_reset(void)
{
    if (!g_ra_initialized) {
        return;
    }
    rc_runtime_deactivate_achievement(&g_ra_runtime, RA_POC_ACHIEVEMENT_ID);
    g_ra_poc_triggered = false;
}

rc_runtime_t *ra_plugin_get_runtime(void)
{
    return &g_ra_runtime;
}
