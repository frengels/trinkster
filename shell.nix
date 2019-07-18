{ nixpkgs ? import <nixpkgs> {} }:
with nixpkgs;
let
  callable_traits = callPackage ./nix/callable_traits.nix {};
in
stdenv.mkDerivation {
  name = "trinkster-dev";
  buildInputs = [ meson ninja pkgconfig gdb udev x11 pixman libxkbcommon libGL wlroots wayland wayland-protocols glm clang_8 gcc9 callable_traits ];
}
