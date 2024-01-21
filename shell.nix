with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_16.stdenv; } {
    buildInputs = [
        feh
        curl
        openssl
        shellcheck
        xorg.libX11
    ];
    shellHook = ''
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
