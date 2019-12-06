/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "c_import.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "types.hpp"
#include "qualifiers.hpp"
#include "source_file.hpp"
#include "anchor.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "scope.hpp"
#include "execution.hpp"
#include "value.hpp"
#include "prover.hpp"
#include "dyn_cast.inc"
#include "timer.hpp"
#include "compiler_flags.hpp"
#include "ordered_map.hpp"

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
        return Anchor::from(key, PLoc.getLine(), PLoc.getColumn(),
            SM.getFileOffset(loc));
    }

    return unknown_anchor();
}

struct CNamespaces {
    typedef OrderedMap<Symbol, ConstPointerRef, Symbol::Hash> TypeMap;
    typedef OrderedMap<Symbol, PureRef, Symbol::Hash> PureMap;

    TypeMap structs;
    TypeMap unions;
    TypeMap enums;
    TypeMap typedefs;
    TypeMap classes;
    PureMap constants;
    PureMap externs;
    PureMap defines;
};

class CVisitor : public clang::RecursiveASTVisitor<CVisitor> {
public:
    CNamespaces *dest;
    clang::ASTContext *Context;
    Result<void> ok;
    std::unordered_map<clang::RecordDecl *, const Type *> record_defined;
    std::unordered_map<clang::EnumDecl *, const Type *> enum_defined;

