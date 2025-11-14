{ pkgs ? import (fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/036660e6294d7ae6e4bfd1b0f4e3f4dc2d53c483.tar.gz";
    sha256 = "sha256:06is1fzmw06y63m0i2zyaj554pwh2p8280pg7b8ws1z0sk4r1k5c";
  }) {}
}:
let
  opencv4100 = pkgs.opencv4.overrideAttrs (oldAttr: rec {
    version = "4.10.0";
    src = pkgs.fetchFromGitHub {
      owner = "opencv";
      repo = "opencv";
      rev = version;
      hash = "sha256-s+KvBrV/BxrxEvPhHzWCVFQdUQwhUdRJyb0wcGDFpeo=" ; 
    };
    
    nativeBuildInputs =
      oldAttr.nativeBuildInputs
      ++ (with pkgs; [
        ant
        openjdk
        python3
        python3Packages.numpy
      ]);
      
    cmakeFlags =
      oldAttr.cmakeFlags
      ++ [
        "-DBUILD_JAVA=ON"
        "-DBUILD_opencv_dnn=OFF"
      ];
  });
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    openjdk17
    cmake
    opencv4100
    clang
    rsync
  ];
  
  shellHook = ''
    export LD_LIBRARY_PATH=${opencv4100}/share/java/opencv4:$LD_LIBRARY_PATH
  '';
}