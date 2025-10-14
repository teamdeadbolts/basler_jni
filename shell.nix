{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    suitesparse
    lapack
    openjdk17
    pnpm
    protobuf_29
    cmake
    ninja
    clang
  ];

  shellHook = ''
    export JAVA_HOME=${pkgs.openjdk17.home}
    export PATH=$JAVA_HOME/bin:$PATH
  '';
}
