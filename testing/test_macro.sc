
define-macro TRUE
    true

syntax-extend
    assert (TRUE)

    let T = (typename "MacroizedType")
    set-type-symbol! T '__macro TRUE
    set-scope-symbol! syntax-scope 'T T
    syntax-scope

# T is now expanded as a macro
assert (T)
# but T is also a type
assert ((typeof T) == type)
