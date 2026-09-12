# Notes for whoever turns this into an actual PR (probably you)

## Per xemu's AGENTS.md

This scaffold was drafted with AI assistance (Claude). Per
`AGENTS.md`'s requirement, any PR built from this needs an explicit
agent declaration in its description, e.g.:

> **Agent Declaration**: Portions of this pull request were drafted
> with assistance from Claude (Anthropic).

More importantly, per `CONTRIBUTING.md`'s "Use of AI Tooling" section
and AGENTS.md's ownership requirement: **you** need to have actually
read, understood, compiled, and tested this before it goes anywhere
near a PR — not just "it compiled" but "I checked the TODO(verify)
comments against the real headers and either confirmed or fixed each
one." The three real RetroAchievements-adjacent PRs referenced in
`FUTURE_RCHEEVOS_UPSTREAM.md` all included a one-line AI-assistance
disclosure from their human author, in the same spirit — that's a live
precedent for the norm here, not just an xemu-specific rule.

## Given mborgerson's stated preference

He doesn't want RetroAchievements support to be a maintainer-driven
core feature — the community wiki tracking RA's stance describes it as
"considered with the help of other contributors, potentially as a
plugin." An implementation that reads as fully AI-authored, dropped in
as a large PR, runs directly against that. This scaffold is built to be
small, isolated (`plugins/retroachievements/`, one off-by-default build
flag, four one-line hook points), and — deliberately — incomplete
enough in the right ways that a human has to actually engage with it to
finish it, rather than just merge it.

## Discord first

Per `CONTRIBUTING.md`: significant new features should be discussed
with maintainers on the xemu Discord before serious work lands as a PR.
Worth doing before opening anything, even in draft — something like:

> Working on a RetroAchievements PoC, scoped as an isolated,
> off-by-default plugin rather than a core feature (see
> `plugins/retroachievements/`). Not proposing to merge yet — wanted to
> check this shape makes sense before going further, given RA's own
> May 2026 announcement that xemu won't be listed as supported until
> compatibility improves regardless of code readiness.

That last point matters: this is worth building because someone has to
do the plumbing before RA's compatibility bar gets cleared, but it's
worth being upfront that "merged" and "live on RetroAchievements" are
two different milestones here, and the second one isn't in anyone's
control yet.
