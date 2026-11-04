#!/usr/bin/env bash

# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

package_version="${PACKAGE_VERSION:-1.0.0}"
target_arch="${TARGET_ARCH:-$(dpkg --print-architecture)}"
deb_host_multiarch="${DEB_HOST_MULTIARCH:-$(dpkg-architecture -qDEB_HOST_MULTIARCH)}"
build_dir="$repo_root/build"
cmake_dir="$build_dir/cmake"
stage_dir="$build_dir/package"

rm -rf "$cmake_dir" "$stage_dir"
mkdir -p "$build_dir"

cmake -S "$repo_root" -B "$cmake_dir" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DKDE_INSTALL_LIBDIR="lib/$deb_host_multiarch" \
    -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON \
    -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
    -DCMAKE_VERBOSE_MAKEFILE=ON

cmake --build "$cmake_dir" --parallel "$(nproc)"

DESTDIR="$stage_dir" cmake --install "$cmake_dir"

mkdir -p "$stage_dir/DEBIAN"
cat > "$stage_dir/DEBIAN/control" <<CONTROL
Package: sbman
Version: ${package_version}
Section: utils
Priority: optional
Architecture: ${target_arch}
Maintainer: uri_herrera@nxos.org
Depends: libkf6coreaddons6,libkf6i18n6,libqt6core6t64,libqt6gui6,libqt6qml6,libqt6quick6,libqt6quickcontrols2-6,mauikit (>= 4.0.5),mokutil,pkexec,qml6-module-qtquick,qml6-module-qtquick-controls,qml6-module-qtquick-layouts
Description: Nitrux Secure Boot certificate enrollment utility
 SBMan imports the Secure Boot certificates provided by Nitrux.
CONTROL

dpkg-deb --build --root-owner-group \
    "$stage_dir" \
    "$build_dir/sbman_${package_version}_${target_arch}.deb"
