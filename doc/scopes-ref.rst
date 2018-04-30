Scopes Language Reference
=========================

The global module for Scopes.

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
.. define:: reference-attribs-key
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
.. type:: Builtin
.. type:: CEnum
.. type:: CStruct
.. type:: CUnion
.. type:: Capture
.. type:: Closure
.. type:: Exception
.. type:: Frame
.. type:: Generator
.. type:: Image
.. type:: Label
.. type:: Macro
.. type:: Nothing
.. type:: NullType
   
   The type of the `null` constant. This type is uninstantiable.
.. type:: Parameter
.. type:: ReturnLabel
.. type:: SampledImage
.. type:: Sampler
.. type:: Scope
.. type:: SourceFile
.. type:: Symbol
.. type:: Syntax
.. type:: Unknown
.. type:: array
.. type:: bool
.. type:: constant
.. type:: exception-pad-type
.. type:: extern
.. type:: f16
.. type:: f32
.. type:: f64
.. type:: f80
.. type:: function
.. type:: hash
.. type:: i16
.. type:: i32
.. type:: i64
.. type:: i8
.. type:: immutable
.. type:: integer
.. type:: list
.. type:: pointer
.. type:: rawstring
.. type:: real
.. type:: reference
.. type:: string
.. type:: tuple
.. type:: type
.. type:: typename
.. type:: u16
.. type:: u32
.. type:: u64
.. type:: u8
.. type:: union
.. type:: usize
.. type:: vector
.. type:: void
.. type:: voidstar
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
.. fn:: (countof x)
.. fn:: (decons val count)
.. fn:: (delete self)
.. fn:: (deref val)
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
.. fn:: (load-module module-name module-path main-module?)
.. fn:: (local cls args...)
.. fn:: (macro f)
.. fn:: (map x f)
   
   Maps function `f (skip values...)` to elements of iterable `x`.
   
   `skip` is a function that can be called to purge the active element
   from the output (allowing map to also act as a filter).
.. fn:: (max a b ...)
.. fn:: (maybe-unsyntax val)
.. fn:: (merge-scope-symbols source target filter)
.. fn:: (min a b ...)
.. fn:: (new cls args...)
.. fn:: (none? val)
.. fn:: (not x)
.. fn:: (op2-dispatch symbol)
.. fn:: (op2-dispatch-bidi symbol fallback)
.. fn:: (op2-ltr-multiop f)
.. fn:: (op2-rtl-multiop f)
.. fn:: (opN-dispatch symbol)
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
.. fn:: (static cls args...)
.. fn:: (string->rawstring s)
.. fn:: (string-compare a b)
.. fn:: (string-countof s)
.. fn:: (string-repr val)
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
.. macro:: (let name ... _:= value ...)

Binds a list of constants and variables specified on the right-hand
side to parameter names defined on the left-hand side.

.. macro:: (let label-name (name ...) _:= value ...)

Performs the same function as the regular `let`, but associates the
entry point with a labelname that can be called to effectively produce
a tail-recursive loop. When some of the arguments on the right hand
side are not constant, the loop will be unrolled.

.. macro:: (let name ...)

Rebinds names already defined in the parent scope to the local scope.
This becomes useful in conjunction with `locals`, when exporting
modules.

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

   ``λ(usize)<~(type)``
.. compiledfn:: (type-debug-abi ...)

   ``λ()<~(type)``
.. compiledfn:: (type-kind ...)

   ``λ(i32)<~(type)``
.. compiledfn:: (type-name ...)

   ``λ(string)<~(type)``
.. compiledfn:: (typename-type ...)

   ``λ(type)<~(string)``
.. compiledfn:: (vector-type ...)

   ``λ(type)<~(type usize)``
.. compiledfn:: (verify-stack! ...)

   ``λ(usize)<-()``
