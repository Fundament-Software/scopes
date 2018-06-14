/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SYMBOL_ENUM_HPP
#define SCOPES_SYMBOL_ENUM_HPP

namespace scopes {

//------------------------------------------------------------------------------
// SYMBOL ENUM
//------------------------------------------------------------------------------

// list of symbols to be exposed as builtins to the default global namespace
#define B_GLOBALS() \
    T(FN_Branch) T(KW_Fn) T(KW_Label) T(KW_Quote) T(KW_Inline) T(KW_Forward) \
    T(KW_Call) T(KW_RawCall) T(KW_CCCall) T(SYM_QuoteForm) T(FN_Dump) T(KW_Do) \
    T(FN_FunctionType) T(FN_TupleType) T(FN_UnionType) T(FN_Alloca) T(FN_AllocaOf) T(FN_Malloc) \
    T(FN_AllocaArray) T(FN_MallocArray) T(FN_ReturnLabelType) T(KW_DoIn) T(FN_AllocaExceptionPad) \
    T(FN_StaticAlloc) \
    T(FN_AnyExtract) T(FN_AnyWrap) T(FN_IsConstant) T(FN_Free) T(KW_Defer) \
    T(OP_ICmpEQ) T(OP_ICmpNE) T(FN_Sample) T(FN_ImageRead) T(FN_ImageWrite) \
    T(FN_ImageQuerySize) T(FN_ImageQueryLod) T(FN_ImageQueryLevels) T(FN_ImageQuerySamples) \
    T(OP_ICmpUGT) T(OP_ICmpUGE) T(OP_ICmpULT) T(OP_ICmpULE) \
    T(OP_ICmpSGT) T(OP_ICmpSGE) T(OP_ICmpSLT) T(OP_ICmpSLE) \
    T(OP_FCmpOEQ) T(OP_FCmpONE) T(OP_FCmpORD) \
    T(OP_FCmpOGT) T(OP_FCmpOGE) T(OP_FCmpOLT) T(OP_FCmpOLE) \
    T(OP_FCmpUEQ) T(OP_FCmpUNE) T(OP_FCmpUNO) \
    T(OP_FCmpUGT) T(OP_FCmpUGE) T(OP_FCmpULT) T(OP_FCmpULE) \
    T(FN_Purify) T(FN_Unconst) T(FN_TypeOf) T(FN_Bitcast) \
    T(FN_IntToPtr) T(FN_PtrToInt) T(FN_Load) T(FN_Store) \
    T(FN_VolatileLoad) T(FN_VolatileStore) T(SFXFN_ExecutionMode) \
    T(FN_ExtractElement) T(FN_InsertElement) T(FN_ShuffleVector) \
    T(FN_ExtractValue) T(FN_InsertValue) T(FN_ITrunc) T(FN_ZExt) T(FN_SExt) \
    T(FN_GetElementPtr) T(FN_OffsetOf) T(SFXFN_CompilerError) T(FN_VaCountOf) T(FN_VaAt) \
    T(FN_VaKeys) T(FN_VaKey) T(FN_VaValues) T(FN_CompilerMessage) T(FN_Undef) T(FN_NullOf) T(KW_Let) \
    T(KW_If) T(SFXFN_DelTypeSymbol) T(FN_ExternSymbol) \
    T(FN_ExternNew) \
    T(SFXFN_Discard) \
    T(FN_TypeAt) T(FN_TypeLocalAt) T(KW_SyntaxExtend) T(FN_Location) T(SFXFN_Unreachable) \
    T(FN_FPTrunc) T(FN_FPExt) T(FN_ScopeOf) \
    T(FN_FPToUI) T(FN_FPToSI) \
    T(FN_UIToFP) T(FN_SIToFP) \
    T(OP_Add) T(OP_AddNUW) T(OP_AddNSW) \
    T(OP_Sub) T(OP_SubNUW) T(OP_SubNSW) \
    T(OP_Mul) T(OP_MulNUW) T(OP_MulNSW) \
    T(OP_SDiv) T(OP_UDiv) \
    T(OP_SRem) T(OP_URem) \
    T(OP_Shl) T(OP_LShr) T(OP_AShr) \
    T(OP_BAnd) T(OP_BOr) T(OP_BXor) \
    T(OP_FAdd) T(OP_FSub) T(OP_FMul) T(OP_FDiv) T(OP_FRem) \
    T(OP_Tertiary) T(KW_SyntaxLog) \
    T(OP_FMix) T(OP_Step) T(OP_SmoothStep) \
    T(FN_Round) T(FN_RoundEven) T(OP_Trunc) \
    T(OP_FAbs) T(OP_FSign) T(OP_SSign) \
    T(OP_Floor) T(FN_Ceil) T(FN_Fract) \
    T(OP_Radians) T(OP_Degrees) \
    T(OP_Sin) T(OP_Cos) T(OP_Tan) \
    T(OP_Asin) T(OP_Acos) T(OP_Atan) T(OP_Atan2) \
    T(OP_Exp) T(OP_Log) T(OP_Exp2) T(OP_Log2) \
    T(OP_Pow) T(OP_Sqrt) T(OP_InverseSqrt) \
    T(FN_Fma) T(FN_Frexp) T(FN_Ldexp) \
    T(FN_Length) T(FN_Distance) T(FN_Cross) T(FN_Normalize)

#define B_SPIRV_STORAGE_CLASS() \
    T(UniformConstant) \
    T(Input) \
    T(Uniform) \
    T(Output) \
    T(Workgroup) \
    T(CrossWorkgroup) \
    T(Private) \
    T(Function) \
    T(Generic) \
    T(PushConstant) \
    T(AtomicCounter) \
    T(Image) \
    T(StorageBuffer)

#define B_SPIRV_DIM() \
    T(1D) \
    T(2D) \
    T(3D) \
    T(Cube) \
    T(Rect) \
    T(Buffer) \
    T(SubpassData)

#define B_SPIRV_IMAGE_FORMAT() \
    T(Unknown) \
    T(Rgba32f) \
    T(Rgba16f) \
    T(R32f) \
    T(Rgba8) \
    T(Rgba8Snorm) \
    T(Rg32f) \
    T(Rg16f) \
    T(R11fG11fB10f) \
    T(R16f) \
    T(Rgba16) \
    T(Rgb10A2) \
    T(Rg16) \
    T(Rg8) \
    T(R16) \
    T(R8) \
    T(Rgba16Snorm) \
    T(Rg16Snorm) \
    T(Rg8Snorm) \
    T(R16Snorm) \
    T(R8Snorm) \
    T(Rgba32i) \
    T(Rgba16i) \
    T(Rgba8i) \
    T(R32i) \
    T(Rg32i) \
    T(Rg16i) \
    T(Rg8i) \
    T(R16i) \
    T(R8i) \
    T(Rgba32ui) \
    T(Rgba16ui) \
    T(Rgba8ui) \
    T(R32ui) \
    T(Rgb10a2ui) \
    T(Rg32ui) \
    T(Rg16ui) \
    T(Rg8ui) \
    T(R16ui) \
    T(R8ui)

#define B_SPIRV_BUILTINS() \
    T(Position) \
    T(PointSize) \
    T(ClipDistance) \
    T(CullDistance) \
    T(VertexId) \
    T(InstanceId) \
    T(PrimitiveId) \
    T(InvocationId) \
    T(Layer) \
    T(ViewportIndex) \
    T(TessLevelOuter) \
    T(TessLevelInner) \
    T(TessCoord) \
    T(PatchVertices) \
    T(FragCoord) \
    T(PointCoord) \
    T(FrontFacing) \
    T(SampleId) \
    T(SamplePosition) \
    T(SampleMask) \
    T(FragDepth) \
    T(HelperInvocation) \
    T(NumWorkgroups) \
    T(WorkgroupSize) \
    T(WorkgroupId) \
    T(LocalInvocationId) \
    T(GlobalInvocationId) \
    T(LocalInvocationIndex) \
    T(WorkDim) \
    T(GlobalSize) \
    T(EnqueuedWorkgroupSize) \
    T(GlobalOffset) \
    T(GlobalLinearId) \
    T(SubgroupSize) \
    T(SubgroupMaxSize) \
    T(NumSubgroups) \
    T(NumEnqueuedSubgroups) \
    T(SubgroupId) \
    T(SubgroupLocalInvocationId) \
    T(VertexIndex) \
    T(InstanceIndex) \
    T(SubgroupEqMaskKHR) \
    T(SubgroupGeMaskKHR) \
    T(SubgroupGtMaskKHR) \
    T(SubgroupLeMaskKHR) \
    T(SubgroupLtMaskKHR) \
    T(BaseVertex) \
    T(BaseInstance) \
    T(DrawIndex) \
    T(DeviceIndex) \
    T(ViewIndex) \
    T(BaryCoordNoPerspAMD) \
    T(BaryCoordNoPerspCentroidAMD) \
    T(BaryCoordNoPerspSampleAMD) \
    T(BaryCoordSmoothAMD) \
    T(BaryCoordSmoothCentroidAMD) \
    T(BaryCoordSmoothSampleAMD) \
    T(BaryCoordPullModelAMD) \
    T(ViewportMaskNV) \
    T(SecondaryPositionNV) \
    T(SecondaryViewportMaskNV) \
    T(PositionPerViewNV) \
    T(ViewportMaskPerViewNV)

#define B_SPIRV_EXECUTION_MODE() \
    T(Invocations) \
    T(SpacingEqual) \
    T(SpacingFractionalEven) \
    T(SpacingFractionalOdd) \
    T(VertexOrderCw) \
    T(VertexOrderCcw) \
    T(PixelCenterInteger) \
    T(OriginUpperLeft) \
    T(OriginLowerLeft) \
    T(EarlyFragmentTests) \
    T(PointMode) \
    T(Xfb) \
    T(DepthReplacing) \
    T(DepthGreater) \
    T(DepthLess) \
    T(DepthUnchanged) \
    T(LocalSize) \
    T(LocalSizeHint) \
    T(InputPoints) \
    T(InputLines) \
    T(InputLinesAdjacency) \
    T(Triangles) \
    T(InputTrianglesAdjacency) \
    T(Quads) \
    T(Isolines) \
    T(OutputVertices) \
    T(OutputPoints) \
    T(OutputLineStrip) \
    T(OutputTriangleStrip) \
    T(VecTypeHint) \
    T(ContractionOff) \
    T(PostDepthCoverage)

#define B_SPIRV_IMAGE_OPERAND() \
    T(Bias) \
    T(Lod) \
    T(GradX) \
    T(GradY) \
    T(ConstOffset) \
    T(Offset) \
    T(ConstOffsets) \
    T(Sample) \
    T(MinLod) \
    /* extra operands not part of mask */ \
    T(Dref) \
    T(Proj) \
    T(Fetch) \
    T(Gather) \
    T(Sparse)

#define B_MAP_SYMBOLS() \
    T(SYM_Unnamed, "") \
    \
    /* keywords and macros */ \
    T(KW_CatRest, "::*") T(KW_CatOne, "::@") T(KW_Forward, "_") \
    T(KW_SyntaxLog, "syntax-log") T(KW_DoIn, "do-in") T(KW_Defer, "__defer") \
    T(KW_Assert, "assert") T(KW_Break, "break") T(KW_Label, "label") \
    T(KW_Call, "call") T(KW_RawCall, "rawcall") T(KW_CCCall, "cc/call") T(KW_Continue, "continue") \
    T(KW_Define, "define") T(KW_Do, "do") T(KW_DumpSyntax, "dump-syntax") \
    T(KW_Else, "else") T(KW_ElseIf, "elseif") T(KW_EmptyList, "empty-list") \
    T(KW_EmptyTuple, "empty-tuple") T(KW_Escape, "escape") \
    T(KW_Except, "except") T(KW_False, "false") T(KW_Fn, "fn") \
    T(KW_FnTypes, "fn-types") T(KW_FnCC, "fn/cc") T(KW_Globals, "globals") \
    T(KW_If, "if") T(KW_In, "in") T(KW_Let, "let") T(KW_Loop, "loop") \
    T(KW_LoopFor, "loop-for") T(KW_None, "none") T(KW_Null, "null") \
    T(KW_QQuoteSyntax, "qquote-syntax") T(KW_Quote, "quote") T(KW_Inline, "inline") \
    T(KW_QuoteSyntax, "quote-syntax") T(KW_Raise, "raise") T(KW_Recur, "recur") \
    T(KW_Return, "return") T(KW_Splice, "splice") \
    T(KW_SyntaxExtend, "syntax-extend") T(KW_True, "true") T(KW_Try, "try") \
    T(KW_Unquote, "unquote") T(KW_UnquoteSplice, "unquote-splice") T(KW_ListEmpty, "eol") \
    T(KW_With, "with") T(KW_XFn, "xfn") T(KW_XLet, "xlet") T(KW_Yield, "yield") \
    \
    /* builtin and global functions */ \
    T(FN_Alignof, "alignof") T(FN_OffsetOf, "offsetof") \
    T(FN_Args, "args") T(FN_Alloc, "alloc") T(FN_Arrayof, "arrayof") \
    T(FN_AnchorPath, "Anchor-path") T(FN_AnchorLineNumber, "Anchor-line-number") \
    T(FN_AnchorColumn, "Anchor-column") T(FN_AnchorOffset, "Anchor-offset") \
    T(FN_AnchorSource, "Anchor-source") \
    T(OP_FMix, "fmix") T(OP_Step, "step") T(OP_SmoothStep, "smoothstep") \
    T(FN_Round, "round") T(FN_RoundEven, "roundeven") T(OP_Trunc, "trunc") \
    T(OP_FAbs, "fabs") T(OP_FSign, "fsign") T(OP_SSign, "ssign") \
    T(OP_Floor, "floor") T(FN_Ceil, "ceil") T(FN_Fract, "fract") \
    T(OP_Radians, "radians") T(OP_Degrees, "degrees") \
    T(OP_Sin, "sin") T(OP_Cos, "cos") T(OP_Tan, "tan") \
    T(OP_Asin, "asin") T(OP_Acos, "acos") T(OP_Atan, "atan") T(OP_Atan2, "atan2") \
    T(OP_Exp, "exp") T(OP_Log, "log") T(OP_Exp2, "exp2") T(OP_Log2, "log2") \
    T(OP_Sqrt, "sqrt") T(OP_InverseSqrt, "inversesqrt") \
    T(FN_Fma, "fma") T(FN_Frexp, "frexp") T(FN_Ldexp, "ldexp") \
    T(FN_Length, "length") T(FN_Distance, "distance") T(FN_Cross, "cross") T(FN_Normalize, "normalize") \
    T(FN_AnyExtract, "Any-extract-constant") T(FN_AnyWrap, "Any-wrap") \
    T(FN_ActiveAnchor, "active-anchor") T(FN_ActiveFrame, "active-frame") \
    T(FN_BitCountOf, "bitcountof") T(FN_IsSigned, "signed?") \
    T(FN_Bitcast, "bitcast") T(FN_IntToPtr, "inttoptr") T(FN_PtrToInt, "ptrtoint") \
    T(FN_BlockMacro, "block-macro") \
    T(FN_BlockScopeMacro, "block-scope-macro") T(FN_BoolEq, "bool==") \
    T(FN_BuiltinEq, "Builtin==") \
    T(FN_Branch, "branch") T(FN_IsCallable, "callable?") T(FN_Cast, "cast") \
    T(FN_Concat, "concat") T(FN_Cons, "cons") T(FN_IsConstant, "constant?") \
    T(FN_Countof, "countof") \
    T(FN_Compile, "__compile") T(FN_CompileSPIRV, "__compile-spirv") \
    T(FN_CompileGLSL, "__compile-glsl") \
    T(FN_CompileObject, "__compile-object") \
    T(FN_ElementIndex, "element-index") \
    T(FN_ElementName, "element-name") \
    T(FN_CompilerMessage, "compiler-message") \
    T(FN_CStr, "cstr") T(FN_DatumToSyntax, "datum->syntax") \
    T(FN_DatumToQuotedSyntax, "datum->quoted-syntax") \
    T(FN_LabelDocString, "Label-docstring") \
    T(FN_LabelSetInline, "Label-set-inline!") \
    T(FN_DefaultStyler, "default-styler") T(FN_StyleToString, "style->string") \
    T(FN_Disqualify, "disqualify") T(FN_Dump, "dump") \
    T(FN_DumpLabel, "dump-label") \
    T(FN_DumpList, "dump-list") \
    T(FN_DumpFrame, "dump-frame") \
    T(FN_ClosureLabel, "Closure-label") \
    T(FN_ClosureFrame, "Closure-frame") \
    T(FN_FormatFrame, "Frame-format") \
    T(FN_ElementType, "element-type") T(FN_IsEmpty, "empty?") \
    T(FN_TypeCountOf, "type-countof") \
    T(FN_Enumerate, "enumerate") T(FN_Eval, "eval") \
    T(FN_Exit, "exit") T(FN_Expand, "expand") \
    T(FN_ExternLibrary, "extern-library") \
    T(FN_ExternSymbol, "extern-symbol") \
    T(FN_ExtractMemory, "extract-memory") \
    T(FN_EnterSolverCLI, "enter-solver-cli!") \
    T(FN_ExtractValue, "extractvalue") T(FN_InsertValue, "insertvalue") \
    T(FN_ExtractElement, "extractelement") T(FN_InsertElement, "insertelement") \
    T(FN_ShuffleVector, "shufflevector") T(FN_GetElementPtr, "getelementptr") \
    T(FN_FFISymbol, "ffi-symbol") T(FN_FFICall, "ffi-call") \
    T(FN_FrameEq, "Frame==") T(FN_Free, "free") \
    T(FN_GetExceptionHandler, "get-exception-handler") \
    T(FN_GetScopeSymbol, "get-scope-symbol") T(FN_Hash, "__hash") \
    T(FN_Hash2x64, "__hash2x64") T(FN_HashBytes, "__hashbytes") \
    T(FN_Sample, "sample") \
    T(FN_ImageRead, "Image-read") T(FN_ImageWrite, "Image-write") \
    T(FN_ImageQuerySize, "Image-query-size") \
    T(FN_ImageQueryLod, "Image-query-lod") \
    T(FN_ImageQueryLevels, "Image-query-levels") \
    T(FN_ImageQuerySamples, "Image-query-samples") \
    T(FN_RealPath, "realpath") \
    T(FN_DirName, "dirname") T(FN_BaseName, "basename") \
    T(OP_ICmpEQ, "icmp==") T(OP_ICmpNE, "icmp!=") \
    T(OP_ICmpUGT, "icmp>u") T(OP_ICmpUGE, "icmp>=u") T(OP_ICmpULT, "icmp<u") T(OP_ICmpULE, "icmp<=u") \
    T(OP_ICmpSGT, "icmp>s") T(OP_ICmpSGE, "icmp>=s") T(OP_ICmpSLT, "icmp<s") T(OP_ICmpSLE, "icmp<=s") \
    T(OP_FCmpOEQ, "fcmp==o") T(OP_FCmpONE, "fcmp!=o") T(OP_FCmpORD, "fcmp-ord") \
    T(OP_FCmpOGT, "fcmp>o") T(OP_FCmpOGE, "fcmp>=o") T(OP_FCmpOLT, "fcmp<o") T(OP_FCmpOLE, "fcmp<=o") \
    T(OP_FCmpUEQ, "fcmp==u") T(OP_FCmpUNE, "fcmp!=u") T(OP_FCmpUNO, "fcmp-uno") \
    T(OP_FCmpUGT, "fcmp>u") T(OP_FCmpUGE, "fcmp>=u") T(OP_FCmpULT, "fcmp<u") T(OP_FCmpULE, "fcmp<=u") \
    T(OP_Add, "add") T(OP_AddNUW, "add-nuw") T(OP_AddNSW, "add-nsw") \
    T(OP_Sub, "sub") T(OP_SubNUW, "sub-nuw") T(OP_SubNSW, "sub-nsw") \
    T(OP_Mul, "mul") T(OP_MulNUW, "mul-nuw") T(OP_MulNSW, "mul-nsw") \
    T(OP_SDiv, "sdiv") T(OP_UDiv, "udiv") \
    T(OP_SRem, "srem") T(OP_URem, "urem") \
    T(OP_Shl, "shl") T(OP_LShr, "lshr") T(OP_AShr, "ashr") \
    T(OP_BAnd, "band") T(OP_BOr, "bor") T(OP_BXor, "bxor") \
    T(FN_IsFile, "file?") T(FN_IsDirectory, "directory?") \
    T(OP_FAdd, "fadd") T(OP_FSub, "fsub") T(OP_FMul, "fmul") T(OP_FDiv, "fdiv") T(OP_FRem, "frem") \
    T(FN_FPTrunc, "fptrunc") T(FN_FPExt, "fpext") \
    T(FN_FPToUI, "fptoui") T(FN_FPToSI, "fptosi") \
    T(FN_UIToFP, "uitofp") T(FN_SIToFP, "sitofp") \
    T(FN_ImportC, "import-c") T(FN_IsInteger, "integer?") \
    T(FN_IntegerType, "integer-type") \
    T(FN_CompilerVersion, "compiler-version") \
    T(FN_Iter, "iter") T(FN_FormatMessage, "format-message") \
    T(FN_IsIterator, "iterator?") T(FN_IsLabel, "label?") \
    T(FN_LabelEq, "Label==") \
    T(FN_LabelNew, "Label-new") T(FN_LabelParameterCount, "Label-parameter-count") \
    T(FN_LabelParameter, "Label-parameter") \
    T(FN_LabelAnchor, "Label-anchor") T(FN_LabelName, "Label-name") \
    T(FN_ClosureEq, "Closure==") T(FN_CheckStack, "verify-stack!") \
    T(FN_ListAtom, "list-atom?") T(FN_ListCountOf, "list-countof") \
    T(FN_ListLoad, "list-load") T(FN_ListJoin, "list-join") \
    T(FN_ListParse, "list-parse") T(FN_IsList, "list?") T(FN_Load, "load") \
    T(FN_LoadLibrary, "load-library") \
    T(FN_VolatileLoad, "volatile-load") \
    T(FN_VolatileStore, "volatile-store") \
    T(FN_LabelCountOfReachable, "Label-countof-reachable") \
    T(FN_ListAt, "list-at") T(FN_ListNext, "list-next") T(FN_ListCons, "list-cons") \
    T(FN_IsListEmpty, "list-empty?") \
    T(FN_Malloc, "malloc") T(FN_MallocArray, "malloc-array") T(FN_Unconst, "unconst") \
    T(FN_Macro, "macro") T(FN_Max, "max") T(FN_Min, "min") \
    T(FN_MemCopy, "memcopy") \
    T(FN_IsMutable, "mutable?") \
    T(FN_IsNone, "none?") \
    T(FN_IsNull, "null?") T(FN_OrderedBranch, "ordered-branch") \
    T(FN_ParameterEq, "Parameter==") \
    T(FN_ParameterNew, "Parameter-new") T(FN_ParameterName, "Parameter-name") \
    T(FN_ParameterAnchor, "Parameter-anchor") \
    T(FN_ParameterIndex, "Parameter-index") \
    T(FN_ParseC, "parse-c") T(FN_PointerOf, "pointerof") \
    T(FN_PointerType, "pointer-type") \
    T(FN_PointerFlags, "pointer-type-flags") \
    T(FN_PointerSetFlags, "pointer-type-set-flags") \
    T(FN_PointerStorageClass, "pointer-type-storage-class") \
    T(FN_PointerSetStorageClass, "pointer-type-set-storage-class") \
    T(FN_PointerSetElementType, "pointer-type-set-element-type") \
    T(FN_ExternLocation, "extern-type-location") \
    T(FN_ExternBinding, "extern-type-binding") \
    T(FN_FunctionType, "function-type") \
    T(FN_FunctionTypeIsVariadic, "function-type-variadic?") \
    T(FN_TupleType, "tuple-type") \
    T(FN_UnionType, "union-type") \
    T(FN_ReturnLabelType, "ReturnLabel-type") \
    T(FN_ArrayType, "array-type") T(FN_ImageType, "Image-type") \
    T(FN_SampledImageType, "SampledImage-type") \
    T(FN_TypenameType, "typename-type") \
    T(FN_Purify, "purify") \
    T(FN_Write, "io-write!") \
    T(FN_Flush, "io-flush") \
    T(FN_Product, "product") T(FN_Prompt, "__prompt") T(FN_Qualify, "qualify") \
    T(FN_SetAutocompleteScope, "set-autocomplete-scope!") \
    T(FN_Range, "range") T(FN_RefNew, "ref-new") T(FN_RefAt, "ref@") \
    T(FN_Repeat, "repeat") T(FN_Repr, "Any-repr") T(FN_AnyString, "Any-string") \
    T(FN_Require, "require") T(FN_ScopeOf, "scopeof") T(FN_ScopeAt, "Scope@") \
    T(FN_ScopeLocalAt, "Scope-local@") \
    T(FN_ScopeEq, "Scope==") \
    T(FN_ScopeNew, "Scope-new") \
    T(FN_ScopeCopy, "Scope-clone") \
    T(FN_ScopeDocString, "Scope-docstring") \
    T(FN_SetScopeDocString, "set-scope-docstring!") \
    T(FN_ScopeNewSubscope, "Scope-new-expand") \
    T(FN_ScopeCopySubscope, "Scope-clone-expand") \
    T(FN_ScopeParent, "Scope-parent") \
    T(FN_ScopeNext, "Scope-next") T(FN_SizeOf, "sizeof") \
    T(FN_TypeNext, "type-next") \
    T(FN_Slice, "slice") T(FN_Store, "store") \
    T(FN_StringAt, "string@") T(FN_StringCmp, "string-compare") \
    T(FN_StringCountOf, "string-countof") T(FN_StringNew, "string-new") \
    T(FN_StringJoin, "string-join") T(FN_StringSlice, "string-slice") \
    T(FN_StructOf, "structof") T(FN_TypeStorage, "storageof") \
    T(FN_IsOpaque, "opaque?") \
    T(FN_SymbolEq, "Symbol==") T(FN_SymbolNew, "string->Symbol") \
    T(FN_StringToRawstring, "string->rawstring") \
    T(FN_IsSymbol, "symbol?") \
    T(FN_SyntaxToAnchor, "syntax->anchor") T(FN_SyntaxToDatum, "syntax->datum") \
    T(FN_SyntaxCons, "syntax-cons") T(FN_SyntaxDo, "syntax-do") \
    T(FN_IsSyntaxHead, "syntax-head?") \
    T(FN_SyntaxList, "syntax-list") T(FN_SyntaxQuote, "syntax-quote") \
    T(FN_IsSyntaxQuoted, "syntax-quoted?") \
    T(FN_SyntaxUnquote, "syntax-unquote") \
    T(FN_SymbolToString, "Symbol->string") \
    T(FN_StringMatch, "string-match?") \
    T(FN_SuperOf, "superof") \
    T(FN_SyntaxNew, "Syntax-new") \
    T(FN_SyntaxWrap, "Syntax-wrap") \
    T(FN_SyntaxStrip, "Syntax-strip") \
    T(FN_Translate, "translate") T(FN_ITrunc, "itrunc") \
    T(FN_ZExt, "zext") T(FN_SExt, "sext") \
    T(FN_TupleOf, "tupleof") T(FN_TypeNew, "type-new") T(FN_TypeName, "type-name") \
    T(FN_TypeSizeOf, "type-sizeof") \
    T(FN_Typify, "__typify") \
    T(FN_TypeEq, "type==") T(FN_IsType, "type?") T(FN_TypeOf, "typeof") \
    T(FN_TypeKind, "type-kind") \
    T(FN_TypeDebugABI, "type-debug-abi") \
    T(FN_TypeAt, "type@") \
    T(FN_TypeLocalAt, "type-local@") \
    T(FN_RuntimeTypeAt, "runtime-type@") \
    T(FN_Undef, "undef") T(FN_NullOf, "nullof") T(FN_Alloca, "alloca") \
    T(FN_AllocaExceptionPad, "alloca-exception-pad") \
    T(FN_AllocaOf, "allocaof") \
    T(FN_AllocaArray, "alloca-array") \
    T(FN_StaticAlloc, "static-alloc") \
    T(FN_Location, "compiler-anchor") \
    T(FN_ExternNew, "extern-new") \
    T(FN_VaCountOf, "va-countof") T(FN_VaKeys, "va-keys") \
    T(FN_VaValues, "va-values") T(FN_VaAt, "va@") \
    T(FN_VaKey, "va-key") \
    T(FN_VectorOf, "vectorof") T(FN_XPCall, "xpcall") T(FN_Zip, "zip") \
    T(FN_VectorType, "vector-type") \
    T(FN_ZipFill, "zip-fill") \
    \
    /* builtin and global functions with side effects */ \
    T(SFXFN_CopyMemory, "copy-memory!") \
    T(SFXFN_Unreachable, "unreachable!") \
    T(SFXFN_Discard, "discard!") \
    T(SFXFN_Error, "__error!") \
    T(SFXFN_AnchorError, "__anchor-error!") \
    T(SFXFN_Raise, "__raise!") \
    T(SFXFN_Abort, "abort!") \
    T(SFXFN_CompilerError, "compiler-error!") \
    T(SFXFN_SetAnchor, "set-anchor!") \
    T(SFXFN_LabelAppendParameter, "label-append-parameter!") \
    T(SFXFN_RefSet, "ref-set!") \
    T(SFXFN_SetExceptionHandler, "set-exception-handler!") \
    T(SFXFN_SetGlobals, "set-globals!") \
    T(SFXFN_SetTypenameSuper, "set-typename-super!") \
    T(SFXFN_SetGlobalApplyFallback, "set-global-apply-fallback!") \
    T(SFXFN_SetScopeSymbol, "__set-scope-symbol!") \
    T(SFXFN_DelScopeSymbol, "delete-scope-symbol!") \
    T(SFXFN_DelTypeSymbol, "delete-type-symbol!") \
    T(SFXFN_ExecutionMode, "set-execution-mode!") \
    T(SFXFN_TranslateLabelBody, "translate-label-body!") \
    \
    /* builtin operator functions that can also be used as infix */ \
    T(OP_NotEq, "!=") T(OP_Mod, "%") T(OP_InMod, "%=") T(OP_BitAnd, "&") T(OP_InBitAnd, "&=") \
    T(OP_IFXMul, "*") T(OP_Pow, "powf") T(OP_InMul, "*=") T(OP_IFXAdd, "+") T(OP_Incr, "++") \
    T(OP_InAdd, "+=") T(OP_Comma, ",") T(OP_IFXSub, "-") T(OP_Decr, "--") T(OP_InSub, "-=") \
    T(OP_Dot, ".") T(OP_Join, "..") T(OP_Div, "/") T(OP_InDiv, "/=") \
    T(OP_Colon, ":") T(OP_Let, ":=") T(OP_Less, "<") T(OP_LeftArrow, "<-") T(OP_Subtype, "<:") \
    T(OP_ShiftL, "<<") T(OP_LessThan, "<=") T(OP_Set, "=") T(OP_Eq, "==") \
    T(OP_Greater, ">") T(OP_GreaterThan, ">=") T(OP_ShiftR, ">>") T(OP_Tertiary, "?") \
    T(OP_At, "@") T(OP_Xor, "^") T(OP_InXor, "^=") T(OP_And, "and") T(OP_Not, "not") \
    T(OP_Or, "or") T(OP_BitOr, "|") T(OP_InBitOr, "|=") T(OP_BitNot, "~") \
    T(OP_InBitNot, "~=") \
    \
    /* globals */ \
    T(SYM_DebugBuild, "debug-build?") \
    T(SYM_CompilerDir, "compiler-dir") \
    T(SYM_CompilerPath, "compiler-path") \
    T(SYM_CompilerTimestamp, "compiler-timestamp") \
    \
    /* parse-c keywords */ \
    T(SYM_Struct, "struct") \
    T(SYM_Union, "union") \
    T(SYM_TypeDef, "typedef") \
    T(SYM_Enum, "enum") \
    T(SYM_Array, "array") \
    T(SYM_Vector, "vector") \
    T(SYM_FNType, "fntype") \
    T(SYM_Extern, "extern") \
    \
    /* styles */ \
    T(Style_None, "style-none") \
    T(Style_Symbol, "style-symbol") \
    T(Style_String, "style-string") \
    T(Style_Number, "style-number") \
    T(Style_Keyword, "style-keyword") \
    T(Style_Function, "style-function") \
    T(Style_SfxFunction, "style-sfxfunction") \
    T(Style_Operator, "style-operator") \
    T(Style_Instruction, "style-instruction") \
    T(Style_Type, "style-type") \
    T(Style_Comment, "style-comment") \
    T(Style_Error, "style-error") \
    T(Style_Warning, "style-warning") \
    T(Style_Location, "style-location") \
    \
    /* builtins, forms, etc */ \
    T(SYM_FnCCForm, "form-fn-body") \
    T(SYM_QuoteForm, "form-quote") \
    T(SYM_DoForm, "form-do") \
    T(SYM_SyntaxScope, "syntax-scope") \
    T(SYM_CallHandler, "__call") \
    \
    /* varargs */ \
    T(SYM_Parenthesis, "...") \
    \
    T(SYM_ListWildcard, "#list") \
    T(SYM_SymbolWildcard, "#symbol") \
    T(SYM_ThisFnCC, "#this-fn/cc") \
    \
    T(SYM_Compare, "compare") \
    T(SYM_Size, "size") \
    T(SYM_Alignment, "alignment") \
    T(SYM_Unsigned, "unsigned") \
    T(SYM_Bitwidth, "bitwidth") \
    T(SYM_Super, "super") \
    T(SYM_ApplyType, "apply-type") \
    T(SYM_ScopeCall, "scope-call") \
    T(SYM_Styler, "styler") \
    \
    /* list styles */ \
    T(SYM_SquareList, "square-list") \
    T(SYM_CurlyList, "curly-list") \
    \
    /* compile flags */ \
    T(SYM_DumpDisassembly, "compile-flag-dump-disassembly") \
    T(SYM_DumpModule, "compile-flag-dump-module") \
    T(SYM_DumpFunction, "compile-flag-dump-function") \
    T(SYM_DumpTime, "compile-flag-dump-time") \
    T(SYM_NoDebugInfo, "compile-flag-no-debug-info") \
    T(SYM_O1, "compile-flag-O1") \
    T(SYM_O2, "compile-flag-O2") \
    T(SYM_O3, "compile-flag-O3") \
    \
    /* function flags */ \
    T(SYM_Variadic, "variadic") \
    T(SYM_Pure, "pure") \
    \
    /* compile targets */ \
    T(SYM_TargetVertex, "vertex") \
    T(SYM_TargetFragment, "fragment") \
    T(SYM_TargetGeometry, "geometry") \
    T(SYM_TargetCompute, "compute") \
    \
    /* extern attributes */ \
    T(SYM_Location, "location") \
    T(SYM_Binding, "binding") \
    T(SYM_Storage, "storage") \
    T(SYM_Buffer, "buffer") \
    T(SYM_Coherent, "coherent") \
    T(SYM_Volatile, "volatile") \
    T(SYM_Restrict, "restrict") \
    T(SYM_ReadOnly, "readonly") \
    T(SYM_WriteOnly, "writeonly") \
    \
    /* PE debugger commands */ \
    T(SYM_C, "c") \
    T(SYM_Skip, "skip") \
    T(SYM_Original, "original") \
    T(SYM_Help, "help") \
    \
    /* timer names */ \
    T(TIMER_Compile, "compile()") \
    T(TIMER_CompileSPIRV, "compile_spirv()") \
    T(TIMER_Generate, "generate()") \
    T(TIMER_GenerateSPIRV, "generate_spirv()") \
    T(TIMER_Optimize, "build_and_run_opt_passes()") \
    T(TIMER_ValidateScope, "validate_scope()") \
    T(TIMER_Main, "main()") \
    T(TIMER_Specialize, "specialize()") \
    \
    /* ad-hoc builtin names */ \
    T(SYM_ExecuteReturn, "execute-return") \
    T(SYM_RCompare, "rcompare") \
    T(SYM_CountOfForwarder, "countof-forwarder") \
    T(SYM_SliceForwarder, "slice-forwarder") \
    T(SYM_JoinForwarder, "join-forwarder") \
    T(SYM_RCast, "rcast") \
    T(SYM_ROp, "rop") \
    T(SYM_CompareListNext, "compare-list-next") \
    T(SYM_ReturnSafecall, "return-safecall") \
    T(SYM_ReturnError, "return-error") \
    T(SYM_XPCallReturn, "xpcall-return")

enum KnownSymbol {
#define T(sym, name) sym,
#define T0 T
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    FN_ ## UNAME ## PFIX,
    B_MAP_SYMBOLS()
#undef T
#undef T0
#undef T1
#undef T2
#undef T2T
#define T(NAME) \
SYM_SPIRV_StorageClass ## NAME,
    B_SPIRV_STORAGE_CLASS()
#undef T
#define T(NAME) \
    SYM_SPIRV_BuiltIn ## NAME,
    B_SPIRV_BUILTINS()
#undef T
#define T(NAME) \
    SYM_SPIRV_ExecutionMode ## NAME,
    B_SPIRV_EXECUTION_MODE()
#undef T
#define T(NAME) \
    SYM_SPIRV_Dim ## NAME,
    B_SPIRV_DIM()
#undef T
#define T(NAME) \
    SYM_SPIRV_ImageFormat ## NAME,
    B_SPIRV_IMAGE_FORMAT()
#undef T
#define T(NAME) \
    SYM_SPIRV_ImageOperand ## NAME,
    B_SPIRV_IMAGE_OPERAND()
#undef T
    SYM_Count,
};

enum {
    KEYWORD_FIRST = KW_CatRest,
    KEYWORD_LAST = KW_Yield,

    FUNCTION_FIRST = FN_Alignof,
    FUNCTION_LAST = FN_ZipFill,

    SFXFUNCTION_FIRST = SFXFN_CopyMemory,
    SFXFUNCTION_LAST = SFXFN_TranslateLabelBody,

    OPERATOR_FIRST = OP_NotEq,
    OPERATOR_LAST = OP_InBitNot,

    STYLE_FIRST = Style_None,
    STYLE_LAST = Style_Location,
};

const char *get_known_symbol_name(KnownSymbol sym);

} // namespace scopes

#endif // SCOPES_SYMBOL_ENUM_HPP
