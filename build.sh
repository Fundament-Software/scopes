# g++, make and cmake must be installed on this machine
./bin/eo install -y genie clang spirv-tools spirv-cross \
 && ./bin/eo sync \
 && ./bin/genie gmake \
 && make -j$(nproc) -C build config=release
