# EzCellular

[![CI](https://github.com/okaestne/ezcellular/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/okaestne/ezcellular/actions/workflows/build-and-test.yml)

Library to easily handle, control and monitor cellular modems, using the ModemManager D-Bus API.

## Pre-Built Debian Packages (for Ubuntu, Debian, Raspbian, ...)

Download the packages for your architecture from the [Releases page](https://github.com/okaestne/ezcellular/releases) and install them with `sudo apt install ./libezcellular*.deb`.
The packages are currently built for Debian 12 (bookworm), but should also work on Ubuntu and Raspbian.
Supported architectures are `amd64`, `armhf` (32 bit) and `arm64`.

## Building and Installing this Library

### Dependencies

```bash
# compiler and libraries
sudo apt install build-essential libsdbus-c++-dev libsystemd-dev meson modemmanager-dev pkg-config

# for documentation
sudo apt install devhelp doxygen graphviz gtk-doc-tools \
        modemmanager-doc libmm-glib-doc network-manager-dev libsdbus-c++-doc
```

### [RECOMMENDED] Build Debian Package (for Ubuntu, Debian, Raspbian, ...)

```bash
# dependencies for debian packaging
sudo apt install debhelper devscripts
# build .deb packages (will be placed into parent directory)
debuild --no-sign -b
# install built packages (and missing dependencies)
sudo debi --with-depends
```

Example code with be installed into `/usr/share/libezcellular/examples/`

### [Alternative] Manually Build and Install into /usr/local

* See `meson_options.txt` for options

```bash
# build everything
meson setup build --libdir=lib -Ddocs=true -Dexamples=true
# install (replace "install" with "uninstall" to remove everything)
sudo ninja -C build install
```

## Usage

### How to use and link against this library

Options (from most to least recommended):

* Use `pkgconfig` of installed version to automatically add the needed include and linking flags
  * **[RECOMMENDED]** meson: https://mesonbuild.com/Dependencies.html
  * cmake: https://cmake.org/cmake/help/latest/module/FindPkgConfig.html
  * manually: `g++ $(pkg-config --cflags ezcellular) example.cpp -o example $(pkg-config --libs ezcellular)`
* Add CXX flags manually (good luck :smile:)
* Build this library manually (maybe as meson subproject) and link it statically
  * https://mesonbuild.com/Dependencies.html#building-dependencies-as-subprojects

An example Meson project definition `meson.build.example` is part of the example files.
Just rename it to `meson.build` and adjust contents as needed.

## Documentation

The code documentation is automatically built using `doxygen`, if enabled in the build configuration (`-Ddocs=true`).
If you have installed the Debian package, you will find it here: <a href="file:///usr/share/doc/ezcellular/html/index.html">file:///usr/share/doc/ezcellular/html/index.html</a>.

## Development Notes

### Running clang-tidy

```bash
clang-tidy ezcellular/* -- -x c++ -I/usr/include/ModemManager -std=c++17
```

## Project Background and License

This library was created as part of Oliver Kästner's master's thesis in the Laboratory for High Frequency Technology and Mobile Communication at the Osnabrück University of Applied Sciences, under the supervision of Prof. Ralf Tönjes and Julian Dreyer.
Prof. Ralf Tönjes agreed to release this software as open source.

This library is therefore released under the **GNU Lesser General Public License (LGPL) v3.0 or later** (see [COPYING](COPYING)).
