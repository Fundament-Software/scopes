

using import C.stdio
using import C.stdlib
using import C.string

# some library functions we're going to export

fn combine_strings (dest a b)
    strcpy dest a
    strcat dest " "
    strcat dest b

fn print_string (s)
    printf "%s\n" s
    () # return nothing

# build the object

let test_genso_object_path = (module-dir .. "/libtest_genso.o")
compile-object
    default-target-triple
    compiler-file-kind-object
    test_genso_object_path
    'bind-symbols (Scope)
        # instantiate functions with type signature
        combine_strings =
            imply combine_strings
                (@ (function (mutable rawstring)
                    (mutable rawstring) rawstring rawstring))
        print_string =
            imply print_string (@ (function void rawstring))
    #'no-debug-info
    'dump-module

# build a shared library
let test_genso_so_path = (module-dir .. "/libtest_genso.so")
let err = (system (report
    (.. "gcc -o " test_genso_so_path " -shared " test_genso_object_path)))
assert (err == 0)

# write test client code to file
let test_genso.c =
    """"char *combine_strings (char *, char *, const char *);
        void print_string (const char *);

        int main (int argc, char **argv) {
            char buf[1024];
            combine_strings(buf, "hello", "world");
            print_string(buf);
            return 0;
        }
let test_genso_path = (module-dir .. "/test_genso.c")
let f = (fopen test_genso_path "wb")
fwrite (test_genso.c as rawstring) 1 (countof test_genso.c) f
fclose f

# compile binary using our shared library
let test_genso_bin_path = (module-dir .. "/test_genso")
let err = (system
    (report (.. "gcc " test_genso_path
        " -L" module-dir " -ltest_genso -o " test_genso_bin_path)))
assert (err == 0)
# make binary executable
system (.. "chmod +x " test_genso_bin_path)
# execute binary
let err = (system "./test_genso")
assert (err == 0)
