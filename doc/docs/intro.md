<style type="text/css" rel="stylesheet">
body { counter-reset: chapter 2; }
</style>

Getting Started
===============

Installation
------------

You can either download a binary distribution of Scopes from the
[website](http://scopes.rocks) or build Scopes from source.

The main repository for scopes is on
[sourcehut](https://hg.sr.ht/~duangle/scopes).

Note: any reference to `scopes-repo` in the instructions refers to wherever
you have checked out the Scopes source tree.

### Building Scopes on Windows ###

Scopes only supports the `mingw64` toolchain for the foreseeable future.

* Install [MSYS2](http://msys2.github.io) and
  [install](https://github.com/valtron/llvm-stuff/wiki/Build-LLVM-with-MSYS2)
  clang, LLVM 10.0.x, polly, cmake and make for `x86_64`. The packages are
  named `mingw64/mingw-w64-x86_64-llvm`, `mingw64/mingw-w64-x86_64-clang`,
  `mingw64/mingw-w64-x86_64-cmake`, `mingw64/mingw-w64-x86_64-polly`
  and `make`.
* Nice to have: `mingw-w64-x86_64-gdb`
* at least `usr/bin` from your MSYS2 installation must be added to the
  `PATH` variable so that the buildscript can find MSYS2.
* You also need the latest source distributions of
  [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) and
  [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) checked out
  into the workspace folder.
* Lastly, you need a build of [GENie](https://github.com/bkaradzic/GENie)
  (binaries available on the page).
* Check `SPIRV-Tools`' build instructions to verify that its dependency on
  `SPIRV-Headers` is satisfied, and all other dependencies are up-to-date.
  Build `SPIRV-Tools` using
  `mkdir build && cd build && cmake .. -G "MSYS Makefiles"
  -DCMAKE_BUILD_TYPE=Release && make` in `scopes-repo/SPIRV-Tools/build`.
* `SPIRV-Cross` does not have to be built.
* In the workspace folder, run `genie gmake` once to generate the project
  Makefiles.
* To build in debug mode, run `make -C build`. For release mode, use
  `make -C build config=release`. Use the option `-j4` to speed up the
  build on a multicore machine, where `4` is a sensible number of CPU
  hardware threads to use.
* There should now be a `scopes.exe` executable in the `bin` folder.
* For the clang bridge to work properly, copy
  `clang/lib/clang/10.0.x/include` to `scopes-repo/lib/clang/include`.
* For a fresh rebuild, just remove the `build` directory before running
  `make` again.

### Building Scopes on Linux ###

* You need build-essentials, clang, libclang and LLVM 10.0.x installed -
  preferably locally:
* Put `llvm-config` in your `$PATH`.
* Alternatively, provide your own clang distribution and symlink it to
  `scopes-repo/clang`.
* You also need the latest source distributions of
  [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) and
  [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) checked out or
  symlinked into the workspace folder.
* Lastly, you need a build of [GENie](https://github.com/bkaradzic/GENie)
  (binaries available on the page).
* Check `SPIRV-Tools`' build instructions to verify that its dependency on
  `SPIRV-Headers` is satisfied, and all other dependencies are up-to-date.
  Build `SPIRV-Tools` using
  `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make`
  in `scopes-repo/SPIRV-Tools/build`.
* `SPIRV-Cross` does not have to be built.
* In the workspace folder, run `genie gmake` once to generate the project
  Makefiles.
* To build in debug mode, run `make -C build`. For release mode, use
  `make -C build config=release`. Use the option `-j4` to speed up the
  build on a multicore machine, where `4` is a sensible number of CPU
  hardware threads to use.
* There should now be a `scopes` executable in the `bin` folder.
* For the clang bridge to work properly, copy or symlink
  `$(llvm-config --prefix)/lib/clang/$(llvm-config --version)/include` to
  `scopes-repo/lib/clang/include`.
* For a fresh rebuild, just remove the `build` directory before running
  `make` again.

### Building Scopes on macOS ###

* Scopes builds on macOS Mojave (10.14) using LLVM 10.0.
* You'll need the following packages from [brew](https://brew.sh/): `llvm`
  and `cmake`. Scopes' build system respects `brew`'s standard installation
  paths.
* Alternatively, provide your own clang distribution and symlink it to
  `scopes-repo/clang`.
* Put `llvm-config` in your `$PATH`. Find it in `$(brew --prefix llvm)/bin`
  if using brew.
* You'll also need an installation of the Xcode Command Line Tools:
  `xcode-select --install`.
* You may also need to force installation of the macOS SDK headers:
  Open `macOS_SDK_headers_for_macOS_10.14.pkg` found in
  `/Library/Developer/CommandLineTools/Packages`
* You also need the latest source distributions of
  [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) and
  [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) checked out or
  symlinked into the workspace folder.
* Lastly, you need a build of [GENie](https://github.com/bkaradzic/GENie)
  (binaries available on the page).
* Check `SPIRV-Tools`' build instructions to verify that its dependency on
  `SPIRV-Headers` is satisfied, and all other dependencies are up-to-date.
  Build `SPIRV-Tools` using
  `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make`
  in `scopes-repo/SPIRV-Tools/build`.
* `SPIRV-Cross` does not have to be built.
* In the workspace folder, run `genie gmake` once to generate the project
  Makefiles.
* To build in debug mode, run `make -C build`. For release mode, use
  `make -C build config=release`. Use the option `-j4` to speed up the
  build on a multicore machine, where `4` is a sensible number of CPU
  hardware threads to use.
* There should now be a `scopes` executable in the `bin` folder.
* For the clang bridge to work properly, copy or symlink
  `$(llvm-config --prefix)/lib/clang/$(llvm-config --version)/include` to
  `scopes-repo/lib/clang/include`.
* For a fresh rebuild, just remove the `build` directory before running
  `make` again.