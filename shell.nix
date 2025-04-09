with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_19.stdenv; } {
    buildInputs = [
        curl
        feh
        openssl
        shellcheck
        xorg.libX11
    ];
    shellHook = ''
        export NIX_ENFORCE_NO_NATIVE=0
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
