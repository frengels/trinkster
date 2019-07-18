trinkster_src = [
  'wl/listener.cpp',
  'keyboard.cpp',
  'server.cpp',
  'output.cpp',
  'view.cpp',
  'main.cpp',
]

trinkster_deps = [
  wayland_server_dep,
  wayland_protos_dep,
  xkbcommon_dep,
  server_protos_dep,
  wlroots_dep,
  glm_dep,
]

executable(
  'trinkster',
  trinkster_src,
  include_directories: [ trinkster_inc ],
  dependencies: trinkster_deps,
  install: true,
)
