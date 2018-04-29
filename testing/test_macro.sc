
define Name
    let T = (typename "Name" (storage = (storageof string)))
    fn apply-type (str)
        bitcast str T
    define-macro constructor
        let name = (decons args)
        list let name '= (list apply-type (name as Syntax as Symbol as string))
    set-type-symbol! T '__macro constructor
    typefn T '__as (self destT)
        if (destT == string)
            bitcast self destT
    T

# Name is a type
assert ((typeof Name) == type)

# but Name can now also be expanded as a macro
Name variable

assert ((typeof variable) == Name)
assert (variable as string == "variable")

