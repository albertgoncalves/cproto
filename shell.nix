with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_21.stdenv; } {
    buildInputs = [
        curl
        feh
        openssl
        libx11
    ];
    shellHook = ''
        export NIX_ENFORCE_NO_NATIVE=0
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
