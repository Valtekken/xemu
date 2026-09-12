# What has to happen in rcheevos itself, before any of this matters for real

This plugin's PoC path (`rc_runtime` + a hand-armed local trigger) works
without any of the below. It's included here because it's the honest
answer to "what's actually blocking real RetroAchievements support for
xemu", and because it just got answered concretely, in public, for a
directly comparable console.

## The PS3 precedent (RPCS3, Aug–Sep 2026)

RPCS3 opened a draft PR ([rpcs3#19143](https://github.com/RPCS3/rpcs3/pull/19143))
with a full `rc_client` integration (login, hashing, memory reads,
session management), gated behind an off-by-default `RPCS3_RA_ENABLED`
compile flag. RetroAchievements admin **wescopeland** responded with a
concrete checklist of what's needed on rcheevos' side before any of it
can go live. In order, as separate, focused PRs into
`RetroAchievements/rcheevos`:

1. **A console ID.** [`rcheevos#535`](https://github.com/RetroAchievements/rcheevos/pull/535) —
   a ~10-line PR adding `RC_CONSOLE_PLAYSTATION_3 = 82` to the enum, a
   name string, and two test cases. Nothing else. Merged.

2. **A hashing implementation, in rcheevos itself, not just internal to
   the emulator.** [`rcheevos#536`](https://github.com/RetroAchievements/rcheevos/pull/536) —
   `rc_hash_ps3`, modeled on the existing `rc_hash_psp`. PS3 games are
   identified by hashing two files off the disc: `PARAM.SFO` (serial/
   title/version metadata) and `USRDIR/EBOOT.BIN` (the executable) —
   hashing the executable alone isn't enough, since the same EBOOT.BIN
   can ship across multiple regional releases. Still under review as of
   this writing — real complications came up around UDF vs. ISO-9660
   table-of-contents parsing for encrypted vs. decrypted disc images.

3. **A memory map, in `consoleinfo.c`.** [`rcheevos#545`](https://github.com/RetroAchievements/rcheevos/pull/545) —
   a `rc_memory_regions_playstation3` table of address ranges, sizes,
   and types (`SYSTEM_RAM` etc.), cross-referenced directly against
   RPCS3's own virtual memory manager source. Merged Sep 11, 2026 (the
   day before this scaffold was written).

Also stated directly by wescopeland: **achievement developers need
`rc_client_raintegration`** (the RAIntegration toolkit binding), not
just `rc_client` — that's a separate integration surface for people
*building* achievement sets, distinct from the player-facing runtime.
And even with all of the above complete, RA gave no firm go-live
timeline — "6-12 months out from integration completion" was the
estimate given, separate from and in addition to whatever compatibility
bar xemu itself needs to clear (see main README).

## What this means for the original Xbox, concretely

The same three PRs would need to happen in rcheevos, and Xbox looks
easier than PS3 on the two axes that turned into real sticking points:

- **Memory map**: Xbox has one flat, fully-preallocated 64MB region
  starting at guest physical address 0. PS3's fight was entirely about
  lazy-allocated pools (game state not yet committed reading back as
  zero rather than "unavailable", and rc_client disabling achievements
  that read out-of-range memory outside a pointer chain — see
  [`rcheevos#545`](https://github.com/RetroAchievements/rcheevos/pull/545)'s
  discussion thread) and 64-bit pointer truncation. Xbox has neither
  problem — see `ra_memory.h`'s comments.

- **Hashing**: original Xbox games boot from a single `default.xbe` at
  the disc root, and the XBE header already embeds a certificate with
  title ID, version, and region — the same kind of metadata PS3 needed
  a *second* file (`PARAM.SFO`) to get. A `rc_hash_xbox` implementation
  may be able to hash `default.xbe` alone, or the certificate plus a
  slice of the executable, without needing to locate and parse a
  separate metadata file. xemu also already supports loading games from
  an extracted folder (not just a disc image), which was raised as an
  open, unresolved question in the PS3 hashing PR — worth designing for
  from the start here rather than bolting on later.

One thing that does **not** apply to Xbox the way it does to PS3: RA
raised, as a real and still-unresolved policy question, how close an
achievement set is allowed to come to a game's *official* trophy/
achievement set. The original Xbox predates Microsoft's own Achievements/
Gamerscore system (introduced with Xbox 360 in 2005) — there's no
official set for an RA set to be compared against or accused of copying.

## Suggested first move, if this ever becomes worth pursuing upstream

Exactly what happened here: open the console-ID PR first. It's small,
self-contained, doesn't depend on anything else being finished, and
gives RA's team something concrete to react to — which is exactly what
Alasonga (the RPCS3 contributor) said was the point of going in with
code rather than just a proposal. Every PR in the PS3 precedent also
included a one-line AI-assistance disclosure in its description or a
top comment; worth doing the same given AGENTS.md's declaration
requirement and mborgerson's stated preference for community-driven,
not AI-driven, ownership of this feature.
