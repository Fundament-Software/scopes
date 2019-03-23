
typedef Name : (storageof string)
    @@ spice-cast-macro
    fn __as (vT T)
        if (T == string)
            return `(inline (self) (bitcast self T))
        `()

run-stage;

'set-symbol Name '__macro
    sugar "constructor" ((name as Symbol))
        inline apply-type (str)
            bitcast str Name
        qq
            let [name] =
                [apply-type] [(name as Symbol as string)]

run-stage;

# Name is a type
assert ((typeof Name) == type)

# but Name can now also be expanded as a macro
Name variable

assert ((typeof variable) == Name)
assert (variable as string == "variable")
