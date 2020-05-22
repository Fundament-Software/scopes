/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VALUE_KIND_HPP
#define SCOPES_VALUE_KIND_HPP

namespace scopes {

#define SCOPES_UNTYPED_VALUE_KIND() \
    T(VK_Template, "value-kind-template", Template) \
    T(VK_LabelTemplate, "value-kind-label-template", LabelTemplate) \
    T(VK_Loop, "value-kind-loop", Loop) \
    T(VK_LoopArguments, "value-kind-loop-arguments", LoopArguments) \
    T(VK_KeyedTemplate, "value-kind-keyed-template", KeyedTemplate) \
    T(VK_Expression, "value-kind-expression", Expression) \
    T(VK_Quote, "value-kind-quote", Quote) \
    T(VK_Unquote, "value-kind-unquote", Unquote) \
    T(VK_CompileStage, "value-kind-compile-stage", CompileStage) \
    T(VK_CondTemplate, "value-kind-cond-template", CondTemplate) \
    T(VK_SwitchTemplate, "value-kind-switch-template", SwitchTemplate) \
    T(VK_MergeTemplate, "value-kind-merge-template", MergeTemplate) \
    T(VK_CallTemplate, "value-kind-call-template", CallTemplate) \
    T(VK_ArgumentListTemplate, "value-kind-argument-list-template", ArgumentListTemplate) \
    T(VK_ExtractArgumentTemplate, "value-kind-extract-argument-template", ExtractArgumentTemplate) \
    T(VK_ParameterTemplate, "value-kind-parameter-template", ParameterTemplate) \


#define SCOPES_CONST_VALUE_KIND() \
    T(VK_ConstInt, "value-kind-const-int", ConstInt) \
    T(VK_ConstReal, "value-kind-const-real", ConstReal) \
    T(VK_ConstAggregate, "value-kind-const-aggregate", ConstAggregate) \
    T(VK_ConstPointer, "value-kind-const-pointer", ConstPointer) \


#define SCOPES_PURE_VALUE_KIND() \
    T(VK_Function, "value-kind-function", Function) \
    T(VK_Global, "value-kind-global", Global) \
    T(VK_GlobalString, "value-kind-global-string", GlobalString) \
    T(VK_PureCast, "value-kind-pure-cast", PureCast) \
    T(VK_Undef, "value-kind-undef", Undef) \
    /* constants (Const::classof) */ \
    SCOPES_CONST_VALUE_KIND() \


#define SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Merge, "value-kind-merge", Merge) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \
    T(VK_Unreachable, "value-kind-unreachable", Unreachable) \
    T(VK_Discard, "value-kind-discard", Discard) \


#define SCOPES_INSTRUCTION_VALUE_KIND() \
    SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Label, "value-kind-label", Label) \
    T(VK_LoopLabel, "value-kind-loop-label", LoopLabel) \
    T(VK_CondBr, "value-kind-condbr", CondBr) \
    T(VK_Switch, "value-kind-switch", Switch) \
    T(VK_Call, "value-kind-call", Call) \
    T(VK_Select, "value-kind-select", Select) \
    T(VK_ExtractValue, "value-kind-extract-value", ExtractValue) \
    T(VK_InsertValue, "value-kind-insert-value", InsertValue) \
    T(VK_GetElementPtr, "value-kind-get-element-ptr", GetElementPtr) \
    T(VK_ExtractElement, "value-kind-extract-element", ExtractElement) \
    T(VK_InsertElement, "value-kind-insert-element", InsertElement) \
    T(VK_ShuffleVector, "value-kind-shuffle-vector", ShuffleVector) \
    T(VK_Alloca, "value-kind-alloca", Alloca) \
    T(VK_Malloc, "value-kind-malloc", Malloc) \
    T(VK_Free, "value-kind-free", Free) \
    T(VK_Load, "value-kind-load", Load) \
    T(VK_Store, "value-kind-store", Store) \
    T(VK_AtomicRMW, "value-kind-atomicrmw", AtomicRMW) \
    T(VK_CmpXchg, "value-kind-cmpxchg", CmpXchg) \
    T(VK_Barrier, "value-kind-barrier", Barrier) \
    T(VK_ICmp, "value-kind-icmp", ICmp) \
    T(VK_FCmp, "value-kind-fcmp", FCmp) \
    T(VK_UnOp, "value-kind-unop", UnOp) \
    T(VK_BinOp, "value-kind-binop", BinOp) \
    T(VK_TriOp, "value-kind-triop", TriOp) \
    T(VK_Annotate, "value-kind-annotate", Annotate) \
    T(VK_Sample, "value-kind-sample", Sample) \
    T(VK_ImageQuerySize, "value-kind-image-query-size", ImageQuerySize) \
    T(VK_ImageQueryLod, "value-kind-image-query-lod", ImageQueryLod) \
    T(VK_ImageQueryLevels, "value-kind-image-query-levels", ImageQueryLevels) \
    T(VK_ImageQuerySamples, "value-kind-image-query-samples", ImageQuerySamples) \
    T(VK_ImageRead, "value-kind-image-read", ImageRead) \
    T(VK_ImageWrite, "value-kind-image-write", ImageWrite) \
    T(VK_ExecutionMode, "value-kind-execution-mode", ExecutionMode) \
    T(VK_Cast, "value-kind-cast", Cast) \


#define SCOPES_TYPED_VALUE_KIND() \
    T(VK_Keyed, "value-kind-keyed", Keyed) \
    T(VK_Parameter, "value-kind-parameter", Parameter) \
    T(VK_Exception, "value-kind-exception", Exception) \
    T(VK_ArgumentList, "value-kind-argument-list", ArgumentList) \
    T(VK_ExtractArgument, "value-kind-extract-argument", ExtractArgument) \
    T(VK_LoopLabelArguments, "value-kind-loop-label-arguments", LoopLabelArguments) \
    /* instructions (Instruction::classof) */ \
    SCOPES_INSTRUCTION_VALUE_KIND() \
    /* pure (Pure::classof), which includes constants */ \
    SCOPES_PURE_VALUE_KIND() \


#define SCOPES_VALUE_KIND() \
    SCOPES_UNTYPED_VALUE_KIND() \
    SCOPES_TYPED_VALUE_KIND() \


#define SCOPES_DEFINED_VALUES() \
    T(Template)

enum ValueKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    SCOPES_VALUE_KIND()
#undef T
};

// forward declarations
#define T(NAME, BNAME, CLASS) struct CLASS;
    SCOPES_VALUE_KIND()
#undef T

} // namespace scopes

#endif // SCOPES_VALUE_KIND_HPP
