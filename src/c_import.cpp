/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "c_import.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "types.hpp"
#include "source_file.hpp"
#include "anchor.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "scope.hpp"
#include "execution.hpp"
#include "value.hpp"
#include "dyn_cast.inc"

#include "scopes/scopes.h"

#include <llvm-c/Core.h>

#include "llvm/IR/Module.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/RecordLayout.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/LiteralSupport.h"

namespace scopes {

//------------------------------------------------------------------------------
// C BRIDGE (CLANG)
//------------------------------------------------------------------------------

static const Anchor *anchor_from_location(clang::SourceManager &SM, clang::SourceLocation loc) {
    auto PLoc = SM.getPresumedLoc(loc);

    if (PLoc.isValid()) {
        auto fname = PLoc.getFilename();
        const String *strpath = String::from_cstr(fname);
        Symbol key(strpath);
        SourceFile *sf = SourceFile::from_file(key);
        if (!sf) {
            sf = SourceFile::from_string(key, Symbol(SYM_Unnamed).name());
        }
        return Anchor::from(sf, PLoc.getLine(), PLoc.getColumn(),
            SM.getFileOffset(loc));
    }

    return get_active_anchor();
}

class CVisitor : public clang::RecursiveASTVisitor<CVisitor> {
public:

    typedef std::unordered_map<Symbol, const Type *, Symbol::Hash> NamespaceMap;

    Scope *dest;
    clang::ASTContext *Context;
    bool ok;
    std::unordered_map<clang::RecordDecl *, bool> record_defined;
    std::unordered_map<clang::EnumDecl *, bool> enum_defined;
    NamespaceMap named_structs;
    NamespaceMap named_classes;
    NamespaceMap named_unions;
    NamespaceMap named_enums;
    NamespaceMap typedefs;

    CVisitor() : dest(nullptr), Context(NULL), ok(true) {
        const Type *T = typename_type(String::from("__builtin_va_list"));
        auto tnt = cast<TypenameType>(const_cast<Type*>(T));
        tnt->finalize(array_type(TYPE_I8, sizeof(va_list)).assert_ok()).assert_ok();
        typedefs.insert({Symbol("__builtin_va_list"), T });
    }

    const Anchor *anchorFromLocation(clang::SourceLocation loc) {
        auto &SM = Context->getSourceManager();
        return anchor_from_location(SM, loc);
    }

    void SetContext(clang::ASTContext * ctx, Scope *_dest) {
        Context = ctx;
        dest = _dest;
    }