    CVisitor() : dest(nullptr), Context(NULL) {
    }

#define SCOPES_COMBINE_RESULT(DEST, EXPR) { \
        auto _result = (EXPR); \
        if (!_result.ok()) \
            DEST = Result<void>::raise(_result.unsafe_error()); \
    }

    const Anchor *anchorFromLocation(clang::SourceLocation loc) {
        auto &SM = Context->getSourceManager();
        return anchor_from_location(SM, loc);
    }

    void SetContext(clang::ASTContext * ctx, CNamespaces *_dest) {
        Context = ctx;
        dest = _dest;
        const Type *T = plain_typename_type(String::from("__builtin_va_list"), nullptr,
            array_type(TYPE_I8, sizeof(va_list)).assert_ok()).assert_ok();
        dest->typedefs.insert(Symbol("__builtin_va_list"), ConstPointer::type_from(T));
    }

    SCOPES_RESULT(void) GetFields(const TypenameType *tni, clang::RecordDecl * rd) {
        SCOPES_RESULT_TYPE(void);
        auto &rl = Context->getASTRecordLayout(rd);

        bool is_union = rd->isUnion();

        if (is_union) {
            assert(tni->super() == TYPE_CUnion);
        } else {
            assert(tni->super() == TYPE_CStruct);
        }

        Types args;
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
                SYM_Unnamed : Symbol(String::from_stdstring(declname.getAsString()));

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
                        args.push_back(array_type(TYPE_U8, pad).assert_ok());
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

            args.push_back(key_type(name, fieldtype));
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
#else
                (void)ST;
#endif
            }
        }

        if (is_union) {
            tni->bind(SYM_UnionFields,
                ConstPointer::type_from(SCOPES_GET_RESULT(tuple_type(args))));
            SCOPES_CHECK_RESULT(tni->complete(SCOPES_GET_RESULT(
                union_storage_type(args, packed, explicit_alignment?al:0)), TNF_Plain));
        } else {
            SCOPES_CHECK_RESULT(tni->complete(SCOPES_GET_RESULT(
                tuple_type(args, packed, explicit_alignment?al:0)), TNF_Plain));
        }

        return {};
    }

    const Type *get_typename(Symbol prefix, Symbol name, CNamespaces::TypeMap &map, const Type *supertype) {
        if (name != SYM_Unnamed) {
            int it = map.find_index(name);
            if (it != -1) {
                return (const Type *)map.values[it]->value;
            }
        }
        const Type *T = incomplete_typename_type(
            String::join(String::from("<"),
                String::join(
                    String::join(prefix.name(), String::from(" ")),
                    String::join(name.name(), String::from(">")))),
            supertype);
        if (name != SYM_Unnamed) {
            auto ok = map.insert(name, ConstPointer::type_from(T));
            assert(ok);
        }
        return T;
    }

    SCOPES_RESULT(const Type *) TranslateRecord(clang::RecordDecl *rd) {
        SCOPES_RESULT_TYPE(const Type *);
        Symbol name = SYM_Unnamed;
        clang::RecordDecl * defn = rd->getDefinition();
        if (defn) {
            rd = defn;
            auto it = record_defined.find(defn);
            if (it != record_defined.end())
                return it->second;
        }

        bool is_anon = rd->isAnonymousStructOrUnion();
        if (is_anon) {
            auto tdn = rd->getTypedefNameForAnonDecl();
            if (tdn) {
                name = Symbol(String::from_stdstring(tdn->getName().data()));
            }
        } else {
            name = Symbol(String::from_stdstring(rd->getName().data()));
        }

        const Type *struct_type = nullptr;
        const Type *super_type = TYPE_CStruct;
        Symbol prefix = SYM_Struct;
        if (rd->isUnion()) {
            super_type = TYPE_CUnion;
            prefix = SYM_Union;
        } else if (rd->isStruct()) {
        } else if (rd->isClass()) {
        } else {
            SCOPES_ERROR(CImportUnsupportedRecordType,
                anchorFromLocation(rd->getSourceRange().getBegin()),
                name);
        }
        CNamespaces::TypeMap *tm = &dest->typedefs;
        if (!is_anon) {
            if (rd->isUnion()) {
                tm = &dest->unions;
            } else if (rd->isStruct()) {
                tm = &dest->structs;
            } else if (rd->isClass()) {
                tm = &dest->classes;
            }
        }
        struct_type = get_typename(prefix, name, *tm, super_type);
        if (defn) {
            record_defined.insert({defn, struct_type});
            auto tni = cast<TypenameType>(struct_type);
            if (!tni->is_complete()) {
                SCOPES_CHECK_RESULT(GetFields(tni, defn));
                //const Anchor *anchor = anchorFromLocation(rd->getSourceRange().getBegin());
            } else {
                // possibly already provided by scope
                /*
                SCOPES_ERROR(CImportDuplicateTypeDefinition,
                    anchorFromLocation(rd->getSourceRange().getBegin()),
                    struct_type);
                */
            }
        }

        return struct_type;
    }

    SCOPES_RESULT(const Type *) TranslateEnum(clang::EnumDecl *ed) {
        SCOPES_RESULT_TYPE(const Type *);

        clang::EnumDecl * defn = ed->getDefinition();
        if (defn) {
            ed = defn;
            auto it = enum_defined.find(defn);
            if (it != enum_defined.end())
                return it->second;
        }

        Symbol name(String::from_stdstring(ed->getName()));

        const Type *enum_type = get_typename(SYM_Enum, name, dest->enums, TYPE_CEnum);

        if (defn) {
            enum_defined.insert({ed, enum_type});
            auto tni = cast<TypenameType>(enum_type);
            if (!tni->is_complete()) {
                auto tag_type = SCOPES_GET_RESULT(TranslateType(ed->getIntegerType()));
                SCOPES_CHECK_RESULT(tni->complete(tag_type, TNF_Plain));
            }

            for (auto it : ed->enumerators()) {
                const Anchor *anchor = anchorFromLocation(it->getSourceRange().getBegin());
                auto &val = it->getInitVal();

                auto name = Symbol(String::from_stdstring(it->getName().data()));
                auto value = ref(anchor,
                    ConstInt::from(enum_type, val.getExtValue()));

                //auto _name = ConstInt::symbol_from(name);
                tni->bind(name, value);
                dest->constants.insert(name, value);
            }
        }

        return enum_type;
    }

    uint64_t PointerFlags(clang::QualType T) {
        using namespace clang;
        uint64_t flags = 0;
        if (T.isLocalConstQualified())
            flags |= PTF_NonWritable;
        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);
        switch (Ty->getTypeClass()) {
        case clang::Type::Elaborated: {
            const ElaboratedType *et = dyn_cast<ElaboratedType>(Ty);
            flags |= PointerFlags(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = dyn_cast<ParenType>(Ty);
            flags |= PointerFlags(pt->getInnerType());
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
            flags |= PointerFlags(DTy->getDecayedType());
        } break;
        case clang::Type::Pointer:
        case clang::Type::VariableArray:
        case clang::Type::IncompleteArray:
        case clang::Type::ConstantArray:
            break;
        case clang::Type::ExtVector:
        case clang::Type::Vector: {
            flags |= PTF_NonWritable;
        } break;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: {
            flags |= PTF_NonWritable | PTF_NonReadable;
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
        Types fields;
        const Type *TB = integer_type(al * 8, false);
        fields.push_back(TB);
        size_t pad = sz - al;
        if (pad)
            fields.push_back(array_type(TYPE_U8, pad).assert_ok());
        return tuple_type(fields);
    }

    SCOPES_RESULT(const Type *) _TranslateType(clang::QualType T) {
        SCOPES_RESULT_TYPE(const Type *);
        using namespace clang;

        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);

        switch (Ty->getTypeClass()) {
        case clang::Type::TypeOfExpr: {
            const TypeOfExprType *toet = cast<TypeOfExprType>(Ty);
            if (toet->isSugared()) {
                return _TranslateType(toet->desugar());
            }
        } break;
        case clang::Type::Attributed: {
            const AttributedType *at = cast<AttributedType>(Ty);
            // we probably want to eventually handle some of the attributes
            // but for now, ignore any attribute
            return TranslateType(at->getEquivalentType());
        } break;
        case clang::Type::Elaborated: {
            const ElaboratedType *et = cast<ElaboratedType>(Ty);
            return TranslateType(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = cast<ParenType>(Ty);
            return TranslateType(pt->getInnerType());
        } break;
        case clang::Type::Typedef: {
            const TypedefType *tt = cast<TypedefType>(Ty);
            TypedefNameDecl * td = tt->getDecl();
            int it = dest->typedefs.find_index(
                Symbol(String::from_stdstring(td->getName().data())));
            if (it == -1) {
                return empty_arguments_type();
            }
            return (const Type *)dest->typedefs.values[it]->value;
        } break;
        case clang::Type::Record: {
            const RecordType *RT = cast<RecordType>(Ty);
            RecordDecl * rd = RT->getDecl();
            return TranslateRecord(rd);
        }  break;
        case clang::Type::Enum: {
            const clang::EnumType *ET = cast<clang::EnumType>(Ty);
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
                return empty_arguments_type();
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
        case clang::Type::BlockPointer: {
            const clang::BlockPointerType *BT = cast<clang::BlockPointerType>(Ty);
	    return _TranslateType(BT->getPointeeType());
	} break;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: {
            const clang::FunctionType *FT = cast<clang::FunctionType>(Ty);
            return TranslateFuncType(FT);
        } break;
        case clang::Type::ObjCObject: break;
        case clang::Type::ObjCInterface: break;
        case clang::Type::ObjCObjectPointer: break;
        case clang::Type::MemberPointer:
        case clang::Type::Atomic:
        default:
            break;
        }

        SCOPES_ERROR(CImportCannotConvertType,
            T.getAsString().c_str(),
            Ty->getTypeClassName());
    }

    SCOPES_RESULT(const Type *) TranslateType(clang::QualType T) {
        //SCOPES_RESULT_TYPE(const Type *);
        using namespace clang;

        const clang::Type *Ty = T.getTypePtr();

        auto result = _TranslateType(T);
        if (!result.ok()) {
            StyledString ss;
            ss.out << T.getAsString().c_str() << " (" << Ty->getTypeClassName() << ")";
            auto val = ConstPointer::string_from(ss.str());
            SCOPES_TRACE_CONVERT_FOREIGN_TYPE(val);
            result.assert_error()->trace(_backtrace);
        }
        return result;
    }

    SCOPES_RESULT(const Type *) TranslateFuncType(const clang::FunctionType * f) {
        SCOPES_RESULT_TYPE(const Type *);

        clang::QualType RT = f->getReturnType();

        const Type *returntype = SCOPES_GET_RESULT(TranslateType(RT));

        uint64_t flags = 0;

        Types argtypes;

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
        dest->typedefs.insert(name, ref(anchor, ConstPointer::type_from(type)));
    }

    void exportExtern(Symbol name, const Type *type, const Anchor *anchor) {
        dest->externs.insert(name, ref(anchor, Global::from(type, name)));
    }

    void exportExternRef(Symbol name, const Type *type, const Anchor *anchor) {
        auto val = ref(anchor, Global::from(type, name));
        auto conv = ref(anchor, PureCast::from(ptr_to_ref(val->get_type()).assert_ok(), val));
        dest->externs.insert(name, conv);
    }

    bool TraverseRecordDecl(clang::RecordDecl *rd) {
        if (!ok.ok()) return false;
        if (rd->isFreeStanding()) {
            SCOPES_COMBINE_RESULT(ok, TranslateRecord(rd));
        }
        return true;
    }

    bool TraverseEnumDecl(clang::EnumDecl *ed) {
        if (!ok.ok()) return false;
        if (ed->isFreeStanding()) {
            SCOPES_COMBINE_RESULT(ok, TranslateEnum(ed));
        }
        return true;
    }

    bool TraverseVarDecl(clang::VarDecl *vd) {
        if (!ok.ok()) return false;
        if (vd->isExternC()) {
            const Anchor *anchor = anchorFromLocation(vd->getSourceRange().getBegin());

            auto type = TranslateType(vd->getType());
            SCOPES_COMBINE_RESULT(ok, type);
            if (!ok.ok()) return false;
            exportExternRef(
                String::from_stdstring(vd->getName().data()),
                type.assert_ok(),
                anchor);
        }

        return true;
    }

    bool TraverseTypedefDecl(clang::TypedefDecl *td) {
        if (!ok.ok()) return false;

        //const Anchor *anchor = anchorFromLocation(td->getSourceRange().getBegin());

        //clang::QualType QT = Context->getCanonicalType(td->getUnderlyingType());

        auto type_result = TranslateType(td->getUnderlyingType());
        SCOPES_COMBINE_RESULT(ok, type_result);
        if (!ok.ok()) return false;
        const Type *type = type_result.assert_ok();

        Symbol name = Symbol(String::from_stdstring(td->getName().data()));
        const Anchor *anchor = anchorFromLocation(td->getSourceRange().getBegin());

        exportType(name, type, anchor);

        return true;
    }

    bool TraverseLinkageSpecDecl(clang::LinkageSpecDecl *ct) {
        if (!ok.ok()) return false;
        if (ct->getLanguage() == clang::LinkageSpecDecl::lang_c) {
            return clang::RecursiveASTVisitor<CVisitor>::TraverseLinkageSpecDecl(ct);
        }
        return false;
    }

    bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *ct) {
        if (!ok.ok()) return false;
        return false;
    }

    bool TraverseFunctionDecl(clang::FunctionDecl *f) {
        if (!ok.ok()) return false;

        clang::DeclarationName DeclName = f->getNameInfo().getName();
        std::string FuncName = DeclName.getAsString();
        const clang::FunctionType * fntyp = f->getType()->getAs<clang::FunctionType>();

        if(!fntyp)
            return true;

        if(f->getStorageClass() == clang::SC_Static) {
            return true;
        }

        auto functype_result = TranslateFuncType(fntyp);
        SCOPES_COMBINE_RESULT(ok, functype_result);
        if (!ok.ok()) return false;

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
#undef SCOPES_COMBINE_RESULT

};

// see ASTConsumers.h for more utilities
class EmitLLVMOnlyAction : public clang::EmitLLVMOnlyAction {
public:
    CNamespaces *dest;
    Result<void> result;

    EmitLLVMOnlyAction(CNamespaces *dest_);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
        clang::StringRef InFile) override;
};

class CodeGenProxy : public clang::ASTConsumer {
public:
    EmitLLVMOnlyAction &act;
    CVisitor visitor;

    CodeGenProxy(EmitLLVMOnlyAction &_act) : act(_act) {
    }
    virtual ~CodeGenProxy() {}

    virtual void Initialize(clang::ASTContext &Context) {
        visitor.SetContext(&Context, act.dest);
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef D) {
        if (!visitor.ok.ok()) {
            act.result = visitor.ok;
            return false;
        }
        for (clang::DeclGroupRef::iterator b = D.begin(), e = D.end(); b != e; ++b) {
            visitor.TraverseDecl(*b);
            if (!visitor.ok.ok()) {
                act.result = visitor.ok;
                return false;
            }
        }
        return true;
    }
};

EmitLLVMOnlyAction::EmitLLVMOnlyAction(CNamespaces *dest_) :
    clang::EmitLLVMOnlyAction((llvm::LLVMContext *)LLVMGetGlobalContext()),
    dest(dest_)
{
}

std::unique_ptr<clang::ASTConsumer> EmitLLVMOnlyAction::CreateASTConsumer(clang::CompilerInstance &CI,
                                                clang::StringRef InFile) {

    std::vector< std::unique_ptr<clang::ASTConsumer> > consumers;
    consumers.push_back(clang::EmitLLVMOnlyAction::CreateASTConsumer(CI, InFile));
    consumers.push_back(llvm::make_unique<CodeGenProxy>(*this));
    return llvm::make_unique<clang::MultiplexConsumer>(std::move(consumers));
}

static std::vector<LLVMModuleRef> llvm_c_modules;

static void add_c_macro(clang::Preprocessor & PP,
    const clang::IdentifierInfo * II,
    clang::MacroDirective * MD, CNamespaces::PureMap &map, std::list< std::pair<Symbol, Symbol> > &aliases) {
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

        map.insert(name, ref(anchor, ConstPointer::string_from(value)));
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
        PureRef val;
        if (Literal.isFloat) {
            val = ConstReal::from(TYPE_F32, V);
        } else {
            val = ConstReal::from(TYPE_F64, V);
        }
        map.insert(name, ref(anchor, val));
    } else {
        llvm::APInt Result(64,0);
        Literal.GetIntegerValue(Result);
        PureRef val;
        if (Literal.isUnsigned) {
            uint64_t i = Result.getZExtValue();
            val = ConstInt::from((Literal.isLongLong?TYPE_U64:TYPE_U32), i);
        } else {
            int64_t i = Result.getSExtValue();
            if (negate)
                i = -i;
            val = ConstInt::from((Literal.isLongLong?TYPE_I64:TYPE_I32), i);
        }
        map.insert(name, ref(anchor, val));
    }
}

