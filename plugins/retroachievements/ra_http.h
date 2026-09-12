/*
 * Minimal async HTTP shim over libcurl, for the future rc_client
 * server_call callback. Not exercised by the PoC trigger path (which
 * needs no network), but real and buildable — xemu already vendors
 * curl (subprojects/curl.wrap), so this adds glue, not a new
 * dependency.
 */
#ifndef XEMU_RA_HTTP_H
#define XEMU_RA_HTTP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ra_http_callback_t)(int http_status, const char *body, size_t body_len,
                                    const char *error, void *userdata);

void ra_http_init(void);
void ra_http_shutdown(void);

/* Fire-and-forget: spawns a detached worker thread that performs the
 * request with libcurl and pushes the result onto a completion
 * queue. The callback does NOT run on the calling thread — it runs
 * later, from ra_http_pump(), on whichever thread calls that. */
void ra_http_get(const char *url, ra_http_callback_t cb, void *userdata);
void ra_http_post(const char *url, const char *post_data, ra_http_callback_t cb, void *userdata);

/* Call once per emulated frame (already wired into ra_plugin_frame).
 * Drains completed requests and invokes their callbacks. Every
 * rc_client callback must run on the main thread — rc_client itself
 * is not thread-safe — so this is the only place those callbacks
 * should ever fire from. */
void ra_http_pump(void);

#ifdef __cplusplus
}
#endif

#endif /* XEMU_RA_HTTP_H */
