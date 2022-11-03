# Installation Guide

This document explains how to build and install this library.
It is meant to be installed using standard Debian packages (*.deb files).
Alternatively, it can be built and installed using `meson`/`ninja`.
This guide was written for Raspberry Pi OS 2023-02-21 (lite, armhf).

- [Installation Guide](#installation-guide)
  - [Additional notes](#additional-notes)
  - [Install using pre-built Debian packages](#install-using-pre-built-debian-packages)
  - [Build Debian packages from source](#build-debian-packages-from-source)
  - [Build and install using meson and ninja](#build-and-install-using-meson-and-ninja)


## Additional notes

Activate the use of NetworkManager on Raspberry Pi:

1. Run `sudo raspi-config`
2. Choose "Advanced Options"
3. Choose "Network Config"
4. Choose "NetworkManager
5. Reboot the system

## Install using pre-built Debian packages

If you already got the pre-built Debian packages,
you can install them from the current directory as follows:

```shell
# for armhf (Raspberry Pi)
sudo apt install ./libezcellular0{,-dev}_0.1.0_armhf.deb
# for x86-64 PCs
sudo apt install ./libezcellular0{,-dev}_0.1.0_amd64.deb
```

This installs both the shared library (`libezcellular0`)
as well as the development files (`libezcellular0-dev`).
The latter include the C++-header files, the documention and example code.
Installing these packages will also automatically install the required
packages this library depends on.

## Build Debian packages from source

You can also manually build the Debian packages from the source repository.
Make sure that you have a current version of ModemManager (>=1.20) installed.

```shell
# 1.) Install required packages.
sudo apt install build-essential devscripts equivs git meson
# 2.) In the repository root directory, run the following command
#     to create and install a special dummy package,
#     which is used to install all other dependencies.
mk-build-deps -s sudo -i
# 3.) Build the Debian package. This will compile the library.
debuild --no-sign -b
# 4.) Install the built packages (which were placed the in parent directory).
sudo debi --with-depends
```

## Build and install using meson and ninja

```shell
# 1.) install dependencies
# 1.1) compiler etc:
sudo apt install build-essential meson
# 1.2) libraries:
sudo apt install libsdbus-c++-dev modemmanager-dev
# 1.3) if you want to build docs (else skip):
sudo apt install devhelp gtk-doc-tools graphviz \
        modemmanager-doc libmm-glib-doc network-manager-dev libsdbus-c++-doc
# 2.) Setup build.
#     For regular Debian (not Raspberry Pi OS) and Ubuntu, add "--libdir=lib".
meson setup build -Ddocs=true -Dexamples=true
# 3.) build and install (replace "install" with "uninstall" to remove everything)
sudo ninja -C build install
```
