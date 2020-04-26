with import <nixpkgs> {};
mkShell{
    buildInputs = [
        gdb
        clang_10
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
