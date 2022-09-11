use crate::ir::{
    Diagnostic, Diagnostics, Expression, ExpressionData, Function, FunctionId, Op, Program,
    SourceProgram, Span, Statement, StatementData, VariableId,
};
use ordered_float::OrderedFloat;

#[salsa::tracked]
pub fn parse_statements(db: &dyn crate::Db, source: SourceProgram) -> Program {
    // Get the source text from the database
    let source_text = source.text(db);

    // Create the parser
    let mut parser = Parser {
        db,
        source_text,
        position: 0,
    };

    // Read in statements until we reach the end of the input
    let mut result = vec![];
    loop {
        // Skip over any whitespace
        parser.skip_whitespace();

        // If there are no more tokens, break
        if parser.peek().is_none() {
            break;
        }

        // Otherwise, there is more input, so parse a statement.
        if let Some(statement) = parser.parse_statement() {
            result.push(statement);
        } else {
            // If we failed, report an error at whatever position the parser
            // got stuck. We could recover here by skipping to the end of the line
            // or something like that. But we leave that as an exercise for the reader!
            parser.report_error();
            break;
        }
    }

    Program::new(db, result)
}

/// The parser tracks the current position in the input.
///
/// There are parsing methods on the parser named `parse_foo`. Each such method tries to parse a
/// `foo` at current position. Once they've recognized a `foo`, they return `Some(foo)` with the
/// result, and they update the position. If there is a parse error
/// (i.e., they don't recognize a `foo` at the current position), they return `None`,
/// and they leave `position` at roughly the spot where parsing failed. You can use this to
/// report errors and recover.
///
/// There are some simpler method that read a single token (e.g., [`Parser::ch`]
/// or [`Parser::word`]). These methods guarantee that, when they return `None`, the position
/// is not changed apart from consuming whitespace. This allows them to be used to probe ahead
/// and test the next token.
struct Parser<'p> {
    db: &'p dyn crate::Db,
    source_text: &'p str,
    position: usize,
}

