# SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
# SPDX-License-Identifier: LGPL-3.0-or-later

libezcellular_dep = get_variable('libezcellular_dep')

example_sources = files(
    'cell_info.cpp',
    'connection.cpp',
    'list_modems.cpp',
    'location.cpp',
    'modem_lifecycle.cpp',
    'signal.cpp',
    'monitor_traffic.cpp',
    'sim.cpp',
    'time.cpp',
)

examples_dir = join_paths(datadir, meson.project_name(), 'examples')

install_data(
    [example_sources, 'meson.build.example'],
    install_dir: examples_dir
)

# if enabled, build example binaries
if examples
    cell_info_exe = executable(
        'cell_info',
        'cell_info.cpp',
        dependencies: [libezcellular_dep]
    )

    connection_exe = executable(
        'connection',
        'connection.cpp',
        dependencies: [libezcellular_dep]
    )

    list_modems_exe = executable(
        'list_modems',
        'list_modems.cpp',
        dependencies: [libezcellular_dep]
    )

    location_exe = executable(
        'location',
        'location.cpp',
        dependencies: [libezcellular_dep]
    )

    modem_lifecycle_exe = executable(
        'modem_lifecycle',
        'modem_lifecycle.cpp',
        dependencies: [libezcellular_dep]
    )

    signal_exe = executable(
        'signal',
        'signal.cpp',
        dependencies: [libezcellular_dep]
    )

    monitor_traffic_exe = executable(
        'monitor_traffic',
        'monitor_traffic.cpp',
        dependencies: [libezcellular_dep]
    )

    sim_exe = executable(
        'sim',
        'sim.cpp',
        dependencies: [libezcellular_dep]
    )

    time_exe = executable(
        'time',
        'time.cpp',
        dependencies: [libezcellular_dep]
    )
endif