globals
=======

These names are bound in every fresh module and main program by default.
Essential symbols are created by the compiler, and subsequent utility
functions, macros and types are defined and documented in `core.sc`.

The core module implements the remaining standard functions and macros,
parses the command-line and optionally enters the REPL.

.. define:: compile-flag-O1

   ``u64``
.. define:: compile-flag-O2

   ``u64``
.. define:: compile-flag-O3

   ``u64``
.. define:: compile-flag-dump-disassembly

   ``u64``
.. define:: compile-flag-dump-function

   ``u64``
.. define:: compile-flag-dump-module

   ``u64``
.. define:: compile-flag-dump-time

   ``u64``
.. define:: compile-flag-no-debug-info

   ``u64``
.. define:: compiler-dir

   ``String``
.. define:: compiler-path

   ``String``
.. define:: compiler-timestamp

   ``String``
.. define:: debug-build?

   ``bool``
.. define:: dot-char

   ``i8``
.. define:: dot-sym

   ``Symbol``
.. define:: e

   ``f32``
   
   Euler's number, also known as Napier's constant. Explicitly type-annotated
   versions of the constant are available as `e:f32` and `e:f64`
.. define:: e:f32

   ``f32``
.. define:: e:f64

   ``f64``
.. define:: ellipsis-symbol

   ``Symbol``
.. define:: false

   ``bool``
.. define:: global-flag-block

   ``u32``
.. define:: global-flag-buffer-block

   ``u32``
.. define:: global-flag-coherent

   ``u32``
.. define:: global-flag-non-readable

   ``u32``
.. define:: global-flag-non-writable

   ``u32``
.. define:: global-flag-restrict

   ``u32``
.. define:: global-flag-volatile

   ``u32``
.. define:: infinite-range

   ``Generator``
.. define:: list-handler-symbol

   ``Symbol``
.. define:: none

   ``Nothing``
.. define:: null

   ``NullType``
.. define:: operating-system

   ``Symbol``
.. define:: package

   ``Scope``
.. define:: pi

   ``f32``
   
   The number π, the ratio of a circle's circumference C to its diameter d.
   Explicitly type-annotated versions of the constant are available as `pi:f32`
   and `pi:f64`.
.. define:: pi:f32

   ``f32``
.. define:: pi:f64

   ``f64``
.. define:: pointer-flag-non-readable

   ``u64``
.. define:: pointer-flag-non-writable

   ``u64``
.. define:: struct-dsl

   ``Scope``
.. define:: style-comment

   ``Symbol``
.. define:: style-error

   ``Symbol``
.. define:: style-function

   ``Symbol``
.. define:: style-instruction

   ``Symbol``
.. define:: style-keyword

   ``Symbol``
.. define:: style-location

   ``Symbol``
.. define:: style-none

   ``Symbol``
.. define:: style-number

   ``Symbol``
.. define:: style-operator

   ``Symbol``
.. define:: style-sfxfunction

   ``Symbol``
.. define:: style-string

   ``Symbol``
.. define:: style-symbol

   ``Symbol``
.. define:: style-type

   ``Symbol``
.. define:: style-warning

   ``Symbol``
.. define:: symbol-handler-symbol

   ``Symbol``
.. define:: true

   ``bool``
.. define:: type-kind-arguments

   ``i32``
.. define:: type-kind-array

   ``i32``
.. define:: type-kind-function

   ``i32``
.. define:: type-kind-image

   ``i32``
.. define:: type-kind-integer

   ``i32``
.. define:: type-kind-pointer

   ``i32``
.. define:: type-kind-qualify

   ``i32``
.. define:: type-kind-real

   ``i32``
.. define:: type-kind-sampled-image

   ``i32``
.. define:: type-kind-tuple

   ``i32``
.. define:: type-kind-typename

   ``i32``
.. define:: type-kind-union

   ``i32``
.. define:: type-kind-vector

   ``i32``
.. define:: typename-flag-plain

   ``u32``
.. define:: unnamed

   ``Symbol``
.. define:: unroll-limit

   ``i32``
.. define:: value-kind-argument-list

   ``i32``
.. define:: value-kind-argument-list-template

   ``i32``
.. define:: value-kind-break

   ``i32``
.. define:: value-kind-call

   ``i32``
.. define:: value-kind-call-template

   ``i32``
.. define:: value-kind-compile-stage

   ``i32``
.. define:: value-kind-condbr

   ``i32``
.. define:: value-kind-const-aggregate

   ``i32``
.. define:: value-kind-const-int

   ``i32``
.. define:: value-kind-const-pointer

   ``i32``
.. define:: value-kind-const-real

   ``i32``
.. define:: value-kind-exception

   ``i32``
.. define:: value-kind-expression

   ``i32``
.. define:: value-kind-extract-argument

   ``i32``
.. define:: value-kind-extract-argument-template

   ``i32``
.. define:: value-kind-function

   ``i32``
.. define:: value-kind-global

   ``i32``
.. define:: value-kind-if

   ``i32``
.. define:: value-kind-keyed

   ``i32``
.. define:: value-kind-keyed-template

   ``i32``
.. define:: value-kind-label

   ``i32``
.. define:: value-kind-label-template

   ``i32``
.. define:: value-kind-loop

   ``i32``
.. define:: value-kind-loop-arguments

   ``i32``
.. define:: value-kind-loop-label

   ``i32``
.. define:: value-kind-loop-label-arguments

   ``i32``
.. define:: value-kind-merge

   ``i32``
.. define:: value-kind-merge-template

   ``i32``
.. define:: value-kind-parameter

   ``i32``
.. define:: value-kind-parameter-template

   ``i32``
.. define:: value-kind-pure-cast

   ``i32``
.. define:: value-kind-quote

   ``i32``
.. define:: value-kind-raise

   ``i32``
.. define:: value-kind-raise-template

   ``i32``
.. define:: value-kind-repeat

   ``i32``
.. define:: value-kind-repeat-template

   ``i32``
.. define:: value-kind-return

   ``i32``
.. define:: value-kind-return-template

   ``i32``
.. define:: value-kind-switch

   ``i32``
.. define:: value-kind-switch-template

   ``i32``
.. define:: value-kind-template

   ``i32``
.. define:: value-kind-unquote

   ``i32``
.. type:: _Value

   ``_Value`` : ``__Value(*)`` 
.. type:: Anchor

   ``Anchor`` : ``_Anchor(*)`` 
.. type:: Arguments

   ``Arguments`` 
.. spice:: (Arguments.__typecall ...)
.. type:: Builtin

   ``Builtin`` : ``u64`` 
.. type:: CEnum

   ``CEnum`` < ``immutable`` 
.. type:: CStruct

   ``CStruct`` 
.. spice:: (CStruct.__typecall ...)
.. type:: CUnion

   ``CUnion`` 
