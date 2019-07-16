wl_proto_dir = wayland_protos_dep.get_pkgconfig_variable('pkgdatadir')

wayland_scanner_dep = dependency('wayland-scanner', required: false, native: true)

if wayland_scanner_dep.found()
  wayland_scanner = find_program(
    wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner'),
    native: true,
  )
else
  wayland_scanner = find_program('wayland-scanner', native: true)
endif

protocols = [
  [wl_proto_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_proto_dir, 'unstable/xdg-shell/xdg-shell-unstable-v6.xml'],
  [wl_proto_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
  [wl_proto_dir, 'unstable/pointer-constraints/pointer-constraints-unstable-v1.xml'],
  ['wlr-layer-shell-unstable-v1.xml'],
  ['idle.xml'],
  ['wlr-input-inhibitor-unstable-v1.xml'],
]

client_protocols = [
  [wl_proto_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_proto_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
  ['wlr-layer-shell-unstable-v1.xml'],
  ['wlr-input-inhibitor-unstable-v1.xml'],
]

wl_protos_src = []
wl_protos_headers = []

foreach p : protocols
  xml = join_paths(p)
  wl_protos_src += custom_target(
    xml.underscorify() + '_server_c',
    input: xml,
    output: '@BASENAME@-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
  )
  wl_protos_headers += custom_target(
    xml.underscorify() + '_server_h',
    input: xml,
    output: '@BASENAME@-protocol.h',
    command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach

foreach p : client_protocols
  xml = join_paths(p)
  wl_protos_headers += custom_target(
    xml.underscorify() + '_client_h',
    input: xml,
    output: '@BASENAME@-client-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach

client_protos_lib = static_library(
  'client_protos',
  wl_protos_src + wl_protos_headers,
  dependencies: wayland_client_dep,
)

client_protos_dep = declare_dependency(
  link_with: client_protos_lib,
  sources: wl_protos_headers,
)

server_protos_lib = static_library(
  'server_protos',
  wl_protos_src + wl_protos_headers,
  dependencies: wayland_server_dep,
)

server_protos_dep = declare_dependency(
  link_with: server_protos_lib,
  sources: wl_protos_headers,
)
