with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_14.stdenv; } {
    buildInputs = [
        feh
        shellcheck
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
