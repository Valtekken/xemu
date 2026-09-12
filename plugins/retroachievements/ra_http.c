/*
 * Deliberately simple: one detached pthread per request doing a
 * blocking curl_easy_perform(). Completed results are pushed onto a
 * mutex-protected singly linked list; ra_http_pump() (called once
 * per frame from the main thread) drains it and invokes callbacks
 * there. No connection pooling, no curl_multi, no cancellation.
 * Fine for a handful of concurrent login/achievement calls; revisit
 * before this carries real traffic.
 *
 * TODO(verify): confirm xemu's vendored curl target/link name to use
 * as this file's meson dependency (see meson.build in this
 * directory) — not something visible from outside the xemu tree.
 */
#include "ra_http.h"

#include <curl/curl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct ra_http_result {
    struct ra_http_result *next;
    int status;
    char *body;
    size_t body_len;
    char *error;
    ra_http_callback_t cb;
    void *userdata;
} ra_http_result_t;

typedef struct {
    char *url;
    char *post_data; /* NULL => GET */
    ra_http_callback_t cb;
    void *userdata;
} ra_http_request_t;

static pthread_mutex_t g_queue_lock = PTHREAD_MUTEX_INITIALIZER;
static ra_http_result_t *g_completed = NULL;

static size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total = size * nmemb;
    ra_http_result_t *r = (ra_http_result_t *)userdata;

    char *grown = realloc(r->body, r->body_len + total + 1);
    if (!grown) {
        return 0; /* signals curl to abort the transfer */
    }
    r->body = grown;
    memcpy(r->body + r->body_len, ptr, total);
    r->body_len += total;
    r->body[r->body_len] = '\0';
    return total;
}

static void *worker_main(void *arg)
{
    ra_http_request_t *req = (ra_http_request_t *)arg;
    ra_http_result_t *result = calloc(1, sizeof(*result));
    result->cb = req->cb;
    result->userdata = req->userdata;

    CURL *curl = curl_easy_init();
    if (!curl) {
        result->status = 0;
        result->error = strdup("curl_easy_init failed");
    } else {
        curl_easy_setopt(curl, CURLOPT_URL, req->url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "xemu-retroachievements-poc/0.1");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        if (req->post_data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->post_data);
        }

        CURLcode rc = curl_easy_perform(curl);
        if (rc != CURLE_OK) {
            result->status = 0;
            result->error = strdup(curl_easy_strerror(rc));
        } else {
            long code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
            result->status = (int)code;
        }
        curl_easy_cleanup(curl);
    }

    pthread_mutex_lock(&g_queue_lock);
    result->next = g_completed;
    g_completed = result;
    pthread_mutex_unlock(&g_queue_lock);

    free(req->url);
    free(req->post_data);
    free(req);
    return NULL;
}

static void dispatch(const char *url, const char *post_data, ra_http_callback_t cb, void *userdata)
{
    ra_http_request_t *req = calloc(1, sizeof(*req));
    req->url = strdup(url);
    req->post_data = post_data ? strdup(post_data) : NULL;
    req->cb = cb;
    req->userdata = userdata;

    pthread_t thread;
    pthread_create(&thread, NULL, worker_main, req);
    pthread_detach(thread);
}

void ra_http_init(void)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void ra_http_shutdown(void)
{
    curl_global_cleanup();
}

void ra_http_get(const char *url, ra_http_callback_t cb, void *userdata)
{
    dispatch(url, NULL, cb, userdata);
}

void ra_http_post(const char *url, const char *post_data, ra_http_callback_t cb, void *userdata)
{
    dispatch(url, post_data, cb, userdata);
}

void ra_http_pump(void)
{
    pthread_mutex_lock(&g_queue_lock);
    ra_http_result_t *batch = g_completed;
    g_completed = NULL;
    pthread_mutex_unlock(&g_queue_lock);

    while (batch) {
        ra_http_result_t *next = batch->next;
        batch->cb(batch->status, batch->body, batch->body_len, batch->error, batch->userdata);
        free(batch->body);
        free(batch->error);
        free(batch);
        batch = next;
    }
}
