
let S = (Scope)
'bind S
    test =
        fn () true

run-stage;

do
    using S filter "^test$"
    test;

do
    using S
    test;

do
    S.test;

