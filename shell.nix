with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_11.stdenv; } {
    buildInputs = [
        cppcheck
        gdb
        shellcheck
        valgrind
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
}
