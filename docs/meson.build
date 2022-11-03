doxyfile_conf = configuration_data()
doxyfile_conf.set('SRC_ROOT', meson.source_root())

doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: doxyfile_conf,
    install: false
)

doxygen = find_program('doxygen', required : true)
public_headers = get_variable('public_headers')

docs_target = custom_target(
    'ezcellular-docs',
    input: doxyfile,
    depend_files: public_headers,  # regenerate docs if header changes
    output: ['html', 'latex'],
    command: [doxygen, doxyfile],
    install: true,
    install_dir: join_paths(datadir, 'doc', meson.project_name())
)
