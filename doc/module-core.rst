globals
=======

These names are bound in every fresh module and main program by default.
Essential symbols are created by the compiler, and subsequent utility
functions, macros and types are defined and documented in `core.sc`.

The core module implements the remaining standard functions and macros,
parses the command-line and optionally enters the REPL.

.. define:: args
.. define:: compile-flag-O1
.. define:: compile-flag-O2
.. define:: compile-flag-O3
.. define:: compile-flag-dump-disassembly
.. define:: compile-flag-dump-function
.. define:: compile-flag-dump-module
.. define:: compile-flag-dump-time
.. define:: compile-flag-no-debug-info
.. define:: compiler-dir
.. define:: compiler-path
.. define:: compiler-timestamp
.. define:: debug-build?
.. define:: e
   
   Euler's number, also known as Napier's constant. Explicitly type-annotated
   versions of the constant are available as `e:f32` and `e:f64`
.. define:: e:f32
.. define:: e:f64
.. define:: eol
.. define:: false
.. define:: none
.. define:: null
   
   A pointer constant of type `NullType` that is always zero and casts to
   any pointer type.
.. define:: operating-system
.. define:: package
.. define:: pi
   
   The number π, the ratio of a circle's circumference C to its diameter d.
   Explicitly type-annotated versions of the constant are available as `pi:f32`
   and `pi:f64`.
