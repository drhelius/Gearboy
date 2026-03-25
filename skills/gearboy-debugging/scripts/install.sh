#!/usr/bin/env bash
# Install Gearboy emulator for use as an MCP server.
# Usage: bash scripts/install.sh
#
# This script installs Gearboy and prints the binary path.
# It supports macOS (Homebrew) and Linux (GitHub releases).

set -euo pipefail

REPO="drhelius/Gearboy"
INSTALL_DIR="${GEARBOY_INSTALL_DIR:-$HOME/.local/bin}"

check_existing() {
    local existing
    existing=$(command -v gearboy 2>/dev/null || true)
    if [[ -n "$existing" ]]; then
        echo "$existing"
        exit 0
    fi
}

install_macos() {
    if command -v brew &>/dev/null; then
        echo "Installing Gearboy via Homebrew..."
        brew install --cask drhelius/geardome/gearboy
        echo "Installed via Homebrew. Run: brew info drhelius/geardome/gearboy to find the binary path."
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

    local asset="Gearboy-${tag}-desktop-macos-${suffix}.zip"
    local url="https://github.com/$REPO/releases/download/${tag}/${asset}"

    echo "Downloading $asset..."
    local tmpdir
    tmpdir=$(mktemp -d)

    if ! curl -fSL "$url" -o "$tmpdir/$asset"; then
        rm -rf "$tmpdir"
        echo "Download failed. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    unzip -q "$tmpdir/$asset" -d "$tmpdir"
    local app
    app=$(find "$tmpdir" -name "*.app" -maxdepth 2 | head -1)

    if [[ -z "$app" ]]; then
        local bin
        bin=$(find "$tmpdir" -name "gearboy" -type f -perm +111 | head -1)
        if [[ -z "$bin" ]]; then
            rm -rf "$tmpdir"
            echo "Could not find binary in archive. Download manually from: https://github.com/$REPO/releases/latest"
            exit 1
        fi
        mkdir -p "$INSTALL_DIR"
        cp "$bin" "$INSTALL_DIR/gearboy"
        chmod +x "$INSTALL_DIR/gearboy"
        rm -rf "$tmpdir"
        echo "$INSTALL_DIR/gearboy"
        return
    fi

    mkdir -p "$INSTALL_DIR"
    cp -R "$app" "$INSTALL_DIR/"
    rm -rf "$tmpdir"

    local app_name
    app_name=$(basename "$app")
    echo "$INSTALL_DIR/$app_name/Contents/MacOS/gearboy"
}

install_linux() {
    # Try PPA first (Ubuntu/Debian)
    if command -v apt-get &>/dev/null; then
        echo "Installing Gearboy via PPA..."
        sudo mkdir -p /etc/apt/keyrings
        curl -fsSL "https://raw.githubusercontent.com/drhelius/ppa-geardome/main/KEY.gpg" | sudo gpg --dearmor -o /etc/apt/keyrings/ppa-geardome.gpg 2>/dev/null || true
        echo "deb [signed-by=/etc/apt/keyrings/ppa-geardome.gpg] https://raw.githubusercontent.com/drhelius/ppa-geardome/main ./" | sudo tee /etc/apt/sources.list.d/ppa-geardome.list > /dev/null
        sudo apt-get update -qq
        if sudo apt-get install -y gearboy; then
            echo "$(command -v gearboy)"
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

    # Detect Ubuntu version or default to 24.04
    local ubuntu_ver="24.04"
    if [[ -f /etc/os-release ]]; then
        local ver
        ver=$(grep VERSION_ID /etc/os-release | cut -d'"' -f2 2>/dev/null || echo "")
        case "$ver" in
            22.04) ubuntu_ver="22.04" ;;
            24.04) ubuntu_ver="24.04" ;;
        esac
    fi

    echo "Fetching latest release info..."
    local tag
    tag=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name"' | head -1 | sed 's/.*"tag_name": *"\([^"]*\)".*/\1/')

    if [[ -z "$tag" ]]; then
        echo "Failed to fetch latest release. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    local asset="Gearboy-${tag}-desktop-ubuntu${ubuntu_ver}-${suffix}.zip"
    local url="https://github.com/$REPO/releases/download/${tag}/${asset}"

    echo "Downloading $asset..."
    local tmpdir
    tmpdir=$(mktemp -d)

    if ! curl -fSL "$url" -o "$tmpdir/$asset"; then
        rm -rf "$tmpdir"
        echo "Download failed. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    unzip -q "$tmpdir/$asset" -d "$tmpdir"
    local bin
    bin=$(find "$tmpdir" -name "gearboy" -type f | head -1)

    if [[ -z "$bin" ]]; then
        rm -rf "$tmpdir"
        echo "Could not find binary in archive. Download manually from: https://github.com/$REPO/releases/latest"
        exit 1
    fi

    mkdir -p "$INSTALL_DIR"
    cp "$bin" "$INSTALL_DIR/gearboy"
    chmod +x "$INSTALL_DIR/gearboy"
    rm -rf "$tmpdir"
    echo "$INSTALL_DIR/gearboy"
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
