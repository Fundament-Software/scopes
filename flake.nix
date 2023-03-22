{
  description = "Scopes retargetable programming language & infrastructure";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-22.11";
    genie-src.url = "github:bkaradzic/genie";
    genie-src.flake = false;
    spirv-cross-src.url = "github:KhronosGroup/SPIRV-Cross";
    spirv-cross-src.flake = false;
    nix-filter.url = "github:numtide/nix-filter";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { self, nixpkgs, genie-src, spirv-cross-src, nix-filter, flake-utils }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });

    in {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
          selfpkgs = self.packages.${system};
          llvmpkgs = pkgs.llvmPackages_13;
        in {

          genie = pkgs.stdenv.mkDerivation {
            name = "genie";
            src = genie-src;
            # Remove x86_64 specific m64 if not on that system
            # see https://github.com/bkaradzic/GENie/issues/425
            prePatch = pkgs.lib.optionalString (!pkgs.stdenv.isx86_64) ''
              find . -name '*.make' -a -exec sed -i 's/-m64//g' build/gmake.linux/genie.make {} +
            '';
            installPhase = ''
              install -D --target-directory="$out/bin" bin/linux/genie
            '';
          };

          scopesStdlib = pkgs.runCommandNoCC "stdlib" { } ''
            mkdir --parents $out/lib/scopes
            cp -r ${./lib/scopes}/* $out/lib/scopes/
          '';

          # mkScopesLibrary = { name, src, postBuild ? "" }@args:
          #   pkgs.runCommandNoCC name (args // { inherit postBuild; }) ''
          #     mkdir --parents $out/lib/scopes/packages/${name}/
          #     cp -r ${src}/ $out/lib/scopes/packages/${name}/
          #   '';

          scopes = let
            withLibraries = scopes: libs:
              let
                newscopes = pkgs.symlinkJoin {
                  name = "scopes";
                  paths = [ scopes ] ++ libs;
                  postBuild = ''
                    rm $out/bin/scopes
                    cp ${scopes}/bin/scopes $out/bin/scopes
                  ''; # scopes looks for libraries relative to the absolute path of the entry point.
                  passthru = {
                    nolibs = scopes.nolibs;
                    boundLibs = scopes.boundlibs ++ libs;
                    withTargets = targets:
                      withLibraries (withTargets targets)
                      (scopes.boundLibs ++ libs);
                    withLibraries = libs2: withLibraries newscopes libs2;
                  };
                };
              in newscopes;
            withTargets = targets:
              let
                scopes-nolibs = pkgs.stdenv.mkDerivation {
                  pname = "scopes";
                  version = "0.18";
                  src = nix-filter.lib.filter {
                    root = ./.;
                    include = [
                      (nix-filter.lib.inDirectory "src")
                      (nix-filter.lib.inDirectory "external")
                      (nix-filter.lib.inDirectory "include")
                      "genie.lua"
                      # (nix-filter.lib.inDirectory "lib")
                    ];
                  };
                  enableParallelBuilding = true;

                  buildInputs = [
                    llvmpkgs.clang
                    llvmpkgs.libclang
                    llvmpkgs.llvm.dev
                    # llvmpkgs.llvm-polly
                    pkgs.spirv-tools
                    selfpkgs.genie
                    pkgs.makeWrapper
                    pkgs.abseil-cpp
                  ];

                  SCOPES_TARGETS = targets;

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

                  buildPhase = ''
                    NIX_CFLAGS_COMPILE+=\ -DSCOPES_ADD_IMPORT_CFLAGS=\"-isystem!${llvmpkgs.clang}/resource-root/include/!-isystem!${
                      nixpkgs.lib.getDev pkgs.stdenv.cc.libc
                    }/include/\"

                    echo $NIX_CFLAGS_COMPILE

                    # echo make $makeFlags scopes
                    make -j$NIX_BUILD_CORES $makeFlags
                    # false
                  '';

                  installPhase = ''
                    install -D --target-directory="$out/bin" bin/scopes
                    # wrapProgram $out/bin/scopes --suffix NIX_CFLAGS_COMPILE "" " -isystem ${llvmpkgs.clang}/resource-root/include/ -isystem ${
                      nixpkgs.lib.getDev pkgs.stdenv.cc.libc
                    }/include/"
                    install -D --target-directory="$out/lib" bin/libscopesrt.so
                    # cp -r ./lib/scopes $out/lib/scopes
                    # echo ${llvmpkgs.clang} >> $out/clangpath
                    # mkdir -p $out
                    # cp -r ./ $out/builddump
                  '';

                  checkInputs = [ pkgs.glibc.dev ];

                  checkPhase = ''
                    SCOPES_CACHE=./scopes_cache bin/scopes testing/test_all.sc
                  '';
                  doCheck = false;

                  passthru = {
                    inherit withTargets;
                    nolibs = scopes-nolibs;
                    boundLibs = [ ];
                    withLibraries = withLibraries scopes-nolibs;
                  };
                };
              in scopes-nolibs;
          in (withTargets [ "native" "webassembly" ]).withLibraries
          [ selfpkgs.scopesStdlib ];
          scopesAllTargets = (selfpkgs.scopes.withTargets [
            "native"
            "webassembly"
            "aarch64"
            "riscv"
          ]).withLibraries [ selfpkgs.scopesStdlib ];

        });

      checks = forAllSystems (system: {
        build = self.defaultPackage.${system};
        unit-tests = nixpkgsFor.${system}.runCommandNoCC "unit-tests" {
          buildInputs = [ self.packages.${system}.scopes ];
          testDir = ./testing;
        }
          "cp -r $testDir ./testing && SCOPES_CACHE=./scopes-cache scopes testing/test_all.sc && touch $out";
      });

      defaultPackage = forAllSystems (system: self.packages.${system}.scopes);

      templates = {
        minimal = {
          description =
            "a minimal scopes project that pulls in scopes as a dep but doesn't define any packages";
          path = ./nix/templates/minimal;
        };
      };
    };
}
