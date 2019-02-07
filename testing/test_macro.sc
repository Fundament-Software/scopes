
define Name
    let T = (typename "Name" (storage = (storageof string)))
    fn apply-type (str)
        bitcast str T
    sugar constructor (name)
        list let name '= (list apply-type (name as Symbol as string))
    'set-symbols T
        __macro = constructor
        __as =
            box-cast
                fn "apply-as" (vT T expr)
                    if (T == string)
                        return `(bitcast expr T)
                    compiler-error! "unsupported type"
    T

compile-stage;

# Name is a type
assert ((typeof Name) == type)

# but Name can now also be expanded as a macro
Name variable

assert ((typeof variable) == Name)
assert (variable as string == "variable")

