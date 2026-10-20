---
name: Issue Triage
description: Triage newly opened Gearboy issues by applying the correct repository labels.
on:
  issues:
    types: [opened]
    lock-for-agent: true
  roles: all
  skip-roles: [admin, maintainer, write]
  skip-bots: [github-actions, dependabot]
permissions:
  contents: read
  issues: read
  copilot-requests: write
strict: true
network:
  allowed:
    - defaults
    - github
tools:
  github:
    mode: gh-proxy
    toolsets: [repos, issues]
safe-outputs:
  report-failure-as-issue: false
  footer: false
  noop:
    report-as-issue: false
  mentions: false
  allowed-github-references: []
  add-labels:
    target: triggering
    issue-intent: false
    max: 3
    allowed:
      - bug
      - duplicate
      - feature request
      - needs info
      - not a bug
      - question
      - wontfix
    blocked:
      - "~*"
      - "*[bot]"
  add-comment:
    target: triggering
    max: 1
    hide-older-comments: true
  close-issue:
    target: triggering
    issue-intent: false
    max: 1
    state-reason: duplicate
---

# Issue Triage

## Task

Triage the newly opened issue #${{ github.event.issue.number }} in `${{ github.repository }}` and apply the correct label or labels from the configured allowlist.

Use GitHub read tools to inspect the issue. Be conservative: it is better to under-label than to apply a speculative label.

`README.md` in the checked-out repository is the source of truth for what Gearboy supports. Use its Features and Tips sections when deciding between `bug`, `feature request`, `question`, and `not a bug`.

## Triage Protocol

### Phase 1: Gather Context

1. Read the issue title, body, author association, and current labels.
2. Read the Features and Tips sections of `README.md`.
3. Search existing open and closed issues for similar titles, affected games, mapper names, hardware names, or error messages.
4. Use only the configured allowlist: `bug`, `duplicate`, `feature request`, `needs info`, `not a bug`, `question`, `wontfix`. Do not create, rename, remove, or edit labels.

### Phase 2: Primary Classification

Choose exactly one primary label whenever the issue type is clear, even when some details are missing:

- `bug`: broken behavior in an already-supported Gearboy feature, hardware, platform, build, package, or workflow. Use this for regressions, crashes, incorrect emulation, save-state problems, audio/video/input defects, frontend bugs, and release or CI breakages caused by this repository.
- `feature request`: a request for new behavior, new platform support, new debugging/MCP capability, new frontend option, or hardware, peripherals, or formats that `README.md` does not list as supported. A game that does not run because it needs unsupported hardware is a `feature request`, not a `bug`.
- `question`: a user question, support request, usage question, or setup problem where the user is asking how something works or how to use Gearboy.
- `not a bug`: expected behavior, invalid or corrupted ROM/save data, a known hardware limitation, an incorrect setting, an antivirus false positive, an external service problem, spam, unsolicited promotion, or behavior caused by the user environment rather than Gearboy.
- `wontfix`: a valid request or limitation that is intentionally outside project scope. Use this sparingly and only when the issue text or maintainer context makes that intent clear.

If the issue already has the correct labels and needs no comment or duplicate close, call `noop`.

### Phase 3: Gearboy-Specific Checks

1. Unlicensed, bootleg, or multicart games that fail to load and need a mapper not listed in `README.md` are `feature request`. The game name is enough; do not ask for hashes, versions, or dump status.
2. A game that uses a supported mapper or accessory and regressed, crashes, corrupts graphics/audio/input, fails save/RTC behavior, or diverges from hardware/reference emulator behavior is `bug`.
3. Bad dumps, patched/trainer ROM issues without clean-ROM evidence, invalid save files, or expected Game Boy hardware behavior are `not a bug` when the evidence is clear.
4. The Force Game Boy (DMG), Force Game Boy Advance, Super Game Boy, Boot ROM, and mapper settings matter only for color, palette, border, startup, or cartridge detection problems. Ask about them only in those cases.

### Phase 4: Missing Information

Ask only for facts the maintainer would otherwise request in the first reply. Add `needs info` and post a clarification comment only when one of them is missing.

