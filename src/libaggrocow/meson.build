libaggrocow_src = files(['lib.c'])

# The library uses some non-standard C, but POSIX compliant, functions.
#
# Below is a list of feature enablement macro definitions and reasons why
# they're defined.
#
# This is mainly to appease glibc, as the BSDs expose these without requiring
# feature enablement, with some exceptions.
extra_args = []
if build_machine.system() == 'linux'
  # strdup(3)
  extra_args += '-D_POSIX_C_SOURCE=200809L'
  # reallocarray(3)
  extra_args += '-D_DEFAULT_SOURCE'
elif build_machine.system() == 'netbsd'
  # reallocarray(3)
  extra_args += '-D_OPENBSD_SOURCE'
endif

libaggrocow = library('aggrocow', libaggrocow_src,
  include_directories : include_directories,
  c_args : extra_args,
  install : true)

doc_source_files += libaggrocow_src

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : libaggrocow,
  name : 'libaggrocow',
  description : 'A library for managing optimal aggressive cow allocation.')
