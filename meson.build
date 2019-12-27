project('render','cpp')
src=[
  'src/client/main.cpp',
]
vulkan=dependency('vulkan')
headers = include_directories('include')
executable(
  'render',
  src,
  dependencies : [
    vulkan
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