1. Ask only on `bug` reports or when the issue type cannot be determined. Never ask on `feature request`, `question`, `not a bug`, `wontfix`, or duplicate issues.
2. If the report is empty or unclear, does not name the affected game when one is needed, or does not describe the symptom, ask for that first.
3. For a bug in emulation or in a specific game, such as a crash, hang, black screen, failure to boot or load, or wrong graphics, audio, or input, ask for whichever of these facts the report does not provide, in this order:
   - The Gearboy version and whether it is the desktop app or the RetroArch/libretro core, as one bullet. Ask only for the part that is missing.
   - The log, only for crashes, hangs, black screens, and failures to start, boot, or load: the output printed when the desktop app is started from a terminal, or the RetroArch log for the core. Mention only the one that matches the frontend when the report names it.
4. When a Phase 3 check names a fact that matters more for this report, ask for it first.
5. For bugs outside emulation, such as builds, packages, the GUI, the debugger, or MCP, ask only for the version and the host platform, and only when the report suggests they matter, such as a crash on launch that does not name the operating system.
6. Ask only for the facts listed in this phase or named in Phase 3. Never ask for:
   - A fact the report already provides in any wording. An operating system counts as the platform, an app or core name counts as the frontend, a RetroArch mention counts as libretro, and imprecise values such as `latest` count as the version.
   - The host platform for emulation bugs, unless the report suggests the behavior differs between platforms.
   - The ROM hash, dump status, region, or revision, unless the symptoms point at a bad dump.
   - Reproduction steps when the report says where the problem happens, expected behavior, how a difference was measured, or settings that Phase 3 does not name.
   - Screenshots, videos, ROM or media uploads, configuration files, secrets, or sensitive paths.
   - Anything the maintainer can check alone, such as whether a game is supported or how it behaves on real hardware.
7. Ask at most two bullets. If nothing from this phase or Phase 3 is missing, post no comment.
8. Add `needs info` alongside the primary label when asking. Use it without a primary label only when the type cannot be determined.

### Phase 5: Duplicate Detection

1. Compare the issue to similar open and closed issues.
2. If there is a high-confidence duplicate, add `duplicate` and close the issue using the configured `close-issue` safe output with duplicate state reason.
3. Include the duplicate explanation in the close comment. Do not call `add-comment` separately for duplicates.
4. Use a clearly automated, polite Markdown close comment with line breaks. Use this shape:

```markdown
Automated triage result: this appears to duplicate #123 because it describes the same affected game and mapper symptom.

Closing this as a duplicate. If this report describes a different case, please add the details and it can be reopened.

This is an automated triage result generated by [Issue Triage](https://github.com/${{ github.repository }}/actions/workflows/issue-triage.lock.yml)
```
5. Do not close issues for any non-duplicate classification.

### Phase 6: Apply Results

1. Apply the chosen labels with `add-labels`.
2. When calling `add-labels`, provide a JSON array of separate exact label strings from the allowlist. For example, use `"labels": ["bug"]`; never use `"labels": ["[bug]"]`, `"labels": "[bug]"`, or a comma-joined label string.
3. Default to labels only. Do not post a comment unless Phase 5 closes a duplicate or Phase 4 requires a clarification.
4. For clarification comments, use this exact Markdown structure. Replace the placeholders with only absent, report-specific details, and end with the linked sentence exactly as written:

```markdown
Please provide:

- <one specific fact that is absent from the report>
- <a second absent fact only when it is also required>

This is an automated triage request generated by [Issue Triage](https://github.com/${{ github.repository }}/actions/workflows/issue-triage.lock.yml)
```

## Style

Keep comments brief, neutral, polite, and explicitly automated. Use direct, specific wording and do not add a generic preamble, restate the issue, or ask the reporter to repeat information already supplied. Do not use first person, apologies, thanks, human-like warmth, or sign-offs other than the required linked automation sentence. Do not expose secrets, private resource identifiers, tokens, generated hostnames, or user-sensitive paths. Redact sensitive values as `<redacted>`.
