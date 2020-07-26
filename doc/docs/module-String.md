<style type="text/css" rel="stylesheet">body { counter-reset: chapter 23; }</style>

String
======

Provides a string type that manages a mutable byte buffer of varying size
on the heap. Strings are guaranteed to be zero-terminated.

*type*{.property} `String`{.descname} [](#scopes.type.String "Permalink to this definition"){.headerlink} {#scopes.type.String}

:   An unique type labeled `<GrowingString i8>` of supertype `GrowingString` and of storage type `(tuple (_items = (mutable@ i8)) (_count = usize) (_capacity = usize))`.

