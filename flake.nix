{
  description = "Scopes retargetable programming language & infrastructure";

  inputs = {
    fenix.url = "github:nix-community/fenix";
    flake-utils.url = "github:numtide/flake-utils";
    genie-src.flake = false;
    genie-src.url = "github:bkaradzic/genie";
    naersk.url = "github:nix-community/naersk";
    naersk.inputs.nixpkgs.follows = "fenix/nixpkgs";
    nix-filter.url = "github:numtide/nix-filter";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-22.05";
    spirv-cross-src.flake = false;
    spirv-cross-src.url = "github:KhronosGroup/SPIRV-Cross";
  };

  outputs =
    { self, fenix, flake-utils, genie-src, naersk, nix-filter, nixpkgs, spirv-cross-src }:
    let
      inherit (self) packages checks defaultPackage templates;
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
      fenixToolchainFor = forAllSystems (system: fenix.packages.${system}.fromToolchainFile {
        file = ./rust-toolchain.toml;
        sha256 = "1dlf1r9b9h1r00nqgsj5xisjn8va6aragivj0wv6iqij7lh7wz19";
      });
    in {
      packages = forAllSystems (system:
        let
          fenixToolchainHost = fenixToolchainFor.${system};
          packagesHost = packages.${system};
          pkgs = nixpkgsFor.${system};

          llvmPackages = pkgs.llvmPackages_13;
          naersk' = pkgs.callPackage naersk {
            cargo = fenixToolchainHost;
            rustc = fenixToolchainHost;
          };
        in {
          genie = pkgs.stdenv.mkDerivation {
            name = "genie";
            src = genie-src;
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
                  name = "scopesWithLibraries";
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
                    llvmPackages.clang
                    llvmPackages.libclang
                    llvmPackages.llvm.dev
                    # llvmPackages.llvm-polly
                    pkgs.spirv-tools
                    packagesHost.genie
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
                    NIX_CFLAGS_COMPILE+=\ -DSCOPES_ADD_IMPORT_CFLAGS=\"-isystem!${llvmPackages.clang}/resource-root/include/!-isystem!${
                      nixpkgs.lib.getDev pkgs.stdenv.cc.libc
                    }/include/\"

                    echo $NIX_CFLAGS_COMPILE

                    # echo make $makeFlags scopes
                    make -j$NIX_BUILD_CORES $makeFlags
                    # false
                  '';

                  installPhase = ''
                    install -D --target-directory="$out/bin" bin/scopes
                    # wrapProgram $out/bin/scopes --suffix NIX_CFLAGS_COMPILE "" " -isystem ${llvmPackages.clang}/resource-root/include/ -isystem ${
                      nixpkgs.lib.getDev pkgs.stdenv.cc.libc
                    }/include/"
                    install -D --target-directory="$out/lib" bin/libscopesrt.so
                    # cp -r ./lib/scopes $out/lib/scopes
                    # echo ${llvmPackages.clang} >> $out/clangpath
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
          [ packagesHost.scopesStdlib ];
          scopesAllTargets = (packagesHost.scopes.withTargets [
            "native"
            "webassembly"
            "aarch64"
            "riscv"
          ]).withLibraries [ packagesHost.scopesStdlib ];

          scopes-rust = naersk'.buildPackage {
            src = ./.;
          };
        });

      checks = forAllSystems (system: {
        build = defaultPackage.${system};
        unit-tests = nixpkgsFor.${system}.runCommandNoCC "unit-tests" {
          buildInputs = [ packages.${system}.scopes ];
          testDir = ./testing;
        }
          "cp -r $testDir ./testing && SCOPES_CACHE=./scopes-cache scopes testing/test_all.sc && touch $out";
      });

      defaultPackage = forAllSystems (system: packages.${system}.scopes);

      devShell = forAllSystems (system: let
        fenixPackagesHost = fenix.packages.${system};
        fenixToolchainHost = fenixToolchainFor.${system};
        nixpkgsHost = nixpkgsFor.${system};
        packagesHost = packages.${system};
      in nixpkgsHost.mkShell {
        inputsFrom = [
          packagesHost.scopes
          packagesHost.scopes-rust
        ];
        nativeBuildInputs = [
          fenixPackagesHost.rust-analyzer
          fenixToolchainHost
          nixpkgsHost.clang-tools
        ];
      });

      templates = {
        minimal = {
          description =
            "a minimal scopes project that pulls in scopes as a dep but doesn't define any packages";
          path = ./nix/templates/minimal;
        };
      };
    };

  nixConfig = {
    extra-trusted-public-keys = "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs=";
    extra-substituters = "https://nix-community.cachix.org";
  };
}
