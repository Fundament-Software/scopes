pub mod compile;
pub mod db;
pub mod ir;
pub mod parser;
pub mod type_check;

use salsa::DebugWithDb;

#[salsa::jar(db = Db)]
pub struct Jar(
    crate::compile::compile,
    crate::ir::SourceProgram,
    crate::ir::Program,
    crate::ir::VariableId,
    crate::ir::FunctionId,
    crate::ir::Function,
    crate::ir::Diagnostics,
    crate::ir::Span,
    crate::parser::parse_statements,
    crate::type_check::type_check_program,
    crate::type_check::type_check_function,
    crate::type_check::find_function,
);

pub trait Db: salsa::DbWithJar<Jar> {}

impl<DB> Db for DB where DB: ?Sized + salsa::DbWithJar<Jar> {}

const SOURCE_PROGRAM_TEXT: &str = r"
fn area_rectangle(w, h) = w * h
fn area_circle(r) = 3.14 * r * r
print area_rectangle(3, 4)
print area_circle(1)
print 11 * 2
";

fn main() -> anyhow::Result<()> {
    let mut db = db::Database::default();
    let source_program_text = String::from(SOURCE_PROGRAM_TEXT);
    let source_program = ir::SourceProgram::new(&mut db, source_program_text);
    compile::compile(&db, source_program);
    let diagnostics = compile::compile::accumulated::<ir::Diagnostics>(&db, source_program);
    eprintln!("diagnostics:\n{diagnostics:?}");
    let parsed_program = parser::parse_statements(&db, source_program);
    println!("parsed:\n{:#?}", parsed_program.debug(&db));
    Ok(())
}
