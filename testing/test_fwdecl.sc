
fn odd?

fn even? (n)
    if (n == 0)
        return true
    odd? (n - 1)
fn odd? (n)
    if (n == 0)
        return false
    even? (n - 1)

assert (even? 12)
assert (odd? 11)

