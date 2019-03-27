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
    T(VK_Closure, "value-kind-closure", Closure) \


#define SCOPES_PURE_VALUE_KIND() \
    T(VK_Function, "value-kind-function", Function) \
    T(VK_Global, "value-kind-global", Global) \
    T(VK_PureCast, "value-kind-pure-cast", PureCast) \
    /* constants (Const::classof) */ \
    SCOPES_CONST_VALUE_KIND() \


#define SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Merge, "value-kind-merge", Merge) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \


#define SCOPES_INSTRUCTION_VALUE_KIND() \
    SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Label, "value-kind-label", Label) \
    T(VK_LoopLabel, "value-kind-loop-label", LoopLabel) \
    T(VK_CondBr, "value-kind-condbr", CondBr) \
    T(VK_Switch, "value-kind-switch", Switch) \
    T(VK_Call, "value-kind-call", Call) \


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
    T(UntypedValue) \
    T(Instruction) \
    T(Parameter) \
    T(LoopLabelArguments) \
    T(Exception) \
    T(Function) \
    T(Global) \


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
