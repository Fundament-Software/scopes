globals
=======

These names are bound in every fresh module and main program by default.
Essential symbols are created by the compiler, and subsequent utility
functions, macros and types are defined and documented in `core.sc`.

The core module implements the remaining standard functions and macros,
parses the command-line and optionally enters the REPL.

.. define:: compile-flag-O1

   A constant of type `u64`.
.. define:: compile-flag-O2

   A constant of type `u64`.
.. define:: compile-flag-O3

   A constant of type `u64`.
.. define:: compile-flag-dump-disassembly

   A constant of type `u64`.
.. define:: compile-flag-dump-function

   A constant of type `u64`.
.. define:: compile-flag-dump-module

   A constant of type `u64`.
.. define:: compile-flag-dump-time

   A constant of type `u64`.
.. define:: compile-flag-no-debug-info

   A constant of type `u64`.
.. define:: compiler-dir
   
   A string containing the folder path to the compiler environment. Typically
   the compiler environment is the folder that contains the ``bin`` folder
   containing the compiler executable.
.. define:: compiler-path
   
   A string constant containing the file path to the compiler executable.
.. define:: compiler-timestamp
   
   A string constant indicating the time and date the compiler was built.
.. define:: debug-build?
   
   A boolean constant indicating if the compiler was built in debug mode.
.. define:: e
   
   Euler's number, also known as Napier's constant. Explicitly type-annotated
   versions of the constant are available as `e:f32` and `e:f64`
.. define:: e:f32
   
   See `e`.
.. define:: e:f64
   
   See `e`.
.. define:: false

   A constant of type `bool`.
.. define:: global-flag-block

   A constant of type `u32`.
.. define:: global-flag-buffer-block

   A constant of type `u32`.
.. define:: global-flag-coherent

   A constant of type `u32`.
.. define:: global-flag-non-readable

   A constant of type `u32`.
.. define:: global-flag-non-writable

   A constant of type `u32`.
.. define:: global-flag-restrict

   A constant of type `u32`.
.. define:: global-flag-volatile

   A constant of type `u32`.
.. define:: infinite-range
   
   A `Generator` that iterates through all 32-bit signed integer values starting
   at 0. This generator does never terminate; when it exceeds the maximum
   positive integer value of 2147483647, it overflows and continues with the
   minimum negative integer value of -2147483648.
.. define:: none

   A constant of type `Nothing`.
.. define:: null

   A constant of type `NullType`.
.. define:: operating-system
   
   A string constant indicating the operating system the compiler was built
   for. It equals to ``"linux"`` for Linux builds, ``"windows"`` for Windows
   builds, ``"macos"`` for macOS builds and ``"unknown"`` otherwise.
.. define:: package
   
   A symbol table of type `Scope` which holds configuration options and module
   contents. It is managed by the module import system.
   
   ``package.path`` holds a list of all search paths in the form of simple
   string patterns. Changing it alters the way modules are searched for in
   the next run stage.
   
   ``package.modules`` is another scope symbol table mapping full module
   paths to their contents. When a module is first imported, its contents
   are cached in this table. Subsequent imports of the same module will be
   resolved to these cached contents.
.. define:: pi
   
   The number π, the ratio of a circle's circumference C to its diameter d.
   Explicitly type-annotated versions of the constant are available as `pi:f32`
   and `pi:f64`.
.. define:: pi:f32
   
   See `pi`.
.. define:: pi:f64
   
   See `pi`.
.. define:: pointer-flag-non-readable

   A constant of type `u64`.
.. define:: pointer-flag-non-writable

   A constant of type `u64`.
.. define:: style-comment

   A constant of type `Symbol`.
.. define:: style-error

   A constant of type `Symbol`.
.. define:: style-function

   A constant of type `Symbol`.
.. define:: style-instruction

   A constant of type `Symbol`.
.. define:: style-keyword

   A constant of type `Symbol`.
.. define:: style-location

   A constant of type `Symbol`.
.. define:: style-none

   A constant of type `Symbol`.
.. define:: style-number

   A constant of type `Symbol`.
.. define:: style-operator

   A constant of type `Symbol`.
.. define:: style-sfxfunction

   A constant of type `Symbol`.
.. define:: style-string

   A constant of type `Symbol`.
.. define:: style-symbol

   A constant of type `Symbol`.
.. define:: style-type

   A constant of type `Symbol`.
.. define:: style-warning

   A constant of type `Symbol`.
.. define:: true

   A constant of type `bool`.
.. define:: type-kind-arguments

   A constant of type `i32`.
.. define:: type-kind-array

   A constant of type `i32`.
.. define:: type-kind-function

   A constant of type `i32`.
