/*
 * NOT COMPILED. Deliberately excluded from meson.build's source list
 * (see the comment there). This is a reference sketch, not working
 * code, for the day RetroAchievements actually opens the original
 * Xbox as a supported console — see FUTURE_RCHEEVOS_UPSTREAM.md for
 * what has to happen upstream in rcheevos before this can do
 * anything real (console ID, memory map, hashing).
 *
 * Adapted from RetroAchievements' own documented rc_client
 * integration pattern (github.com/RetroAchievements/rcheevos/wiki/
 * rc_client-integration), swapped over to this plugin's existing
 * ra_memory.c / ra_http.c instead of writing new glue.
 *
 * Note: this file, like the rest of this scaffold, was drafted with
 * AI assistance and has not been compiled or tested — see
 * AGENT_NOTES.md. It exists to show the target shape, not to be
 * merged as-is.
 */
#if 0 /* excluded from the build on purpose — see header comment */

#include "ra_plugin.h"
#include "ra_http.h"

#include "rc_client.h"

#include <stdio.h>
#include <string.h>

static rc_client_t *g_client = NULL;

/* rc_client's read_memory callback is buffer-fill style — a
 * different shape than rc_runtime_peek_t used by the PoC path (see
 * ra_memory.h). Reuses the same underlying guest-memory access, just
 * adapted to this signature. */
static uint32_t future_read_memory(uint32_t address, uint8_t *buffer,
                                    uint32_t num_bytes, rc_client_t *client)
{
    (void)client;
    if ((uint64_t)address + num_bytes > RA_XBOX_RAM_SIZE) {
        return 0;
    }
    cpu_physical_memory_read((hwaddr)address, buffer, (hwaddr)num_bytes);
    return num_bytes;
}

static void future_http_done(int status, const char *body, size_t body_len,
                              const char *error, void *userdata)
{
    rc_api_server_response_t response;
    memset(&response, 0, sizeof(response));
    response.body = body;
    response.body_length = body_len;
    response.http_status_code = status;

    rc_client_server_callback_t callback = (rc_client_server_callback_t)userdata;
    /* TODO(verify): rc_client_server_call_t's exact callback/userdata
     * shape against the real rc_client.h — this is sketched from the
     * documented pattern, not compiled against the header. */
    (void)callback;
    (void)error;
}

static void future_server_call(const rc_api_request_t *request,
                                rc_client_server_callback_t callback,
                                void *callback_data, rc_client_t *client)
{
    (void)client;
    if (request->post_data) {
        ra_http_post(request->url, request->post_data, future_http_done, callback_data);
    } else {
        ra_http_get(request->url, future_http_done, callback_data);
    }
    (void)callback;
}

static void future_event_handler(const rc_client_event_t *event, rc_client_t *client)
{
    (void)client;
    switch (event->type) {
    case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
        fprintf(stderr, "[retroachievements] unlocked: %s\n",
                event->achievement->title);
        break;
    default:
        break;
    }
}

void future_rc_client_init(void)
{
    g_client = rc_client_create(future_read_memory, future_server_call);
    rc_client_set_event_handler(g_client, future_event_handler);

    /* For achievement developers, not just players — separate from
     * rc_client itself. See rc_client_begin_load_raintegration in
     * the rcheevos wiki. Out of scope for this PoC. */
    /* rc_client_begin_load_raintegration(g_client, ...); */
}

#endif /* 0 */
