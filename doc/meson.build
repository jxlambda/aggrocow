cldoc = find_program('cldoc')

# sickest of hacks
inc_dir = meson.project_source_root() + '/include'

doc_build_target = run_target('doc',
  command : [
    cldoc,
    'generate',
    '--language',
    'c',
    '-I' + inc_dir,
    '--',
    '--report',
    '--output=' + meson.current_build_dir(),
    doc_source_files
  ]
)
