

let x =
    embed
        using import testing
        assert (not (none? assert-error))

# should also be accessible in outer scope
assert (not (none? assert-error))
