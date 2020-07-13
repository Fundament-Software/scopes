About Scopes
============

Scopes is a general purpose programming language and compiler infrastructure
specifically suited for short turnaround prototyping and development of high
performance applications in need of multi-stage compilation at runtime.

The project was started as an alternative to C++ for programming computer games
and related tools, specifically for solutions that depend heavily on live
procedural generation or aim to provide good modding support.

The compiler is written in about 30k lines of C++ code, supports a LLVM
as well as a SPIR-V backend (targeting both CPU and GPU with a single codebase),
and exports a minimal runtime environment. The remaining features are
bootstrapped from within the language.

The language is expression-based, but primarily imperative. The syntactical
style marries concepts from Scheme and Python, describing source code with
S-expressions but delimiting blocks by indentation rather than braces. Closures
are supported as a zero-cost abstraction. The type system is strongly statically
typed but fully inferred, therefore every function is a template. Both nominal
and structural typing are supported. Type primitives roughly match C level,
but are aimed to be expandable without limitations. The memory model is
compatible to C/C++ and supports simple unmanaged stack and heap memory, as well
as "view propagation", a declaration free variant of borrow checking
specifically designed for Scopes. A custom lightweight exception protocol
that is fully compatible with C is supported.

Scopes provides many metaprogramming facilities such as programmable
syntax sugar and AST macros, metadata necessary for basic step-by-step debugging
as well as inspection of types, constants, intermediate code, optimized output
and disassembly. The environment is suitable for development of domain specific
languages to describe configuration files, user interfaces, state machines or
processing graphs.

Scopes embeds the clang compiler infrastructure and is therefore fully C
compatible. C libraries can be imported and executed at compile time and
runtime without overhead and without requiring special bindings.

The Scopes Compiler Intermediate Language is suitable for painless translation
to SSA forms such as LLVM IR and SPIR-V, of which both are supported.
The SPIR-V backend also emits GLSL shader code on the fly.

The Scopes compiler fundamentally differs from C++ and other traditional AOT
(ahead of time) compilers, in that the compiler is designed to remain on-line
at runtime so that functions can be recompiled when the need arises, and
generated machine code can adapt to the instruction set present on the target
machine. This also diminishes the need for a build system. Still, Scopes is
**not** a JIT compiler. Compilation is always explicitly initiated by the user.