    SCOPES_RESULT(void) GetFields(TypenameType *tni, clang::RecordDecl * rd) {
        SCOPES_RESULT_TYPE(void);
        auto &rl = Context->getASTRecordLayout(rd);

        bool is_union = rd->isUnion();

        if (is_union) {
            tni->super_type = TYPE_CUnion;
        } else {
            tni->super_type = TYPE_CStruct;
        }

        KeyedTypes args;
        //auto anchors = new std::vector<Anchor>();
        //StyledStream ss;
        const Type *ST = tni;

        size_t sz = 0;
        size_t al = 1;
        bool packed = false;
        bool has_bitfield = false;
        for(clang::RecordDecl::field_iterator it = rd->field_begin(), end = rd->field_end(); it != end; ++it) {
            clang::DeclarationName declname = it->getDeclName();

            if (!it->isAnonymousStructOrUnion() && !declname) {
                continue;
            }

            clang::QualType FT = it->getType();
            auto fieldtype = SCOPES_GET_RESULT(TranslateType(FT));

            if(it->isBitField()) {
                has_bitfield = true;
                break;
            }

            //unsigned width = it->getBitWidthValue(*Context);

            Symbol name = it->isAnonymousStructOrUnion() ?
                Symbol("") : Symbol(String::from_stdstring(declname.getAsString()));

            if (!is_union) {
                //ss << "type " << ST << " field " << name << " : " << fieldtype << std::endl;

                unsigned idx = it->getFieldIndex();
                auto offset = rl.getFieldOffset(idx) / 8;
                size_t newsz = sz;
                if (!packed) {
                    size_t etal = SCOPES_GET_RESULT(align_of(fieldtype));
                    newsz = scopes::align(sz, etal);
                    al = std::max(al, etal);
                }
                if (newsz != offset) {
                    //ss << "offset mismatch " << newsz << " != " << offset << std::endl;
                    if (newsz < offset) {
                        size_t pad = offset - newsz;
                        args.push_back(KeyedType(SYM_Unnamed,
                            array_type(TYPE_U8, pad).assert_ok()));
                    } else {
                        // our computed offset is later than the real one
                        // structure is likely packed
                        packed = true;
                    }
                }
                sz = offset + SCOPES_GET_RESULT(size_of(fieldtype));
            } else {
                sz = std::max(sz, SCOPES_GET_RESULT(size_of(fieldtype)));
                al = std::max(al, SCOPES_GET_RESULT(align_of(fieldtype)));
            }

            args.push_back(KeyedType(name, fieldtype));
        }
        if (packed) {
            al = 1;
        }
        bool explicit_alignment = false;
        if (has_bitfield) {
            // ignore for now and hope that an underlying union fixes the problem
        } else {
            size_t needalign = rl.getAlignment().getQuantity();
            size_t needsize = rl.getSize().getQuantity();
            #if 0
            if (!is_union && (needalign != al)) {
                al = needalign;
                explicit_alignment = true;
            }
            #endif
            sz = scopes::align(sz, al);
            bool align_ok = (al == needalign);
            bool size_ok = (sz == needsize);
            if (!(align_ok && size_ok)) {
#ifdef SCOPES_DEBUG
                StyledStream ss;
                auto anchor = anchorFromLocation(rd->getSourceRange().getBegin());
                if (al != needalign) {
                    ss << anchor << " type " << ST << " alignment mismatch: " << al << " != " << needalign << std::endl;
                }
                if (sz != needsize) {
                    ss << anchor << " type " << ST << " size mismatch: " << sz << " != " << needsize << std::endl;
                }
                #if 0
                set_active_anchor(anchor);
                location_error(String::from("clang-bridge: imported record doesn't fit"));
                #endif
#endif
            }
        }

        SCOPES_CHECK_RESULT(tni->finalize(is_union?SCOPES_GET_RESULT(keyed_union_type(args)):
            SCOPES_GET_RESULT(keyed_tuple_type(args, packed, explicit_alignment?al:0))));
        return true;
    }

    const Type *get_typename(Symbol name, NamespaceMap &map) {
        if (name != SYM_Unnamed) {
            auto it = map.find(name);
            if (it != map.end()) {
                return it->second;
            }
            const Type *T = typename_type(name.name());
            auto ok = map.insert({name, T});
            assert(ok.second);
            return T;
        }
        return typename_type(name.name());
    }