impl Parser<'_> {
    // Invoke `f` and, if it returns `None`, then restore the parsing position.
    fn probe<T: std::fmt::Debug>(&mut self, f: impl FnOnce(&mut Self) -> Option<T>) -> Option<T> {
        let p = self.position;
        if let Some(v) = f(self) {
            Some(v)
        } else {
            self.position = p;
            None
        }
    }

    // ANCHOR: report_error
    /// Report an error diagnostic at the current position.
    fn report_error(&self) {
        let next_position = match self.peek() {
            Some(ch) => self.position + ch.len_utf8(),
            None => self.position,
        };
        Diagnostics::push(
            self.db,
            Diagnostic {
                start: self.position,
                end: next_position,
                message: "unexpected character".to_string(),
            },
        );
    }
    // ANCHOR_END: report_error

    fn peek(&self) -> Option<char> {
        self.source_text[self.position..].chars().next()
    }

    // Returns a span ranging from `start_position` until the current position (exclusive)
    fn span_from(&self, start_position: usize) -> Span {
        Span::new(self.db, start_position, self.position)
    }

    fn consume(&mut self, ch: char) {
        debug_assert!(self.peek() == Some(ch));
        self.position += ch.len_utf8();
    }

    /// Skips whitespace and returns the new position.
    fn skip_whitespace(&mut self) -> usize {
        while let Some(ch) = self.peek() {
            if ch.is_whitespace() {
                self.consume(ch);
            } else {
                break;
            }
        }
        self.position
    }

    // ANCHOR: parse_statement
    fn parse_statement(&mut self) -> Option<Statement> {
        let start_position = self.skip_whitespace();
        let word = self.word()?;
        if word == "fn" {
            let func = self.parse_function()?;
            Some(Statement::new(
                self.span_from(start_position),
                StatementData::Function(func),
            ))
        } else if word == "print" {
            let expr = self.parse_expression()?;
            Some(Statement::new(
                self.span_from(start_position),
                StatementData::Print(expr),
            ))
        } else {
            None
        }
    }
    // ANCHOR_END: parse_statement

    // ANCHOR: parse_function
    fn parse_function(&mut self) -> Option<Function> {
        let start_position = self.skip_whitespace();
        let name = self.word()?;
        let name_span = self.span_from(start_position);
        let name: FunctionId = FunctionId::new(self.db, name);
        //                     ^^^^^^^^^^^^^^^
        //                Create a new interned struct.
        self.ch('(')?;
        let args = self.parameters()?;
        self.ch(')')?;
        self.ch('=')?;
        let body = self.parse_expression()?;
        Some(Function::new(self.db, name, name_span, args, body))
        //   ^^^^^^^^^^^^^
        // Create a new entity struct.
    }
    // ANCHOR_END: parse_function

    fn parse_expression(&mut self) -> Option<Expression> {
        self.parse_op_expression(Self::parse_expression1, Self::low_op)
    }

    fn low_op(&mut self) -> Option<Op> {
        if self.ch('+').is_some() {
            Some(Op::Add)
        } else if self.ch('-').is_some() {
            Some(Op::Subtract)
        } else {
            None
        }
    }

    /// Parses a high-precedence expression (times, div).
    ///
    /// On failure, skips arbitrary tokens.
    fn parse_expression1(&mut self) -> Option<Expression> {
        self.parse_op_expression(Self::parse_expression2, Self::high_op)
    }

    fn high_op(&mut self) -> Option<Op> {
        if self.ch('*').is_some() {
            Some(Op::Multiply)
        } else if self.ch('/').is_some() {
            Some(Op::Divide)
        } else {
            None
        }
    }

    fn parse_op_expression(
        &mut self,
        mut parse_expr: impl FnMut(&mut Self) -> Option<Expression>,
        mut op: impl FnMut(&mut Self) -> Option<Op>,
    ) -> Option<Expression> {
        let start_position = self.skip_whitespace();
        let mut expr1 = parse_expr(self)?;

        while let Some(op) = op(self) {
            let expr2 = parse_expr(self)?;
            expr1 = Expression::new(
                self.span_from(start_position),
                ExpressionData::Op(Box::new(expr1), op, Box::new(expr2)),
            );
        }

        Some(expr1)
    }

    /// Parses a "base expression" (no operators).
    ///
    /// On failure, skips arbitrary tokens.
    fn parse_expression2(&mut self) -> Option<Expression> {
        let start_position = self.skip_whitespace();
        if let Some(w) = self.word() {
            if self.ch('(').is_some() {
                let f = FunctionId::new(self.db, w);
                let args = self.parse_expressions()?;
                self.ch(')')?;
                return Some(Expression::new(
                    self.span_from(start_position),
                    ExpressionData::Call(f, args),
                ));
            }

            let v = VariableId::new(self.db, w);
            Some(Expression::new(
                self.span_from(start_position),
                ExpressionData::Variable(v),
            ))
        } else if let Some(n) = self.number() {
            Some(Expression::new(
                self.span_from(start_position),
                ExpressionData::Number(OrderedFloat::from(n)),
            ))
        } else if self.ch('(').is_some() {
            let expr = self.parse_expression()?;
            self.ch(')')?;
            Some(expr)
        } else {
            None
        }
    }

    fn parse_expressions(&mut self) -> Option<Vec<Expression>> {
        let mut r = vec![];
        loop {
            let expr = self.parse_expression()?;
            r.push(expr);
            if self.ch(',').is_none() {
                return Some(r);
            }
        }
    }

    /// Parses a list of variable identifiers, like `a, b, c`.
    /// No trailing commas because I am lazy.
    ///
    /// On failure, skips arbitrary tokens.
    fn parameters(&mut self) -> Option<Vec<VariableId>> {
        let mut r = vec![];
        loop {
            let name = self.word()?;
            let vid = VariableId::new(self.db, name);
            r.push(vid);

            if self.ch(',').is_none() {
                return Some(r);
            }
        }
    }

    /// Parses a single character.
    ///
    /// Even on failure, only skips whitespace.
    fn ch(&mut self, c: char) -> Option<Span> {
        let start_position = self.skip_whitespace();
        match self.peek() {
            Some(p) if c == p => {
                self.consume(c);
                Some(self.span_from(start_position))
            }
            _ => None,
        }
    }

    /// Parses an identifier.
    ///
    /// Even on failure, only skips whitespace.
    fn word(&mut self) -> Option<String> {
        self.skip_whitespace();

        // In this loop, if we consume any characters, we always
        // return `Some`.
        let mut s = String::new();
        let _position = self.position;
        while let Some(ch) = self.peek() {
            if ch.is_alphabetic() || ch == '_' || (!s.is_empty() && ch.is_numeric()) {
                s.push(ch);
            } else {
                break;
            }

            self.consume(ch);
        }

        if s.is_empty() {
            None
        } else {
            Some(s)
        }
    }

    /// Parses a number.
    ///
    /// Even on failure, only skips whitespace.
    fn number(&mut self) -> Option<f64> {
        let _start_position = self.skip_whitespace();

        self.probe(|this| {
            // 👆 We need the call to `probe` here because we could consume
            //    some characters like `3.1.2.3`, invoke `str::parse`, and then
            //    still return `None`.
            let mut s = String::new();
            while let Some(ch) = this.peek() {
                if ch.is_numeric() || ch == '.' {
                    s.push(ch);
                } else {
                    break;
                }

                this.consume(ch);
            }

            if s.is_empty() {
                None
            } else if let Ok(n) = str::parse(&s) {
                Some(n)
            } else {
                None
            }
        })
    }
}

