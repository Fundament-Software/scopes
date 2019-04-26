

let x =
    embed
        using import testing
        test (not (none? test-error))

# should also be accessible in outer scope
test (not (none? test-error))