template<typename T>
static void build_namespace_symbols (const Scope *scope, Symbol symbol, T&map) {
    auto &&top = scope->table();
    int idx = top.find_index(ConstInt::symbol_from(symbol));
    if (idx < 0) return;
    auto value = top.values[idx].value;
    if (!value.isa<ConstPointer>()) return;
    auto sub = (const Scope *)value.cast<ConstPointer>()->value;
    auto &&subtable = sub->table();
    for (int i = 0; i < subtable.keys.size(); ++i) {
        auto key = subtable.keys[i].dyn_cast<ConstInt>();
        if (!key) continue;
        if (key->get_type() != TYPE_Symbol) continue;
        auto value = subtable.values[i].value.dyn_cast<ConstPointer>();
        if (!value) continue;
        if (value->get_type() != TYPE_Type) continue;
        auto symbol = Symbol::wrap(key->msw());
        map.insert(symbol, value);
    }
}

template<typename T>
static void merge_namespace_symbols (const Scope *&scope, Symbol symbol, const T&map) {
    const Scope *sub = Scope::from(nullptr, nullptr);
    for (int i = 0; i < map.keys.size(); ++i) {
        auto &&symbol = map.keys[i];
        auto &&value = map.values[i];
        sub = Scope::bind_from(
            ref(value.anchor(), ConstInt::symbol_from(symbol)),
            value, nullptr, sub);
    }
    sub->table();
    scope = Scope::bind_from(ConstInt::symbol_from(symbol),
        ConstPointer::scope_from(sub), nullptr, scope);
}