    SCOPES_RESULT(const Type *) TranslateRecord(clang::RecordDecl *rd) {
        SCOPES_RESULT_TYPE(const Type *);
        Symbol name = SYM_Unnamed;
        if (rd->isAnonymousStructOrUnion()) {
            auto tdn = rd->getTypedefNameForAnonDecl();
            if (tdn) {
                name = Symbol(String::from_stdstring(tdn->getName().data()));
            }
        } else {
            name = Symbol(String::from_stdstring(rd->getName().data()));
        }

        const Type *struct_type = nullptr;
        if (rd->isUnion()) {
            struct_type = get_typename(name, named_unions);
        } else if (rd->isStruct()) {
            struct_type = get_typename(name, named_structs);
        } else if (rd->isClass()) {
            struct_type = get_typename(name, named_classes);
        } else {
            SCOPES_ANCHOR(anchorFromLocation(rd->getSourceRange().getBegin()));
            StyledString ss;
            ss.out << "clang-bridge: can't translate record of unuspported type " << name;
            SCOPES_LOCATION_ERROR(ss.str());
        }

        clang::RecordDecl * defn = rd->getDefinition();
        if (defn && !record_defined[rd]) {
            record_defined[rd] = true;

            auto tni = cast<TypenameType>(const_cast<Type *>(struct_type));
            if (tni->finalized()) {
                SCOPES_ANCHOR(anchorFromLocation(rd->getSourceRange().getBegin()));
                StyledString ss;
                ss.out << "clang-bridge: duplicate body defined for type " << struct_type;
                SCOPES_LOCATION_ERROR(ss.str());
            }

            SCOPES_CHECK_RESULT(GetFields(tni, defn));

            if (name != SYM_Unnamed) {
                const Anchor *anchor = anchorFromLocation(rd->getSourceRange().getBegin());
                ScopeEntry target;
                // don't overwrite names already bound
                if (!dest->lookup(name, target)) {
                    dest->bind(name, ConstPointer::type_from(anchor, struct_type));
                }
            }
        }

        return struct_type;
    }

    SCOPES_RESULT(const Type *) TranslateEnum(clang::EnumDecl *ed) {
        SCOPES_RESULT_TYPE(const Type *);

        Symbol name(String::from_stdstring(ed->getName()));

        const Type *enum_type = get_typename(name, named_enums);

        clang::EnumDecl * defn = ed->getDefinition();
        if (defn && !enum_defined[ed]) {
            enum_defined[ed] = true;

            auto tag_type = SCOPES_GET_RESULT(TranslateType(ed->getIntegerType()));

            auto tni = cast<TypenameType>(const_cast<Type *>(enum_type));
            tni->super_type = TYPE_CEnum;
            SCOPES_CHECK_RESULT(tni->finalize(tag_type));

            for (auto it : ed->enumerators()) {
                const Anchor *anchor = anchorFromLocation(it->getSourceRange().getBegin());
                auto &val = it->getInitVal();

                auto name = Symbol(String::from_stdstring(it->getName().data()));
                auto value = ConstInt::from(anchor, enum_type, val.getExtValue());

                tni->bind(name, value);
                dest->bind(name, value);
            }
        }

        return enum_type;
    }

    bool always_immutable(clang::QualType T) {
        using namespace clang;
        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);
        switch (Ty->getTypeClass()) {
        case clang::Type::Elaborated: {
            const ElaboratedType *et = dyn_cast<ElaboratedType>(Ty);
            return always_immutable(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = dyn_cast<ParenType>(Ty);
            return always_immutable(pt->getInnerType());
        } break;
        case clang::Type::Typedef:
        case clang::Type::Record:
        case clang::Type::Enum:
            break;
        case clang::Type::Builtin:
            switch (cast<BuiltinType>(Ty)->getKind()) {
            case clang::BuiltinType::Void:
            case clang::BuiltinType::Bool:
            case clang::BuiltinType::Char_S:
            case clang::BuiltinType::SChar:
            case clang::BuiltinType::Char_U:
            case clang::BuiltinType::UChar:
            case clang::BuiltinType::Short:
            case clang::BuiltinType::UShort:
            case clang::BuiltinType::Int:
            case clang::BuiltinType::UInt:
            case clang::BuiltinType::Long:
            case clang::BuiltinType::ULong:
            case clang::BuiltinType::LongLong:
            case clang::BuiltinType::ULongLong:
            case clang::BuiltinType::WChar_S:
            case clang::BuiltinType::WChar_U:
            case clang::BuiltinType::Char16:
            case clang::BuiltinType::Char32:
            case clang::BuiltinType::Half:
            case clang::BuiltinType::Float:
            case clang::BuiltinType::Double:
            case clang::BuiltinType::LongDouble:
            case clang::BuiltinType::NullPtr:
            case clang::BuiltinType::UInt128:
            default:
                break;
            }
        case clang::Type::Complex:
        case clang::Type::LValueReference:
        case clang::Type::RValueReference:
            break;
        case clang::Type::Decayed: {
            const clang::DecayedType *DTy = cast<clang::DecayedType>(Ty);
            return always_immutable(DTy->getDecayedType());
        } break;
        case clang::Type::Pointer:
        case clang::Type::VariableArray:
        case clang::Type::IncompleteArray:
        case clang::Type::ConstantArray:
            break;
        case clang::Type::ExtVector:
        case clang::Type::Vector: return true;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: return true;
        case clang::Type::ObjCObject: break;
        case clang::Type::ObjCInterface: break;
        case clang::Type::ObjCObjectPointer: break;
        case clang::Type::BlockPointer:
        case clang::Type::MemberPointer:
        case clang::Type::Atomic:
        default:
            break;
        }
        if (T.isLocalConstQualified())
            return true;
        return false;
    }

