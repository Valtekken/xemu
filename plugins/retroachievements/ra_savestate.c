/*
 * TODO(verify against your local xemu checkout) — this file is the
 * second-biggest "verify before trusting" spot, alongside
 * ra_memory.c:
 *
 *  - "qemu/osdep.h" must be the first include, per QEMU convention.
 *  - VMStateDescription / vmstate_register() / VMSTATE_* macros live
 *    in "migration/vmstate.h" as of recent QEMU — confirm the path
 *    hasn't moved in xemu's current QEMU base.
 *  - vmstate_register(NULL, 0, &desc, opaque) with a NULL owner
 *    registers singleton, non-device migration state. This pattern
 *    exists elsewhere in QEMU core for state that isn't tied to a
 *    specific QOM device, but double-check it's still the preferred
 *    spelling in the QEMU version xemu is currently based on.
 *
 * Uses a fixed-size VMSTATE_UINT8_ARRAY rather than a dynamically
 * sized VMSTATE_VBUFFER, on purpose: VMSTATE_UINT8_ARRAY is a very
 * standard, unambiguous macro, which matters more for a first pass
 * than the wasted space. rc_runtime_progress_size() is checked
 * against the fixed bound at save time and asserts loudly rather
 * than silently truncating if it's ever exceeded. Revisit with a
 * real size budget once real Xbox achievement sets exist to measure
 * against — see FUTURE_RCHEEVOS_UPSTREAM.md.
 */
#include "qemu/osdep.h"
#include "migration/vmstate.h"

#include "rc_runtime.h"
#include "ra_plugin.h"

#define RA_SAVESTATE_MAX_BYTES (64 * 1024)

typedef struct {
    uint32_t size;
    uint8_t data[RA_SAVESTATE_MAX_BYTES];
} ra_savestate_blob_t;

static ra_savestate_blob_t g_ra_savestate_blob;

static int ra_vmstate_pre_save(void *opaque)
{
    ra_savestate_blob_t *blob = (ra_savestate_blob_t *)opaque;
    rc_runtime_t *runtime = ra_plugin_get_runtime();

    uint32_t needed = rc_runtime_progress_size(runtime, NULL);
    /* Fail loudly rather than silently truncate achievement progress. */
    g_assert(needed <= RA_SAVESTATE_MAX_BYTES);

    memset(blob->data, 0, sizeof(blob->data));
    rc_runtime_serialize_progress_sized(blob->data, RA_SAVESTATE_MAX_BYTES, runtime, NULL);
    blob->size = needed;
    return 0;
}

static int ra_vmstate_post_load(void *opaque, int version_id)
{
    (void)version_id;
    ra_savestate_blob_t *blob = (ra_savestate_blob_t *)opaque;
    rc_runtime_t *runtime = ra_plugin_get_runtime();

    if (blob->size > 0 && blob->size <= RA_SAVESTATE_MAX_BYTES) {
        rc_runtime_deserialize_progress_sized(runtime, blob->data, blob->size, NULL);
    }
    return 0;
}

static const VMStateDescription vmstate_ra_progress = {
    .name = "retroachievements_poc",
    .version_id = 1,
    .minimum_version_id = 1,
    .pre_save = ra_vmstate_pre_save,
    .post_load = ra_vmstate_post_load,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(size, ra_savestate_blob_t),
        VMSTATE_UINT8_ARRAY(data, ra_savestate_blob_t, RA_SAVESTATE_MAX_BYTES),
        VMSTATE_END_OF_LIST()
    }
};

void ra_savestate_register(void)
{
    vmstate_register(NULL, 0, &vmstate_ra_progress, &g_ra_savestate_blob);
}