/// Create a new database with the given source text and parse the result.
/// Returns the statements and the diagnostics generated.
#[cfg(test)]
fn parse_string(source_text: &str) -> String {
    use salsa::debug::DebugWithDb;

    // Create the database
    let mut db = crate::db::Database::default();

    // Create the source program
    let source_program = SourceProgram::new(&mut db, source_text.to_string());

    // Invoke the parser
    let statements = parse_statements(&db, source_program);

    // Read out any diagnostics
    let accumulated = parse_statements::accumulated::<Diagnostics>(&db, source_program);

    // Format the result as a string and return it
    format!("{:#?}", (statements.debug(&db), accumulated))
}

#[test]
fn parse_print() {
    let actual = parse_string("print 1 + 2");
    let expected = expect_test::expect![[r#"
        (
            Program {
                [salsa id]: 0,
                statements: [
                    Statement {
                        span: Span(
                            Id {
                                value: 5,
                            },
                        ),
                        data: Print(
                            Expression {
                                span: Span(
                                    Id {
                                        value: 4,
                                    },
                                ),
                                data: Op(
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 1,
                                            },
                                        ),
                                        data: Number(
                                            OrderedFloat(
                                                1.0,
                                            ),
                                        ),
                                    },
                                    Add,
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 3,
                                            },
                                        ),
                                        data: Number(
                                            OrderedFloat(
                                                2.0,
                                            ),
                                        ),
                                    },
                                ),
                            },
                        ),
                    },
                ],
            },
            [],
        )"#]];
    expected.assert_eq(&actual);
}

