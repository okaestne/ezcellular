# SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
# SPDX-License-Identifier: LGPL-3.0-or-later

public_headers = files(
    'any_map.h',
    'connection.h',
    'enums.h',
    'exception.h',
    'ezcellular.h',
    'helpers.h',
    'modem.h',
    'modem_manager.h',
    'sim.h',
    'structs.h',
)

install_headers(
    public_headers,
    subdir: meson.project_name(),
)

sources = files(
    'connection.cpp',
    'helpers.cpp',
    'modem.cpp',
    'modem_manager.cpp',
    'sim.cpp',
)

deps = [modemmanager, sdbus]

# build (shared) library
# https://mesonbuild.com/Reference-manual_functions.html#library
libezcellular = library(
    meson.project_name(),
    sources,
    dependencies: deps,
    include_directories: include_dirs,
    install: true,
    version: meson.project_version(),
)

# generate pkg-config file
# https://mesonbuild.com/Pkgconfig-module.html
pkg.generate(
    libezcellular,
    subdirs: meson.project_name(),
)

libezcellular_dep = declare_dependency(
    dependencies: deps,
    link_with: libezcellular,
    include_directories: include_dirs
)
