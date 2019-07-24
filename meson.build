project(
  'trinkster',
  'c', 'cpp',
  version: '0.0.1',
  license: 'GPL3',
  meson_version: '>=0.48.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
  ],
)

add_project_arguments(
  [
    '-DWLR_USE_UNSTABLE',
  ],
  language: [ 'c', 'cpp' ],
)

trinkster_inc = include_directories('src')

wayland_server_dep = dependency('wayland-server')
wayland_client_dep = dependency('wayland-client')
wayland_protos_dep = dependency('wayland-protocols')
xkbcommon_dep = dependency('xkbcommon')
wlroots_dep = dependency('wlroots', version: '>=0.6')
glm_dep = dependency('glm')
waysig_dep = dependency('waysig')

subdir('protocol')
subdir('src')

if get_option('tests')
  catch2_dep = dependency('catch2')
  subdir('tests')
endif