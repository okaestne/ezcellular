# SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
# SPDX-License-Identifier: LGPL-3.0-or-later

project('ezcellular', 'cpp',
    version: '0.1.0',
    default_options: [
        'buildtype=debugoptimized',
        'cpp_std=c++17',
    ],
    meson_version: '>= 0.53.0',
)

# paths
prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
includedir = join_paths(prefix, get_option('includedir'))

# options
docs = get_option('docs')
examples = get_option('examples')

# dependencies
pkg = import('pkgconfig')
modemmanager = dependency('ModemManager', version: '>=1.20')
sdbus = dependency('sdbus-c++', version: '>=0.8')

include_dirs = include_directories(['.', 'ezcellular'])

subdir('ezcellular')

if docs
    subdir('docs')
endif

subdir('examples')

summary({'prefix': prefix,
         'libdir': libdir,
         'includedir': includedir,
         'datadir': datadir,
        }, section: 'Install directories')
summary({'docs': docs,
         'examples': examples,
        }, section: 'Options')