with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        gdb
        glibcLocales
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
