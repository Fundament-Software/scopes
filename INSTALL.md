# Environment Setup for Windows

## Scopes / VCPKG install instructions

1. `git clone https://github.com/Fundament-Software/vcpkg`
2. `cd vcpkg`
3. `.\bootstrap-vcpkg.bat` build VCPKG.
4. `.\vcpkg.exe install llvm[clang,enable-zlib,target-webassembly,target-x86,target-aarch64,target-riscv,polly,disable-assertions,disable-clang-static-analyzer,enable-bindings,enable-terminfo]:x64-windows-static` targeted install script for LLVM (this should take ~1 hour).
5. `./vcpkg install spirv-cross:x64-windows-static spirv-headers:x64-windows-static spirv-tools:x64-windows-static zlib:x64-windows-static abseil:x64-windows-static` the other deps.
6. Go into the scopes project root, make a directory called build (or whatever you want, really), enter the directory, and run: `cmake .. -DCMAKE_TOOLCHAIN_FILE=[VCPKG LOCATION HERE]\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static`

This will produce a visual studio solution you can then open and build scopes with.
`(IMPORTANT: do NOT attempt to run scopes in debug mode. It doesn't work, because scopes uses invalid LLVM IR that triggers assertions. Somehow the invalid LLVM IR compiles to the correct machine instructions on x86-64... for now. I haven't figured out a way to get rid of it all, yet. Use RelWithDebInfo and disable optimizations to debug scopes.)`
