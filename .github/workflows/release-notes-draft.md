---
name: Release Notes Draft
description: Draft release notes for newly created draft releases using the established emulator release style.
on:
  workflow_dispatch:
    inputs:
      release_tag:
        description: "Release tag to draft notes for (e.g. 1.7.12)"
        required: false
        type: string
        default: ""
permissions:
  contents: read
  actions: read
  issues: read
  pull-requests: read
  copilot-requests: write
strict: true
network:
  allowed:
    - defaults
    - github
    - github-actions
tools:
  github:
    mode: gh-proxy
    toolsets: [repos, issues, pull_requests, actions]
  cache-memory: true
safe-outputs:
  report-failure-as-issue: false
  footer: false
  noop:
    report-as-issue: false
  mentions: false
  allowed-github-references: []
  jobs:
    update-draft-release:
      description: "Replace the draft release notes for the target release tag with the provided markdown body."
      runs-on: ubuntu-latest
      permissions:
        contents: write
      output: "Draft release notes updated."
      env:
        GH_TOKEN: ${{ github.token }}
        RELEASE_TAG: ${{ github.event.inputs.release_tag }}
      inputs:
        body:
          description: "The complete release notes markdown to set as the draft release body."
          required: true
          type: string
      steps:
        - name: Update draft release notes
          run: |
            if [ -z "$RELEASE_TAG" ]; then
              echo "No release tag provided; nothing to update."
              exit 0
            fi
            if [ ! -f "$GH_AW_AGENT_OUTPUT" ]; then
              echo "No agent output found"
              exit 1
            fi
            BODY=$(jq -r '.items[] | select(.type == "update_draft_release") | .body' "$GH_AW_AGENT_OUTPUT")
            if [ -z "$BODY" ]; then
              echo "No body provided by agent"
              exit 1
            fi
            printf '%s' "$BODY" > /tmp/release-notes.md
            gh release edit "$RELEASE_TAG" --repo "$GITHUB_REPOSITORY" --notes-file /tmp/release-notes.md
---

# Release Notes Draft

## Task

Detect the newest Gearboy draft release created by the `Build and Release` workflow and replace its generated release notes with short headline notes that match the established drhelius emulator release style.

## Procedure

### Phase 1: Identify The Target Release

1. The target release tag is provided as the `release_tag` input: `${{ github.event.inputs.release_tag }}`. The `Build and Release` workflow sets this when it dispatches this workflow after creating the draft release.
2. Important: this workflow runs with a read-only token, which cannot see draft releases when listing them. Do not try to list or detect draft releases. Treat the release for tag `${{ github.event.inputs.release_tag }}` as the target.
3. If `${{ github.event.inputs.release_tag }}` is empty, call `noop` with a short explanation.
4. If `/tmp/gh-aw/agent/memory/release-notes-draft/processed-drafts.json` shows that tag `${{ github.event.inputs.release_tag }}` was already processed, call `noop`.

### Phase 2: Learn The Style

1. Read at least six published releases from the last year in this repository as the style reference for tone and recurring bullets. The rules and examples in Phase 4 summarize that style; follow them when an older release differs.
2. The format is a direct bullet list with emoji, without headings or `## What's Changed`, followed by a `**Full Changelog**:` compare link. Keep a `New Contributors` section only when a first-time external contributor landed a pull request in the range.

### Phase 3: Collect Changes

1. The target release tag is `${{ github.event.inputs.release_tag }}`. Identify the previous published release tag from the list of published releases (published releases and pre-releases are visible to the read-only token).
2. List both the merged pull requests and the full commit log between the previous published tag and `${{ github.event.inputs.release_tag }}`. Most feature work in this repository lands as direct commits without a pull request, so the commit log is the primary source of truth for features; read every commit subject in the range. Use pull requests mainly to attribute external contributors and confirm the range.
3. Do not read the draft release body or call GitHub's generate-notes API; the read-only token cannot access drafts. Reconstruct the changelog from the commit log and merged pull requests instead.
4. Commit-message prefixes signal the affected area (`[libretro]` for the RetroArch core, `[debugger]` for debugger windows and tooling, `[mcp]` for the MCP server), and many feature commits have no prefix at all. Group related commits into one candidate per feature or subsystem. Never drop a notable feature just because it did not arrive through a pull request.
5. Note whether `platforms/shared/gamecontrollerdb.txt` changed in the range (commit subjects start with `Update gamecontrollerdb`).
6. Note the total number of commits in the range; it decides the wording of the catch-all bullet.
7. Note the GitHub login of each notable pull request's author. Treat authors other than the repository owner (`drhelius`) as external contributors to be credited in Phase 4.

### Phase 4: Draft Notes

Write headlines, not a changelog. The notes read like the feature list on a box: a few short bullets naming what a player or homebrew developer will notice, followed by a catch-all bullet.