.. define:: type-kind-image

   A constant of type `i32`.
.. define:: type-kind-integer

   A constant of type `i32`.
.. define:: type-kind-pointer

   A constant of type `i32`.
.. define:: type-kind-qualify

   A constant of type `i32`.
.. define:: type-kind-real

   A constant of type `i32`.
.. define:: type-kind-sampled-image

   A constant of type `i32`.
.. define:: type-kind-tuple

   A constant of type `i32`.
.. define:: type-kind-typename

   A constant of type `i32`.
.. define:: type-kind-union

   A constant of type `i32`.
.. define:: type-kind-vector

   A constant of type `i32`.
.. define:: typename-flag-plain

   A constant of type `u32`.
.. define:: unnamed

   A constant of type `Symbol`.
.. define:: unroll-limit
   
   A constant of type `i32` indicating the maximum number of recursions
   permitted for an inline. When this number is exceeded, an error is raised
   during typechecking. Currently, the limit is set at 64 recursions. This
   restriction has been put in place to prevent the compiler from overflowing
   its stack memory.
.. define:: value-kind-argument-list

   A constant of type `i32`.
.. define:: value-kind-argument-list-template

   A constant of type `i32`.
.. define:: value-kind-break

   A constant of type `i32`.
.. define:: value-kind-call

   A constant of type `i32`.
.. define:: value-kind-call-template

   A constant of type `i32`.
.. define:: value-kind-compile-stage

   A constant of type `i32`.
.. define:: value-kind-condbr

   A constant of type `i32`.
.. define:: value-kind-const-aggregate

   A constant of type `i32`.
.. define:: value-kind-const-int

   A constant of type `i32`.
.. define:: value-kind-const-pointer

   A constant of type `i32`.
.. define:: value-kind-const-real

   A constant of type `i32`.
.. define:: value-kind-exception

   A constant of type `i32`.
.. define:: value-kind-expression

   A constant of type `i32`.
.. define:: value-kind-extract-argument

   A constant of type `i32`.
.. define:: value-kind-extract-argument-template

   A constant of type `i32`.
.. define:: value-kind-function

   A constant of type `i32`.
.. define:: value-kind-global

   A constant of type `i32`.
.. define:: value-kind-if

   A constant of type `i32`.
.. define:: value-kind-keyed

   A constant of type `i32`.
.. define:: value-kind-keyed-template

   A constant of type `i32`.
.. define:: value-kind-label

   A constant of type `i32`.
.. define:: value-kind-label-template

   A constant of type `i32`.
.. define:: value-kind-loop

   A constant of type `i32`.
.. define:: value-kind-loop-arguments

   A constant of type `i32`.
.. define:: value-kind-loop-label

   A constant of type `i32`.
.. define:: value-kind-loop-label-arguments

   A constant of type `i32`.
.. define:: value-kind-merge

   A constant of type `i32`.
.. define:: value-kind-merge-template

   A constant of type `i32`.
.. define:: value-kind-parameter

   A constant of type `i32`.
.. define:: value-kind-parameter-template

   A constant of type `i32`.
.. define:: value-kind-pure-cast

   A constant of type `i32`.
.. define:: value-kind-quote

   A constant of type `i32`.
.. define:: value-kind-raise

   A constant of type `i32`.
.. define:: value-kind-raise-template

   A constant of type `i32`.
.. define:: value-kind-repeat

   A constant of type `i32`.
.. define:: value-kind-repeat-template

   A constant of type `i32`.
.. define:: value-kind-return

   A constant of type `i32`.
.. define:: value-kind-return-template

   A constant of type `i32`.
.. define:: value-kind-switch

   A constant of type `i32`.
.. define:: value-kind-switch-template

   A constant of type `i32`.
.. define:: value-kind-template

   A constant of type `i32`.
.. define:: value-kind-unquote

   A constant of type `i32`.
.. type:: Anchor

   A plain type of storage type `_Anchor(*)`.

.. type:: Arguments

   An opaque type.

   .. spice:: (__typecall ...)
.. type:: Builtin

   A plain type of storage type `u64`.

.. type:: CEnum

   An opaque type of supertype `immutable`.

.. type:: CStruct

   An opaque type.

   .. spice:: (__typecall ...)
.. type:: CUnion

   An opaque type.

   .. inline:: (__typecall cls)
.. type:: Closure

   A plain type of storage type `_Closure(*)`.

   .. compiledfn:: (docstring ...)

      An external function of type ``String<-(Closure)``.
.. type:: Collector

   A plain type of storage type `_Closure(*)`.

   .. spice:: (__call ...)
   .. inline:: (__typecall cls init valid? at collect)
.. type:: CompileStage

   A plain type of storage type `{_Value Anchor}`.

