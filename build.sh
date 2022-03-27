eo install clang spirv-tools spirv-cross \
 && genie gmake \
 && make -j$(nproc) -C build config=release
