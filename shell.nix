with import <nixpkgs> {};
mkShell{
    buildInputs = [
        clang_10
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