template<typename T>
static bool find_value_in_namespace(Symbol symbol, const T&map, PureRef &result) {
    int index = map.find_index(symbol);
    if (index != -1) {
        result = map.values[index];
        return true;
    } else {
        return false;
    }
}

static bool find_value_in_namespaces(const CNamespaces &ns, Symbol symbol, PureRef &result) {
    if (find_value_in_namespace(symbol, ns.typedefs, result)) return true;
    if (find_value_in_namespace(symbol, ns.constants, result)) return true;
    if (find_value_in_namespace(symbol, ns.externs, result)) return true;
    if (find_value_in_namespace(symbol, ns.defines, result)) return true;
    return false;
}

SCOPES_RESULT(const Scope *) import_c_module (
    const std::string &path, const std::vector<std::string> &args,
    const char *buffer,
    const Scope *scope) {
    using namespace clang;
    SCOPES_RESULT_TYPE(const Scope *);
    Timer sum_clang_time(TIMER_ImportC);

    std::vector<const char *> aargs;
    aargs.push_back("clang");
    aargs.push_back(path.c_str());
    aargs.push_back("-I");
    aargs.push_back(scopes_clang_include_dir);
    aargs.push_back("-I");
    aargs.push_back(scopes_include_dir);
    aargs.push_back("-fno-common");
    auto argcount = args.size();
    std::string object_file;
    for (size_t i = 0; i < argcount; ++i) {
        if ((args[i] == "-c") && ((i + 1) < argcount)) {
            object_file = args[i + 1];
            i += 2;
            continue;
        }
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

    CNamespaces ns;
    if (scope) {
        build_namespace_symbols(scope, SYM_Struct, ns.structs);
        build_namespace_symbols(scope, SYM_Union, ns.unions);
        build_namespace_symbols(scope, SYM_Enum, ns.enums);
        build_namespace_symbols(scope, KW_Define, ns.defines);
        build_namespace_symbols(scope, SYM_Const, ns.constants);
        build_namespace_symbols(scope, SYM_TypeDef, ns.typedefs);
        build_namespace_symbols(scope, SYM_Extern, ns.externs);
    }

    // Create and execute the frontend to generate an LLVM bitcode module.
    std::unique_ptr<EmitLLVMOnlyAction> Act(new EmitLLVMOnlyAction(&ns));
    if (compiler.ExecuteAction(*Act)) {
        SCOPES_CHECK_RESULT(Act->result);

        clang::Preprocessor & PP = compiler.getPreprocessor();
        PP.getDiagnostics().setClient(new IgnoringDiagConsumer(), true);

        std::list< std::pair<Symbol, Symbol> > todo;
        for(Preprocessor::macro_iterator it = PP.macro_begin(false),end = PP.macro_end(false);
            it != end; ++it) {
            const IdentifierInfo * II = it->first;
            MacroDirective * MD = it->second.getLatest();

            add_c_macro(PP, II, MD, ns.defines, todo);
        }

        while (!todo.empty()) {
            auto sz = todo.size();
            for (auto it = todo.begin(); it != todo.end();) {
                PureRef val;
                Symbol sym = it->second;
                if (find_value_in_namespaces(ns, sym, val)) {
                    ns.defines.insert(it->first, val);
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
        if (!object_file.empty()) {
            auto target_machine = get_object_target_machine();
            assert(target_machine);

            char *errormsg;
            static char filename[PATH_MAX];
            strncpy(filename, object_file.c_str(), PATH_MAX);
            if (LLVMTargetMachineEmitToFile(target_machine, M,
                filename, LLVMObjectFile, &errormsg)) {
                SCOPES_ERROR(CGenBackendFailed, errormsg);
            }
        }
        SCOPES_CHECK_RESULT(add_module(M, PointerMap(), CF_Cache));

        const Scope *result = Scope::from(nullptr, nullptr);
        merge_namespace_symbols(result, SYM_Struct, ns.structs);
        merge_namespace_symbols(result, SYM_Union, ns.unions);
        merge_namespace_symbols(result, SYM_Enum, ns.enums);
        merge_namespace_symbols(result, KW_Define, ns.defines);
        merge_namespace_symbols(result, SYM_Const, ns.constants);
        merge_namespace_symbols(result, SYM_TypeDef, ns.typedefs);
        merge_namespace_symbols(result, SYM_Extern, ns.externs);
        result->table();
        return result;
    } else {
        SCOPES_ERROR(CImportCompilationFailed);
    }
}

} // namespace scopes
