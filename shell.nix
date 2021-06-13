with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_12.stdenv; } {
    buildInputs = [
        feh
        gdb
        shellcheck
        valgrind
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
}
