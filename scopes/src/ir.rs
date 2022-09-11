use derive_new::new;
use ordered_float::OrderedFloat;

macro_rules! impl_debug_with_db {
    (<$db_type:ty> for struct $ident:ident { $($field_idents:tt : $field_types:ty),* $(,)? }) => {
        impl ::salsa::DebugWithDb<$db_type> for $ident {
            fn fmt(&self, f: &mut ::core::fmt::Formatter<'_>, _db: &$db_type) -> ::core::fmt::Result {
                #[allow(unused_imports)]
                use ::salsa::debug::helper::Fallback;
                f.debug_struct(::core::stringify!($ident))
                    $(.field(
                        ::core::stringify!($field_idents),
                        &::salsa::debug::helper::SalsaDebug::<$field_types, $db_type>::salsa_debug(
                            #[allow(clippy::needless_borrow)]
                            &self.$field_idents,
                            _db
                        )
                    ))*
                    .finish()
            }
        }
    };
    (<$db_type:ty> for enum $ident:ident { $($variant_match:pat => $variant_idents:ident $variants:tt),* $(,)? }) => {
        impl ::salsa::DebugWithDb<$db_type> for $ident {
            fn fmt(&self, _f: &mut ::core::fmt::Formatter<'_>, _db: &$db_type) -> ::core::fmt::Result {
                #[allow(unused_imports)]
                use ::salsa::debug::helper::Fallback;
                match self {
                    $($variant_match => impl_debug_with_db!(
                        @enum_variant (&self: $ident, _f, (_db): $db_type) => $variant_idents $variants,
                    ),)*
                }
            }
        }
    };
    (@enum_variant (&self: $ident:ident, $f:expr, ($db:expr): $db_type:ty) =>
        $variant_ident:ident ($($variant_field_ident:ident : $variant_field_type:ty),* $(,)?),
    ) => (
        $f.debug_tuple(::core::stringify!($variant_ident))
            $(.field(
                &::salsa::debug::helper::SalsaDebug::<$variant_field_type, $db_type>::salsa_debug(
                    #[allow(clippy::needless_borrow)]
                    &$variant_field_ident,
                    $db
                )
            ))*
            .finish()
    );
    // (<$db_type:ty> for enum $ident:ident { $($variant_idents:ident $variants:tt),* $(,)? }) => {
    //     impl ::salsa::DebugWithDb<$db_type> for $ident {
    //         fn fmt(&self, _f: &mut ::core::fmt::Formatter<'_>, _db: &$db_type) -> ::core::fmt::Result {
    //             #[allow(unused_imports)]
    //             use ::salsa::debug::helper::Fallback;
    //             impl_debug_with_db!(@enum_variants (&self: $ident, _f, (_db): $db_type) {
    //                 $($variant_idents $variants),*
    //             } => { })
    //         }
    //     }
    // };
    // (@enum_variants (&self: $ident:ident, $f:expr, ($db:expr): $db_type:ty) { } => { $($variant_tts:tt)* }) => (
    //     match self {
    //         $(variant_tts)*
    //     }
    // );
    // (@enum_variants (&self: $ident:ident, $f:expr, ($db:expr): $db_type:ty) {
    //     $variant_ident:ident ($variant_field_type:ty $(,)?),
    //     $($variant_idents:ident $variants:tt),*
    // } => { $(variant_tts:tt)* }) => (impl_debug_with_db!(@enum_variants (&self: $ident, $f, ($db): $db_type) {
    //     $($variant_idents $variants),*
    // } => {
    //     $ident::$variant_ident(variant_field: $variant_field_type) => $f.debug_tuple(::core::stringify!($variant_ident))
    //         .field(
    //             &::salsa::debug::helper::SalsaDebug::<$variant_field_type, $db_type>::salsa_debug(
    //                 #[allow(clippy::needless_borrow)]
    //                 &variant_field,
    //                 $db
    //             )
    //         )
    //         .finish(),
    //     $(variant_tts)*
    // }));
}

#[salsa::input]
pub struct SourceProgram {
    #[return_ref]
    pub text: String,
}

#[salsa::tracked]
pub struct Program {
    #[return_ref]
    pub statements: Vec<Statement>,
}

#[salsa::tracked]
pub struct Function {
    #[id]
    pub name: FunctionId,

    name_span: Span,

    #[return_ref]
    pub args: Vec<VariableId>,

    #[return_ref]
    pub body: Expression,
}

#[salsa::interned]
pub struct VariableId {
    #[return_ref]
    pub text: String,
}

#[salsa::interned]
pub struct FunctionId {
    #[return_ref]
    pub text: String,
}

#[derive(Debug, Eq, Hash, PartialEq, new)]
pub struct Statement {
    pub span: Span,

    pub data: StatementData,
}

impl_debug_with_db!(< <crate::Jar as salsa::jar::Jar<'_>>::DynDb > for struct Statement {
    span: Span,
    data: StatementData,
});

#[derive(Debug, Eq, Hash, PartialEq)]
pub enum StatementData {
    /// Defines `fn <name>(<args>) = <body>`
    Function(Function),
    /// Defines `print <expr>`
    Print(Expression),
}

impl_debug_with_db!(< <crate::Jar as salsa::jar::Jar<'_>>::DynDb > for enum StatementData {
    StatementData::Function(f) => Function(f: Function),
    StatementData::Print(expr) => Print(expr: Expression),
});

#[derive(Debug, Eq, Hash, PartialEq, new)]
pub struct Expression {
    pub span: Span,

    pub data: ExpressionData,
}

impl_debug_with_db!(< <crate::Jar as salsa::jar::Jar<'_>>::DynDb > for struct Expression {
    span: Span,
    data: ExpressionData,
});

#[derive(Debug, Eq, Hash, PartialEq)]
pub enum ExpressionData {
    Op(Box<Expression>, Op, Box<Expression>),
    Number(OrderedFloat<f64>),
    Variable(VariableId),
    Call(FunctionId, Vec<Expression>),
}

impl_debug_with_db!(< <crate::Jar as salsa::jar::Jar<'_>>::DynDb > for enum ExpressionData {
    ExpressionData::Op(lhs, op, rhs) => Op(lhs: Box<Expression>, op: Op, rhs: Box<Expression>),
    ExpressionData::Number(x) => Number(x: OrderedFloat<f64>),
    ExpressionData::Variable(id) => Variable(id: VariableId),
    ExpressionData::Call(f, args) => Call(f: FunctionId, args: Vec<Expression>),
});

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum Op {
    Add,
    Subtract,
    Multiply,
    Divide,
}

#[salsa::tracked]
pub struct Span {
    pub start: usize,
    pub end: usize,
}

#[salsa::accumulator]
pub struct Diagnostics(Diagnostic);

#[derive(Clone, Debug, new)]
pub struct Diagnostic {
    pub start: usize,
    pub end: usize,
    pub message: String,
}
