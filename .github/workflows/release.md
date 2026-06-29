---
name: Release
description: Bump the patch version and create a release tag for Gearboy.
on:
  workflow_dispatch:
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
    toolsets: [repos]
safe-outputs:
  mentions: false
  allowed-github-references: []
  jobs:
    create-release:
      description: "Bump the patch version in README.md and MCP_README.md, commit, push, tag, and push the tag."
      runs-on: ubuntu-latest
      permissions:
        contents: write
      output: "Release created."
      env:
        GH_TOKEN: ${{ github.token }}
      inputs:
        current_version:
          description: "The current x.y.z version found in README.md."
          required: true
          type: string
        new_version:
          description: "The new x.y.z version after bumping the patch component."
          required: true
          type: string
      steps:
        - name: Checkout
          uses: actions/checkout@v4
          with:
            fetch-depth: 0
        - name: Bump version and tag
          run: |
            if [ ! -f "$GH_AW_AGENT_OUTPUT" ]; then
              echo "No agent output found"
              exit 1
            fi
            CURRENT=$(jq -r '.items[] | select(.type == "create_release") | .current_version' "$GH_AW_AGENT_OUTPUT")
            NEW=$(jq -r '.items[] | select(.type == "create_release") | .new_version' "$GH_AW_AGENT_OUTPUT")
            if [ -z "$CURRENT" ] || [ -z "$NEW" ]; then
              echo "Missing current or new version"
              exit 1
            fi
            if ! echo "$CURRENT" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
              echo "Invalid current version: $CURRENT"
              exit 1
            fi
            if ! echo "$NEW" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
              echo "Invalid new version: $NEW"
              exit 1
            fi
            grep -q "$CURRENT" README.md || { echo "Current version not found in README.md"; exit 1; }
            grep -q "$CURRENT" MCP_README.md || { echo "Current version not found in MCP_README.md"; exit 1; }
            sed -i "s/$CURRENT/$NEW/g" README.md MCP_README.md
            git config user.name "github-actions[bot]"
            git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
            git add README.md MCP_README.md
            git commit -m "Bump version to $NEW"
            git push
            git tag "$NEW" -m "$NEW"
            git push origin "$NEW"
---

# Release

## Task

Create a new release for Gearboy by bumping the patch version, committing, and tagging, following the established release process. Do this autonomously: nobody provides any input when dispatching this workflow. If anything is inconsistent or unexpected, stop and fail without making changes.

## Procedure

### Phase 1: Determine The Version

1. Read README.md and find the current version `x.y.z`. The same number appears many times, used only in download links.
2. Read MCP_README.md and confirm it uses the same `x.y.z` version.
3. If README.md and MCP_README.md disagree on the version, or the version is not a clean `x.y.z`, call `noop` with a short explanation and stop.
4. Compute the new version by bumping only the patch component `z` by one, keeping the major and minor unchanged.

### Phase 2: Create The Release

Call the `create_release` safe-output tool with `current_version` set to the detected `x.y.z` and `new_version` set to the bumped version. A privileged job updates every occurrence in README.md and MCP_README.md, commits `Bump version to <new_version>`, pushes, creates the tag `<new_version>`, and pushes the tag.

## Style

Do not edit any other files. Do not change the major or minor version. If anything is inconsistent or unexpected, stop and report instead of guessing.