.. type:: Error

   A plain type of storage type `_Error(*)`.

.. type:: Generator

   A plain type of storage type `_Closure(*)`.

   .. spice:: (__call ...)
   .. inline:: (__typecall cls start valid? at next)
.. type:: Image

   An opaque type.

   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``.
.. type:: Nothing

   A plain type of storage type `{}`.

.. type:: NullType

   A plain type of storage type `void(*)`.

.. type:: OverloadedFunction

   An opaque type.

   .. spice:: (__typecall ...)
   .. spice:: (append ...)
.. type:: Qualify

   An opaque type.

.. type:: Raises

   An opaque type.

.. type:: SampledImage

   An opaque type.

   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<-(type)``.
.. type:: Sampler

   An opaque type.

.. type:: Scope

   A plain type of storage type `_Scope(*)`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<->Error(Scope Symbol)``.
   .. spice:: (__typecall ...)
   .. spice:: (define-internal-symbol ...)
   .. spice:: (define-symbol ...)
   .. inline:: (define-symbols self values...)
   .. inline:: (deleted self)
   .. compiledfn:: (docstring ...)

      An external function of type ``String<-(Scope Symbol)``.
   .. compiledfn:: (next ...)

      An external function of type ``λ(Symbol Value)<-(Scope Symbol)``.
   .. compiledfn:: (next-deleted ...)

      An external function of type ``Symbol<-(Scope Symbol)``.
   .. compiledfn:: (parent ...)

      An external function of type ``Scope<-(Scope)``.
   .. compiledfn:: (set-docstring! ...)

      An external function of type ``void<-(Scope Symbol String)``.
   .. spice:: (set-symbol ...)
   .. inline:: (set-symbols self values...)
.. type:: SourceFile

   A plain type of storage type `_SourceFile(*)`.

.. type:: SpiceMacro

   A plain type of storage type `Value<->Error(Value)(*)`.

.. type:: SpiceMacroFunction

   A plain type labeled ``Value<->Error(Value)(*)`` of supertype `pointer` and of storage type `Value<->Error(Value)(*)`.

.. type:: Struct

   An opaque type.

   .. spice:: (__typecall ...)
.. type:: SugarMacro

   A plain type of storage type `λ(List Scope)<->Error(List Scope)(*)`.

   .. spice:: (__call ...)
.. type:: SugarMacroFunction

   A plain type labeled ``λ(List Scope)<->Error(List Scope)(*)`` of supertype `pointer` and of storage type `λ(List Scope)<->Error(List Scope)(*)`.

.. type:: Symbol

   A plain type of supertype `immutable` and of storage type `u64`.

   .. spice:: (__call ...)
   .. inline:: (__typecall cls str)
   .. inline:: (unique cls name)
   .. compiledfn:: (variadic? ...)

      An external function of type ``bool<-(Symbol)``.
.. type:: TypeArrayPointer

   A plain type labeled ``type(*)`` of supertype `pointer` and of storage type `type(*)`.

.. type:: Unknown

   A plain type of storage type `_type(*)`.

.. type:: Value

   A plain type of storage type `{_Value Anchor}`.

   .. spice:: (__typecall ...)
   .. compiledfn:: (anchor ...)

      An external function of type ``Anchor<-(Value)``.
   .. inline:: (append-sink self)
   .. compiledfn:: (argcount ...)

      An external function of type ``i32<-(Value)``.
   .. inline:: (args self)
   .. compiledfn:: (constant? ...)

      An external function of type ``bool<-(Value)``.
   .. fn:: (dekey self)
   .. inline:: (dump self)
   .. compiledfn:: (getarg ...)

      An external function of type ``Value<-(Value i32)``.
   .. compiledfn:: (getarglist ...)

      An external function of type ``Value<-(Value i32)``.
   .. compiledfn:: (kind ...)

      An external function of type ``i32<-(Value)``.
   .. compiledfn:: (none? ...)

      A compiled function of type ``bool<-(Value)``.
   .. compiledfn:: (pure? ...)

      An external function of type ``bool<-(Value)``.
   .. compiledfn:: (qualified-typeof ...)

      An external function of type ``type<-(Value)``.
   .. inline:: (reverse-args self)
   .. compiledfn:: (spice-repr ...)

      An external function of type ``String<-(Value)``.
   .. inline:: (tag self anchor)
   .. compiledfn:: (typeof ...)

      An external function of type ``type<-(Value)``.
.. type:: ValueArrayPointer

   A plain type labeled ``Value(*)`` of supertype `pointer` and of storage type `Value(*)`.

.. type:: Variadic

   An opaque type labeled ``...``.

.. type:: aggregate

   An opaque type.

