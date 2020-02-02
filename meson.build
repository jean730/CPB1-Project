project('render','cpp')
src=[
  'src/client/main.cpp',
  'src/client/entity.cpp',
]
vulkan=dependency('vulkan')
glm=dependency('glm')
headers = include_directories('include')
executable(
  'render',
  src,
  dependencies : [
    vulkan,
    glm
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
