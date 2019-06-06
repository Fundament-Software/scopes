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
    T(VK_If, "value-kind-if", If) \
    T(VK_SwitchTemplate, "value-kind-switch-template", SwitchTemplate) \
    T(VK_MergeTemplate, "value-kind-merge-template", MergeTemplate) \
    T(VK_CallTemplate, "value-kind-call-template", CallTemplate) \
    T(VK_RepeatTemplate, "value-kind-repeat-template", RepeatTemplate) \
    T(VK_ReturnTemplate, "value-kind-return-template", ReturnTemplate) \
    T(VK_RaiseTemplate, "value-kind-raise-template", RaiseTemplate) \
    T(VK_Break, "value-kind-break", Break) \
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
    T(VK_PureCast, "value-kind-pure-cast", PureCast) \
    T(VK_Undef, "value-kind-undef", Undef) \
    /* constants (Const::classof) */ \
    SCOPES_CONST_VALUE_KIND() \


#define SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Merge, "value-kind-merge", Merge) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \


#define SCOPES_CAST_VALUE_KIND() \
    T(VK_Bitcast, "value-kind-bitcast", Bitcast) \
    T(VK_IntToPtr, "value-kind-inttoptr", IntToPtr) \
    T(VK_PtrToInt, "value-kind-ptrotint", PtrToInt) \
    T(VK_SExt, "value-kind-sext", SExt) \
    T(VK_ITrunc, "value-kind-itrunc", ITrunc) \
    T(VK_ZExt, "value-kind-zext", ZExt) \
    T(VK_FPTrunc, "value-kind-fptrunc", FPTrunc) \
    T(VK_FPExt, "value-kind-fpext", FPExt) \
    T(VK_FPToUI, "value-kind-fptoui", FPToUI) \
    T(VK_FPToSI, "value-kind-fptosi", FPToSI) \
    T(VK_UIToFP, "value-kind-uitofp", UIToFP) \
    T(VK_SIToFP, "value-kind-sitofp", SIToFP) \


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
    /* casts (Cast::classof) */ \
    SCOPES_CAST_VALUE_KIND() \


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


#define SCOPES_DEFINED_VALUES()


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