.. type:: array

   An opaque type of supertype `aggregate`.

   .. spice:: (__typecall ...)
   .. inline:: (type element-type size)
.. type:: bool

   A plain type of supertype `integer` and of storage type `bool`.

.. type:: constant

   An opaque type.

.. type:: f16

   A plain type of supertype `real` and of storage type `f16`.

.. type:: f32

   A plain type of supertype `real` and of storage type `f32`.

.. type:: f64

   A plain type of supertype `real` and of storage type `f64`.

.. type:: f80

   A plain type of supertype `real` and of storage type `f80`.

.. type:: function

   An opaque type.

   .. spice:: (__typecall ...)
   .. spice:: (type ...)
.. type:: hash

   A plain type of storage type `u64`.

   .. spice:: (__typecall ...)
   .. inline:: (from-bytes data size)
.. type:: i16

   A plain type of supertype `integer` and of storage type `i16`.

.. type:: i32

   A plain type of supertype `integer` and of storage type `i32`.

.. type:: i64

   A plain type of supertype `integer` and of storage type `i64`.

.. type:: i8

   A plain type of supertype `integer` and of storage type `i8`.

.. type:: immutable

   An opaque type.

.. type:: incomplete

   An opaque type.

.. type:: integer

   An opaque type of supertype `immutable`.

   .. inline:: (__typecall cls value)
.. type:: intptr

   A plain type labeled ``u64`` of supertype `integer` and of storage type `u64`.

.. type:: list

   A plain type labeled ``List`` of storage type `_List(*)`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<-(List)``.
   .. spice:: (__typecall ...)
   .. inline:: (cons-sink self)
   .. inline:: (decons self count)
   .. compiledfn:: (dump ...)

      An external function of type ``List<-(List)``.
   .. compiledfn:: (join ...)

      An external function of type ``List<-(List List)``.
   .. compiledfn:: (next ...)

      An external function of type ``List<-(List)``.
   .. compiledfn:: (reverse ...)

      An external function of type ``List<-(List)``.
   .. fn:: (rjoin lside rside)
   .. fn:: (token-split expr token errmsg)
.. type:: noreturn

   An opaque type.

.. type:: opaquepointer

   An opaque type.

.. type:: pointer

   An opaque type.

   .. spice:: (__call ...)
   .. spice:: (__typecall ...)
   .. inline:: (type T)
.. type:: rawstring

   A plain type labeled ``i8(*)`` of supertype `pointer` and of storage type `i8(*)`.

.. type:: real

   An opaque type of supertype `immutable`.

   .. inline:: (__typecall cls value)
.. type:: string

   A plain type labeled ``String`` of supertype `opaquepointer` and of storage type `_String(*)`.

   .. compiledfn:: (buffer ...)

      An external function of type ``λ(i8(*) usize)<-(String)``.
   .. compiledfn:: (join ...)

      An external function of type ``String<-(String String)``.
   .. compiledfn:: (match? ...)

      An external function of type ``bool<->Error(String String)``.
.. type:: tuple

   An opaque type of supertype `aggregate`.

   .. spice:: (__typecall ...)
   .. spice:: (type ...)
