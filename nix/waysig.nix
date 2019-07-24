{ stdenv, pkgconfig, meson, ninja, fetchFromGitHub }:

stdenv.mkDerivation rec {
    name = "waysig-${version}";
    version = "0.1.0";

    src = fetchFromGitHub {
        owner = "dreyri";
	repo = "waysig";
        rev = "321846924170df42a267e9c7a906b3ccca371b2e";
	sha256 = "0qgzyi2fqg4drw4kb14mwcajvcfxfy914dfz9fv6f99dc2xl5q66";
    };

    nativeBuildInputs = [ meson ninja pkgconfig ];

    meta = with stdenv.lib; {
        homepage = "https://github.com/dreyri/waysig";
        description = "C++ signal/slot implementation based on wl_signal/listener";
        license = licenses.mit;
    };
}
