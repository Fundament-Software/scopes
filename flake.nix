{
  description = "Scopes retargetable programming language & infrastructure";

  inputs = {
    genie-src.url = "github:bkaradzic/genie";
    genie-src.flake = false;
    spirv-cross-src.url = "github:KhronosGroup/SPIRV-Cross";
    spirv-cross-src.flake = false;
  };

  outputs = { self, nixpkgs, genie-src, spirv-cross-src }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });

    in {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
          selfpkgs = self.packages.${system};
          llvmpkgs = pkgs.llvmPackages_12;
        in {

          genie = pkgs.stdenv.mkDerivation {
            name = "genie";
            src = genie-src;
            installPhase = ''
              install -D --target-directory="$out/bin" bin/linux/genie
            '';
          };

          scopes = pkgs.stdenv.mkDerivation {
            pname = "scopes";
            version = "0.16";
            src = ./.;
            enableParallelBuilding = true;

            buildInputs = [
              llvmpkgs.clang
              llvmpkgs.libclang
              llvmpkgs.llvm.dev
              # llvmpkgs.llvm-polly
              pkgs.spirv-tools
              selfpkgs.genie
            ];

            configurePhase = ''
              # Only source is needed of spirv-cross
              ln --symbolic ${spirv-cross-src} SPIRV-Cross

              # Pretend that we built spirv-tools
              mkdir --parents SPIRV-Tools/build/source/opt
              ln --symbolic --target-directory=SPIRV-Tools/build/source     ${pkgs.spirv-tools}/lib/libSPIRV-Tools.a
              ln --symbolic --target-directory=SPIRV-Tools/build/source/opt ${pkgs.spirv-tools}/lib/libSPIRV-Tools-opt.a

              genie gmake
            '';

            makeFlags = [ "-C build" "config=release" ];

            installPhase = ''
              install -D --target-directory="$out/bin" bin/scopes
              install -D --target-directory="$out/lib" bin/libscopesrt.so
              cp -r ./lib/scopes $out/lib/scopes
              cp -r ./ $out/builddump
            '';

            checkPhase = ''
              bin/scopes testing/test_all.sc
            '';
            doCheck = false;
          };

        });

      defaultPackage = forAllSystems (system: self.packages.${system}.scopes);
    };
}
