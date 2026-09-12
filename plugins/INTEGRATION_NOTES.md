# Wiring this into an actual xemu checkout

I built this scaffold without a real xemu checkout in front of me (no
network access to clone it), so this is a list of what needs to happen,
not a diff. Everything below is either a single added line or a small,
easy-to-review, easy-to-revert addition — intentionally, so the "plugin,
not core" footprint stays honest and small.

## Build system

1. Add to the root `meson_options.txt`:
   ```meson
   option('retroachievements_poc', type: 'boolean', value: false,
          description: 'Build the RetroAchievements proof-of-concept plugin')
   ```

2. Add to the root `meson.build`, near where other subdirectories get
   pulled in:
   ```meson
   subdir('plugins/retroachievements')
   ```
   `plugins/retroachievements/meson.build` already no-ops itself via
   `subdir_done()` when the option is off, so this line is always safe
   to have present.

3. Wherever the main `xemu` executable's `dependencies:` list is
   declared, conditionally add `retroachievements_poc_dep`:
   ```meson
   xemu_deps = [ ... ]
   if get_option('retroachievements_poc')
     xemu_deps += retroachievements_poc_dep
   endif
   ```
   TODO(verify): I don't know the actual variable name xemu's root
   meson.build uses for this list — grep for where `sdl3_dep` or
   `curl_dep` get consumed as a concrete anchor point.

4. Fetch rcheevos source into `plugins/retroachievements/thirdparty/rcheevos/`
   (git submodule is the simplest option, matching both PCSX2's and
   RPCS3's approach — `git submodule add https://github.com/RetroAchievements/rcheevos.git plugins/retroachievements/thirdparty/rcheevos`).
   `meson.build` here only actually compiles the `src/rcheevos/*.c`
   files this PoC calls — see its comments for exactly which, and why
   the rest isn't needed yet.

## Runtime hooks (four call sites, each one line)

All four just need `#include "plugins/retroachievements/ra_plugin.h"`
added to whichever file they land in.

1. **Startup**, once, after whatever owns guest physical memory exists:
   `ra_plugin_init();`

2. **Per-frame**, in the same place xemu already does its own per-frame
   housekeeping — needs to run every emulated frame, including
   fast-forwarded ones, per rc_runtime's own requirement:
   `ra_plugin_frame();`
   TODO(verify): the actual call site. `deepwiki` situates xemu's entry
   point at `system/vl.c` with a custom SDL3 + Dear ImGui interface —
   the per-frame hook likely belongs either right after that loop's
   `qemu_main_loop()` iteration or wherever `ui/xui/` already does its
   own once-per-frame work, whichever runs reliably every emulated
   frame rather than every host-rendered frame (those differ under
   fast-forward or frame-skip).

3. **Machine reset**, wherever xemu handles a guest reset / new title
   boot: `ra_plugin_reset();`

4. **UI**, wherever `ui/xui/` draws its own debug/settings panels each
   host frame: `ra_plugin_draw_debug_ui(&show_ra_debug_window);` (with
   `show_ra_debug_window` as a bool toggled from a menu entry — not
   included here, since I don't know xui's actual menu-registration
   pattern; see `ra_ui.cc`'s own TODO on this).

## What I'd want to see before trusting this compiles

In rough order of "most likely to need fixing first":

1. `ra_memory.c`'s `cpu_physical_memory_read()` header path — this is
   the one piece of genuine QEMU-internals guessing in the whole
   scaffold.
2. `ra_savestate.c`'s `migration/vmstate.h` path and whether
   `vmstate_register(NULL, 0, ...)` is still the right call for
   singleton state in xemu's current QEMU base.
3. `meson.build`'s `curl_dep` / `imgui_dep` variable names.
4. Whatever xui's real panel/window registration convention turns out
   to be, once `ra_ui.cc` needs to look less like a bolted-on debug
   window and more like a native part of the UI.

Paste me the actual compiler errors once you drop this into a real
checkout and run `meson compile` — happy to iterate against real
errors instead of guessing further in the dark.
