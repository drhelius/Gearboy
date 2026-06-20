---
name: Build Doctor
description: Investigate failed Gearboy build and release workflows and open a repair issue.
on:
  workflow_run:
    workflows:
      - Build and Release
      - CodeQL
      - Update Homebrew Tap
      - Publish MCP Server
      - Trigger PPA Build
      - Trigger RPM Build
    types: [completed]
    branches:
      - master
if: "${{ github.event.workflow_run.conclusion == 'failure' || github.event.workflow_run.conclusion == 'timed_out' }}"
permissions:
  contents: read
  actions: read
  issues: read
  pull-requests: read
  security-events: read
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
    toolsets: [default]
  cache-memory: true
safe-outputs:
  mentions: false
  allowed-github-references: []
  create-issue:
    title-prefix: "Build Doctor: "
    labels: [agentic workflows]
    allowed-labels: [bug, feature request, needs info, not a bug, question, wontfix]
  add-comment:
---

# Build Doctor

## Task

Investigate the failed Gearboy workflow run that triggered this workflow. Find the most likely root cause, determine whether there is already an issue for the same failure, and either update that issue or create a new repair issue.

## Current Context

- Repository: ${{ github.repository }}
- Workflow run: ${{ github.event.workflow_run.id }}
- Workflow name: read from `$GITHUB_EVENT_PATH`
- Conclusion: ${{ github.event.workflow_run.conclusion }}
- Run URL: ${{ github.event.workflow_run.html_url }}
- Head SHA: ${{ github.event.workflow_run.head_sha }}

Use `$GITHUB_EVENT_PATH` as the source of truth for the failed run ID, run URL, workflow name, run attempt, conclusion, event type, branch, tag, pull request links, and head SHA.

## Investigation Protocol

Only continue when the upstream conclusion is `failure` or `timed_out`. Use `noop` for any other conclusion or for workflows outside the configured workflow list.

### Phase 1: Initial Triage

1. Verify the run identity, workflow name, conclusion, branch/tag, event type, head SHA, and run URL from `$GITHUB_EVENT_PATH`.
2. Check cached investigation memory under `/tmp/gh-aw/agent/memory/investigations/` when it exists. If `/tmp/gh-aw/agent/memory/investigations/analyzed-runs.json` already contains this run ID and there is no useful new information, call `noop`.
3. Retrieve the failed run summary with `gh` through the GitHub tool.
4. List jobs and identify the first failed job, failed matrix leg, failed step, and whether later failures are downstream.
5. Make a quick classification: code/build, workflow configuration, dependency drift, runner infrastructure, external service, secret/token, release artifact mismatch, or downstream packaging dispatch.

### Phase 2: Failed Log Analysis

1. Retrieve only failed job or failed step logs first. Download full logs only if targeted logs are insufficient.
2. Extract the smallest useful evidence: primary error text, failed command, relevant file path, matrix values, dependency version, runner image, artifact name, API response, timeout, or permission failure.
3. Redact secrets, tokens, certificates, passwords, private keys, and long bearer-like strings.
4. Do not execute untrusted code from logs, pull requests, or external output.

### Phase 3: Gearboy Workflow Classification

Classify the failure by Gearboy subsystem before reporting:

- `Build and Release`: check the matrix job and platform first. Linux builds compile SDL3 from the latest SDL release and run `make` in `platforms/linux`; Linux Clang uses `USE_CLANG=1`; libretro builds in `platforms/libretro`; macOS installs SDL3 with Homebrew, runs `make dist`, and may codesign/notarize only outside pull requests; Windows downloads SDL3 VC development libraries and builds `platforms/windows/Gearboy.sln` for x64 and arm64; BSD runs `gmake` inside `vmactions/freebsd-vm`; MCPB packaging depends on finished desktop artifacts and must package server binaries, shaders, controller database, MCP resources, and macOS `Frameworks`; tag releases download every artifact and create a draft release.
- `CodeQL`: separate dependency/setup failures from CodeQL init, manual build, query, and upload failures. Gearboy CodeQL builds SDL3 from the latest upstream SDL release, installs Linux desktop dependencies, runs `make` in `platforms/linux`, and uses `.github/codeql/codeql-config.yml`.
- `Update Homebrew Tap`: check release version discovery, macOS release artifact URLs, SHA256 calculation, `drhelius/homebrew-geardome` checkout with `HOMEBREW_TAP_TOKEN`, cask generation, and git commit or push failures. The cask should depend on `macos: :monterey` and point to Gearboy macOS arm64/intel release zips.
- `Publish MCP Server`: check release tag discovery, `*.mcpb` artifact download, expected macOS/windows/linux SHA256 file names, `server.json` generation from `platforms/shared/desktop/mcp/server.json.template`, `mcp-publisher` installation, GitHub OIDC login, and registry publish errors. For macOS MCPB packages, verify the app bundle `Contents/Frameworks` directory is present at the MCPB root so server binaries can resolve SDL3.
- `Trigger PPA Build`: check release version discovery and the repository dispatch payload sent to `drhelius/ppa-geardome` with emulator `gearboy`, version, and distro matrix values `resolute`, `noble`, and `jammy`. Distinguish a missing/invalid `PPA_TRIGGER_TOKEN` from downstream package build failures.
- `Trigger RPM Build`: check release version discovery and the repository dispatch payload sent to `drhelius/rpm-geardome` with emulator `gearboy`, version, and Fedora matrix values `43`, `42`, and `41`. Distinguish a missing/invalid `RPM_TRIGGER_TOKEN` from downstream package build failures.