.. type:: type

   A plain type of supertype `opaquepointer` and of storage type `_type(*)`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<->Error(type Symbol)``.
   .. spice:: (__call ...)
   .. compiledfn:: (alignof ...)

      An external function of type ``usize<->Error(type)``.
   .. compiledfn:: (bitcount ...)

      An external function of type ``i32<-(type)``.
   .. fn:: (change-element-type cls ET)
   .. fn:: (change-storage-class cls storage-class)
   .. spice:: (define-symbol ...)
   .. inline:: (define-symbols self values...)
   .. spice:: (dispatch-attr ...)
   .. compiledfn:: (element-count ...)

      An external function of type ``i32<->Error(type)``.
   .. compiledfn:: (element@ ...)

      An external function of type ``type<->Error(type i32)``.
   .. inline:: (elements self)
   .. fn:: (function-pointer? cls)
   .. fn:: (function? cls)
   .. fn:: (immutable cls)
   .. compiledfn:: (key ...)

      An external function of type ``λ(Symbol type)<-(type)``.
   .. inline:: (key-type self key)
   .. compiledfn:: (kind ...)

      An external function of type ``i32<-(type)``.
   .. compiledfn:: (local@ ...)

      An external function of type ``Value<->Error(type Symbol)``.
   .. fn:: (mutable cls)
   .. compiledfn:: (opaque? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (plain? ...)

      An external function of type ``bool<-(type)``.
   .. fn:: (pointer->refer-type cls)
   .. fn:: (pointer-storage-class cls)
   .. fn:: (pointer? cls)
   .. spice:: (raises ...)
   .. fn:: (readable? cls)
   .. compiledfn:: (refer? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (return-type ...)

      An external function of type ``λ(type type)<-(type)``.
   .. inline:: (set-plain-storage type storage-type)
   .. inline:: (set-storage type storage-type)
   .. spice:: (set-symbol ...)
   .. inline:: (set-symbols self values...)
   .. compiledfn:: (signed? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (sizeof ...)

      An external function of type ``usize<->Error(type)``.
   .. compiledfn:: (storageof ...)

      An external function of type ``type<->Error(type)``.
   .. compiledfn:: (string ...)

      An external function of type ``String<-(type)``.
   .. fn:: (strip-pointer-storage-class cls)
   .. compiledfn:: (superof ...)

      An external function of type ``type<-(type)``.
   .. inline:: (symbols self)
   .. compiledfn:: (unique-type ...)

      An external function of type ``type<-(type i32)``.
   .. compiledfn:: (variadic? ...)

      An external function of type ``bool<-(type)``.
   .. inline:: (view-type self id)
   .. fn:: (writable? cls)
.. type:: typename

   An opaque type.

   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<->Error(String type)``.
.. type:: u16

   A plain type of supertype `integer` and of storage type `u16`.

.. type:: u32

   A plain type of supertype `integer` and of storage type `u32`.

.. type:: u64

   A plain type of supertype `integer` and of storage type `u64`.

.. type:: u8

   A plain type of supertype `integer` and of storage type `u8`.

.. type:: union

   An opaque type.

.. type:: usize

   A plain type of supertype `integer` and of storage type `u64`.

.. type:: vector

   An opaque type of supertype `immutable`.

   .. spice:: (__typecall ...)
   .. spice:: (smear ...)
   .. inline:: (type element-type size)
.. type:: void

   An opaque type of supertype `Arguments`.

.. type:: voidstar

   A plain type labeled ``void(*)`` of supertype `pointer` and of storage type `void(*)`.

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
.. spice:: (* ...)
.. spice:: (+ ...)
.. spice:: (- ...)
.. spice:: (/ ...)
.. spice:: (< ...)
.. spice:: (= ...)
.. spice:: (> ...)
.. spice:: (@ ...)
.. spice:: (^ ...)
.. spice:: (| ...)
.. spice:: (~ ...)
.. spice:: (!= ...)
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

   An external function of type ``λ(i32 i32 i32)<-()``.
.. compiledfn:: (default-styler ...)

   An external function of type ``String<-(Symbol String)``.
.. compiledfn:: (exit ...)

   An external function of type ``noreturn<-(i32)``.
.. compiledfn:: (function->SugarMacro ...)

   A compiled function of type ``SugarMacro<-(λ(List Scope)<->Error(List Scope)(*))``.
.. compiledfn:: (globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (io-write! ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (launch-args ...)

   An external function of type ``λ(i32 i8(*)(*))<-()``.
.. compiledfn:: (list-load ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (list-parse ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (load-library ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (parse-infix-expr ...)

   A compiled function of type ``λ(Value List)<->Error(Scope Value List i32)``.
.. compiledfn:: (realpath ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_abort ...)

   An external function of type ``noreturn<-()``.
.. compiledfn:: (sc_anchor_offset ...)

   An external function of type ``Anchor<-(Anchor i32)``.
.. compiledfn:: (sc_argcount ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_argument_list_append ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_argument_list_new ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_arguments_type ...)

   An external function of type ``type<-(i32 type(*))``.
.. compiledfn:: (sc_arguments_type_argcount ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_arguments_type_getarg ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_arguments_type_join ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_array_type ...)

   An external function of type ``type<->Error(type usize)``.
.. compiledfn:: (sc_basename ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_break_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_call_append_argument ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_call_is_rawcall ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_call_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_call_set_rawcall ...)

   An external function of type ``void<-(Value bool)``.
.. compiledfn:: (sc_closure_get_context ...)

   An external function of type ``Value<-(Closure)``.
.. compiledfn:: (sc_closure_get_docstring ...)

   An external function of type ``String<-(Closure)``.
.. compiledfn:: (sc_closure_get_template ...)

   An external function of type ``Value<-(Closure)``.
.. compiledfn:: (sc_compile ...)

   An external function of type ``Value<->Error(Value u64)``.
.. compiledfn:: (sc_compile_glsl ...)

   An external function of type ``String<->Error(Symbol Value u64)``.
.. compiledfn:: (sc_compile_object ...)

   An external function of type ``void<->Error(String Scope u64)``.
.. compiledfn:: (sc_compile_spirv ...)

   An external function of type ``String<->Error(Symbol Value u64)``.
.. compiledfn:: (sc_compiler_version ...)

   An external function of type ``λ(i32 i32 i32)<-()``.
.. compiledfn:: (sc_const_aggregate_new ...)

   An external function of type ``Value<-(type i32 Value(*))``.
.. compiledfn:: (sc_const_extract_at ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_const_int_extract ...)

   An external function of type ``u64<-(Value)``.
.. compiledfn:: (sc_const_int_new ...)

   An external function of type ``Value<-(type u64)``.
.. compiledfn:: (sc_const_pointer_extract ...)

   An external function of type ``void(*)<-(Value)``.
.. compiledfn:: (sc_const_pointer_new ...)

   An external function of type ``Value<-(type void(*))``.
.. compiledfn:: (sc_const_real_extract ...)

   An external function of type ``f64<-(Value)``.
.. compiledfn:: (sc_const_real_new ...)

   An external function of type ``Value<-(type f64)``.
.. compiledfn:: (sc_default_styler ...)

   An external function of type ``String<-(Symbol String)``.
.. compiledfn:: (sc_dirname ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_dump_error ...)

   An external function of type ``void<-(Error)``.
.. compiledfn:: (sc_empty_argument_list ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_enter_solver_cli ...)

   An external function of type ``void<-()``.
.. compiledfn:: (sc_error_append_calltrace ...)

   An external function of type ``void<-(Error Value)``.
.. compiledfn:: (sc_error_new ...)

   An external function of type ``Error<-(String)``.
.. compiledfn:: (sc_eval ...)

   An external function of type ``Value<->Error(Anchor List Scope)``.
.. compiledfn:: (sc_eval_inline ...)

   An external function of type ``Anchor<->Error(Value List Scope)``.
.. compiledfn:: (sc_exit ...)

   An external function of type ``noreturn<-(i32)``.
.. compiledfn:: (sc_expand ...)

   An external function of type ``λ(Value List Scope)<->Error(Value List Scope)``.
.. compiledfn:: (sc_expression_append ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_expression_new ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_expression_set_scoped ...)

   An external function of type ``void<-(Value)``.
.. compiledfn:: (sc_extract_argument_list_new ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_extract_argument_new ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_format_error ...)

   An external function of type ``String<-(Error)``.
.. compiledfn:: (sc_format_message ...)

   An external function of type ``String<-(Anchor String)``.
.. compiledfn:: (sc_function_type ...)

   An external function of type ``type<-(type i32 type(*))``.
.. compiledfn:: (sc_function_type_is_variadic ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_function_type_raising ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_function_type_return_type ...)

   An external function of type ``λ(type type)<-(type)``.
.. compiledfn:: (sc_get_globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_get_original_globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_getarg ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_getarglist ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_global_new ...)

   An external function of type ``Value<-(Symbol type u32 Symbol i32 i32)``.
.. compiledfn:: (sc_hash ...)

   An external function of type ``u64<-(u64 usize)``.
.. compiledfn:: (sc_hash2x64 ...)

   An external function of type ``u64<-(u64 u64)``.
.. compiledfn:: (sc_hashbytes ...)

   An external function of type ``u64<-(i8(*) usize)``.
.. compiledfn:: (sc_if_append_else_clause ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_if_append_then_clause ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_if_new ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_image_type ...)

   An external function of type ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``.
.. compiledfn:: (sc_import_c ...)

   An external function of type ``Scope<->Error(String String List)``.
.. compiledfn:: (sc_integer_type ...)

   An external function of type ``type<-(i32 bool)``.
.. compiledfn:: (sc_integer_type_is_signed ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_is_directory ...)

   An external function of type ``bool<-(String)``.
.. compiledfn:: (sc_is_file ...)

   An external function of type ``bool<-(String)``.
.. compiledfn:: (sc_key_type ...)

   An external function of type ``type<-(Symbol type)``.
.. compiledfn:: (sc_keyed_new ...)

   An external function of type ``Value<-(Symbol Value)``.
.. compiledfn:: (sc_label_new ...)

   An external function of type ``Value<-(i32 Symbol)``.
.. compiledfn:: (sc_label_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_launch_args ...)

   An external function of type ``λ(i32 i8(*)(*))<-()``.
.. compiledfn:: (sc_list_at ...)

   An external function of type ``Value<-(List)``.
.. compiledfn:: (sc_list_compare ...)

   An external function of type ``bool<-(List List)``.
.. compiledfn:: (sc_list_cons ...)

   An external function of type ``List<-(Value List)``.
.. compiledfn:: (sc_list_count ...)

   An external function of type ``i32<-(List)``.
.. compiledfn:: (sc_list_decons ...)

   An external function of type ``λ(Value List)<-(List)``.
.. compiledfn:: (sc_list_dump ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_list_join ...)

   An external function of type ``List<-(List List)``.
.. compiledfn:: (sc_list_next ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_list_repr ...)

   An external function of type ``String<-(List)``.
.. compiledfn:: (sc_list_reverse ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_load_library ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (sc_loop_arguments ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_loop_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_loop_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_map_get ...)

   An external function of type ``Value<->Error(Value)``.
.. compiledfn:: (sc_map_set ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_merge_new ...)

   An external function of type ``Value<-(Value Value)``.
.. compiledfn:: (sc_mutate_type ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_parameter_is_variadic ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_parameter_name ...)

   An external function of type ``Symbol<-(Value)``.
.. compiledfn:: (sc_parameter_new ...)

   An external function of type ``Value<-(Symbol)``.
.. compiledfn:: (sc_parse_from_path ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (sc_parse_from_string ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (sc_pointer_type ...)

   An external function of type ``type<-(type u64 Symbol)``.
.. compiledfn:: (sc_pointer_type_get_flags ...)

   An external function of type ``u64<-(type)``.
.. compiledfn:: (sc_pointer_type_get_storage_class ...)

   An external function of type ``Symbol<-(type)``.
.. compiledfn:: (sc_pointer_type_set_element_type ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_pointer_type_set_flags ...)

   An external function of type ``type<-(type u64)``.
.. compiledfn:: (sc_pointer_type_set_storage_class ...)

   An external function of type ``type<-(type Symbol)``.
.. compiledfn:: (sc_prompt ...)

   An external function of type ``λ(bool String)<-(String String)``.
.. compiledfn:: (sc_prove ...)

   An external function of type ``Value<->Error(Value)``.
.. compiledfn:: (sc_quote_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_raise_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_realpath ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_refer_type ...)

   An external function of type ``type<-(type u64 Symbol)``.
.. compiledfn:: (sc_repeat_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_return_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_sampled_image_type ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_scope_at ...)

   An external function of type ``Value<->Error(Scope Symbol)``.
.. compiledfn:: (sc_scope_clone ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_scope_clone_subscope ...)

   An external function of type ``Scope<-(Scope Scope)``.
.. compiledfn:: (sc_scope_del_symbol ...)

   An external function of type ``void<-(Scope Symbol)``.
.. compiledfn:: (sc_scope_get_docstring ...)

   An external function of type ``String<-(Scope Symbol)``.
.. compiledfn:: (sc_scope_get_parent ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_scope_local_at ...)

   An external function of type ``Value<->Error(Scope Symbol)``.
.. compiledfn:: (sc_scope_new ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_scope_new_subscope ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_scope_next ...)

   An external function of type ``λ(Symbol Value)<-(Scope Symbol)``.
.. compiledfn:: (sc_scope_next_deleted ...)

   An external function of type ``Symbol<-(Scope Symbol)``.
.. compiledfn:: (sc_scope_set_docstring ...)

   An external function of type ``void<-(Scope Symbol String)``.
.. compiledfn:: (sc_scope_set_symbol ...)

   An external function of type ``void<-(Scope Symbol Value)``.
.. compiledfn:: (sc_set_autocomplete_scope ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (sc_set_globals ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (sc_set_signal_abort ...)

   An external function of type ``void<-(bool)``.
.. compiledfn:: (sc_string_buffer ...)

   An external function of type ``λ(i8(*) usize)<-(String)``.
.. compiledfn:: (sc_string_compare ...)

   An external function of type ``i32<-(String String)``.
.. compiledfn:: (sc_string_count ...)

   An external function of type ``usize<-(String)``.
.. compiledfn:: (sc_string_join ...)

   An external function of type ``String<-(String String)``.
.. compiledfn:: (sc_string_lslice ...)

   An external function of type ``String<-(String usize)``.
.. compiledfn:: (sc_string_match ...)

   An external function of type ``bool<->Error(String String)``.
.. compiledfn:: (sc_string_new ...)

   An external function of type ``String<-(i8(*) usize)``.
.. compiledfn:: (sc_string_new_from_cstr ...)

   An external function of type ``String<-(i8(*))``.
.. compiledfn:: (sc_string_rslice ...)

   An external function of type ``String<-(String usize)``.
.. compiledfn:: (sc_strip_qualifiers ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_switch_append_case ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_switch_append_default ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_switch_append_pass ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_switch_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_symbol_is_variadic ...)

   An external function of type ``bool<-(Symbol)``.
.. compiledfn:: (sc_symbol_new ...)

   An external function of type ``Symbol<-(String)``.
.. compiledfn:: (sc_symbol_new_unique ...)

   An external function of type ``Symbol<-(String)``.
.. compiledfn:: (sc_symbol_to_string ...)

   An external function of type ``String<-(Symbol)``.
.. compiledfn:: (sc_template_append_parameter ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_template_get_name ...)

   An external function of type ``Symbol<-(Value)``.
.. compiledfn:: (sc_template_is_inline ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_template_new ...)

   An external function of type ``Value<-(Symbol)``.
.. compiledfn:: (sc_template_parameter ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_template_parameter_count ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_template_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_template_set_inline ...)

   An external function of type ``void<-(Value)``.
.. compiledfn:: (sc_template_set_name ...)

   An external function of type ``void<-(Value Symbol)``.
.. compiledfn:: (sc_tuple_type ...)

   An external function of type ``type<->Error(i32 type(*))``.
.. compiledfn:: (sc_type_alignof ...)

   An external function of type ``usize<->Error(type)``.
.. compiledfn:: (sc_type_at ...)

   An external function of type ``Value<->Error(type Symbol)``.
.. compiledfn:: (sc_type_bitcountof ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_type_countof ...)

   An external function of type ``i32<->Error(type)``.
.. compiledfn:: (sc_type_debug_abi ...)

   An external function of type ``void<-(type)``.
.. compiledfn:: (sc_type_element_at ...)

   An external function of type ``type<->Error(type i32)``.
.. compiledfn:: (sc_type_field_index ...)

   An external function of type ``i32<->Error(type Symbol)``.
.. compiledfn:: (sc_type_field_name ...)

   An external function of type ``Symbol<->Error(type i32)``.
.. compiledfn:: (sc_type_is_default_suffix ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_opaque ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_plain ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_refer ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_superof ...)

   An external function of type ``bool<-(type type)``.
.. compiledfn:: (sc_type_key ...)

   An external function of type ``λ(Symbol type)<-(type)``.
.. compiledfn:: (sc_type_kind ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_type_local_at ...)

   An external function of type ``Value<->Error(type Symbol)``.
.. compiledfn:: (sc_type_next ...)

   An external function of type ``λ(Symbol Value)<-(type Symbol)``.
.. compiledfn:: (sc_type_set_symbol ...)

   An external function of type ``void<-(type Symbol Value)``.
.. compiledfn:: (sc_type_sizeof ...)

   An external function of type ``usize<->Error(type)``.
.. compiledfn:: (sc_type_storage ...)

   An external function of type ``type<->Error(type)``.
.. compiledfn:: (sc_type_string ...)

   An external function of type ``String<-(type)``.
.. compiledfn:: (sc_typename_type ...)

   An external function of type ``type<->Error(String type)``.
.. compiledfn:: (sc_typename_type_get_super ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_typename_type_set_storage ...)

   An external function of type ``void<->Error(type type u32)``.
.. compiledfn:: (sc_typify ...)

   An external function of type ``Value<->Error(Closure i32 type(*))``.
.. compiledfn:: (sc_typify_template ...)

   An external function of type ``Value<->Error(Value i32 type(*))``.
.. compiledfn:: (sc_union_type ...)

   An external function of type ``type<->Error(i32 type(*))``.
.. compiledfn:: (sc_unique_type ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_unquote_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_value_anchor ...)

   An external function of type ``Anchor<-(Value)``.
.. compiledfn:: (sc_value_ast_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_compare ...)

   An external function of type ``bool<-(Value Value)``.
.. compiledfn:: (sc_value_content_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_is_constant ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_value_is_pure ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_value_kind ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_value_qualified_type ...)

   An external function of type ``type<-(Value)``.
.. compiledfn:: (sc_value_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_tostring ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_type ...)

   An external function of type ``type<-(Value)``.
.. compiledfn:: (sc_value_unwrap ...)

   An external function of type ``Value<-(type Value)``.
.. compiledfn:: (sc_value_wrap ...)

   An external function of type ``Value<-(type Value)``.
.. compiledfn:: (sc_valueref_tag ...)

   An external function of type ``Value<-(Anchor Value)``.
.. compiledfn:: (sc_vector_type ...)

   An external function of type ``type<->Error(type usize)``.
.. compiledfn:: (sc_verify_stack ...)

   An external function of type ``usize<->Error()``.
.. compiledfn:: (sc_view_type ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_write ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (set-autocomplete-scope! ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (set-globals! ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (set-signal-abort! ...)

   An external function of type ``void<-(bool)``.
.. compiledfn:: (spice-macro-verify-signature ...)

   A compiled function of type ``void<-(Value<->Error(Value)(*))``.
.. compiledfn:: (type> ...)

   An external function of type ``bool<-(type type)``.
