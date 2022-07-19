subdir('libaggrocow')

libs = [libaggrocow]

aggrocow_src = files(['main.c'])
aggrocow = executable('aggrocow', aggrocow_src,
  include_directories : include_directories,
  link_with : libs,
  install : true)
