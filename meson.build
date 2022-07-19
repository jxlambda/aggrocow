project('aggrocow', 'c',
  version : '0.1.0',
  license : 'ISC',
  default_options : [
    'c_std=c17',
    'buildtype=debugoptimized',
    'werror=true',
    'warning_level=3',
  ]
)

doc_source_files = []
include_directories = include_directories(['include'])

subdir('include')
subdir('src')
subdir('doc')

#if get_option('enable-docs')
#  subdir('doc')
#endif
#
#if get_option('enable-tests')
#  subdir('test')
#endif
