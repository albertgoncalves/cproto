with import <nixpkgs> {};
mkShell{
    buildInputs = [
        clang_10
        gdb
        glibcLocales
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
