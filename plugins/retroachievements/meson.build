# RetroAchievements plugin (proof of concept)
#
# TODO(verify): adjust to match how xemu's other subdirectories
# declare static libs and how they get linked into the main xemu
# target — I don't have xemu's actual root meson.build to check
# against, only its public repo listing. Modeled on the pattern
# xemu already uses for vendoring SDL3, nlohmann_json, and curl as
# meson subprojects/wraps.
#
# Mirrors RPCS3's own WIP RetroAchievements integration (see
# ../../FUTURE_RCHEEVOS_UPSTREAM.md), which gates the entire feature
# behind a compile-time flag, off by default, specifically so it
# can't affect standard builds while still living in-tree. Do the
# same here with -Dretroachievements_poc=true, matching this
# scaffold's "plugin, not core" goal.

if not get_option('retroachievements_poc')
  subdir_done()
endif

# rcheevos does not ship its own meson.build (checked: its repo root
# only has include/, src/, test/, validator/, Package.swift — no
# CMakeLists.txt or meson.build). Every consumer hand-rolls a build
# target; PCSX2 does this with a CMake target enumerating individual
# .c files rather than a wildcard glob. This list is adapted from
# PCSX2's own 3rdparty/rcheevos CMake target and trimmed to only the
# rc_runtime dependency chain this PoC actually calls — no rapi/,
# rhash/, or rc_client.c, since none of that is exercised without a
# server round-trip. TODO(verify): re-check this list against current
# rcheevos master before trusting it; rc_client.c and
# rc_client_raintegration.c in particular are newer additions that
# post-date the PCSX2 commit this list was drawn from, and aren't
# needed here regardless.
rcheevos_dir = meson.current_source_dir() / 'thirdparty' / 'rcheevos'
rcheevos_src = files(
  rcheevos_dir / 'src' / 'rcheevos' / 'alloc.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'compat.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'condition.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'condset.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'consoleinfo.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'format.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'lboard.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'memref.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'operand.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'rc_validate.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'richpresence.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'runtime.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'runtime_progress.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'trigger.c',
  rcheevos_dir / 'src' / 'rcheevos' / 'value.c',
)

retroachievements_poc_src = files(
  'ra_plugin.c',
  'ra_memory.c',
  'ra_http.c',
  'ra_savestate.c',
  'ra_ui.cc',
)

# TODO(verify): curl_dep / imgui_dep names against xemu's actual
# meson.build — guessed from what its public dependency list implies,
# not confirmed against source I could read.
retroachievements_poc = static_library(
  'retroachievements_poc',
  retroachievements_poc_src + rcheevos_src,
  dependencies: [curl_dep, imgui_dep],
  include_directories: include_directories('.', 'thirdparty/rcheevos/include'),
)

retroachievements_poc_dep = declare_dependency(
  link_with: retroachievements_poc,
  include_directories: include_directories('.'),
)