    uint64_t PointerFlags(clang::QualType T) {
        uint64_t flags = 0;
        if (always_immutable(T))
            flags |= PTF_NonWritable;
        return flags;
    }

    // generate a storage type that matches alignment and size of the
    // original type; used for types that we can't translate
    SCOPES_RESULT(const Type *) TranslateStorage(clang::QualType T) {
        // retype as a tuple of aligned integer and padding byte array
        size_t sz = Context->getTypeSize(T);
        size_t al = Context->getTypeAlign(T);
        assert (sz % 8 == 0);
        assert (al % 8 == 0);
        sz = (sz + 7) / 8;
        al = (al + 7) / 8;
        assert (sz > al);
        ArgTypes fields;
        const Type *TB = integer_type(al * 8, false);
        fields.push_back(TB);
        size_t pad = sz - al;
        if (pad)
            fields.push_back(array_type(TYPE_U8, pad).assert_ok());
        return tuple_type(fields);
    }

    SCOPES_RESULT(const Type *) TranslateType(clang::QualType T) {
        SCOPES_RESULT_TYPE(const Type *);
        using namespace clang;

        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);

        switch (Ty->getTypeClass()) {
        case clang::Type::Attributed: {
            const AttributedType *at = dyn_cast<AttributedType>(Ty);
            // we probably want to eventually handle some of the attributes
            // but for now, ignore any attribute
            return TranslateType(at->getEquivalentType());
        } break;
        case clang::Type::Elaborated: {
            const ElaboratedType *et = dyn_cast<ElaboratedType>(Ty);
            return TranslateType(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = dyn_cast<ParenType>(Ty);
            return TranslateType(pt->getInnerType());
        } break;
        case clang::Type::Typedef: {
            const TypedefType *tt = dyn_cast<TypedefType>(Ty);
            TypedefNameDecl * td = tt->getDecl();
            auto it = typedefs.find(
                Symbol(String::from_stdstring(td->getName().data())));
            if (it == typedefs.end()) {
                return TYPE_Void;
            }
            return it->second;
        } break;
        case clang::Type::Record: {
            const RecordType *RT = dyn_cast<RecordType>(Ty);
            RecordDecl * rd = RT->getDecl();
            return TranslateRecord(rd);
        }  break;
        case clang::Type::Enum: {
            const clang::EnumType *ET = dyn_cast<clang::EnumType>(Ty);
            EnumDecl * ed = ET->getDecl();
            return TranslateEnum(ed);
        } break;
        case clang::Type::SubstTemplateTypeParm: {
            return TranslateType(T.getCanonicalType());
        } break;
        case clang::Type::TemplateSpecialization: {
            return TranslateStorage(T);
        } break;
        case clang::Type::Builtin:
            switch (cast<BuiltinType>(Ty)->getKind()) {
            case clang::BuiltinType::Void:
                return TYPE_Void;
            case clang::BuiltinType::Bool:
                return TYPE_Bool;
            case clang::BuiltinType::Char_S:
            case clang::BuiltinType::SChar:
            case clang::BuiltinType::Char_U:
            case clang::BuiltinType::UChar:
            case clang::BuiltinType::Short:
            case clang::BuiltinType::UShort:
            case clang::BuiltinType::Int:
            case clang::BuiltinType::UInt:
            case clang::BuiltinType::Long:
            case clang::BuiltinType::ULong:
            case clang::BuiltinType::LongLong:
            case clang::BuiltinType::ULongLong:
            case clang::BuiltinType::WChar_S:
            case clang::BuiltinType::WChar_U:
            case clang::BuiltinType::Char16:
            case clang::BuiltinType::Char32: {
                int sz = Context->getTypeSize(T);
                return integer_type(sz, !Ty->isUnsignedIntegerType());
            } break;
            case clang::BuiltinType::Half: return TYPE_F16;
            case clang::BuiltinType::Float:
                return TYPE_F32;
            case clang::BuiltinType::Double:
                return TYPE_F64;
            case clang::BuiltinType::LongDouble: return TYPE_F80;
            case clang::BuiltinType::NullPtr:
            case clang::BuiltinType::UInt128:
            default:
                break;
            }
        case clang::Type::Complex:
        case clang::Type::LValueReference: {
            const clang::LValueReferenceType *PTy =
                cast<clang::LValueReferenceType>(Ty);
            QualType ETy = PTy->getPointeeType();
            return pointer_type(SCOPES_GET_RESULT(TranslateType(ETy)), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::RValueReference:
            break;
        case clang::Type::Decayed: {
            const clang::DecayedType *DTy = cast<clang::DecayedType>(Ty);
            return TranslateType(DTy->getDecayedType());
        } break;
        case clang::Type::Pointer: {
            const clang::PointerType *PTy = cast<clang::PointerType>(Ty);
            QualType ETy = PTy->getPointeeType();
            return pointer_type(SCOPES_GET_RESULT(TranslateType(ETy)), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::VariableArray:
            break;
        case clang::Type::IncompleteArray: {
            const IncompleteArrayType *ATy = cast<IncompleteArrayType>(Ty);
            QualType ETy = ATy->getElementType();
            return pointer_type(SCOPES_GET_RESULT(TranslateType(ETy)), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::ConstantArray: {
            const ConstantArrayType *ATy = cast<ConstantArrayType>(Ty);
            const Type *at = SCOPES_GET_RESULT(TranslateType(ATy->getElementType()));
            uint64_t sz = ATy->getSize().getZExtValue();
            return array_type(at, sz);
        } break;
        case clang::Type::ExtVector:
        case clang::Type::Vector: {
            const clang::VectorType *VT = cast<clang::VectorType>(T);
            const Type *at = SCOPES_GET_RESULT(TranslateType(VT->getElementType()));
            uint64_t n = VT->getNumElements();
            return vector_type(at, n);
        } break;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: {
            const clang::FunctionType *FT = cast<clang::FunctionType>(Ty);
            return TranslateFuncType(FT);
        } break;
        case clang::Type::ObjCObject: break;
        case clang::Type::ObjCInterface: break;
        case clang::Type::ObjCObjectPointer: break;
        case clang::Type::BlockPointer:
        case clang::Type::MemberPointer:
        case clang::Type::Atomic:
        default:
            break;
        }
        SCOPES_LOCATION_ERROR(format("clang-bridge: cannot convert type: %s (%s)",
            T.getAsString().c_str(),
            Ty->getTypeClassName()));
    }

    SCOPES_RESULT(const Type *) TranslateFuncType(const clang::FunctionType * f) {
        SCOPES_RESULT_TYPE(const Type *);

        clang::QualType RT = f->getReturnType();

        const Type *returntype = SCOPES_GET_RESULT(TranslateType(RT));

        uint64_t flags = 0;

        ArgTypes argtypes;

        const clang::FunctionProtoType * proto = f->getAs<clang::FunctionProtoType>();
        if(proto) {
            if (proto->isVariadic()) {
                flags |= FF_Variadic;
            }
            for(size_t i = 0; i < proto->getNumParams(); i++) {
                clang::QualType PT = proto->getParamType(i);
                argtypes.push_back(SCOPES_GET_RESULT(TranslateType(PT)));
            }
        }

        return function_type(returntype, argtypes, flags);
    }

    void exportType(Symbol name, const Type *type, const Anchor *anchor) {
        dest->bind(name, ConstPointer::type_from(anchor, type));
    }

    void exportExtern(Symbol name, const Type *type, const Anchor *anchor) {
        dest->bind(name, Extern::from(anchor, type, name));
    }

    bool TraverseRecordDecl(clang::RecordDecl *rd) {
        if (!ok) return false;
        if (rd->isFreeStanding()) {
            ok = ok && TranslateRecord(rd).ok();
        }
        return true;
    }

    bool TraverseEnumDecl(clang::EnumDecl *ed) {
        if (!ok) return false;
        if (ed->isFreeStanding()) {
            ok = ok && TranslateEnum(ed).ok();
        }
        return true;
    }

    bool TraverseVarDecl(clang::VarDecl *vd) {
        if (!ok) return false;
        if (vd->isExternC()) {
            const Anchor *anchor = anchorFromLocation(vd->getSourceRange().getBegin());

            auto type = TranslateType(vd->getType());
            ok = ok && type.ok();
            if (!ok) return false;
            exportExtern(
                String::from_stdstring(vd->getName().data()),
                type.assert_ok(),
                anchor);
        }

        return true;
    }

    bool TraverseTypedefDecl(clang::TypedefDecl *td) {
        if (!ok) return false;

        //const Anchor *anchor = anchorFromLocation(td->getSourceRange().getBegin());

        auto type_result = TranslateType(td->getUnderlyingType());
        ok = ok && type_result.ok();
        if (!ok) return false;
        const Type *type = type_result.assert_ok();

        Symbol name = Symbol(String::from_stdstring(td->getName().data()));
        const Anchor *anchor = anchorFromLocation(td->getSourceRange().getBegin());

        typedefs.insert({name, type});
        exportType(name, type, anchor);

        return true;
    }

    bool TraverseLinkageSpecDecl(clang::LinkageSpecDecl *ct) {
        if (!ok) return false;
        if (ct->getLanguage() == clang::LinkageSpecDecl::lang_c) {
            return clang::RecursiveASTVisitor<CVisitor>::TraverseLinkageSpecDecl(ct);
        }
        return false;
    }

    bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *ct) {
        if (!ok) return false;
        return false;
    }

    bool TraverseFunctionDecl(clang::FunctionDecl *f) {
        if (!ok) return false;

        clang::DeclarationName DeclName = f->getNameInfo().getName();
        std::string FuncName = DeclName.getAsString();
        const clang::FunctionType * fntyp = f->getType()->getAs<clang::FunctionType>();

        if(!fntyp)
            return true;

        if(f->getStorageClass() == clang::SC_Static) {
            return true;
        }

        auto functype_result = TranslateFuncType(fntyp);
        ok = ok && functype_result.ok();
        if (!ok) return false;

        const Type *functype = functype_result.assert_ok();

        std::string InternalName = FuncName;
        clang::AsmLabelAttr * asmlabel = f->getAttr<clang::AsmLabelAttr>();
        if(asmlabel) {
            InternalName = asmlabel->getLabel();
            #ifndef __linux__
                //In OSX and Windows LLVM mangles assembler labels by adding a '\01' prefix
                InternalName.insert(InternalName.begin(), '\01');
            #endif
        }
        const Anchor *anchor = anchorFromLocation(f->getSourceRange().getBegin());

        exportExtern(Symbol(String::from_stdstring(FuncName)),
            functype, anchor);

        return true;
    }

};

class CodeGenProxy : public clang::ASTConsumer {
public:
    Scope *dest;

    static bool ok;
    CVisitor visitor;

    CodeGenProxy(Scope *dest_) : dest(dest_) {
        ok = true;
    }
    virtual ~CodeGenProxy() {}

    virtual void Initialize(clang::ASTContext &Context) {
        visitor.SetContext(&Context, dest);
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef D) {
        if (!ok) return false;
        for (clang::DeclGroupRef::iterator b = D.begin(), e = D.end(); b != e; ++b) {
            ok = ok && visitor.TraverseDecl(*b);
            ok = ok && visitor.ok;
            if (!ok) return false;
        }
        return true;
    }
};

bool CodeGenProxy::ok = true;

// see ASTConsumers.h for more utilities
class EmitLLVMOnlyAction : public clang::EmitLLVMOnlyAction {
public:
    Scope *dest;

    EmitLLVMOnlyAction(Scope *dest_) :
        clang::EmitLLVMOnlyAction((llvm::LLVMContext *)LLVMGetGlobalContext()),
        dest(dest_)
    {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                 clang::StringRef InFile) override {

        std::vector< std::unique_ptr<clang::ASTConsumer> > consumers;
        consumers.push_back(clang::EmitLLVMOnlyAction::CreateASTConsumer(CI, InFile));
        consumers.push_back(llvm::make_unique<CodeGenProxy>(dest));
        return llvm::make_unique<clang::MultiplexConsumer>(std::move(consumers));
    }
};

static std::vector<LLVMModuleRef> llvm_c_modules;

static void add_c_macro(clang::Preprocessor & PP,
    const clang::IdentifierInfo * II,
    clang::MacroDirective * MD, Scope *scope, std::list< std::pair<Symbol, Symbol> > &aliases) {
    if(!II->hasMacroDefinition())
        return;
    clang::MacroInfo * MI = MD->getMacroInfo();
    if(MI->isFunctionLike())
        return;
    bool negate = false;
    const clang::Token * Tok;
    auto numtokens = MI->getNumTokens();
    if(numtokens == 2 && MI->getReplacementToken(0).is(clang::tok::minus)) {
        negate = true;
        Tok = &MI->getReplacementToken(1);
    } else if(numtokens == 1) {
        Tok = &MI->getReplacementToken(0);
    } else {
        return;
    }

    if ((numtokens == 1) && Tok->is(clang::tok::identifier)) {
        // aliases need to be resolved once the whole namespace is known
        const String *name = String::from_cstr(II->getName().str().c_str());
        const String *value = String::from_cstr(Tok->getIdentifierInfo()->getName().str().c_str());
        aliases.push_back({ Symbol(name), Symbol(value) });
        return;
    }

    if ((numtokens == 1) && Tok->is(clang::tok::string_literal)) {
        clang::Token tokens[] = { *Tok };
        clang::StringLiteralParser Literal(tokens, PP, false);
        const String *name = String::from_cstr(II->getName().str().c_str());
        std::string svalue = Literal.GetString();
        const String *value = String::from(svalue.c_str(), svalue.size());
        const Anchor *anchor = anchor_from_location(PP.getSourceManager(),
            MI->getDefinitionLoc());
        scope->bind(Symbol(name), ConstPointer::string_from(anchor, value));
        return;
    }

    if(Tok->isNot(clang::tok::numeric_constant))
        return;

    clang::SmallString<64> IntegerBuffer;
    bool NumberInvalid = false;
    clang::StringRef Spelling = PP.getSpelling(*Tok, IntegerBuffer, &NumberInvalid);
    clang::NumericLiteralParser Literal(Spelling, Tok->getLocation(), PP);
    if(Literal.hadError)
        return;
    const String *name = String::from_cstr(II->getName().str().c_str());
    std::string suffix;
    if (Literal.hasUDSuffix()) {
        suffix = Literal.getUDSuffix();
        std::cout << "TODO: macro literal suffix: " << suffix << std::endl;
    }
    const Anchor *anchor = anchor_from_location(PP.getSourceManager(),
        MI->getDefinitionLoc());
    if(Literal.isFloatingLiteral()) {
        llvm::APFloat Result(0.0);
        Literal.GetFloatValue(Result);
        double V = Result.convertToDouble();
        if (negate)
            V = -V;
        scope->bind(Symbol(name), ConstReal::from(anchor, TYPE_F64, V));
    } else {
        llvm::APInt Result(64,0);
        Literal.GetIntegerValue(Result);
        int64_t i = Result.getSExtValue();
        if (negate)
            i = -i;
        scope->bind(Symbol(name), ConstInt::from(anchor, TYPE_I64, i));
    }
}

SCOPES_RESULT(Scope *) import_c_module (
    const std::string &path, const std::vector<std::string> &args,
    const char *buffer) {
    using namespace clang;
    SCOPES_RESULT_TYPE(Scope *);

    std::vector<const char *> aargs;
    aargs.push_back("clang");
    aargs.push_back(path.c_str());
    aargs.push_back("-I");
    aargs.push_back(scopes_clang_include_dir);
    aargs.push_back("-I");
    aargs.push_back(scopes_include_dir);
    for (size_t i = 0; i < args.size(); ++i) {
        aargs.push_back(args[i].c_str());
    }

    CompilerInstance compiler;
    compiler.setInvocation(createInvocationFromCommandLine(aargs));

    if (buffer) {
        auto &opts = compiler.getPreprocessorOpts();

        llvm::MemoryBuffer * membuffer =
            llvm::MemoryBuffer::getMemBuffer(buffer, "<buffer>").release();

        opts.addRemappedFile(path, membuffer);
    }

    // Create the compilers actual diagnostics engine.
    compiler.createDiagnostics();

    // Infer the builtin include path if unspecified.
    //~ if (compiler.getHeaderSearchOpts().UseBuiltinIncludes &&
        //~ compiler.getHeaderSearchOpts().ResourceDir.empty())
        //~ compiler.getHeaderSearchOpts().ResourceDir =
            //~ CompilerInvocation::GetResourcesPath(scopes_argv[0], MainAddr);

    LLVMModuleRef M = NULL;


    Scope *result = Scope::from();

    // Create and execute the frontend to generate an LLVM bitcode module.
    std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction(result));
    if (compiler.ExecuteAction(*Act)) {
        SCOPES_CHECK_OK(CodeGenProxy::ok);

        clang::Preprocessor & PP = compiler.getPreprocessor();
        PP.getDiagnostics().setClient(new IgnoringDiagConsumer(), true);

        std::list< std::pair<Symbol, Symbol> > todo;
        for(Preprocessor::macro_iterator it = PP.macro_begin(false),end = PP.macro_end(false);
            it != end; ++it) {
            const IdentifierInfo * II = it->first;
            MacroDirective * MD = it->second.getLatest();

            add_c_macro(PP, II, MD, result, todo);
        }

        while (!todo.empty()) {
            auto sz = todo.size();
            for (auto it = todo.begin(); it != todo.end();) {
                Value *value;
                if (result->lookup(it->second, value)) {
                    result->bind(it->first, value);
                    auto oldit = it++;
                    todo.erase(oldit);
                } else {
                    it++;
                }
            }
            // couldn't resolve any more keys, abort
            if (todo.size() == sz) break;
        }

        M = (LLVMModuleRef)Act->takeModule().release();
        assert(M);
        llvm_c_modules.push_back(M);
        SCOPES_CHECK_RESULT(add_module(M));
        return result;
    } else {
        SCOPES_LOCATION_ERROR(String::from("compilation failed"));
    }
}

} // namespace scopes
