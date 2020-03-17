project('render','cpp')
src=[
  'src/client/main.cpp',
  'src/client/entity.cpp',
  'src/client/terrain.cpp',
]
vulkan=dependency('vulkan')
glm=dependency('glm')
libnoise=meson.get_compiler('cpp').find_library('noise')
headers = include_directories('include')
executable(
  'render',
  src,
  dependencies : [
    vulkan,
    glm,
    libnoise
    ],
  include_directories : headers,
  install : true,
  link_args : ['-lglfw']
  )
#executable(
#  'server',
#  'src/server/main.cpp',
#  install : true,
#  link_args : ['']
#  )
