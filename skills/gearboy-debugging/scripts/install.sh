#!/usr/bin/env bash
# Install emulator for use as an MCP server.
# Usage: bash scripts/install.sh
#
# This script installs the emulator and prints the binary path.
# It supports macOS (Homebrew) and Linux (GitHub releases).

set -euo pipefail

EMULATOR_NAME="Gearboy"   # Title case name (used in repo and asset names)
EMULATOR_CMD="gearboy"    # Lowercase name (used for binary, package, and cask)

REPO="drhelius/${EMULATOR_NAME}"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/bin}"

check_existing() {
    local existing
    existing=$(command -v "$EMULATOR_CMD" 2>/dev/null || true)
    if [[ -n "$existing" ]]; then
        echo "$existing"
        exit 0
    fi
}

install_macos() {
    if command -v brew &>/dev/null; then
        echo "Installing ${EMULATOR_NAME} via Homebrew..." >&2
        brew install --cask "drhelius/geardome/${EMULATOR_CMD}" >&2
        local bin
        bin=$(command -v "$EMULATOR_CMD" 2>/dev/null || true)
        if [[ -z "$bin" ]]; then
            bin=$(find /Applications -name "$EMULATOR_CMD" -type f -maxdepth 5 2>/dev/null | head -1 || true)
        fi
        if [[ -n "$bin" ]]; then
            echo "$bin"
        else
            echo "Installed via Homebrew. Run: brew info drhelius/geardome/${EMULATOR_CMD} to find the binary path." >&2
        fi
        return
    fi

    echo "Homebrew not found, downloading from GitHub..."
    local arch
    arch=$(uname -m)
    local suffix
    case "$arch" in
        arm64) suffix="arm64" ;;
        x86_64) suffix="intel" ;;
        *)
            echo "Unsupported architecture: $arch. Download manually from: https://github.com/$REPO/releases/latest"
            exit 1
            ;;
    esac

    echo "Fetching latest release info..."
    local tag
    tag=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name"' | head -1 | sed 's/.*"tag_name": *"\([^"]*\)".*/\1/')

    if [[ -z "$tag" ]]; then
        echo "Failed to fetch latest release. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    local asset="${EMULATOR_NAME}-${tag}-desktop-macos-${suffix}.zip"
    local url="https://github.com/$REPO/releases/download/${tag}/${asset}"

    echo "Downloading $asset..."
    local tmpdir
    tmpdir=$(mktemp -d)

    if ! curl -fSL "$url" -o "$tmpdir/$asset"; then
        rm -rf "$tmpdir"
        echo "Download failed. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    if ! unzip -q "$tmpdir/$asset" -d "$tmpdir"; then
        rm -rf "$tmpdir"
        echo "Failed to extract archive. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi
    local app
    app=$(find "$tmpdir" -name "*.app" -maxdepth 2 | head -1)

    if [[ -z "$app" ]]; then
        local bin
        bin=$(find "$tmpdir" -name "$EMULATOR_CMD" -type f -perm +111 | head -1)
        if [[ -z "$bin" ]]; then
            rm -rf "$tmpdir"
            echo "Could not find binary in archive. Download manually from: https://github.com/$REPO/releases/latest"
            exit 1
        fi
        mkdir -p "$INSTALL_DIR"
        cp "$bin" "$INSTALL_DIR/$EMULATOR_CMD"
        chmod +x "$INSTALL_DIR/$EMULATOR_CMD"
        rm -rf "$tmpdir"
        echo "$INSTALL_DIR/$EMULATOR_CMD"
        return
    fi

    mkdir -p "$INSTALL_DIR"
    cp -R "$app" "$INSTALL_DIR/"
    rm -rf "$tmpdir"

    local app_name
    app_name=$(basename "$app")
    echo "$INSTALL_DIR/$app_name/Contents/MacOS/$EMULATOR_CMD"
}

install_linux() {
    # Detect Ubuntu/Debian version once for both PPA and GitHub fallback
    local codename="noble"
    local ubuntu_ver="24.04"
    if [[ -f /etc/os-release ]]; then
        local ver_id
        ver_id=$(grep VERSION_ID /etc/os-release | cut -d'"' -f2 2>/dev/null || echo "")
        case "$ver_id" in
            22.04) codename="jammy";  ubuntu_ver="22.04" ;;
            24.04) codename="noble";  ubuntu_ver="24.04" ;;
            26.04) codename="resolute"; ubuntu_ver="26.04" ;;
        esac
    fi

    # Try PPA first (Ubuntu/Debian)
    if command -v apt-get &>/dev/null; then
        echo "Installing ${EMULATOR_NAME} via PPA..."
        curl -fsSL "https://drhelius.github.io/ppa-geardome/geardome-ppa.gpg" | \
            sudo tee /usr/share/keyrings/geardome-archive-keyring.gpg > /dev/null || true
        echo "deb [arch=amd64,arm64 signed-by=/usr/share/keyrings/geardome-archive-keyring.gpg] https://drhelius.github.io/ppa-geardome ${codename} main" | \
            sudo tee /etc/apt/sources.list.d/geardome.list > /dev/null
        sudo apt-get update -qq 2>/dev/null || true
        if sudo apt-get install -y "$EMULATOR_CMD" 2>/dev/null; then
            echo "$(command -v "$EMULATOR_CMD")"
            return
        fi
        echo "PPA install failed, falling back to GitHub release..."
    fi

    local arch
    arch=$(uname -m)
    local suffix
    case "$arch" in
        x86_64) suffix="x64" ;;
        aarch64) suffix="arm64" ;;
        *)
            echo "Unsupported architecture: $arch. Download manually from: https://github.com/$REPO/releases/latest"
            exit 1
            ;;
    esac

    echo "Fetching latest release info..."
    local tag
    tag=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name"' | head -1 | sed 's/.*"tag_name": *"\([^"]*\)".*/\1/')

    if [[ -z "$tag" ]]; then
        echo "Failed to fetch latest release. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    local asset="${EMULATOR_NAME}-${tag}-desktop-ubuntu${ubuntu_ver}-${suffix}.zip"
    local url="https://github.com/$REPO/releases/download/${tag}/${asset}"

    echo "Downloading $asset..."
    local tmpdir
    tmpdir=$(mktemp -d)

    if ! curl -fSL "$url" -o "$tmpdir/$asset"; then
        rm -rf "$tmpdir"
        echo "Download failed. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    if ! unzip -q "$tmpdir/$asset" -d "$tmpdir"; then
        rm -rf "$tmpdir"
        echo "Failed to extract archive. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi
    local bin
    bin=$(find "$tmpdir" -name "$EMULATOR_CMD" -type f | head -1)

    if [[ -z "$bin" ]]; then
        rm -rf "$tmpdir"
        echo "Could not find binary in archive. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    mkdir -p "$INSTALL_DIR"
    cp "$bin" "$INSTALL_DIR/$EMULATOR_CMD"
    chmod +x "$INSTALL_DIR/$EMULATOR_CMD"
    rm -rf "$tmpdir"
    echo "$INSTALL_DIR/$EMULATOR_CMD"
}

main() {
    check_existing

    local os
    os=$(uname -s)
    case "$os" in
        Darwin) install_macos ;;
        Linux)  install_linux ;;
        *)
            echo "Unsupported OS: $os. Download manually from: https://github.com/$REPO/releases/latest"
            exit 1
            ;;
    esac
}

main