Rules:
- Keep 2 to 6 feature bullets. Choose the changes users will notice most and fold everything else into the catch-all bullet.
- Keep each bullet to roughly 3 to 10 words on a single line. No sub-bullets, semicolons, or trailing `and ...` clauses listing extra fixes. A short colon or parenthetical is fine only to name supported items such as models, mappers or event types (`New trace logger events: missed frame and sprite budgets`), never to list fixes or implementation details.
- Name what changed, not how. Name the feature or subsystem users recognize (Game Boy Color, Super Game Boy, sound, cartridge mappers, link cable, cheats, save states, libretro core, or a specific debugger window) and leave out registers, timings, function names, implementation details, and lists of individual fixes.
- A new feature gets its own name: `Added video recording`, `Ubuntu 26.04 builds`, `Variable Refresh Rate ready`. Several fixes in one subsystem become a single `Improved <subsystem> accuracy` or `Improved <subsystem> compatibility` bullet; never enumerate them. Avoid a vague `Improved X` only when one concrete new feature describes the change better.
- Use names users know instead of internal ones, and spell out unfamiliar acronyms (`LaserActive`, not `MMI`).
- Prefix a bullet with `[debugger]` when the feature only concerns the debugger, after the emoji: `⚙️ [debugger] CPU Profiler`.
- When an MCP change only exposes a debugger feature already listed, fold it into that bullet's area instead of adding a separate MCP bullet.
- Leave out changes that do not reach users: refactors, tests, CI, documentation, build-system or packaging fixes, and preparatory work for something not shipped yet. New download targets or platforms are user-facing and stay.
- Credit external contributors on the bullets that are kept by appending `by [username](https://github.com/username)` using the author's GitHub login.
- Do not invent features.

Order:
1. Feature bullets, most significant first.
2. `🎮 Updated Game Controller DB` if `platforms/shared/gamecontrollerdb.txt` changed in the range.
3. The catch-all bullet, always last, scaled to the release size: `🐛 Bug fixes` for about 5 commits or fewer, `🐛 Bug fixes and improvements` for up to about 25 commits, `🐛 Many bug fixes and improvements` for larger releases.

Emoji guide (pick the closest match): `🎯` accuracy or compatibility, `🖥️` display, video and host integration, `🔊` audio, `🎵` sound chips and VGM recording, `🎮` input and controllers, `💾` save states, BIOS, patching and files, `🎨` themes, shaders and visuals, `🚀` performance and latency, `🎥` video recording, `🔔` notifications, `🔌` link cables and multiplayer, `📦` builds and packages, `📱` mobile apps, `🕹️` libretro core, `🧠` MCP server, `⚙️` debugger and tooling, `⚠️` trace logger events, `🤖` AI, `🐛` bug fixes.

Too detailed versus preferred, taken from real edits made to drafts in Geargrafx, a sibling emulator:

| Too detailed | Preferred |
| --- | --- |
| `💿 CD-ROM: SCSI seek timing for pause/delay commands, on-demand CD audio track loading, smaller CD image chunks` | `💿 Improved CD-ROM accuracy` |
| `🎯 Improved PSG accuracy: trigger, counter and modulation timing fixes` | `🎯 Improved PSG accuracy` |
| `🎮 Reduced input lag and improved mouse capture handling (persistent capture hotkey, released on launch)` | `🎮 Reduced input lag` |
| `⚙️ New function profiler debugger window with cycle stats` | `⚙️ [debugger] CPU Profiler` |
| `💾 Smaller savestates and fixed input restoration and validation` | `💾 Smaller save states` |
| `💿 Improved libretro MMI implementation` | `💿 Improved libretro LaserActive implementation` |

Style example (a real release of this repository, illustrative only — every bullet must reflect this repository's actual changes; never copy these items):

```markdown
- 🎥 Added video recording
- 🖥️ Improved fractional DPI display rendering and scaling
- ⚙️ [debugger] CPU Profiler
- ⚠️ [debugger] New trace logger events: missed frame and sprite budgets
- 💾 Smaller save states
- 🎮 Updated Game Controller DB
- 🐛 Bug fixes and improvements

**Full Changelog**: https://github.com/drhelius/Gearboy/compare/<previous>...<current>
```

Before submitting, re-read every bullet and shorten any that breaks the rules above.

### Phase 5: Update Release

Call the `update_draft_release` safe-output tool with `body` set to the complete new release notes markdown. A privileged job applies it to the draft release for tag `${{ github.event.inputs.release_tag }}` using `gh release edit`.

After requesting the update, store a compact record under `/tmp/gh-aw/agent/memory/release-notes-draft/processed-drafts.json` with tag `${{ github.event.inputs.release_tag }}` and timestamp.

## Style

Keep the notes short, factual, and in the established emulator release style. Do not add a sign-off, long generated summary, or internal analysis.
