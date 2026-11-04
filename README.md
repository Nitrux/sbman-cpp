# Secure Boot Manager | [![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Secure Boot certificate enrollment utility for Nitrux.

Secure Boot Manager is a C++ rewrite of [sb-manager](https://github.com/Nitrux/sb-manager) built with MauiKit. It imports the Secure Boot certificates provided by Nitrux for its GRUB and kernel packages.

## Usage

Select **Enroll certificates** to queue both certificates. Reboot and approve both requests in MOK Manager.

## Requirements

- Nitrux with the signed GRUB and kernel packages installed.
- A system booted in UEFI mode.

## Runtime Requirements

```text
mauikit (>= 4.0.5)
mokutil
pkexec
```

# Licensing

This repository and its contents are licensed under **BSD-3-Clause**.

# Issues

If you find problems with the contents of this repository, please create an issue and use the **🐞 Bug report** template.

## Submitting a bug report

Before submitting a bug, you should look at the [existing bug reports](https://github.com/Nitrux/sbman-cpp/issues) to verify that no one has reported the bug already.

©2026 Nitrux Latinoamericana S.C.
