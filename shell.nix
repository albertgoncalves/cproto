with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_18.stdenv; } {
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
