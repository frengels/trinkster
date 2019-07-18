{ stdenv, fetchurl, cmake }:
let
    pname = "callable_traits";
in
stdenv.mkDerivation rec {
    name = "${pname}-${version}";
    version = "2.3.2";

    src = fetchurl {
        url = "https://github.com/boostorg/${pname}/archive/${version}.tar.gz";
        sha256 = "07wr3lhj6057ansv30y91nr30pmyiadnh32qn0ygfcw5b9x3dxmx";
    };

    nativeBuildInputs = [ cmake ];

    meta = with stdenv.lib; {
        description = "Modern C++ type traits and metafunctions for callable types";
        homepage = "https://github.com/boostorg/callable_traits";
        license = licenses.boost;
    };
}