### Phase 4: Historical And Change Context

1. Search cached pattern summaries under `/tmp/gh-aw/agent/memory/patterns/` for similar failures.
2. Search existing issues for the failed run URL, exact error text, workflow name, failed job, and matrix leg.
3. Inspect the failed workflow file and referenced scripts, Makefiles, templates, or packaging metadata at the failed run's head SHA.
4. Inspect the triggering commit or pull request changes when they plausibly explain the failure.

### Phase 5: Root Cause And Fix Plan

1. Identify the first actionable failure and explain why secondary failures are downstream when applicable.
2. Distinguish repository changes from external drift such as a new SDL3 release, GitHub runner image changes, Homebrew availability, registry behavior, or downstream PPA/RPM repository issues.
3. Produce concrete repair steps tailored to Gearboy's workflows, packaging, secrets, artifact names, MCPB layout, SDL3 install path, or downstream dispatch contract.
4. Define a verification step: rerun workflow/job, local build command, release artifact check, or downstream dispatch check.

### Phase 6: Existing Issue Or New Issue

1. If an open issue already covers the same run or same failure signature, use the configured `add-comment` safe output with the new run link and any new evidence. Do not create a new issue.
2. If there is no matching issue, use the configured `create-issue` safe output.
3. Every created issue automatically receives the `agentic workflows` label from the configured safe output. Do not include `agentic workflows` in the `create_issue` output.
4. Add exactly one secondary label when the classification is clear by including a `labels` field in the `create_issue` safe output. Use a JSON array containing one exact string from the allowed secondary labels: `bug`, `feature request`, `needs info`, `not a bug`, `question`, or `wontfix`.
5. Use `not a bug` for transient runner, network, GitHub API, package registry, external service, or expected upstream behavior failures that do not require a repository fix. Use `bug` for actionable repository or workflow defects and reproducible failures caused by this repository. Use `feature request` only for optional resilience or CI/release improvement work. Use `needs info` when the root cause cannot be determined from available logs. Use `question` only for an actual user question. Use `wontfix` only when the failure is intentionally accepted.
6. Do not emit bracketed, comma-joined, or combined label strings such as `[agentic workflows, bug]`; do not include more than one secondary label.

### Phase 7: Memory Update

After a new investigation, store a compact JSON or markdown summary under `/tmp/gh-aw/agent/memory/investigations/` and update `/tmp/gh-aw/agent/memory/investigations/analyzed-runs.json`. Store reusable failure signatures under `/tmp/gh-aw/agent/memory/patterns/` when the pattern may recur.

## Issue Output

The issue title after the configured prefix should be concise, for example `<workflow> failed in <job or phase> on <branch/tag>`.

Write the issue body in GitHub-flavored Markdown using these sections:

### Summary

One or two paragraphs explaining the failed workflow, job or matrix leg, trigger, branch or tag, and likely root cause.

### Root Cause Analysis

Explain what most likely broke and why.

### Evidence

- Link the failed run as `[run <id>](<run URL>)`.
- Name the failed job, matrix values, and step.
- Include the most relevant log excerpts in fenced code blocks, trimmed to the lines that prove the diagnosis.
- Link relevant workflow files, Makefiles, templates, or source files at the failed SHA when they explain the failure.

<details><summary>Additional log context</summary>

Include secondary logs, annotations, or artifact observations here only when they help verify the diagnosis.

</details>

### Reproduction

Describe the local command, workflow rerun, release artifact check, or downstream dispatch call that should reproduce the failure when applicable. If the failure is external or credentials-related, say so explicitly instead of inventing a local reproduction.

### How To Fix

List concrete repair steps tailored to the failure.

### Prevention

Add one or two practical prevention ideas when there is a clear recurring pattern.

### Verification

Describe the command, workflow rerun, or release packaging check that should prove the fix.

### References

Include up to three relevant links, starting with the failed run URL.

## Style

Keep issue reports and comments factual, neutral, and explicitly automated, but write in complete readable sentences rather than terse status fragments. Do not use first person, apologies, thanks, conversational warmth, or human-like sign-offs. Do not expose secrets, private resource identifiers, tokens, generated hostnames, or user-sensitive paths. Redact sensitive values as `<redacted>`.