.. define:: pi:f32
.. define:: pi:f64
.. define:: pointer-flag-non-readable
.. define:: pointer-flag-non-writable
.. define:: ref-attribs-key
.. define:: style-comment
.. define:: style-error
.. define:: style-function
.. define:: style-instruction
.. define:: style-keyword
.. define:: style-location
.. define:: style-none
.. define:: style-number
.. define:: style-operator
.. define:: style-sfxfunction
.. define:: style-string
.. define:: style-symbol
.. define:: style-type
.. define:: style-warning
.. define:: tmp
.. define:: true
.. define:: type-kind-array
.. define:: type-kind-extern
.. define:: type-kind-function
.. define:: type-kind-image
.. define:: type-kind-integer
.. define:: type-kind-pointer
.. define:: type-kind-real
.. define:: type-kind-return-label
.. define:: type-kind-sampled-image
.. define:: type-kind-tuple
.. define:: type-kind-typename
.. define:: type-kind-union
.. define:: type-kind-vector
.. define:: unnamed
.. define:: unroll-limit
.. type:: Anchor
.. type:: Any
.. typefn:: (Any '__typecall cls value)
.. typefn:: (Any 'typeof val)
.. reftypefn:: (Any 'typeof self)
.. type:: Builtin
.. type:: CEnum
.. type:: CStruct
.. typefn:: (CStruct '__typecall cls args...)
.. typefn:: (CStruct 'structof cls args...)
.. reftypefn:: (CStruct '__new self args...)
.. type:: CUnion
.. typefn:: (CUnion '__typecall cls)
.. reftypefn:: (CUnion '__new self)
.. type:: Capture
.. type:: Closure
.. type:: Exception
.. type:: Frame
.. type:: FunctionMemory
.. typefn:: (FunctionMemory 'free-array cls value count)
.. typefn:: (FunctionMemory 'allocate-array cls T count)
.. typefn:: (FunctionMemory 'free cls value)
.. typefn:: (FunctionMemory 'allocate cls T)
.. type:: Generator
.. typefn:: (Generator '__call self)
.. typefn:: (Generator '__typecall cls iter init)
.. type:: GlobalMemory
.. typefn:: (GlobalMemory 'free-array cls value count)
.. typefn:: (GlobalMemory 'allocate-array cls T count)
.. typefn:: (GlobalMemory 'free cls value)
.. typefn:: (GlobalMemory 'allocate cls T)
.. type:: HeapMemory
.. typefn:: (HeapMemory 'free-array cls value count)
.. typefn:: (HeapMemory 'allocate-array cls T count)
.. typefn:: (HeapMemory 'free cls value)
.. typefn:: (HeapMemory 'allocate cls T)
.. type:: Image
.. type:: Label
.. typefn:: (Label 'parameters self)
.. type:: Macro
.. typefn:: (Macro '__call self at next scope)
.. typefn:: (Macro '__typecall cls f)
.. type:: Memory
.. typefn:: (Memory 'new cls T args...)
.. typefn:: (Memory 'delete cls value)
.. typefn:: (Memory 'copy cls value)
.. typefn:: (Memory '__typecall cls T args...)
.. type:: Nothing
.. type:: NullType
   
   The type of the `null` constant. This type is uninstantiable.
.. type:: Parameter
.. typefn:: (Parameter 'return-label? self)
.. typefn:: (Parameter '__typecall cls params...)
.. type:: ReturnLabel
.. typefn:: (ReturnLabel '__typecall cls ...)
.. type:: SampledImage
.. type:: Sampler
.. type:: Scope
.. typefn:: (Scope '__typecall cls parent clone)
.. type:: SourceFile
.. type:: Symbol
.. typefn:: (Symbol '__call name self ...)
.. typefn:: (Symbol '__typecall cls value)
.. reftypefn:: (Symbol '__new self args...)
.. type:: Syntax
.. type:: Unknown
.. type:: aggregate
.. type:: array
.. typefn:: (array '__typecall cls ...)
.. reftypefn:: (array '__new self)
.. type:: bool
.. typefn:: (bool '__typecall destT val)
.. reftypefn:: (bool '__new self args...)
.. type:: constant
.. type:: exception-pad-type
.. reftypefn:: (exception-pad-type '__new self)
.. type:: extern
.. typefn:: (extern '__call self ...)
.. typefn:: (extern '__typecall cls ...)
.. type:: f16
.. reftypefn:: (f16 '__new self args...)
.. type:: f32
.. typefn:: (f32 '__typecall destT val)
.. reftypefn:: (f32 '__new self args...)
.. type:: f64
.. typefn:: (f64 '__typecall destT val)
.. reftypefn:: (f64 '__new self args...)
.. type:: f80
.. reftypefn:: (f80 '__new self args...)
.. type:: function
.. typefn:: (function '__typecall cls ...)
.. type:: hash
.. typefn:: (hash '__typecall cls values...)
.. type:: i16
.. typefn:: (i16 '__typecall destT val)
.. reftypefn:: (i16 '__new self args...)
.. type:: i32
.. typefn:: (i32 '__typecall destT val)
.. reftypefn:: (i32 '__new self args...)
.. type:: i64
.. typefn:: (i64 '__typecall destT val)
.. reftypefn:: (i64 '__new self args...)
.. type:: i8
.. typefn:: (i8 '__typecall destT val)
.. reftypefn:: (i8 '__new self args...)
.. type:: immutable
.. reftypefn:: (immutable '__new self args...)
.. type:: integer
.. typefn:: (integer '__typecall cls ...)
.. reftypefn:: (integer '__new self args...)
.. type:: list
.. typefn:: (list '__typecall cls ...)
.. type:: opaquepointer
.. type:: pointer
.. typefn:: (pointer 'immutable cls ET)
.. typefn:: (pointer 'set-storage cls storage)
.. typefn:: (pointer '__typecall cls T opt)
.. typefn:: (pointer 'set-element-type cls ET)
.. typefn:: (pointer 'strip-storage cls ET)
.. typefn:: (pointer 'writable? cls)
.. typefn:: (pointer 'storage cls)
.. typefn:: (pointer 'readable? cls)
.. typefn:: (pointer 'mutable cls ET)
.. reftypefn:: (pointer '__new self args...)
.. type:: rawstring
.. reftypefn:: (rawstring '__new self args...)
.. type:: real
.. reftypefn:: (real '__new self args...)
.. type:: ref
.. typefn:: (ref '__typecall cls T)
.. typefn:: (ref '__call self args...)
.. type:: string
.. typefn:: (string 'from-cstr value)
.. type:: tuple
.. typefn:: (tuple '__typecall cls ...)
.. reftypefn:: (tuple '__new self)
.. type:: type
.. typefn:: (type '__call cls ...)
.. type:: typename
.. typefn:: (typename 'elements self)
.. typefn:: (typename 'symbols self)
.. typefn:: (typename '__typecall cls args...)
.. type:: u16
.. typefn:: (u16 '__typecall destT val)
.. reftypefn:: (u16 '__new self args...)
.. type:: u32
.. typefn:: (u32 '__typecall destT val)
.. reftypefn:: (u32 '__new self args...)
.. type:: u64
.. typefn:: (u64 '__typecall destT val)
.. reftypefn:: (u64 '__new self args...)
.. type:: u8
.. typefn:: (u8 '__typecall destT val)
.. reftypefn:: (u8 '__new self args...)
.. type:: union
.. typefn:: (union '__typecall cls ...)
.. type:: usize
.. typefn:: (usize '__typecall destT val)
.. reftypefn:: (usize '__new self args...)
.. type:: vector
.. typefn:: (vector '__typecall cls ...)
.. reftypefn:: (vector '__new self args...)
.. type:: void
.. type:: voidstar
.. reftypefn:: (voidstar '__new self args...)
.. fn:: (% a b)
.. fn:: (& a b)
.. fn:: (* ...)
.. fn:: (+ ...)
.. fn:: (- a b)
.. fn:: (/ a b)
.. fn:: (< a b)
.. fn:: (= obj value)
.. fn:: (> a b)
.. fn:: (@ ...)
.. fn:: (^ a b)
.. fn:: (_ ...)
   
   A pass-through function that allows expressions to evaluate to multiple
   arguments.
.. fn:: (| ...)
.. fn:: (~ x)
.. fn:: (!= a b)
.. fn:: (%= x y)
.. fn:: (&= x y)
.. fn:: (*= x y)
.. fn:: (+= x y)
.. fn:: (-= x y)
.. fn:: (.. ...)
.. fn:: (// a b)
.. fn:: (//= x y)
.. fn:: (/= x y)
.. fn:: (<< a b)
.. fn:: (<<= x y)
.. fn:: (<= a b)
.. fn:: (== a b)
.. fn:: (>= a b)
.. fn:: (>> a b)
.. fn:: (>>= x y)
.. fn:: (^= x y)
.. fn:: (|= x y)
.. fn:: (Anchor-column x)
.. fn:: (Anchor-file x)
.. fn:: (Anchor-lineno x)
.. fn:: (Any-extract val T)
.. fn:: (Any-list? val)
.. fn:: (Any-new val)
.. fn:: (Any-payload val)
.. fn:: (Any-typeof val)
.. fn:: (CStruct->tuple self)
.. fn:: (Exception-anchor sx)
.. fn:: (Exception-message sx)
.. fn:: (Symbol? val)
.. fn:: (Syntax->datum sx)
.. fn:: (Syntax-anchor sx)
.. fn:: (Syntax-quoted? sx)
.. fn:: (abs x)
.. fn:: (all? v)
.. fn:: (any? v)
.. fn:: (array-type? T)
.. fn:: (array? val)
.. fn:: (arrayof T ...)
.. fn:: (as value dest-type)
.. fn:: (assert-type T)
.. fn:: (assert-typeof a T)
.. fn:: (block-scope-macro f)
.. fn:: (chain-fn-dispatch ...)
.. fn:: (chain-fn-dispatch2 f1 f2)
.. fn:: (char s)
.. fn:: (clamp x mn mx)
.. fn:: (clone-scope-symbols source target)
.. fn:: (compile f opts...)
.. fn:: (compile-flags opts...)
.. fn:: (compile-glsl f target opts...)
.. fn:: (compile-object path table opts...)
.. fn:: (compile-spirv f target opts...)
.. fn:: (cond-const a b)
.. fn:: (cons ...)
.. fn:: (construct value args...)
   
   Invokes the constructor for `value` of reference-like type,
   passing along optional argument set `args...`.
.. fn:: (construct-array n value args...)
   
   Invokes the constructor for an array `value` of reference-like type,
   assuming that value is a pointer to an array element, passing along
   optional argument set `args...`.
.. fn:: (copy-construct value source)
   
   Invokes the copy constructor for `value` of reference-like type if
   present, passing `source` as a value from which to copy.
   
   `source` does not have to be of reference type, but can also be of
   immutable element type.
.. fn:: (copy-construct-array n value source)
   
   Invokes the copy constructor for an array `value` of reference-like type,
   passing `source` as a value from which to copy.
   
   `source` has to be the first (referenced) element of an array too.
.. fn:: (countof x)
.. fn:: (decons val count)
.. fn:: (delete self)
   
   destructs and frees `value` of types that have the `__delete` method
   implemented. The free method must also invoke the destructor.
.. fn:: (deref values...)
.. fn:: (deref1 value)
.. fn:: (destruct value)
   
   Invokes the destructor for `value` of reference-like type.
.. fn:: (destruct-array n value)
   
   Invokes the destructor for an array `value` of reference-like type,
   assuming that value is a pointer to an array element.
.. fn:: (docstring f)
.. fn:: (empty? x)
.. fn:: (enumerate x)
.. fn:: (error! msg)
.. fn:: (extern-type? T)
.. fn:: (extern? val)
.. fn:: (fn-dispatch-error-handler msgf get-types...)
.. fn:: (fn-dispatcher args...)
.. fn:: (fold init gen f)
.. fn:: (format-exception exc)
.. fn:: (format-type-signature types...)
.. fn:: (forward-as value dest-type)
.. fn:: (forward-getattr self name)
.. fn:: (forward-hash value)
.. fn:: (forward-imply value dest-type)
.. fn:: (forward-repr value)
.. fn:: (forward-typeattr T name)
.. fn:: (function-pointer-type? T)
.. fn:: (function-pointer? val)
.. fn:: (function-type? T)
.. fn:: (gen-get-option opts...)
   
   Given a variadic list of keyed arguments, generate a function
   ``(get-option name default)`` that either returns an option with the
   given key from ``opts...`` or ``default`` if no such key exists.
   
   If ``default`` is a function, then the function will be evaluated
   and the result returned.
.. fn:: (gen-type-op2 f)
.. fn:: (getattr self name)
.. fn:: (imply value dest-type)
.. fn:: (integer-type? T)
.. fn:: (integer? val)
.. fn:: (list-at l)
.. fn:: (list-at-next l)
.. fn:: (list-countof l)
.. fn:: (list-empty? l)
.. fn:: (list-new ...)
.. fn:: (list-next l)
.. fn:: (list-reverse l tail)
.. fn:: (list? val)
.. fn:: (load-module module-name module-path opts...)
.. fn:: (local T args...)
.. fn:: (macro f)
.. fn:: (map x f)
   
   Maps function `f (skip values...)` to elements of iterable `x`.
   
   `skip` is a function that can be called to purge the active element
   from the output (allowing map to also act as a filter).
.. fn:: (max a b ...)
.. fn:: (maybe-unsyntax val)
.. fn:: (merge-scope-symbols source target filter)
.. fn:: (min a b ...)
.. fn:: (move-construct value source)
   
   Invokes the move constructor for `value` of reference-like type,
   passing `source` as the reference from which to move.
.. fn:: (move-construct-array n value source)
   
   Invokes the move constructor for an array of pointers `value`
   passing `source` as an array of pointers from which to move.
.. fn:: (new T args...)
.. fn:: (none? val)
.. fn:: (not x)
.. fn:: (op-prettyname symbol)
.. fn:: (op2-dispatch symbol)
.. fn:: (op2-dispatch-bidi symbol fallback)
.. fn:: (op2-ltr-multiop f)
.. fn:: (op2-rtl-multiop f)
.. fn:: (opN-dispatch symbol)
.. fn:: (pointer-each n op value args...)
.. fn:: (pointer-each2 n op value other)
.. fn:: (pointer-type-imply? src dest)
.. fn:: (pointer-type? T)
.. fn:: (pointer== a b)
.. fn:: (pointer? val)
.. fn:: (pow x y)
.. fn:: (powi base exponent)
.. fn:: (print ...)
.. fn:: (print-spaces depth)
.. fn:: (prompt prefix preload)
.. fn:: (raise! value)
.. fn:: (range a b c)
.. fn:: (real-type? T)
.. fn:: (real? val)
.. fn:: (repr value)
.. fn:: (require-from base-dir name)
.. fn:: (sabs x)
.. fn:: (scalar-type T)
.. fn:: (scope-macro f)
.. fn:: (select-op T sop fop)
.. fn:: (set-scope-symbol! scope sym value)
.. fn:: (set-type-symbol!& T name value)
.. fn:: (sign x)
.. fn:: (slice obj start-index end-index)
.. fn:: (static T args...)
.. fn:: (string->rawstring s)
.. fn:: (string-compare a b)
.. fn:: (string-countof s)
.. fn:: (string-repr val)
.. fn:: (supercall cls methodname self args...)
.. fn:: (syntax-error! anchor msg)
.. fn:: (tie-const a b)
.. fn:: (todo! msg)
.. fn:: (tuple-type? T)
.. fn:: (tuple? val)
.. fn:: (tupleof ...)
.. fn:: (type-matcher types...)
.. fn:: (type-mismatch-string want-T have-T)
.. fn:: (type< T superT)
.. fn:: (type<= T superT)
.. fn:: (type== a b)
.. fn:: (type? T)

   returns `true` if ``T`` is a value of type `type`, otherwise
   `false`.

.. fn:: (type@& T name)
.. fn:: (typeattr T name)
.. fn:: (typename-type? T)
.. fn:: (typename? val)
.. fn:: (typeof& self)
.. fn:: (typify f types...)
.. fn:: (unconst-all args...)
.. fn:: (unknownof T)
.. fn:: (unpack x)
.. fn:: (unroll-range a b c)
.. fn:: (va-each values...)
.. fn:: (va-each-reversed values...)
.. fn:: (va-empty? ...)
.. fn:: (va-join a...)
.. fn:: (va-types params...)
.. fn:: (vector-op2-dispatch symbol)
.. fn:: (vector-reduce f v)
.. fn:: (vector-signed-dispatch fsigned funsigned)
.. fn:: (vector-type? T)
.. fn:: (vector? T)
.. fn:: (vectorof T ...)
.. fn:: (walk-list on-leaf l depth)
.. fn:: (xpcall f errorf)
.. fn:: (zip a b)
.. macro:: (. ...)
.. macro:: (and ...)
.. macro:: (assert ...)
.. macro:: (breakable-block ...)
.. macro:: (capture ...)
.. macro:: (defer ...)
.. macro:: (define ...)
.. macro:: (define-block-scope-macro ...)
.. macro:: (define-doc ...)
.. macro:: (define-infix< ...)
.. macro:: (define-infix> ...)
.. macro:: (define-macro ...)
.. macro:: (define-scope-macro ...)
.. macro:: (del ...)
.. macro:: (enum ...)
.. macro:: (fn... ...)
.. macro:: (for ...)
.. macro:: (from ...)
.. macro:: (import ...)
.. macro:: (locals ...)
   
   export locals as a chain of two new scopes: a scope that contains
   all the constant values in the immediate scope, and a scope that contains
   the runtime values.
.. macro:: (loop ...)
.. macro:: (match ...)
.. macro:: (or ...)
.. macro:: (struct ...)
.. macro:: (typefn ...)
.. macro:: (typefn! ...)
.. macro:: (typefn!& ...)
.. macro:: (typefn& ...)
.. macro:: (using ...)
.. macro:: (while ...)
.. builtin:: (? ...)
.. builtin:: (Any-extract-constant ...)
.. builtin:: (Any-wrap ...)
.. builtin:: (Image-read ...)
.. builtin:: (Image-write ...)
.. builtin:: (ReturnLabel-type ...)
.. builtin:: (acos ...)
.. builtin:: (add ...)
.. builtin:: (add-nsw ...)
.. builtin:: (add-nuw ...)
.. builtin:: (alloca ...)
.. builtin:: (alloca-array ...)
.. builtin:: (alloca-exception-pad ...)
.. builtin:: (allocaof ...)
.. builtin:: (ashr ...)
.. builtin:: (asin ...)
.. builtin:: (atan ...)
.. builtin:: (atan2 ...)
.. builtin:: (band ...)
.. builtin:: (bitcast ...)
.. builtin:: (bor ...)
.. builtin:: (branch ...)
.. builtin:: (bxor ...)
.. builtin:: (call ...)
.. builtin:: (cc/call ...)
.. builtin:: (ceil ...)
.. builtin:: (compiler-anchor ...)
.. builtin:: (compiler-error! ...)
.. builtin:: (compiler-message ...)
.. builtin:: (constant? ...)
.. builtin:: (cos ...)
.. builtin:: (cross ...)
.. builtin:: (degrees ...)
.. builtin:: (delete-type-symbol! ...)
.. builtin:: (discard! ...)
.. builtin:: (distance ...)
.. builtin:: (do ...)
.. builtin:: (do-in ...)
.. builtin:: (dump ...)
.. builtin:: (exp ...)
.. builtin:: (exp2 ...)
.. builtin:: (extern-new ...)
.. builtin:: (extern-symbol ...)
.. builtin:: (extractelement ...)
.. builtin:: (extractvalue ...)
.. builtin:: (fabs ...)
.. builtin:: (fadd ...)
.. builtin:: (fcmp!=o ...)
.. builtin:: (fcmp!=u ...)
.. builtin:: (fcmp-ord ...)
.. builtin:: (fcmp-uno ...)
.. builtin:: (fcmp<=o ...)
.. builtin:: (fcmp<=u ...)
.. builtin:: (fcmp<o ...)
.. builtin:: (fcmp<u ...)
.. builtin:: (fcmp==o ...)
.. builtin:: (fcmp==u ...)
.. builtin:: (fcmp>=o ...)
.. builtin:: (fcmp>=u ...)
.. builtin:: (fcmp>o ...)
.. builtin:: (fcmp>u ...)
.. builtin:: (fdiv ...)
.. builtin:: (floor ...)
.. builtin:: (fma ...)
.. builtin:: (fmul ...)
.. builtin:: (fn ...)
.. builtin:: (fn! ...)
.. builtin:: (form-quote ...)
.. builtin:: (fpext ...)
.. builtin:: (fptosi ...)
.. builtin:: (fptoui ...)
.. builtin:: (fptrunc ...)
.. builtin:: (fract ...)
.. builtin:: (free ...)
.. builtin:: (frem ...)
.. builtin:: (frexp ...)
.. builtin:: (fsign ...)
.. builtin:: (fsub ...)
.. builtin:: (function-type ...)
.. builtin:: (getelementptr ...)
.. builtin:: (icmp!= ...)
.. builtin:: (icmp<=s ...)
.. builtin:: (icmp<=u ...)
.. builtin:: (icmp<s ...)
.. builtin:: (icmp<u ...)
.. builtin:: (icmp== ...)
.. builtin:: (icmp>=s ...)
.. builtin:: (icmp>=u ...)
.. builtin:: (icmp>s ...)
.. builtin:: (icmp>u ...)
.. builtin:: (if ...)
.. builtin:: (insertelement ...)
.. builtin:: (insertvalue ...)
.. builtin:: (inttoptr ...)
.. builtin:: (inversesqrt ...)
.. builtin:: (itrunc ...)
.. builtin:: (label ...)
.. builtin:: (ldexp ...)
.. builtin:: (length ...)
.. builtin:: (let ...)
.. builtin:: (load ...)
.. builtin:: (log ...)
.. builtin:: (log2 ...)
.. builtin:: (lshr ...)
.. builtin:: (malloc ...)
.. builtin:: (malloc-array ...)
.. builtin:: (mix ...)
.. builtin:: (mul ...)
.. builtin:: (mul-nsw ...)
.. builtin:: (mul-nuw ...)
.. builtin:: (normalize ...)
.. builtin:: (nullof ...)
.. builtin:: (offsetof ...)
.. builtin:: (powf ...)
.. builtin:: (ptrtoint ...)
.. builtin:: (purify ...)
.. builtin:: (quote ...)
.. builtin:: (radians ...)
.. builtin:: (rawcall ...)
.. builtin:: (round ...)
.. builtin:: (roundeven ...)
.. builtin:: (sample ...)
.. builtin:: (scopeof ...)
.. builtin:: (sdiv ...)
.. builtin:: (set-execution-mode! ...)
.. builtin:: (set-type-symbol! ...)
.. builtin:: (set-typename-storage! ...)
.. builtin:: (sext ...)
.. builtin:: (shl ...)
.. builtin:: (shufflevector ...)
.. builtin:: (sin ...)
.. builtin:: (sitofp ...)
.. builtin:: (smoothstep ...)
.. builtin:: (sqrt ...)
.. builtin:: (srem ...)
.. builtin:: (ssign ...)
.. builtin:: (static-alloc ...)
.. builtin:: (step ...)
.. builtin:: (store ...)
.. builtin:: (sub ...)
.. builtin:: (sub-nsw ...)
.. builtin:: (sub-nuw ...)
.. builtin:: (syntax-extend ...)
.. builtin:: (syntax-log ...)
.. builtin:: (tan ...)
.. builtin:: (trunc ...)
.. builtin:: (tuple-type ...)
.. builtin:: (type-local@ ...)
.. builtin:: (type@ ...)
.. builtin:: (typeof ...)
.. builtin:: (udiv ...)
.. builtin:: (uitofp ...)
.. builtin:: (unconst ...)
.. builtin:: (undef ...)
.. builtin:: (union-type ...)
.. builtin:: (unreachable! ...)
.. builtin:: (urem ...)
.. builtin:: (va-countof ...)
.. builtin:: (va-key ...)
.. builtin:: (va-keys ...)
.. builtin:: (va-values ...)
.. builtin:: (va@ ...)
.. builtin:: (volatile-load ...)
.. builtin:: (volatile-store ...)
.. builtin:: (zext ...)
.. compiledfn:: (Any-repr ...)

   ``λ(string)<~(Any)``
.. compiledfn:: (Any-string ...)

   ``λ(string)<~(Any)``
.. compiledfn:: (Any== ...)

   ``λ(bool)<~(Any Any)``
.. compiledfn:: (Closure-frame ...)

   ``λ(Frame)<~(Closure)``
.. compiledfn:: (Closure-label ...)

   ``λ(Label)<~(Closure)``
.. compiledfn:: (Image-type ...)

   ``λ(type)<~(type Symbol i32 i32 i32 i32 Symbol Symbol)``
.. compiledfn:: (Label-anchor ...)

   ``λ(Anchor)<~(Label)``
.. compiledfn:: (Label-countof-reachable ...)

   ``λ(usize)<~(Label)``
.. compiledfn:: (Label-docstring ...)

   ``λ(string)<~(Label)``
.. compiledfn:: (Label-name ...)

   ``λ(Symbol)<~(Label)``
.. compiledfn:: (Label-parameter ...)

   ``λ(Parameter)<~(Label usize)``
.. compiledfn:: (Label-parameter-count ...)

   ``λ(usize)<~(Label)``
.. compiledfn:: (Parameter-index ...)

   ``λ(i32)<~(Parameter)``
.. compiledfn:: (Parameter-name ...)

   ``λ(Symbol)<~(Parameter)``
.. compiledfn:: (Parameter-new ...)

   ``λ(Parameter)<~(Anchor Symbol type)``
.. compiledfn:: (SampledImage-type ...)

   ``λ(type)<~(type)``
.. compiledfn:: (Scope-clone ...)

   ``λ(Scope)<-(Scope)``
.. compiledfn:: (Scope-clone-expand ...)

   ``λ(Scope)<-(Scope Scope)``
.. compiledfn:: (Scope-docstring ...)

   ``λ(string)<~(Scope Symbol)``
.. compiledfn:: (Scope-local@ ...)

   ``λ(Any bool)<~(Scope Symbol)``
.. compiledfn:: (Scope-new ...)

   ``λ(Scope)<-()``
.. compiledfn:: (Scope-new-expand ...)

   ``λ(Scope)<-(Scope)``
.. compiledfn:: (Scope-next ...)

   ``λ(Symbol Any)<~(Scope Symbol)``
.. compiledfn:: (Scope-parent ...)

   ``λ(Scope)<-(Scope)``
.. compiledfn:: (Scope@ ...)

   ``λ(Any bool)<~(Scope Symbol)``
.. compiledfn:: (Symbol->string ...)

   ``λ(string)<~(Symbol)``
.. compiledfn:: (Syntax-new ...)

   ``λ(Syntax)<~(Anchor Any bool)``
.. compiledfn:: (Syntax-strip ...)

   ``λ(Any)<~(Any)``
.. compiledfn:: (Syntax-wrap ...)

   ``λ(Any)<~(Anchor Any bool)``
.. compiledfn:: (abort! ...)

   ``λ()<-()``
.. compiledfn:: (active-anchor ...)

   ``λ(Anchor)<-()``
.. compiledfn:: (alignof ...)

   ``λ(usize)<~(type)``
.. compiledfn:: (array-type ...)

   ``λ(type)<~(type usize)``
.. compiledfn:: (basename ...)

   ``λ(string)<-(string)``
.. compiledfn:: (bitcountof ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (catch-exception ...)

   ``λ(i32)<-([u8 x 216]*)``
.. compiledfn:: (compiler-version ...)

   ``λ(i32 i32 i32)<~()``
.. compiledfn:: (default-styler ...)

   ``λ(string)<~(Symbol string)``
.. compiledfn:: (delete-scope-symbol! ...)

   ``λ()<-(Scope Symbol)``
.. compiledfn:: (directory? ...)

   ``λ(bool)<-(string)``
.. compiledfn:: (dirname ...)

   ``λ(string)<-(string)``
.. compiledfn:: (dump-frame ...)

   ``λ()<~(Frame)``
.. compiledfn:: (dump-label ...)

   ``λ()<~(Label)``
.. compiledfn:: (dump-list ...)

   ``λ(list)<~(list)``
.. compiledfn:: (element-index ...)

   ``λ(i32)<~(type Symbol)``
.. compiledfn:: (element-name ...)

   ``λ(Symbol)<~(type i32)``
.. compiledfn:: (element-type ...)

   ``λ(type)<~(type i32)``
.. compiledfn:: (enter-solver-cli! ...)

   ``λ()<~()``
.. compiledfn:: (eval ...)

   ``λ(Label)<~(Syntax Scope)``
.. compiledfn:: (exception-value ...)

   ``λ(Any)<-([u8 x 216]*)``
.. compiledfn:: (exit ...)

   ``λ()<-(i32)``
.. compiledfn:: (extern-type-binding ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (extern-type-location ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (file? ...)

   ``λ(bool)<-(string)``
.. compiledfn:: (format-message ...)

   ``λ(string)<-(Anchor string)``
.. compiledfn:: (function-type-variadic? ...)

   ``λ(bool)<~(type)``
.. compiledfn:: (globals ...)

   ``λ(Scope)<-()``
.. compiledfn:: (import-c ...)

   ``λ(Scope)<~(string string list)``
.. compiledfn:: (integer-type ...)

   ``λ(type)<~(i32 bool)``
.. compiledfn:: (io-write! ...)

   ``λ()<-(string)``
.. compiledfn:: (list-cons ...)

   ``λ(list)<~(Any list)``
.. compiledfn:: (list-join ...)

   ``λ(list)<~(list list)``
.. compiledfn:: (list-load ...)

   ``λ(Syntax)<-(string)``
.. compiledfn:: (list-parse ...)

   ``λ(Syntax)<-(string)``
.. compiledfn:: (load-library ...)

   ``λ()<-(string)``
.. compiledfn:: (opaque? ...)

   ``λ(bool)<~(type)``
.. compiledfn:: (pointer-type ...)

   ``λ(type)<~(type u64 Symbol)``
.. compiledfn:: (pointer-type-flags ...)

   ``λ(u64)<~(type)``
.. compiledfn:: (pointer-type-set-element-type ...)

   ``λ(type)<~(type type)``
.. compiledfn:: (pointer-type-set-flags ...)

   ``λ(type)<~(type u64)``
.. compiledfn:: (pointer-type-set-storage-class ...)

   ``λ(type)<~(type Symbol)``
.. compiledfn:: (pointer-type-storage-class ...)

   ``λ(Symbol)<~(type)``
.. compiledfn:: (realpath ...)

   ``λ(string)<-(string)``
.. compiledfn:: (runtime-type@ ...)

   ``λ(Any bool)<~(type Symbol)``
.. compiledfn:: (set-anchor! ...)

   ``λ()<-(Anchor)``
.. compiledfn:: (set-autocomplete-scope! ...)

   ``λ()<-(Scope)``
.. compiledfn:: (set-exception-pad ...)

   ``λ([u8 x 216]*)<-([u8 x 216]*)``
.. compiledfn:: (set-globals! ...)

   ``λ()<-(Scope)``
.. compiledfn:: (set-scope-docstring! ...)

   ``λ()<-(Scope Symbol string)``
.. compiledfn:: (set-signal-abort! ...)

   ``λ()<-(bool)``
.. compiledfn:: (set-typename-super! ...)

   ``λ()<~(type type)``
.. compiledfn:: (signed? ...)

   ``λ(bool)<~(type)``
.. compiledfn:: (sizeof ...)

   ``λ(usize)<~(type)``
.. compiledfn:: (storageof ...)

   ``λ(type)<~(type)``
.. compiledfn:: (string->Symbol ...)

   ``λ(Symbol)<~(string)``
.. compiledfn:: (string-join ...)

   ``λ(string)<~(string string)``
.. compiledfn:: (string-match? ...)

   ``λ(bool)<~(string string)``
.. compiledfn:: (string-new ...)

   ``λ(string)<~(i8(*) usize)``
.. compiledfn:: (superof ...)

   ``λ(type)<~(type)``
.. compiledfn:: (type-countof ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (type-debug-abi ...)

   ``λ()<~(type)``
.. compiledfn:: (type-kind ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (type-name ...)

   ``λ(string)<~(type)``
.. compiledfn:: (type-next ...)

   ``λ(Symbol Any)<~(type Symbol)``
.. compiledfn:: (typename-type ...)

   ``λ(type)<~(string)``
.. compiledfn:: (vector-type ...)

   ``λ(type)<~(type usize)``
.. compiledfn:: (verify-stack! ...)

   ``λ(usize)<-()``
