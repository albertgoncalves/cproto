with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_17.stdenv; } {
    buildInputs = [
        curl
        feh
        openssl
        shellcheck
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