#[test]
fn parse_example() {
    let actual = parse_string(
        "
            fn area_rectangle(w, h) = w * h
            fn area_circle(r) = 3.14 * r * r
            print area_rectangle(3, 4)
            print area_circle(1)
            print 11 * 2
        ",
    );
    let expected = expect_test::expect![[r#"
        (
            Program {
                [salsa id]: 0,
                statements: [
                    Statement {
                        span: Span(
                            Id {
                                value: 10,
                            },
                        ),
                        data: Function(
                            Function(
                                Id {
                                    value: 1,
                                },
                            ),
                        ),
                    },
                    Statement {
                        span: Span(
                            Id {
                                value: 22,
                            },
                        ),
                        data: Function(
                            Function(
                                Id {
                                    value: 2,
                                },
                            ),
                        ),
                    },
                    Statement {
                        span: Span(
                            Id {
                                value: 29,
                            },
                        ),
                        data: Print(
                            Expression {
                                span: Span(
                                    Id {
                                        value: 28,
                                    },
                                ),
                                data: Call(
                                    FunctionId(
                                        Id {
                                            value: 1,
                                        },
                                    ),
                                    [
                                        Expression {
                                            span: Span(
                                                Id {
                                                    value: 24,
                                                },
                                            ),
                                            data: Number(
                                                OrderedFloat(
                                                    3.0,
                                                ),
                                            ),
                                        },
                                        Expression {
                                            span: Span(
                                                Id {
                                                    value: 26,
                                                },
                                            ),
                                            data: Number(
                                                OrderedFloat(
                                                    4.0,
                                                ),
                                            ),
                                        },
                                    ],
                                ),
                            },
                        ),
                    },
                    Statement {
                        span: Span(
                            Id {
                                value: 34,
                            },
                        ),
                        data: Print(
                            Expression {
                                span: Span(
                                    Id {
                                        value: 33,
                                    },
                                ),
                                data: Call(
                                    FunctionId(
                                        Id {
                                            value: 2,
                                        },
                                    ),
                                    [
                                        Expression {
                                            span: Span(
                                                Id {
                                                    value: 31,
                                                },
                                            ),
                                            data: Number(
                                                OrderedFloat(
                                                    1.0,
                                                ),
                                            ),
                                        },
                                    ],
                                ),
                            },
                        ),
                    },
                    Statement {
                        span: Span(
                            Id {
                                value: 39,
                            },
                        ),
                        data: Print(
                            Expression {
                                span: Span(
                                    Id {
                                        value: 38,
                                    },
                                ),
                                data: Op(
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 35,
                                            },
                                        ),
                                        data: Number(
                                            OrderedFloat(
                                                11.0,
                                            ),
                                        ),
                                    },
                                    Multiply,
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 37,
                                            },
                                        ),
                                        data: Number(
                                            OrderedFloat(
                                                2.0,
                                            ),
                                        ),
                                    },
                                ),
                            },
                        ),
                    },
                ],
            },
            [],
        )"#]];
    expected.assert_eq(&actual);
}

#[test]
fn parse_error() {
    let source_text: &str = "print 1 + + 2";
    //                       0123456789^ <-- this is the position 10, where the error is reported
    let actual = parse_string(source_text);
    let expected = expect_test::expect![[r#"
        (
            Program {
                [salsa id]: 0,
                statements: [],
            },
            [
                Diagnostic {
                    start: 10,
                    end: 11,
                    message: "unexpected character",
                },
            ],
        )"#]];
    expected.assert_eq(&actual);
}

#[test]
fn parse_precedence() {
    // this parses as `(1 + (2 * 3)) + 4`
    let source_text: &str = "print 1 + 2 * 3 + 4";
    let actual = parse_string(source_text);
    let expected = expect_test::expect![[r#"
        (
            Program {
                [salsa id]: 0,
                statements: [
                    Statement {
                        span: Span(
                            Id {
                                value: 11,
                            },
                        ),
                        data: Print(
                            Expression {
                                span: Span(
                                    Id {
                                        value: 10,
                                    },
                                ),
                                data: Op(
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 7,
                                            },
                                        ),
                                        data: Op(
                                            Expression {
                                                span: Span(
                                                    Id {
                                                        value: 1,
                                                    },
                                                ),
                                                data: Number(
                                                    OrderedFloat(
                                                        1.0,
                                                    ),
                                                ),
                                            },
                                            Add,
                                            Expression {
                                                span: Span(
                                                    Id {
                                                        value: 6,
                                                    },
                                                ),
                                                data: Op(
                                                    Expression {
                                                        span: Span(
                                                            Id {
                                                                value: 3,
                                                            },
                                                        ),
                                                        data: Number(
                                                            OrderedFloat(
                                                                2.0,
                                                            ),
                                                        ),
                                                    },
                                                    Multiply,
                                                    Expression {
                                                        span: Span(
                                                            Id {
                                                                value: 5,
                                                            },
                                                        ),
                                                        data: Number(
                                                            OrderedFloat(
                                                                3.0,
                                                            ),
                                                        ),
                                                    },
                                                ),
                                            },
                                        ),
                                    },
                                    Add,
                                    Expression {
                                        span: Span(
                                            Id {
                                                value: 9,
                                            },
                                        ),
                                        data: Number(
                                            OrderedFloat(
                                                4.0,
                                            ),
                                        ),
                                    },
                                ),
                            },
                        ),
                    },
                ],
            },
            [],
        )"#]];
    expected.assert_eq(&actual);
}
