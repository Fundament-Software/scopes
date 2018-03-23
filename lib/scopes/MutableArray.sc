
fn FixedMutableArray (element-type capacity)
    struct
        .. "<MutableArray "
            string-repr element-type
            " x "
            string-repr (i32 capacity)
            ">"

fn VariableMutableArray (element-type)
    struct
        .. "<MutableArray "
            string-repr element-type
            ">"

fn MutableArray (element-type capacity)
    """"Construct a mutable array type of ``element-type`` with a variable or
        fixed maximum capacity.

        If ``capacity`` is defined, then it specifies the maximum number
        of array elements permitted. If it is undefined, then an initial
        capacity of 32 elements is assumed, which is doubled whenever
        it is exceeded, allowing for an indefinite number of elements.
    if (none? capacity)
        VariableMutableArray element-type
    else
        assert (constant? capacity) "capacity must be constant"
        FixedMutableArray element-type (capacity as usize)
