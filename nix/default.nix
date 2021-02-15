{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  name = "emenu-dev";
  buildInputs = with pkgs; [ gcc xorg.libX11 xorg.libX11.dev xorg.libXft xorg.libXft.dev fontconfig.dev xorg.libXinerama xorg.libXinerama.dev gnumake ];
  shellHook = ''
    export INCLUDE_PATH_LIBXFT=${pkgs.xorg.libXft.dev}/include
    export INCLUDE_PATH_FC=${pkgs.fontconfig.dev}/include
    export INCLUDE_PATH_XINERAMA=${pkgs.xorg.libXinerama.dev}/include
  '';
}
