---
name: Issue Triage
description: Triage newly opened Gearboy issues by applying the correct repository labels.
on:
  issues:
    types: [opened]
    lock-for-agent: true
  roles: all
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
  mentions: false
  allowed-github-references: []
  add-labels:
    target: triggering
    max: 3
    allowed:
      - agentic workflows
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
    max: 1
    state-reason: duplicate
---

# Issue Triage

## Task

Triage the newly opened issue in `${{ github.repository }}` and apply the correct label or labels from the configured allowlist.

Use GitHub read tools to inspect the triggering issue. Search existing open and closed issues for likely duplicates before deciding. Be conservative: it is better to under-label than to apply a speculative label.

## Triage Protocol

### Phase 1: Gather Context

1. Read the triggering issue title, body, author association, and current labels.
2. Search existing open and closed issues for similar titles, affected games, mapper names, hardware names, error messages, or workflow names.
3. Consider the available label allowlist only: `agentic workflows`, `bug`, `duplicate`, `feature request`, `needs info`, `not a bug`, `question`, `wontfix`.
4. Do not create, rename, remove, or edit labels.

### Phase 2: Quality And Completeness Check

1. If the issue already has the correct label set and no duplicate close or clarification is needed, call `noop`.
2. If there is not enough information to choose a primary label, add `needs info`. Do not add `question` unless the issue is actually a user question.
3. Most incomplete issues should receive only the `needs info` label with no comment.
4. Post a clarification comment only when the issue is likely a `bug`, the next engineering action depends on Gearboy version or platform/frontend, and the issue body does not already provide that information.
5. When asking, ask only for Gearboy version and platform/frontend. Do not ask for ROM hashes, logs, screenshots, save files, long reproduction forms, or extra context during initial triage.

### Phase 3: Primary Classification

Apply labels only through the configured `add-labels` safe output. Choose exactly one primary label when there is enough information:

- `bug`: broken behavior in an already-supported Gearboy feature, mapper, platform, build, package, or workflow. Use this for regressions, crashes, incorrect emulation, save-state problems, audio/video/input defects, frontend bugs, and release or CI breakages caused by this repository.
- `feature request`: a request for new behavior, new platform support, new debugging/MCP capability, new frontend option, or support for currently unsupported cartridge hardware. If a user reports that a game does not run and the likely reason is a missing mapper, unsupported mapper variant, unsupported cartridge accessory, or unsupported special hardware, label it `feature request`, not `bug`.
- `question`: a user question, support request, usage question, or troubleshooting question where the user is asking how something works or how to use Gearboy.
- `not a bug`: expected behavior, invalid or corrupted ROM/save data, a known hardware limitation, an external service problem, spam/test content, or behavior caused by user environment rather than Gearboy.
- `wontfix`: a valid request or limitation that is intentionally outside project scope. Use this sparingly and only when the issue text or maintainer context makes that intent clear.
- `agentic workflows`: an issue about gh-aw, Build Doctor, Issue Triage, workflow agents, safe outputs, generated lock files, or other agentic workflow behavior in this repository. Do not add this label to ordinary emulator bugs, game compatibility reports, or feature requests.

### Phase 4: Gearboy-Specific Checks

For game compatibility reports, separate bugs from unsupported cartridge hardware:

1. Treat likely unsupported or missing mappers as `feature request`. Clues include mapper support requests, bootleg/multicart hardware, unusual MBC behavior, special sensors/accessories, camera/printer-like hardware, flashcarts, or a game that fails only because Gearboy has no implementation for that cartridge type.
2. Treat an already-supported mapper or known supported game that regressed, crashes, corrupts graphics/audio/input, fails save/RTC behavior, or diverges from hardware/reference emulator behavior as `bug`.
3. Treat bad dumps, patched/trainer ROM issues without clean-ROM evidence, invalid save files, or expected Game Boy hardware behavior as `not a bug` when the evidence is clear.
4. Use `needs info` when the report says only that a game does not run but omits the exact ROM title/region/revision, ROM hash, Gearboy version, platform, steps, or whether a clean dump was tested.

### Phase 5: Duplicate Detection

1. Compare the issue to similar open and closed issues.
2. If there is a high-confidence duplicate, add `duplicate` and close the triggering issue using the configured `close-issue` safe output with duplicate state reason.
3. Include the duplicate explanation in the close comment. Do not call `add-comment` separately for duplicates.
4. Use a clearly automated, polite Markdown close comment with line breaks. Use this shape:

```markdown
Automated triage result: this appears to duplicate #123 because it describes the same affected game and mapper symptom.

Closing this as a duplicate. If this report describes a different case, please add the details and it can be reopened.
```
5. Do not close issues for any non-duplicate classification.

### Phase 6: Apply Results

1. Apply the chosen labels with `add-labels`.
2. When calling `add-labels`, provide each label as a separate exact label string from the allowlist. Do not emit bracketed, comma-joined, or combined label strings such as `[bug, needs info]`.
3. Default to labels only. Do not post a comment unless Phase 5 closes a duplicate or Phase 2 requires a narrowly scoped version/platform clarification.
4. For clarification comments, use a clearly automated, polite Markdown request with line breaks. Use this shape:

```markdown
Automated triage request: please provide the Gearboy version and platform/frontend so this bug report can be investigated.

No other details are needed for initial triage.
```

## Style

Keep comments brief, neutral, polite, and explicitly automated, but write in complete readable sentences rather than terse status fragments. Every comment body must use Markdown line breaks: one short automated summary sentence, one blank line, then one short follow-up sentence. Do not write comments as a single paragraph, a single long sentence, a form, or a checklist. Do not use first person, apologies, thanks, human-like warmth, or sign-offs. Do not expose secrets, private resource identifiers, tokens, generated hostnames, or user-sensitive paths. Redact sensitive values as `<redacted>`.