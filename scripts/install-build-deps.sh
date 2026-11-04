#!/usr/bin/env bash

# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

set -euo pipefail

if [ "$EUID" -ne 0 ]; then
    APT_COMMAND="sudo apt"
else
    APT_COMMAND="apt"
fi

$APT_COMMAND update -q
$APT_COMMAND install -y --no-install-recommends \
    build-essential \
    cmake \
    curl \
    devscripts \
    dpkg-dev \
    extra-cmake-modules \
    git \
    gnupg \
    libkf6config-dev \
    libkf6coreaddons-dev \
    libkf6i18n-dev \
    libkf6notifications-dev \
    libkf6windowsystem-dev \
    lintian \
    pkg-config \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-declarative-dev \
    qt6-declarative-dev-tools \
    qt6-multimedia-dev \
    qt6-svg-dev \
    qt6-wayland-dev

mkdir -p /etc/apt/keyrings
curl -fsSL https://packagecloud.io/nitrux/mauikit/gpgkey \
    | gpg --dearmor -o /etc/apt/keyrings/nitrux-mauikit.gpg

cat <<EOF > /etc/apt/sources.list.d/nitrux-mauikit.sources
Types: deb
Description: Nitrux MauiKit Repo
URIs: https://packagecloud.io/nitrux/mauikit/debian/
Suites: duke
Components: main
Signed-By: /etc/apt/keyrings/nitrux-mauikit.gpg
Enabled: yes
EOF

$APT_COMMAND update -q
$APT_COMMAND install -y --no-install-recommends mauikit
