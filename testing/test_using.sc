
let S =
    'bind (Scope)
        test =
            fn () true

run-stage;

do
    let pattern = "^test$"
    using S filter pattern
    test;

do
    using S
    test;

do
    S.test;