.. typefn:: (CUnion '__typecall cls)
.. type:: Closure

   ``Closure`` : ``_Closure(*)`` 
.. compiledfn:: (Closure.docstring ...)

   ``String<-(Closure)``
.. type:: Collector

   ``Collector`` : ``_Closure(*)`` 
.. spice:: (Collector.__call ...)
.. typefn:: (Collector '__typecall cls init valid? at collect)
.. type:: CompileStage

   ``CompileStage`` : ``{_Value Anchor}`` 
.. type:: Error

   ``Error`` : ``_Error(*)`` 
.. type:: Generator

   ``Generator`` : ``_Closure(*)`` 
.. spice:: (Generator.__call ...)
.. typefn:: (Generator '__typecall cls start valid? at next)
.. type:: Image

   ``Image`` 
.. spice:: (Image.__typecall ...)
.. compiledfn:: (Image.type ...)

   ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``
.. type:: Nothing

   ``Nothing`` : ``{}`` 
.. type:: NullType

   ``NullType`` : ``void(*)`` 
.. type:: OverloadedFunction

   ``OverloadedFunction`` 
.. spice:: (OverloadedFunction.__typecall ...)
.. spice:: (OverloadedFunction.append ...)
.. type:: Qualify

   ``Qualify`` 
.. type:: Raises

   ``Raises`` 
.. type:: SampledImage

   ``SampledImage`` 
.. spice:: (SampledImage.__typecall ...)
.. compiledfn:: (SampledImage.type ...)

   ``type<-(type)``
.. type:: Sampler

   ``Sampler`` 
.. type:: Scope

   ``Scope`` : ``_Scope(*)`` 
.. spice:: (Scope.__typecall ...)
.. compiledfn:: (Scope.parent ...)

   ``Scope<-(Scope)``
.. compiledfn:: (Scope.next ...)

   ``λ(Symbol Value)<-(Scope Symbol)``
.. spice:: (Scope.set-symbol ...)
.. spice:: (Scope.define-internal-symbol ...)
.. compiledfn:: (Scope.set-docstring! ...)

   ``void<-(Scope Symbol String)``
.. compiledfn:: (Scope.docstring ...)

   ``String<-(Scope Symbol)``
.. compiledfn:: (Scope.@ ...)

   ``Value<->Error(Scope Symbol)``
.. spice:: (Scope.define-symbol ...)
.. typefn:: (Scope 'set-symbols self values...)
.. typefn:: (Scope 'define-symbols self values...)
.. type:: SourceFile

   ``SourceFile`` : ``_SourceFile(*)`` 
.. type:: SpiceMacro

   ``SpiceMacro`` : ``Value<->Error(Value)(*)`` 
.. type:: SpiceMacroFunction

   ``Value<->Error(Value)(*)`` < ``pointer`` : ``Value<->Error(Value)(*)`` 
.. type:: Struct

   ``Struct`` 
.. spice:: (Struct.__typecall ...)
.. type:: SugarMacro

   ``SugarMacro`` : ``λ(List Scope)<->Error(List Scope)(*)`` 
.. spice:: (SugarMacro.__call ...)
.. type:: SugarMacroFunction

   ``λ(List Scope)<->Error(List Scope)(*)`` < ``pointer`` : ``λ(List Scope)<->Error(List Scope)(*)`` 
.. type:: Symbol

   ``Symbol`` < ``immutable`` : ``u64`` 
.. typefn:: (Symbol '__typecall cls str)
.. compiledfn:: (Symbol.variadic? ...)

   ``bool<-(Symbol)``
.. typefn:: (Symbol 'unique cls name)
.. spice:: (Symbol.__call ...)
.. type:: TypeArrayPointer

   ``type(*)`` < ``pointer`` : ``type(*)`` 
.. type:: Unknown

   ``Unknown`` : ``_type(*)`` 
.. type:: Value

   ``Value`` : ``{_Value Anchor}`` 
.. typefn:: (Value 'append-sink self)
.. typefn:: (Value 'args self)
.. compiledfn:: (Value.kind ...)

   ``i32<-(Value)``
.. typefn:: (Value 'tag self anchor)
.. spice:: (Value.__typecall ...)
.. compiledfn:: (Value.anchor ...)

   ``Anchor<-(Value)``
.. compiledfn:: (Value.none? ...)

   ``bool<-(Value)``
.. typefn:: (Value 'dump self)
.. compiledfn:: (Value.getarglist ...)

   ``Value<-(Value i32)``
.. typefn:: (Value 'dekey self)
.. compiledfn:: (Value.constant? ...)

   ``bool<-(Value)``
.. compiledfn:: (Value.pure? ...)

   ``bool<-(Value)``
.. compiledfn:: (Value.spice-repr ...)

   ``String<-(Value)``
.. compiledfn:: (Value.qualified-typeof ...)

   ``type<-(Value)``
.. compiledfn:: (Value.typeof ...)

   ``type<-(Value)``
.. compiledfn:: (Value.argcount ...)

   ``i32<-(Value)``
.. typefn:: (Value 'reverse-args self)
.. compiledfn:: (Value.getarg ...)

   ``Value<-(Value i32)``
.. type:: ValueArrayPointer

   ``Value(*)`` < ``pointer`` : ``Value(*)`` 
.. type:: Variadic

   ``...`` 
.. type:: aggregate

   ``aggregate`` 
.. type:: array

   ``array`` < ``aggregate`` 
.. spice:: (array.__typecall ...)
.. typefn:: (array 'type element-type size)
.. type:: bool

   ``bool`` < ``integer`` : ``bool`` 
.. type:: constant

   ``constant`` 
.. type:: f16

   ``f16`` < ``real`` : ``f16`` 
.. type:: f32

   ``f32`` < ``real`` : ``f32`` 
.. type:: f64

   ``f64`` < ``real`` : ``f64`` 
.. type:: f80

   ``f80`` < ``real`` : ``f80`` 
.. type:: function

   ``function`` 
.. spice:: (function.type ...)
.. spice:: (function.__typecall ...)
.. type:: hash

   ``hash`` : ``u64`` 
.. typefn:: (hash 'from-bytes data size)
.. spice:: (hash.__typecall ...)
.. type:: i16

   ``i16`` < ``integer`` : ``i16`` 
.. type:: i32

   ``i32`` < ``integer`` : ``i32`` 
.. type:: i64

   ``i64`` < ``integer`` : ``i64`` 
.. type:: i8

   ``i8`` < ``integer`` : ``i8`` 
.. type:: immutable

   ``immutable`` 
.. type:: incomplete

   ``incomplete`` 
.. type:: integer

   ``integer`` < ``immutable`` 
.. typefn:: (integer '__typecall cls value)
.. type:: intptr

   ``u64`` < ``integer`` : ``u64`` 
.. type:: list

   ``List`` : ``_List(*)`` 
.. typefn:: (list 'token-split expr token errmsg)
.. spice:: (list.__typecall ...)
.. compiledfn:: (list.dump ...)

   ``List<-(List)``
.. compiledfn:: (list.join ...)

   ``List<-(List List)``
.. compiledfn:: (list.next ...)

   ``List<-(List)``
.. typefn:: (list 'rjoin lside rside)
.. compiledfn:: (list.@ ...)

   ``Value<-(List)``
.. typefn:: (list 'cons-sink self)
.. typefn:: (list 'decons self count)
.. compiledfn:: (list.reverse ...)

   ``List<-(List)``
.. type:: noreturn

   ``noreturn`` 
.. type:: opaquepointer

   ``opaquepointer`` 
.. type:: pointer

   ``pointer`` 
.. typefn:: (pointer 'type T)
.. spice:: (pointer.__typecall ...)
.. spice:: (pointer.__call ...)
.. type:: rawstring

   ``i8(*)`` < ``pointer`` : ``i8(*)`` 
.. type:: real

   ``real`` < ``immutable`` 
.. typefn:: (real '__typecall cls value)
.. type:: string

   ``String`` < ``opaquepointer`` : ``_String(*)`` 
.. compiledfn:: (string.join ...)

   ``String<-(String String)``
.. compiledfn:: (string.buffer ...)

   ``λ(i8(*) usize)<-(String)``
.. compiledfn:: (string.match? ...)

   ``bool<->Error(String String)``
.. type:: tuple

   ``tuple`` < ``aggregate`` 
.. spice:: (tuple.__typecall ...)
.. spice:: (tuple.type ...)
.. type:: type

   ``type`` < ``opaquepointer`` : ``_type(*)`` 
.. typefn:: (type 'elements self)
.. spice:: (type.dispatch-attr ...)
.. typefn:: (type 'pointer->refer-type cls)
.. typefn:: (type 'readable? cls)
.. typefn:: (type 'strip-pointer-storage-class cls)
.. compiledfn:: (type.alignof ...)

   ``usize<->Error(type)``
.. compiledfn:: (type.storageof ...)

   ``type<->Error(type)``
.. compiledfn:: (type.bitcount ...)

   ``i32<-(type)``
.. compiledfn:: (type.local@ ...)

   ``Value<->Error(type Symbol)``
.. compiledfn:: (type.kind ...)

   ``i32<-(type)``
.. compiledfn:: (type.element-count ...)

   ``i32<->Error(type)``
.. compiledfn:: (type.sizeof ...)

   ``usize<->Error(type)``
.. compiledfn:: (type.element@ ...)

   ``type<->Error(type i32)``
.. typefn:: (type 'symbols self)
.. compiledfn:: (type.signed? ...)

   ``bool<-(type)``
.. compiledfn:: (type.unique-type ...)

   ``type<-(type i32)``
.. typefn:: (type 'immutable cls)
.. compiledfn:: (type.@ ...)

   ``Value<->Error(type Symbol)``
.. typefn:: (type 'set-plain-storage type storage-type)
.. typefn:: (type 'pointer-storage-class cls)
.. spice:: (type.set-symbol ...)
.. compiledfn:: (type.variadic? ...)

   ``bool<-(type)``
.. spice:: (type.__call ...)
.. spice:: (type.raises ...)
.. compiledfn:: (type.plain? ...)

   ``bool<-(type)``
.. compiledfn:: (type.key ...)

   ``λ(Symbol type)<-(type)``
.. compiledfn:: (type.refer? ...)

   ``bool<-(type)``
.. typefn:: (type 'set-symbols self values...)
.. typefn:: (type 'define-symbols self values...)
.. typefn:: (type 'writable? cls)
.. typefn:: (type 'view-type self id)
.. typefn:: (type 'change-element-type cls ET)
.. compiledfn:: (type.opaque? ...)

   ``bool<-(type)``
.. typefn:: (type 'key-type self key)
.. typefn:: (type 'set-storage type storage-type)
.. compiledfn:: (type.return-type ...)

   ``λ(type type)<-(type)``
.. typefn:: (type 'change-storage-class cls storage-class)
.. compiledfn:: (type.superof ...)

   ``type<-(type)``
.. typefn:: (type 'pointer? cls)
.. compiledfn:: (type.string ...)

   ``String<-(type)``
.. typefn:: (type 'function-pointer? cls)
.. spice:: (type.define-symbol ...)
.. typefn:: (type 'function? cls)
.. typefn:: (type 'mutable cls)
.. type:: typename

   ``typename`` 
.. spice:: (typename.__typecall ...)
.. compiledfn:: (typename.type ...)

   ``type<->Error(String type)``
.. type:: u16

   ``u16`` < ``integer`` : ``u16`` 
.. type:: u32

   ``u32`` < ``integer`` : ``u32`` 
.. type:: u64

   ``u64`` < ``integer`` : ``u64`` 
.. type:: u8

   ``u8`` < ``integer`` : ``u8`` 
.. type:: union

   ``union`` 
.. type:: usize

   ``usize`` < ``integer`` : ``u64`` 
.. type:: vector

   ``vector`` < ``immutable`` 
.. typefn:: (vector 'type element-type size)
.. spice:: (vector.__typecall ...)
.. spice:: (vector.smear ...)
.. type:: void

   ``void`` < ``Arguments`` 
.. type:: voidstar

   ``void(*)`` < ``pointer`` : ``void(*)`` 
.. inline:: (%= lhs rhs)
.. inline:: (&= lhs rhs)
.. inline:: (*= lhs rhs)
.. inline:: (+= lhs rhs)
.. inline:: (-= lhs rhs)
.. inline:: (..= lhs rhs)
.. inline:: (//= lhs rhs)
.. inline:: (/= lhs rhs)
.. inline:: (<<= lhs rhs)
.. inline:: (>>= lhs rhs)
.. inline:: (^= lhs rhs)
.. inline:: (_memo f)
.. inline:: (|= lhs rhs)
.. fn:: (Value-none? value)
.. fn:: (all? v)
.. fn:: (any? v)
.. inline:: (append self anchor traceback-msg)
.. fn:: (as-converter vT T static?)
.. fn:: (autoboxer T x)
.. inline:: (balanced-binary-op-dispatch symbol rsymbol friendly-op-name)
.. fn:: (balanced-binary-operation args symbol rsymbol friendly-op-name)
.. fn:: (balanced-binary-operator symbol rsymbol lhsT rhsT lhs-static? rhs-static?)
   
   for an operation performed on two argument types, of which either
   type can provide a suitable candidate, return a matching operator.
   This function only works inside a spice macro.
.. fn:: (binary-op-error friendly-op-name lhsT rhsT)
.. fn:: (binary-operator symbol lhsT rhsT)
   
   for an operation performed on two argument types, of which only
   the left type can provide a suitable candidate, find a matching
   operator function. This function only works inside a spice macro.
.. fn:: (binary-operator-r rsymbol lhsT rhsT)
   
   for an operation performed on two argument types, of which only
   the right type can provide a suitable candidate, find a matching
   operator function. This function only works inside a spice macro.
.. fn:: (box-empty)
.. fn:: (box-integer value)
.. fn:: (box-none)
.. fn:: (box-pointer value)
.. inline:: (box-spice-macro l)
.. fn:: (box-symbol value)
.. fn:: (build-typify-function f)
.. fn:: (cast-converter symbol rsymbol vT T)
   
   for two given types, find a matching conversion function
   this function only works inside a spice macro
.. inline:: (cast-error intro-string vT T)
.. inline:: (char s)
.. fn:: (check-count count mincount maxcount)
.. inline:: (clamp x mn mx)
.. fn:: (clone-scope-contents a b)
   
   Join two scopes ``a`` and ``b`` into a new scope so that the
   root of ``a`` descends from ``b``.
.. fn:: (compare-type args f)
.. inline:: (convert-assert-args args cond msg)
.. inline:: (decons self count)
.. inline:: (define-symbols self values...)
.. fn:: (delete value)
.. fn:: (dispatch-and-or args flip)
.. fn:: (dots-to-slashes pattern)
.. fn:: (dotted-symbol? env head)
.. fn:: (empty? value)
.. inline:: (enumerate x)
.. fn:: (error msg)
.. fn:: (error@ anchor traceback-msg error-msg)
   
   usage example::
       error@ ('anchor value) "while checking parameter" "error in value"
.. fn:: (error@+ error anchor traceback-msg)
   
   usage example::
       except (err)
           error@+ err ('anchor value) "while processing stream"
.. fn:: (exec-module expr eval-scope)
.. fn:: (expand-and-or expr f)
.. fn:: (expand-apply expr)
.. fn:: (expand-define expr)
.. fn:: (expand-define-infix args scope order)
.. fn:: (expand-infix-let expr)
.. inline:: (extern-new name T attrs...)
.. fn:: (extract-name-params-body expr)
.. fn:: (extract-single-arg args)
.. inline:: (floordiv a b)
.. inline:: (function->SpiceMacro f)
.. inline:: (gen-allocator-sugar name f)
.. inline:: (gen-cast-op f str)
.. inline:: (gen-match-block-parser handle-case)
.. fn:: (gen-match-matcher failfunc expr scope cond)
   
   features:
   <constant> -> (input == <constant>)
   (or <expr_a> <expr_b>) -> (or <expr_a> <expr_b>)
   
   TODO:
   (: x T) -> ((typeof input) == T), let x = input
   <unknown symbol> -> unpack as symbol
.. fn:: (gen-or-matcher failfunc expr scope params)
.. fn:: (gen-sugar-matcher failfunc expr scope params)
.. fn:: (gen-vector-reduction f v sz)
.. fn:: (get-ifx-op env op)
.. fn:: (get-ifx-symbol name)
.. fn:: (has-infix-ops? infix-table expr)
.. fn:: (imply-converter vT T static?)
.. inline:: (infix-op pred)
.. fn:: (infix-op-ge infix-table token prec)
.. fn:: (infix-op-gt infix-table token prec)
.. fn:: (integer-as vT T)
.. fn:: (integer-imply vT T)
.. fn:: (integer-static-imply vT T)
.. fn:: (integer-tobool args)
.. fn:: (list-handler topexpr env)
.. fn:: (load-module module-name module-path opts...)
.. fn:: (ltr-multiop args target mincount)
.. inline:: (make-const-type-property-function func)
.. inline:: (make-expand-and-or f)
.. inline:: (make-expand-define-infix order)
.. inline:: (make-inplace-let-op op)
.. inline:: (make-inplace-op op)
.. fn:: (make-module-path pattern name)
.. inline:: (make-unpack-function extractf)
.. inline:: (memo f)
.. inline:: (memoize f)
.. fn:: (merge-scope-symbols source target filter)
.. fn:: (next-head? next)
.. inline:: (not value)
.. fn:: (operator-valid? value)
.. fn:: (patterns-from-namestr base-dir namestr)
.. fn:: (pointer-imply vT T)
.. fn:: (pointer-type-imply? src dest)
.. fn:: (powi base exponent)
.. inline:: (print values...)
.. fn:: (ptrcmp!= t1 t2)
.. fn:: (ptrcmp== t1 t2)
.. inline:: (quasiquote-any x)
.. fn:: (quasiquote-list x)
.. inline:: (raises-compile-error)
.. inline:: (range a b c)
.. fn:: (real-as vT T)
.. fn:: (real-imply vT T)
.. fn:: (require-from base-dir name)
.. fn:: (rtl-infix-op-eq infix-table token prec)
.. fn:: (rtl-multiop args target mincount)
.. inline:: (sabs x)
.. inline:: (safe-integer-cast self T)
.. inline:: (select-op-macro sop fop numargs)
.. inline:: (set-symbols self values...)
.. inline:: (signed-vector-binary-op sf uf)
.. inline:: (simple-binary-op f)
   
   for cases where the type only interacts with itself
.. inline:: (simple-folding-autotype-binary-op f unboxer)
.. inline:: (simple-folding-autotype-signed-binary-op sf uf unboxer)
.. inline:: (simple-folding-binary-op f unboxer boxer)
.. inline:: (simple-signed-binary-op sf uf)
.. inline:: (slice value start end)
.. inline:: (spice-binary-op-macro f)
   
   to be used for binary operators of which either type can
   provide an operation. returns a callable operator (f lhs rhs) that
   performs the operation or no arguments if the operation can not be
   performed.
.. inline:: (spice-cast-macro f)
   
   to be used for __as, __ras, __imply and __rimply
   returns a callable converter (f value) that performs the cast or
   no arguments if the cast can not be performed.
.. inline:: (spice-converter-macro f)
   
   to be used for converter that need to do additional
   dispatch, e.g. do something else when the value is a constant
   returns a quote that performs the cast (f value T)
.. inline:: (spice-macro l)
.. fn:: (split-dotted-symbol name)
.. fn:: (string@ self i)
.. inline:: (sugar-block-scope-macro f)
.. inline:: (sugar-macro f)
.. inline:: (sugar-scope-macro f)
.. fn:: (symbol-handler topexpr env)
.. inline:: (type-comparison-func f)
.. inline:: (type-factory f)
.. inline:: (type< T superT)
.. inline:: (unary-op-dispatch symbol friendly-op-name)
.. fn:: (unary-op-error friendly-op-name T)
.. fn:: (unary-operation args symbol friendly-op-name)
.. fn:: (unary-operator symbol T)
   
   for an operation performed on one variable argument type, find a
   matching operator function. This function only works inside a spice
   macro.
.. inline:: (unary-or-balanced-binary-op-dispatch usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
.. fn:: (unary-or-balanced-binary-operation args usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
.. inline:: (unary-or-unbalanced-binary-op-dispatch usymbol ufriendly-op-name symbol rtype friendly-op-name)
.. fn:: (unary-or-unbalanced-binary-operation args usymbol ufriendly-op-name symbol rtype friendly-op-name)
.. inline:: (unbalanced-binary-op-dispatch symbol rtype friendly-op-name)
.. fn:: (unbalanced-binary-operation args symbol rtype friendly-op-name)
.. inline:: (unbox value T)
.. inline:: (unbox-integer value T)
.. inline:: (unbox-pointer value T)
.. inline:: (unbox-symbol value T)
.. fn:: (unbox-verify value wantT)
.. fn:: (uncomma l)
   
   uncomma list l, wrapping all comma separated symbols as new lists
   example::
   
       (uncomma '(a , b c d , e f , g h)) -> '(a (b c d) (e f) (g h))
.. fn:: (unpack-infix-op op)
.. fn:: (unpack2 args)
.. inline:: (va-join a...)
.. fn:: (value-as vT T expr)
.. inline:: (vector-binary-op-dispatch symbol)
.. fn:: (vector-binary-operator symbol lhsT rhsT)
.. fn:: (verify-count count mincount maxcount)
.. sugar:: (. ...)
.. sugar:: (:= ...)
.. sugar:: (<- ...)
.. sugar:: (@@ ...)
.. sugar:: (and ...)
.. sugar:: (as:= ...)
.. sugar:: (assert ...)
.. sugar:: (decorate-fn ...)
.. sugar:: (decorate-inline ...)
.. sugar:: (decorate-let ...)
.. sugar:: (decorate-struct ...)
.. sugar:: (decorate-typedef ...)
.. sugar:: (decorate-vvv ...)
.. sugar:: (define ...)
.. sugar:: (define-infix< ...)
.. sugar:: (define-infix> ...)
.. sugar:: (define-sugar-block-scope-macro ...)
.. sugar:: (define-sugar-macro ...)
.. sugar:: (define-sugar-scope-macro ...)
.. sugar:: (enum ...)
.. sugar:: (fn... ...)
.. sugar:: (fold ...)
.. sugar:: (fold-locals ...)
.. sugar:: (for ...)
.. sugar:: (from ...)
.. sugar:: (global ...)
.. sugar:: (import ...)
.. sugar:: (include ...)
.. sugar:: (inline... ...)
.. sugar:: (local ...)
.. sugar:: (locals ...)
   
   export locals as a chain of two new scopes: a scope that contains
   all the constant values in the immediate scope, and a scope that contains
   the runtime values.
.. sugar:: (match ...)
.. sugar:: (new ...)
.. sugar:: (or ...)
.. sugar:: (qq ...)
.. sugar:: (spice ...)
.. sugar:: (static-assert ...)
.. sugar:: (static-if ...)
.. sugar:: (struct ...)
.. sugar:: (sugar ...)
.. sugar:: (sugar-eval ...)
.. sugar:: (sugar-if ...)
.. sugar:: (sugar-match ...)
.. sugar:: (sugar-set-scope! ...)
.. sugar:: (typedef ...)
   
   a type declaration syntax; when the name is a string, the type is declared
   at runtime.
.. sugar:: (typedef+ ...)
.. sugar:: (unlet ...)
.. sugar:: (using ...)
.. sugar:: (va-option ...)
.. sugar:: (vvv ...)
.. sugar:: (while ...)
.. builtin:: (? ...)
.. builtin:: (_ ...)
.. builtin:: (Any-extract-constant ...)
.. builtin:: (Any-wrap ...)
.. builtin:: (Image-query-levels ...)
.. builtin:: (Image-query-lod ...)
.. builtin:: (Image-query-samples ...)
.. builtin:: (Image-query-size ...)
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
.. builtin:: (assign ...)
.. builtin:: (atan ...)
.. builtin:: (atan2 ...)
.. builtin:: (band ...)
.. builtin:: (bitcast ...)
.. builtin:: (bor ...)
.. builtin:: (branch ...)
.. builtin:: (break ...)
.. builtin:: (bxor ...)
.. builtin:: (call ...)
.. builtin:: (cc/call ...)
.. builtin:: (ceil ...)
.. builtin:: (compiler-anchor ...)
.. builtin:: (compiler-message ...)
.. builtin:: (copy ...)
.. builtin:: (cos ...)
.. builtin:: (cross ...)
.. builtin:: (degrees ...)
.. builtin:: (delete-type-symbol! ...)
.. builtin:: (deref ...)
.. builtin:: (discard! ...)
.. builtin:: (distance ...)
.. builtin:: (do ...)
.. builtin:: (dump ...)
.. builtin:: (dump-debug ...)
.. builtin:: (dump-spice ...)
.. builtin:: (dump-template ...)
.. builtin:: (dump-uniques ...)
.. builtin:: (dupe ...)
.. builtin:: (embed ...)
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
.. builtin:: (fmix ...)
.. builtin:: (fmul ...)
.. builtin:: (fn ...)
.. builtin:: (follow ...)
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
.. builtin:: (getelementref ...)
.. builtin:: (hide-traceback ...)
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
.. builtin:: (inline ...)
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
.. builtin:: (loop ...)
.. builtin:: (lose ...)
.. builtin:: (lshr ...)
.. builtin:: (malloc ...)
.. builtin:: (malloc-array ...)
.. builtin:: (merge ...)
.. builtin:: (move ...)
.. builtin:: (mul ...)
.. builtin:: (mul-nsw ...)
.. builtin:: (mul-nuw ...)
.. builtin:: (normalize ...)
.. builtin:: (nullof ...)
.. builtin:: (offsetof ...)
.. builtin:: (powf ...)
.. builtin:: (ptrtoint ...)
.. builtin:: (ptrtoref ...)
.. builtin:: (purify ...)
.. builtin:: (radians ...)
.. builtin:: (raise ...)
.. builtin:: (rawcall ...)
.. builtin:: (reftoptr ...)
.. builtin:: (repeat ...)
.. builtin:: (return ...)
.. builtin:: (round ...)
.. builtin:: (roundeven ...)
.. builtin:: (run-stage ...)
.. builtin:: (sample ...)
.. builtin:: (scopeof ...)
.. builtin:: (sdiv ...)
.. builtin:: (set-execution-mode! ...)
.. builtin:: (sext ...)
.. builtin:: (shl ...)
.. builtin:: (shufflevector ...)
.. builtin:: (sin ...)
.. builtin:: (sitofp ...)
.. builtin:: (smoothstep ...)
.. builtin:: (spice-quote ...)
.. builtin:: (spice-unquote ...)
.. builtin:: (spice-unquote-arguments ...)
.. builtin:: (sqrt ...)
.. builtin:: (square-list ...)
.. builtin:: (srem ...)
.. builtin:: (ssign ...)
.. builtin:: (static-alloc ...)
.. builtin:: (step ...)
.. builtin:: (store ...)
.. builtin:: (sub ...)
.. builtin:: (sub-nsw ...)
.. builtin:: (sub-nuw ...)
.. builtin:: (sugar-log ...)
.. builtin:: (sugar-quote ...)
.. builtin:: (switch ...)
.. builtin:: (tan ...)
.. builtin:: (trunc ...)
.. builtin:: (try ...)
.. builtin:: (tuple-type ...)
.. builtin:: (type-local@ ...)
.. builtin:: (type@ ...)
.. builtin:: (typeof ...)
.. builtin:: (udiv ...)
.. builtin:: (uitofp ...)
.. builtin:: (unconst ...)
.. builtin:: (undef ...)
.. builtin:: (union-type ...)
.. builtin:: (unique-visible? ...)
.. builtin:: (unreachable! ...)
.. builtin:: (urem ...)
.. builtin:: (va-countof ...)
.. builtin:: (va-key ...)
.. builtin:: (va-keys ...)
.. builtin:: (va-values ...)
.. builtin:: (va@ ...)
.. builtin:: (view ...)
.. builtin:: (viewing ...)
.. builtin:: (volatile-load ...)
.. builtin:: (volatile-store ...)
.. builtin:: (zext ...)
.. spice:: (% ...)
.. spice:: (& ...)
.. spice:: (& ...)
.. spice:: (* ...)
.. spice:: (* ...)
.. spice:: (+ ...)
.. spice:: (+ ...)
.. spice:: (- ...)
.. spice:: (/ ...)
.. spice:: (< ...)
.. spice:: (= ...)
.. spice:: (> ...)
.. spice:: (@ ...)
.. spice:: (@ ...)
.. spice:: (^ ...)
.. spice:: (| ...)
.. spice:: (| ...)
.. spice:: (~ ...)
.. spice:: (!= ...)
.. spice:: (.. ...)
.. spice:: (.. ...)
.. spice:: (// ...)
.. spice:: (<< ...)
.. spice:: (<= ...)
.. spice:: (== ...)
.. spice:: (>= ...)
.. spice:: (>> ...)
.. spice:: (Closure->Collector ...)
.. spice:: (Closure->Generator ...)
.. spice:: (abs ...)
.. spice:: (alignof ...)
.. spice:: (and-branch ...)
   
   The type of the `null` constant. This type is uninstantiable.
.. spice:: (append-to-type ...)
.. spice:: (arrayof ...)
.. spice:: (as ...)
.. spice:: (coerce-call-arguments ...)
.. spice:: (cons ...)
.. spice:: (const.add.i32.i32 ...)
.. spice:: (const.icmp<=.i32.i32 ...)
.. spice:: (constant? ...)
.. spice:: (countof ...)
.. spice:: (extern ...)
.. spice:: (getattr ...)
.. spice:: (hash-storage ...)
.. spice:: (hash1 ...)
.. spice:: (imply ...)
.. spice:: (integer->integer ...)
.. spice:: (list-constructor ...)
.. spice:: (lslice ...)
.. spice:: (max ...)
.. spice:: (memocall ...)
.. spice:: (min ...)
.. spice:: (mutable ...)
.. spice:: (none? ...)
.. spice:: (not ...)
.. spice:: (or-branch ...)
.. spice:: (overloaded-fn-append ...)
.. spice:: (parse-compile-flags ...)
.. spice:: (pow ...)
.. spice:: (private ...)
.. spice:: (raises ...)
.. spice:: (report ...)
.. spice:: (repr ...)
.. spice:: (rslice ...)
.. spice:: (safe-shl ...)
.. spice:: (sign ...)
.. spice:: (sizeof ...)
.. spice:: (static-branch ...)
.. spice:: (static-error ...)
.. spice:: (static-integer->integer ...)
.. spice:: (static-integer->real ...)
.. spice:: (static-typify ...)
.. spice:: (storagecast ...)
.. spice:: (storageof ...)
.. spice:: (superof ...)
.. spice:: (tostring ...)
.. spice:: (tupleof ...)
.. spice:: (type!= ...)
.. spice:: (type<= ...)
.. spice:: (type== ...)
.. spice:: (type>= ...)
.. spice:: (typify ...)
.. spice:: (unpack ...)
.. spice:: (va-append-va ...)
   
    (va-append-va (inline () (_ b ...)) a...) -> a... b...
.. spice:: (va-empty? ...)
.. spice:: (va-lfold ...)
.. spice:: (va-lifold ...)
.. spice:: (va-option-branch ...)
.. spice:: (va-rfold ...)
.. spice:: (va-rifold ...)
.. spice:: (va-split ...)
   
    (va-split n a...) -> (inline () a...[n .. (va-countof a...)-1]) a...[0 .. n-1]
.. spice:: (va-unnamed ...)
   
    filter all keyed values
.. spice:: (vector-reduce ...)
.. spice:: (vectorof ...)
.. spice:: (wrap-if-not-run-stage ...)
.. spice:: (zip ...)
.. compiledfn:: (compiler-version ...)

   ``λ(i32 i32 i32)<-()``
.. compiledfn:: (default-styler ...)

   ``String<-(Symbol String)``
.. compiledfn:: (exit ...)

   ``noreturn<-(i32)``
.. compiledfn:: (function->SugarMacro ...)

   ``SugarMacro<-(λ(List Scope)<->Error(List Scope)(*))``
.. compiledfn:: (globals ...)

   ``Scope<-()``
.. compiledfn:: (io-write! ...)

   ``void<-(String)``
.. compiledfn:: (launch-args ...)

   ``λ(i32 i8(*)(*))<-()``
.. compiledfn:: (list-load ...)

   ``Value<->Error(String)``
.. compiledfn:: (list-parse ...)

   ``Value<->Error(String)``
.. compiledfn:: (load-library ...)

   ``void<->Error(String)``
.. compiledfn:: (parse-infix-expr ...)

   ``λ(Value List)<->Error(Scope Value List i32)``
.. compiledfn:: (realpath ...)

   ``String<-(String)``
.. compiledfn:: (sc_abort ...)

   ``noreturn<-()``
.. compiledfn:: (sc_anchor_offset ...)

   ``Anchor<-(Anchor i32)``
.. compiledfn:: (sc_argcount ...)

   ``i32<-(Value)``
.. compiledfn:: (sc_argument_list_append ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_argument_list_new ...)

   ``Value<-()``
.. compiledfn:: (sc_arguments_type ...)

   ``type<-(i32 type(*))``
.. compiledfn:: (sc_arguments_type_argcount ...)

   ``i32<-(type)``
.. compiledfn:: (sc_arguments_type_getarg ...)

   ``type<-(type i32)``
.. compiledfn:: (sc_arguments_type_join ...)

   ``type<-(type type)``
.. compiledfn:: (sc_array_type ...)

   ``type<->Error(type usize)``
.. compiledfn:: (sc_basename ...)

   ``String<-(String)``
.. compiledfn:: (sc_break_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_call_append_argument ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_call_is_rawcall ...)

   ``bool<-(Value)``
.. compiledfn:: (sc_call_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_call_set_rawcall ...)

   ``void<-(Value bool)``
.. compiledfn:: (sc_closure_get_context ...)

   ``Value<-(Closure)``
.. compiledfn:: (sc_closure_get_docstring ...)

   ``String<-(Closure)``
.. compiledfn:: (sc_closure_get_template ...)

   ``Value<-(Closure)``
.. compiledfn:: (sc_compile ...)

   ``Value<->Error(Value u64)``
.. compiledfn:: (sc_compile_glsl ...)

   ``String<->Error(Symbol Value u64)``
.. compiledfn:: (sc_compile_object ...)

   ``void<->Error(String Scope u64)``
.. compiledfn:: (sc_compile_spirv ...)

   ``String<->Error(Symbol Value u64)``
.. compiledfn:: (sc_compiler_version ...)

   ``λ(i32 i32 i32)<-()``
.. compiledfn:: (sc_const_aggregate_new ...)

   ``Value<-(type i32 Value(*))``
.. compiledfn:: (sc_const_extract_at ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_const_int_extract ...)

   ``u64<-(Value)``
.. compiledfn:: (sc_const_int_new ...)

   ``Value<-(type u64)``
.. compiledfn:: (sc_const_pointer_extract ...)

   ``void(*)<-(Value)``
.. compiledfn:: (sc_const_pointer_new ...)

   ``Value<-(type void(*))``
.. compiledfn:: (sc_const_real_extract ...)

   ``f64<-(Value)``
.. compiledfn:: (sc_const_real_new ...)

   ``Value<-(type f64)``
.. compiledfn:: (sc_default_styler ...)

   ``String<-(Symbol String)``
.. compiledfn:: (sc_dirname ...)

   ``String<-(String)``
.. compiledfn:: (sc_dump_error ...)

   ``void<-(Error)``
.. compiledfn:: (sc_empty_argument_list ...)

   ``Value<-()``
.. compiledfn:: (sc_enter_solver_cli ...)

   ``void<-()``
.. compiledfn:: (sc_error_append_calltrace ...)

   ``void<-(Error Value)``
.. compiledfn:: (sc_error_new ...)

   ``Error<-(String)``
.. compiledfn:: (sc_eval ...)

   ``Value<->Error(Anchor List Scope)``
.. compiledfn:: (sc_eval_inline ...)

   ``Anchor<->Error(Value List Scope)``
.. compiledfn:: (sc_exit ...)

   ``noreturn<-(i32)``
.. compiledfn:: (sc_expand ...)

   ``λ(Value List Scope)<->Error(Value List Scope)``
.. compiledfn:: (sc_expression_append ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_expression_new ...)

   ``Value<-()``
.. compiledfn:: (sc_expression_set_scoped ...)

   ``void<-(Value)``
.. compiledfn:: (sc_extract_argument_list_new ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_extract_argument_new ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_format_error ...)

   ``String<-(Error)``
.. compiledfn:: (sc_format_message ...)

   ``String<-(Anchor String)``
.. compiledfn:: (sc_function_type ...)

   ``type<-(type i32 type(*))``
.. compiledfn:: (sc_function_type_is_variadic ...)

   ``bool<-(type)``
.. compiledfn:: (sc_function_type_raising ...)

   ``type<-(type type)``
.. compiledfn:: (sc_function_type_return_type ...)

   ``λ(type type)<-(type)``
.. compiledfn:: (sc_get_globals ...)

   ``Scope<-()``
.. compiledfn:: (sc_get_original_globals ...)

   ``Scope<-()``
.. compiledfn:: (sc_getarg ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_getarglist ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_global_new ...)

   ``Value<-(Symbol type u32 Symbol i32 i32)``
.. compiledfn:: (sc_hash ...)

   ``u64<-(u64 usize)``
.. compiledfn:: (sc_hash2x64 ...)

   ``u64<-(u64 u64)``
.. compiledfn:: (sc_hashbytes ...)

   ``u64<-(i8(*) usize)``
.. compiledfn:: (sc_if_append_else_clause ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_if_append_then_clause ...)

   ``void<-(Value Value Value)``
.. compiledfn:: (sc_if_new ...)

   ``Value<-()``
.. compiledfn:: (sc_image_type ...)

   ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``
.. compiledfn:: (sc_import_c ...)

   ``Scope<->Error(String String List)``
.. compiledfn:: (sc_integer_type ...)

   ``type<-(i32 bool)``
.. compiledfn:: (sc_integer_type_is_signed ...)

   ``bool<-(type)``
.. compiledfn:: (sc_is_directory ...)

   ``bool<-(String)``
.. compiledfn:: (sc_is_file ...)

   ``bool<-(String)``
.. compiledfn:: (sc_key_type ...)

   ``type<-(Symbol type)``
.. compiledfn:: (sc_keyed_new ...)

   ``Value<-(Symbol Value)``
.. compiledfn:: (sc_label_new ...)

   ``Value<-(i32 Symbol)``
.. compiledfn:: (sc_label_set_body ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_launch_args ...)

   ``λ(i32 i8(*)(*))<-()``
.. compiledfn:: (sc_list_at ...)

   ``Value<-(List)``
.. compiledfn:: (sc_list_compare ...)

   ``bool<-(List List)``
.. compiledfn:: (sc_list_cons ...)

   ``List<-(Value List)``
.. compiledfn:: (sc_list_count ...)

   ``i32<-(List)``
.. compiledfn:: (sc_list_decons ...)

   ``λ(Value List)<-(List)``
.. compiledfn:: (sc_list_dump ...)

   ``List<-(List)``
.. compiledfn:: (sc_list_join ...)

   ``List<-(List List)``
.. compiledfn:: (sc_list_next ...)

   ``List<-(List)``
.. compiledfn:: (sc_list_repr ...)

   ``String<-(List)``
.. compiledfn:: (sc_list_reverse ...)

   ``List<-(List)``
.. compiledfn:: (sc_load_library ...)

   ``void<->Error(String)``
.. compiledfn:: (sc_loop_arguments ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_loop_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_loop_set_body ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_map_get ...)

   ``Value<->Error(Value)``
.. compiledfn:: (sc_map_set ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_merge_new ...)

   ``Value<-(Value Value)``
.. compiledfn:: (sc_mutate_type ...)

   ``type<-(type)``
.. compiledfn:: (sc_parameter_is_variadic ...)

   ``bool<-(Value)``
.. compiledfn:: (sc_parameter_name ...)

   ``Symbol<-(Value)``
.. compiledfn:: (sc_parameter_new ...)

   ``Value<-(Symbol)``
.. compiledfn:: (sc_parse_from_path ...)

   ``Value<->Error(String)``
.. compiledfn:: (sc_parse_from_string ...)

   ``Value<->Error(String)``
.. compiledfn:: (sc_pointer_type ...)

   ``type<-(type u64 Symbol)``
.. compiledfn:: (sc_pointer_type_get_flags ...)

   ``u64<-(type)``
.. compiledfn:: (sc_pointer_type_get_storage_class ...)

   ``Symbol<-(type)``
.. compiledfn:: (sc_pointer_type_set_element_type ...)

   ``type<-(type type)``
.. compiledfn:: (sc_pointer_type_set_flags ...)

   ``type<-(type u64)``
.. compiledfn:: (sc_pointer_type_set_storage_class ...)

   ``type<-(type Symbol)``
.. compiledfn:: (sc_prompt ...)

   ``λ(bool String)<-(String String)``
.. compiledfn:: (sc_prove ...)

   ``Value<->Error(Value)``
.. compiledfn:: (sc_quote_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_raise_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_realpath ...)

   ``String<-(String)``
.. compiledfn:: (sc_refer_type ...)

   ``type<-(type u64 Symbol)``
.. compiledfn:: (sc_repeat_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_return_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_sampled_image_type ...)

   ``type<-(type)``
.. compiledfn:: (sc_scope_at ...)

   ``Value<->Error(Scope Symbol)``
.. compiledfn:: (sc_scope_clone ...)

   ``Scope<-(Scope)``
.. compiledfn:: (sc_scope_clone_subscope ...)

   ``Scope<-(Scope Scope)``
.. compiledfn:: (sc_scope_del_symbol ...)

   ``void<-(Scope Symbol)``
.. compiledfn:: (sc_scope_get_docstring ...)

   ``String<-(Scope Symbol)``
.. compiledfn:: (sc_scope_get_parent ...)

   ``Scope<-(Scope)``
.. compiledfn:: (sc_scope_local_at ...)

   ``Value<->Error(Scope Symbol)``
.. compiledfn:: (sc_scope_new ...)

   ``Scope<-()``
.. compiledfn:: (sc_scope_new_subscope ...)

   ``Scope<-(Scope)``
.. compiledfn:: (sc_scope_next ...)

   ``λ(Symbol Value)<-(Scope Symbol)``
.. compiledfn:: (sc_scope_set_docstring ...)

   ``void<-(Scope Symbol String)``
.. compiledfn:: (sc_scope_set_symbol ...)

   ``void<-(Scope Symbol Value)``
.. compiledfn:: (sc_set_autocomplete_scope ...)

   ``void<-(Scope)``
.. compiledfn:: (sc_set_globals ...)

   ``void<-(Scope)``
.. compiledfn:: (sc_set_signal_abort ...)

   ``void<-(bool)``
.. compiledfn:: (sc_string_buffer ...)

   ``λ(i8(*) usize)<-(String)``
.. compiledfn:: (sc_string_compare ...)

   ``i32<-(String String)``
.. compiledfn:: (sc_string_count ...)

   ``usize<-(String)``
.. compiledfn:: (sc_string_join ...)

   ``String<-(String String)``
.. compiledfn:: (sc_string_lslice ...)

   ``String<-(String usize)``
.. compiledfn:: (sc_string_match ...)

   ``bool<->Error(String String)``
.. compiledfn:: (sc_string_new ...)

   ``String<-(i8(*) usize)``
.. compiledfn:: (sc_string_new_from_cstr ...)

   ``String<-(i8(*))``
.. compiledfn:: (sc_string_rslice ...)

   ``String<-(String usize)``
.. compiledfn:: (sc_strip_qualifiers ...)

   ``type<-(type)``
.. compiledfn:: (sc_switch_append_case ...)

   ``void<-(Value Value Value)``
.. compiledfn:: (sc_switch_append_default ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_switch_append_pass ...)

   ``void<-(Value Value Value)``
.. compiledfn:: (sc_switch_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_symbol_is_variadic ...)

   ``bool<-(Symbol)``
.. compiledfn:: (sc_symbol_new ...)

   ``Symbol<-(String)``
.. compiledfn:: (sc_symbol_new_unique ...)

   ``Symbol<-(String)``
.. compiledfn:: (sc_symbol_to_string ...)

   ``String<-(Symbol)``
.. compiledfn:: (sc_template_append_parameter ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_template_get_name ...)

   ``Symbol<-(Value)``
.. compiledfn:: (sc_template_is_inline ...)

   ``bool<-(Value)``
.. compiledfn:: (sc_template_new ...)

   ``Value<-(Symbol)``
.. compiledfn:: (sc_template_parameter ...)

   ``Value<-(Value i32)``
.. compiledfn:: (sc_template_parameter_count ...)

   ``i32<-(Value)``
.. compiledfn:: (sc_template_set_body ...)

   ``void<-(Value Value)``
.. compiledfn:: (sc_template_set_inline ...)

   ``void<-(Value)``
.. compiledfn:: (sc_template_set_name ...)

   ``void<-(Value Symbol)``
.. compiledfn:: (sc_tuple_type ...)

   ``type<->Error(i32 type(*))``
.. compiledfn:: (sc_type_alignof ...)

   ``usize<->Error(type)``
.. compiledfn:: (sc_type_at ...)

   ``Value<->Error(type Symbol)``
.. compiledfn:: (sc_type_bitcountof ...)

   ``i32<-(type)``
.. compiledfn:: (sc_type_countof ...)

   ``i32<->Error(type)``
.. compiledfn:: (sc_type_debug_abi ...)

   ``void<-(type)``
.. compiledfn:: (sc_type_element_at ...)

   ``type<->Error(type i32)``
.. compiledfn:: (sc_type_field_index ...)

   ``i32<->Error(type Symbol)``
.. compiledfn:: (sc_type_field_name ...)

   ``Symbol<->Error(type i32)``
.. compiledfn:: (sc_type_is_default_suffix ...)

   ``bool<-(type)``
.. compiledfn:: (sc_type_is_opaque ...)

   ``bool<-(type)``
.. compiledfn:: (sc_type_is_plain ...)

   ``bool<-(type)``
.. compiledfn:: (sc_type_is_refer ...)

   ``bool<-(type)``
.. compiledfn:: (sc_type_is_superof ...)

   ``bool<-(type type)``
.. compiledfn:: (sc_type_key ...)

   ``λ(Symbol type)<-(type)``
.. compiledfn:: (sc_type_kind ...)

   ``i32<-(type)``
.. compiledfn:: (sc_type_local_at ...)

   ``Value<->Error(type Symbol)``
.. compiledfn:: (sc_type_next ...)

   ``λ(Symbol Value)<-(type Symbol)``
.. compiledfn:: (sc_type_set_symbol ...)

   ``void<-(type Symbol Value)``
.. compiledfn:: (sc_type_sizeof ...)

   ``usize<->Error(type)``
.. compiledfn:: (sc_type_storage ...)

   ``type<->Error(type)``
.. compiledfn:: (sc_type_string ...)

   ``String<-(type)``
.. compiledfn:: (sc_typename_type ...)

   ``type<->Error(String type)``
.. compiledfn:: (sc_typename_type_get_super ...)

   ``type<-(type)``
.. compiledfn:: (sc_typename_type_set_storage ...)

   ``void<->Error(type type u32)``
.. compiledfn:: (sc_typify ...)

   ``Value<->Error(Closure i32 type(*))``
.. compiledfn:: (sc_typify_template ...)

   ``Value<->Error(Value i32 type(*))``
.. compiledfn:: (sc_union_type ...)

   ``type<->Error(i32 type(*))``
.. compiledfn:: (sc_unique_type ...)

   ``type<-(type i32)``
.. compiledfn:: (sc_unquote_new ...)

   ``Value<-(Value)``
.. compiledfn:: (sc_value_anchor ...)

   ``Anchor<-(Value)``
.. compiledfn:: (sc_value_ast_repr ...)

   ``String<-(Value)``
.. compiledfn:: (sc_value_compare ...)

   ``bool<-(Value Value)``
.. compiledfn:: (sc_value_content_repr ...)

   ``String<-(Value)``
.. compiledfn:: (sc_value_is_constant ...)

   ``bool<-(Value)``
.. compiledfn:: (sc_value_is_pure ...)

   ``bool<-(Value)``
.. compiledfn:: (sc_value_kind ...)

   ``i32<-(Value)``
.. compiledfn:: (sc_value_qualified_type ...)

   ``type<-(Value)``
.. compiledfn:: (sc_value_repr ...)

   ``String<-(Value)``
.. compiledfn:: (sc_value_tostring ...)

   ``String<-(Value)``
.. compiledfn:: (sc_value_type ...)

   ``type<-(Value)``
.. compiledfn:: (sc_value_unwrap ...)

   ``Value<-(type Value)``
.. compiledfn:: (sc_value_wrap ...)

   ``Value<-(type Value)``
.. compiledfn:: (sc_valueref_tag ...)

   ``Value<-(Anchor Value)``
.. compiledfn:: (sc_vector_type ...)

   ``type<->Error(type usize)``
.. compiledfn:: (sc_verify_stack ...)

   ``usize<->Error()``
.. compiledfn:: (sc_view_type ...)

   ``type<-(type i32)``
.. compiledfn:: (sc_write ...)

   ``void<-(String)``
.. compiledfn:: (set-autocomplete-scope! ...)

   ``void<-(Scope)``
.. compiledfn:: (set-globals! ...)

   ``void<-(Scope)``
.. compiledfn:: (set-signal-abort! ...)

   ``void<-(bool)``
.. compiledfn:: (spice-macro-verify-signature ...)

   ``void<-(Value<->Error(Value)(*))``
.. compiledfn:: (type> ...)

   ``bool<-(type type)``
