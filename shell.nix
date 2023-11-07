{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell {
	buildInputs = [
		gnumake
		gcc

	] ++ (with xorg;[
		libX11
		libXi
		libXtst
	]);
}
