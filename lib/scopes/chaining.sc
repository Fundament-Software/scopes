#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""chaining
    ========

    chaining provides the `-->` operator, which allows the nesting of
    expressions by chaining them in a sequence.

""""Expands a processing chain into nested expressions so that each expression
    is passed as a tailing argument to the following expression.

    `__` can be used as a placeholder token to position the previous expression.

    Example:

        :::scopes
        --> x
            f
            g
            h 2 __
            k

    Expands to:

        :::scopes
        k
            h 2
                g
                    f x
sugar --> (expr ...)
    fn placeholder? (elem)
        (('typeof elem) == Symbol) and (elem as Symbol == '__)

    fold (outp = expr) for expr in ...
        let anchor = ('anchor expr)
        'tag
            match ('typeof expr)
            case list
                let prev-outp = outp
                let expr = (expr as list)
                let outp found =
                    fold (outp found = '() false) for elem in expr
                        if (placeholder? elem)
                            if found
                                hide-traceback;
                                error@ ('anchor elem) "while expanding expression" "duplicate placeholder token"
                            _ (cons prev-outp outp) true
                        else
                            _ (cons elem outp) found
                if found
                    `[('reverse outp)]
                else
                    `[(.. expr (list prev-outp))]
            default
                `[(list expr outp)]
            anchor

do
    let -->

    locals;

