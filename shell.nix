let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-unstable";
  pkgs = import nixpkgs {
    config = {
      allowUnfree = true;
    };
  };
in

pkgs.mkShell {
  packages = with pkgs; [
    protozero
    libosmium
    osmium-tool
    wget
  ];
  shellHook = ''
    alias p:run="bash ./OSM-Importer/import-and-extract-aalborg-data.sh"
  '';
}
