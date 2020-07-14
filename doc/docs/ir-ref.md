IR Language Reference
=====================

This chapter will provide reference information for Scopes IR in a future release.

Built-in Types
--------------

*ir-type*{.property} `void`{.descname} [](#scopes.ir-type.void "Permalink to this definition"){.headerlink} {#scopes.ir-type.void}

:	

*ir-type*{.property} `i1`{.descname} [](#scopes.ir-type.i1 "Permalink to this definition"){.headerlink} {#scopes.ir-type.i1}

:	

*ir-type*{.property} `i8`{.descname} [](#scopes.ir-type.i8 "Permalink to this definition"){.headerlink} {#scopes.ir-type.i8}

:	

*ir-type*{.property} `i16`{.descname} [](#scopes.ir-type.i16 "Permalink to this definition"){.headerlink} {#scopes.ir-type.i16}

:	

*ir-type*{.property} `i32`{.descname} [](#scopes.ir-type.i32 "Permalink to this definition"){.headerlink} {#scopes.ir-type.i32}

:	

*ir-type*{.property} `i64`{.descname} [](#scopes.ir-type.i64 "Permalink to this definition"){.headerlink} {#scopes.ir-type.i64}

:	

*ir-type*{.property} `float`{.descname} [](#scopes.ir-type.float "Permalink to this definition"){.headerlink} {#scopes.ir-type.float}

:	

*ir-type*{.property} `double`{.descname} [](#scopes.ir-type.double "Permalink to this definition"){.headerlink} {#scopes.ir-type.double}

:	

*ir-type*{.property} `rawstring`{.descname} [](#scopes.ir-type.rawstring "Permalink to this definition"){.headerlink} {#scopes.ir-type.rawstring}

:	

*ir-type*{.property} `opaque`{.descname} [](#scopes.ir-type.opaque "Permalink to this definition"){.headerlink} {#scopes.ir-type.opaque}

:	

Type Constructors
-----------------

*ir-special*{.property} `(function return-type (? param-type ...) (? _:...))`{.descname} [](#scopes.ir-special.function "Permalink to this definition"){.headerlink} {#scopes.ir-special.function}

:	

*ir-special*{.property} `(pointer type)`{.descname} [](#scopes.ir-special.pointer "Permalink to this definition"){.headerlink} {#scopes.ir-special.pointer}

:	

*ir-special*{.property} `(array type count)`{.descname} [](#scopes.ir-special.array "Permalink to this definition"){.headerlink} {#scopes.ir-special.array}

:	

*ir-special*{.property} `(vector type count)`{.descname} [](#scopes.ir-special.vector "Permalink to this definition"){.headerlink} {#scopes.ir-special.vector}

:	

*ir-special*{.property} `(struct name (? _:packed) (? type ...))`{.descname} [](#scopes.ir-special.struct "Permalink to this definition"){.headerlink} {#scopes.ir-special.struct}

:	

*ir-special*{.property} `(typeof value)`{.descname} [](#scopes.ir-special.typeof "Permalink to this definition"){.headerlink} {#scopes.ir-special.typeof}

:	

*ir-special*{.property} `(getelementtype type (? index ...))`{.descname} [](#scopes.ir-special.getelementtype "Permalink to this definition"){.headerlink} {#scopes.ir-special.getelementtype}

:	

Definitions
-----------

*ir-special*{.property} `(defvalue value)`{.descname} [](#scopes.ir-special.defvalue "Permalink to this definition"){.headerlink} {#scopes.ir-special.defvalue}

:	

*ir-special*{.property} `(deftype value)`{.descname} [](#scopes.ir-special.deftype "Permalink to this definition"){.headerlink} {#scopes.ir-special.deftype}

:	

*ir-special*{.property} `(defstruct name (? _:packed) (? type ...))`{.descname} [](#scopes.ir-special.defstruct "Permalink to this definition"){.headerlink} {#scopes.ir-special.defstruct}

:	

*ir-special*{.property} `(define name ((? param ...)) type (? expression ...))`{.descname} [](#scopes.ir-special.define "Permalink to this definition"){.headerlink} {#scopes.ir-special.define}

:	

*ir-special*{.property} `(declare symbol-string type)`{.descname} [](#scopes.ir-special.declare "Permalink to this definition"){.headerlink} {#scopes.ir-special.declare}

:	

Aggregate Constructors
----------------------

*ir-special*{.property} `(structof (| (? "" (? _:packed)) type) (? const-value ...))`{.descname} [](#scopes.ir-special.structof "Permalink to this definition"){.headerlink} {#scopes.ir-special.structof}

:	

*ir-special*{.property} `(arrayof type (? const-value ...))`{.descname} [](#scopes.ir-special.arrayof "Permalink to this definition"){.headerlink} {#scopes.ir-special.arrayof}

:	

*ir-special*{.property} `(vectorof const-value (? ...))`{.descname} [](#scopes.ir-special.vectorof "Permalink to this definition"){.headerlink} {#scopes.ir-special.vectorof}

:	

Constant Values
---------------

*ir-special*{.property} `(int type integer-value)`{.descname} [](#scopes.ir-special.int "Permalink to this definition"){.headerlink} {#scopes.ir-special.int}

:	Constructs an integer constant of `type`.

    A naked integer is shorthand for `(int i32 <number>)`.

*ir-special*{.property} `(real type real-value)`{.descname} [](#scopes.ir-special.real "Permalink to this definition"){.headerlink} {#scopes.ir-special.real}

:	Constructs a real constant of `type`.

*ir-special*{.property} `(null type)`{.descname} [](#scopes.ir-special.null "Permalink to this definition"){.headerlink} {#scopes.ir-special.null}

:	Constructs a zero initializer for `type`.

*ir-special*{.property} `(alignof type)`{.descname} [](#scopes.ir-special.alignof "Permalink to this definition"){.headerlink} {#scopes.ir-special.alignof}

:	

*ir-special*{.property} `(sizeof type)`{.descname} [](#scopes.ir-special.sizeof "Permalink to this definition"){.headerlink} {#scopes.ir-special.sizeof}

:	

*ir-special*{.property} `(lengthof type)`{.descname} [](#scopes.ir-special.lengthof "Permalink to this definition"){.headerlink} {#scopes.ir-special.lengthof}

:	

*ir-macro*{.property} `(&str string)`{.descname} [](#scopes.ir-macro.&str "Permalink to this definition"){.headerlink} {#scopes.ir-macro.&str}

:	

    Constructs a global string constant and returns it as `rawstring`.

Flow Control
------------

*ir-special*{.property} `(block name)`{.descname} [](#scopes.ir-special.block "Permalink to this definition"){.headerlink} {#scopes.ir-special.block}

:	

*ir-special*{.property} `(set-block block-expr)`{.descname} [](#scopes.ir-special.set-block "Permalink to this definition"){.headerlink} {#scopes.ir-special.set-block}

:	

*ir-special*{.property} `(call callee (? argument-value ...))`{.descname} [](#scopes.ir-special.call "Permalink to this definition"){.headerlink} {#scopes.ir-special.call}

:	

*ir-special*{.property} `(ret (? return-value))`{.descname} [](#scopes.ir-special.ret "Permalink to this definition"){.headerlink} {#scopes.ir-special.ret}

:	

*ir-special*{.property} `(br label-value)`{.descname} [](#scopes.ir-special.br "Permalink to this definition"){.headerlink} {#scopes.ir-special.br}

:	

*ir-special*{.property} `(cond-br value then-label-value else-label-value)`{.descname} [](#scopes.ir-special.cond-br "Permalink to this definition"){.headerlink} {#scopes.ir-special.cond-br}

:	

*ir-special*{.property} `(phi type (? (value label-value) ...))`{.descname} [](#scopes.ir-special.phi "Permalink to this definition"){.headerlink} {#scopes.ir-special.phi}

:	

*ir-special*{.property} `(incoming phi-value (? (value label-value) ...))`{.descname} [](#scopes.ir-special.incoming "Permalink to this definition"){.headerlink} {#scopes.ir-special.incoming}

:	

*ir-macro*{.property} `(? condition-expr then-expr else-expr)`{.descname} [](#scopes.ir-macro.? "Permalink to this definition"){.headerlink} {#scopes.ir-macro.?}

:	

*ir-macro*{.property} `(and? a-expr b-expr)`{.descname} [](#scopes.ir-macro.and? "Permalink to this definition"){.headerlink} {#scopes.ir-macro.and?}

:	

*ir-macro*{.property} `(or? a-expr b-expr)`{.descname} [](#scopes.ir-macro.or? "Permalink to this definition"){.headerlink} {#scopes.ir-macro.or?}

:	

*ir-macro*{.property} `(if (condition-expr expression ...) ... (? (_:else expression ...)))`{.descname} [](#scopes.ir-macro.if "Permalink to this definition"){.headerlink} {#scopes.ir-macro.if}

:	

*ir-macro*{.property} `(loop var-name init-expr condition-expr iterate-expr expression ...)`{.descname} [](#scopes.ir-macro.loop "Permalink to this definition"){.headerlink} {#scopes.ir-macro.loop}

:	

*ir-macro*{.property} `(assert condition-expr (? exception-expr))`{.descname} [](#scopes.ir-macro.assert "Permalink to this definition"){.headerlink} {#scopes.ir-macro.assert}

:	

Unary Operators
---------------

*ir-macro*{.property} `(not pointer-expr)`{.descname} [](#scopes.ir-macro.not "Permalink to this definition"){.headerlink} {#scopes.ir-macro.not}

:	

Binary Operators
----------------

*ir-special*{.property} `(add lhs rhs)`{.descname} [](#scopes.ir-special.add "Permalink to this definition"){.headerlink} {#scopes.ir-special.add}

:	

*ir-special*{.property} `(add-nsw lhs rhs)`{.descname} [](#scopes.ir-special.add-nsw "Permalink to this definition"){.headerlink} {#scopes.ir-special.add-nsw}

:	

*ir-special*{.property} `(add-nuw lhs rhs)`{.descname} [](#scopes.ir-special.add-nuw "Permalink to this definition"){.headerlink} {#scopes.ir-special.add-nuw}

:	

*ir-special*{.property} `(fadd lhs rhs)`{.descname} [](#scopes.ir-special.fadd "Permalink to this definition"){.headerlink} {#scopes.ir-special.fadd}

:	

*ir-special*{.property} `(sub lhs rhs)`{.descname} [](#scopes.ir-special.sub "Permalink to this definition"){.headerlink} {#scopes.ir-special.sub}

:	

*ir-special*{.property} `(sub-nsw lhs rhs)`{.descname} [](#scopes.ir-special.sub-nsw "Permalink to this definition"){.headerlink} {#scopes.ir-special.sub-nsw}

:	

*ir-special*{.property} `(sub-nuw lhs rhs)`{.descname} [](#scopes.ir-special.sub-nuw "Permalink to this definition"){.headerlink} {#scopes.ir-special.sub-nuw}

:	

*ir-special*{.property} `(fsub lhs rhs)`{.descname} [](#scopes.ir-special.fsub "Permalink to this definition"){.headerlink} {#scopes.ir-special.fsub}

:	

*ir-special*{.property} `(mul lhs rhs)`{.descname} [](#scopes.ir-special.mul "Permalink to this definition"){.headerlink} {#scopes.ir-special.mul}

:	

*ir-special*{.property} `(mul-nsw lhs rhs)`{.descname} [](#scopes.ir-special.mul-nsw "Permalink to this definition"){.headerlink} {#scopes.ir-special.mul-nsw}

:	

*ir-special*{.property} `(mul-nuw lhs rhs)`{.descname} [](#scopes.ir-special.mul-nuw "Permalink to this definition"){.headerlink} {#scopes.ir-special.mul-nuw}

:	

*ir-special*{.property} `(fmul lhs rhs)`{.descname} [](#scopes.ir-special.fmul "Permalink to this definition"){.headerlink} {#scopes.ir-special.fmul}

:	

*ir-special*{.property} `(udiv lhs rhs)`{.descname} [](#scopes.ir-special.udiv "Permalink to this definition"){.headerlink} {#scopes.ir-special.udiv}

:	

*ir-special*{.property} `(sdiv lhs rhs)`{.descname} [](#scopes.ir-special.sdiv "Permalink to this definition"){.headerlink} {#scopes.ir-special.sdiv}

:	

*ir-special*{.property} `(exact-sdiv lhs rhs)`{.descname} [](#scopes.ir-special.exact-sdiv "Permalink to this definition"){.headerlink} {#scopes.ir-special.exact-sdiv}

:	

*ir-special*{.property} `(urem lhs rhs)`{.descname} [](#scopes.ir-special.urem "Permalink to this definition"){.headerlink} {#scopes.ir-special.urem}

:	

*ir-special*{.property} `(srem lhs rhs)`{.descname} [](#scopes.ir-special.srem "Permalink to this definition"){.headerlink} {#scopes.ir-special.srem}

:	

*ir-special*{.property} `(frem lhs rhs)`{.descname} [](#scopes.ir-special.frem "Permalink to this definition"){.headerlink} {#scopes.ir-special.frem}

:	

*ir-special*{.property} `(shl lhs rhs)`{.descname} [](#scopes.ir-special.shl "Permalink to this definition"){.headerlink} {#scopes.ir-special.shl}

:	

*ir-special*{.property} `(lshr lhs rhs)`{.descname} [](#scopes.ir-special.lshr "Permalink to this definition"){.headerlink} {#scopes.ir-special.lshr}

:	

*ir-special*{.property} `(ashr lhs rhs)`{.descname} [](#scopes.ir-special.ashr "Permalink to this definition"){.headerlink} {#scopes.ir-special.ashr}

:	

*ir-special*{.property} `(and lhs rhs)`{.descname} [](#scopes.ir-special.and "Permalink to this definition"){.headerlink} {#scopes.ir-special.and}

:	

*ir-special*{.property} `(or lhs rhs)`{.descname} [](#scopes.ir-special.or "Permalink to this definition"){.headerlink} {#scopes.ir-special.or}

:	

*ir-special*{.property} `(xor lhs rhs)`{.descname} [](#scopes.ir-special.xor "Permalink to this definition"){.headerlink} {#scopes.ir-special.xor}

:	

Comparators
-----------

*ir-special*{.property} `(icmp op lhs rhs)`{.descname} [](#scopes.ir-special.icmp "Permalink to this definition"){.headerlink} {#scopes.ir-special.icmp}

:	

*ir-special*{.property} `(fcmp op lhs rhs)`{.descname} [](#scopes.ir-special.fcmp "Permalink to this definition"){.headerlink} {#scopes.ir-special.fcmp}

:	

*ir-special*{.property} `(select condition-expr then-value else-value)`{.descname} [](#scopes.ir-special.select "Permalink to this definition"){.headerlink} {#scopes.ir-special.select}

:	

*ir-macro*{.property} `(null? pointer-expr)`{.descname} [](#scopes.ir-macro.null? "Permalink to this definition"){.headerlink} {#scopes.ir-macro.null?}

:	

Composition
-----------

*ir-special*{.property} `(getelementptr value (? index-value ...))`{.descname} [](#scopes.ir-special.getelementptr "Permalink to this definition"){.headerlink} {#scopes.ir-special.getelementptr}

:	

*ir-special*{.property} `(extractelement value index)`{.descname} [](#scopes.ir-special.extractelement "Permalink to this definition"){.headerlink} {#scopes.ir-special.extractelement}

:	

*ir-special*{.property} `(insertelement value element index)`{.descname} [](#scopes.ir-special.insertelement "Permalink to this definition"){.headerlink} {#scopes.ir-special.insertelement}

:	

*ir-special*{.property} `(shufflevector value1 value2 mask)`{.descname} [](#scopes.ir-special.shufflevector "Permalink to this definition"){.headerlink} {#scopes.ir-special.shufflevector}

:	

*ir-special*{.property} `(extractvalue value index)`{.descname} [](#scopes.ir-special.extractvalue "Permalink to this definition"){.headerlink} {#scopes.ir-special.extractvalue}

:	

*ir-special*{.property} `(insertvalue value element index)`{.descname} [](#scopes.ir-special.insertvalue "Permalink to this definition"){.headerlink} {#scopes.ir-special.insertvalue}

:	

Memory
------

*ir-special*{.property} `(align value bytes)`{.descname} [](#scopes.ir-special.align "Permalink to this definition"){.headerlink} {#scopes.ir-special.align}

:	

*ir-special*{.property} `(load from-value)`{.descname} [](#scopes.ir-special.load "Permalink to this definition"){.headerlink} {#scopes.ir-special.load}

:	

*ir-special*{.property} `(store from-value to-value)`{.descname} [](#scopes.ir-special.store "Permalink to this definition"){.headerlink} {#scopes.ir-special.store}

:	

*ir-special*{.property} `(alloca type (? count-value))`{.descname} [](#scopes.ir-special.alloca "Permalink to this definition"){.headerlink} {#scopes.ir-special.alloca}

:	

*ir-special*{.property} `(va_arg va_list-value type)`{.descname} [](#scopes.ir-special.va_arg "Permalink to this definition"){.headerlink} {#scopes.ir-special.va_arg}

:	

Global Values
-------------

*ir-special*{.property} `(constant global-value)`{.descname} [](#scopes.ir-special.constant "Permalink to this definition"){.headerlink} {#scopes.ir-special.constant}

:	

*ir-special*{.property} `(global name constant-value)`{.descname} [](#scopes.ir-special.global "Permalink to this definition"){.headerlink} {#scopes.ir-special.global}

:	

Casting
-------

*ir-special*{.property} `(trunc value type)`{.descname} [](#scopes.ir-special.trunc "Permalink to this definition"){.headerlink} {#scopes.ir-special.trunc}

:	

*ir-special*{.property} `(zext value type)`{.descname} [](#scopes.ir-special.zext "Permalink to this definition"){.headerlink} {#scopes.ir-special.zext}

:	

*ir-special*{.property} `(sext value type)`{.descname} [](#scopes.ir-special.sext "Permalink to this definition"){.headerlink} {#scopes.ir-special.sext}

:	

*ir-special*{.property} `(fptrunc value type)`{.descname} [](#scopes.ir-special.fptrunc "Permalink to this definition"){.headerlink} {#scopes.ir-special.fptrunc}

:	

*ir-special*{.property} `(fpext value type)`{.descname} [](#scopes.ir-special.fpext "Permalink to this definition"){.headerlink} {#scopes.ir-special.fpext}

:	

*ir-special*{.property} `(fptoui value type)`{.descname} [](#scopes.ir-special.fptoui "Permalink to this definition"){.headerlink} {#scopes.ir-special.fptoui}

:	

*ir-special*{.property} `(fptosi value type)`{.descname} [](#scopes.ir-special.fptosi "Permalink to this definition"){.headerlink} {#scopes.ir-special.fptosi}

:	

*ir-special*{.property} `(uitofp value type)`{.descname} [](#scopes.ir-special.uitofp "Permalink to this definition"){.headerlink} {#scopes.ir-special.uitofp}

:	

*ir-special*{.property} `(sitofp value type)`{.descname} [](#scopes.ir-special.sitofp "Permalink to this definition"){.headerlink} {#scopes.ir-special.sitofp}

:	

*ir-special*{.property} `(ptrtoint value type)`{.descname} [](#scopes.ir-special.ptrtoint "Permalink to this definition"){.headerlink} {#scopes.ir-special.ptrtoint}

:	

*ir-special*{.property} `(intotptr value type)`{.descname} [](#scopes.ir-special.intotptr "Permalink to this definition"){.headerlink} {#scopes.ir-special.intotptr}

:	

*ir-special*{.property} `(bitcast value type)`{.descname} [](#scopes.ir-special.bitcast "Permalink to this definition"){.headerlink} {#scopes.ir-special.bitcast}

:	

*ir-special*{.property} `(addrspacecast value type)`{.descname} [](#scopes.ir-special.addrspacecast "Permalink to this definition"){.headerlink} {#scopes.ir-special.addrspacecast}

:	

Debugging
---------

*ir-special*{.property} `(dump-module)`{.descname} [](#scopes.ir-special.dump-module)`{.descname} "Permalink to this definition"){.headerlink} {#scopes.ir-special.dump-module)`{.descname}}

:	

*ir-special*{.property} `(dump value)`{.descname} [](#scopes.ir-special.dump "Permalink to this definition"){.headerlink} {#scopes.ir-special.dump}

:	

*ir-special*{.property} `(dumptype type)`{.descname} [](#scopes.ir-special.dumptype "Permalink to this definition"){.headerlink} {#scopes.ir-special.dumptype}

:	

Metaprogramming
---------------

*ir-special*{.property} `(include filename-string)`{.descname} [](#scopes.ir-special.include "Permalink to this definition"){.headerlink} {#scopes.ir-special.include}

:	Includes expressions from another source file into the module currently being
    defined. `filename-string` is the path to the source file to be included,
    relative to the path of the expression's anchor.

*ir-special*{.property} `(execute function-value)`{.descname} [](#scopes.ir-special.execute "Permalink to this definition"){.headerlink} {#scopes.ir-special.execute}

:	Executes a function in the module as it is being defined. The function must
    match the signature `(function void [Environment])`. If the environment
    parameter is defined, then the currently active translation environment
    will be passed.

*ir-macro*{.property} `(run expression ...)`{.descname} [](#scopes.ir-macro.run "Permalink to this definition"){.headerlink} {#scopes.ir-macro.run}

:	

*ir-special*{.property} `(module name (| _:IR language) (? expression ...))`{.descname} [](#scopes.ir-special.module "Permalink to this definition"){.headerlink} {#scopes.ir-special.module}

:	Declares a new LLVM module with a new empty namespace. `language` must be
    a name with which a preprocessor has been registered, or `IR` for the
    default.

*ir-special*{.property} `(quote type element)`{.descname} [](#scopes.ir-special.quote "Permalink to this definition"){.headerlink} {#scopes.ir-special.quote}

:	Adds the symbolic expression `element` as global constant pointer to the
    module currently being defined and returns its value handle. This allows
    programs to create and process properly anchored expressions.

*ir-special*{.property} `(splice (? expression ...))`{.descname} [](#scopes.ir-special.splice "Permalink to this definition"){.headerlink} {#scopes.ir-special.splice}

:	

*ir-macro*{.property} `(print (? value ...))`{.descname} [](#scopes.ir-macro.print "Permalink to this definition"){.headerlink} {#scopes.ir-macro.print}

:	

*ir-macro*{.property} `(handle pointer-value)`{.descname} [](#scopes.ir-macro.handle "Permalink to this definition"){.headerlink} {#scopes.ir-macro.handle}

:	

*ir-macro*{.property} `(table (? (key value) ...))`{.descname} [](#scopes.ir-macro.table "Permalink to this definition"){.headerlink} {#scopes.ir-macro.table}

:	

*ir-special*{.property} `(error message-string expression)`{.descname} [](#scopes.ir-special.error "Permalink to this definition"){.headerlink} {#scopes.ir-special.error}

:	

*ir-special*{.property} `(nop)`{.descname} [](#scopes.ir-special.nop "Permalink to this definition"){.headerlink} {#scopes.ir-special.nop}

:	Ponders the futility of existence.
