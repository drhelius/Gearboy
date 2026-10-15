#!/bin/bash

set -e

cd "$(dirname "$0")"

name=$(/usr/libexec/PlistBuddy -c "Print :CFBundleName" Info.plist | tr '[:upper:]' '[:lower:]')
input_filepath="image.png"
work_dir=$(mktemp -d)
generator="${work_dir}/generate_icon"
output_iconset_name="${work_dir}/iconfile.iconset"

trap 'rm -rf "$work_dir"' EXIT

xcrun swiftc -O generate_icon.swift -o "$generator"

mkdir "$output_iconset_name"
for size in 16 32 128 256 512; do
    "$generator" png $input_filepath "${output_iconset_name}/icon_${size}x${size}.png" $size
    "$generator" png $input_filepath "${output_iconset_name}/icon_${size}x${size}@2x.png" $((size * 2))
done
iconutil -c icns "$output_iconset_name" -o iconfile.icns

"$generator" ico $input_filepath ../windows/iconfile.ico 16 20 24 32 40 48 64 96 128 256

mkdir -p ../shared/desktop/icons
for size in 16 24 32 48 64 128 256 512; do
    "$generator" png $input_filepath "../shared/desktop/icons/${name}-${size}.png" $size
done
cp "../shared/desktop/icons/${name}-128.png" ../shared/desktop/mcp/icon.png

for appiconset in ../ios/*/Resources/Assets.xcassets/AppIcon.appiconset; do
    if [ -d "$appiconset" ]; then
        "$generator" appiconset $input_filepath "$appiconset"
    fi
done
