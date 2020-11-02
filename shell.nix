with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        gdb
        glibcLocales
        shellcheck
        valgrind
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
}
