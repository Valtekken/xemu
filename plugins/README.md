# xemu RetroAchievements plugin — proof of concept

## What this actually proves

There is no RetroAchievements account, login, game hash, or achievement
set for the original Xbox yet, and RA's own admin team announced on May
28, 2026 that xemu won't be added to their supported-emulator list until
a much larger share of the library is fully playable, regardless of
code readiness. So a real server round-trip can't be tested against
production right now, no matter how correct the code is.

What CAN be proven today, for real, against live Xbox guest RAM: the
actual pipeline this whole feature depends on —

```
xemu frame loop → memory read → rcheevos condition evaluation
→ unlock event → UI → savestate round-trip
```

— using `rc_runtime` (the low-level, server-independent half of the
`rcheevos` library RetroAchievements itself maintains) with a
hand-entered test condition instead of a downloaded one. You type in
any known address and value from a live game via the debug UI, and
watch a real rcheevos trigger fire against real guest memory.

## Layout

```
plugins/retroachievements/
  ra_plugin.h / .c        core: rc_runtime lifecycle, frame pump, PoC trigger
  ra_memory.h / .c        bridges rc_runtime reads to QEMU guest physical RAM
  ra_http.h / .c          async curl-based HTTP shim (for the future networked path)
  ra_savestate.h / .c     hooks rc_runtime progress into QEMU's vmstate/snapshot system
  ra_ui.cc                minimal ImGui debug window (arm/reset trigger, unlock indicator)
  future_rc_client_reference.c   NOT compiled — sketch of the eventual networked path
  meson.build              off-by-default build flag, matching RPCS3's own WIP pattern
INTEGRATION_NOTES.md      exact hook points needed in a real xemu checkout
FUTURE_RCHEEVOS_UPSTREAM.md   what has to happen in rcheevos itself, modeled on the
                           real, currently-in-progress PS3 precedent
AGENT_NOTES.md            AI-disclosure + suggested Discord-first framing
```

## What's actually verified vs. what needs checking

I don't have a real xemu checkout available (no network access to
clone it), so nothing here has been compiled. Confidence varies a lot
by file — every file's header comments say explicitly what's checked
vs. guessed, but as a summary:

**Checked against real sources, not memory:**
- Every `rc_runtime_*` function signature in `ra_plugin.c` /
  `ra_savestate.c` — taken from rcheevos' actual `include/rc_runtime.h`
  (found vendored in another project's tree), not reconstructed.
- The achievement condition syntax in `ra_plugin.c`
  (`0xH<addr>=<value>`) — taken from
  docs.retroachievements.org's condition-syntax reference.
- The overall shape (vendor rcheevos, platform-agnostic core module,
  UI adapter, savestate hook) — cross-checked against PCSX2's real,
  merged RetroAchievements integration and RPCS3's real, in-review one.

**Genuinely guessed, flagged with `TODO(verify)` at the point of use:**
- `ra_memory.c`: the exact header for `cpu_physical_memory_read()` —
  stable, real QEMU API, but the include path drifts between QEMU
  versions and I don't know xemu's exact base.
- `ra_savestate.c`: `migration/vmstate.h`'s path and whether
  `vmstate_register(NULL, 0, ...)` is the current idiom in xemu's QEMU
  base.
- `meson.build`: `curl_dep` / `imgui_dep` variable names, and where the
  main xemu target's dependency list actually lives.
- `ra_ui.cc`: whether raw `ImGui::Begin()` is appropriate or whether
  `ui/xui/` has its own panel-registration convention to follow instead.

None of this is hidden in the code — grep for `TODO(verify)` to find
every instance. Per `AGENT_NOTES.md` (and xemu's own `AGENTS.md`), this
needs a real compile pass and a human reading through those spots
before it's anything more than a starting point.

## Known unknowns / explicitly out of scope for this PoC

- **Debug-kit 128MB RAM.** `RA_XBOX_RAM_SIZE` assumes a retail 64MB
  console.
- **Game hashing / identification.** Not attempted — rcheevos has no
  Xbox hashing implementation yet regardless (see
  `FUTURE_RCHEEVOS_UPSTREAM.md`).
- **Login / accounts / hardcore mode.** Not attempted — no server to
  log into for this console yet.
- **`ra_http.c`'s threading model** is intentionally simple (one thread
  per request, no pooling) — fine for a PoC, worth hardening before it
  carries real traffic.
- **Achievement-set development tooling** (`rc_client_raintegration`) —
  a separate integration surface from what players need. Not attempted.

## Where to go next

Read `INTEGRATION_NOTES.md` for the specific hook points into a real
xemu checkout, `FUTURE_RCHEEVOS_UPSTREAM.md` for what would need to
happen in rcheevos itself before any of this can reach real players,
and `AGENT_NOTES.md` before opening anything on Discord or as a PR.
