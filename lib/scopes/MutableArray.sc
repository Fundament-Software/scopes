
fn FixedMutableArray (element-type capacity)
    let arrayT =
        pointer element-type 'mutable

    fn assert-reference (self)
        assert ((typeof self) < reference) "array must be reference"

    struct
        .. "<MutableArray "
            string-repr element-type
            " x "
            string-repr (i32 capacity)
            ">"
        count : usize
        items : arrayT

        method 'apply-type (cls)
            CStruct.apply-type cls
                count = 0:usize
                items = (malloc-array element-type capacity)

        method 'countof (self)
            self.count

        method 'countof& (self)
            self.count as immutable

        method '@& (self index)
            assert ((index < capacity) & (index >= 0:usize)) "index out of bounds"
            'from-pointer reference
                getelementptr (load self.items) index

        method 'append (self value)
            assert-reference self
            let count = self.count
            assert (count < capacity) "capacity exceeded"
            store
                value as immutable
                getelementptr (load self.items) (count as immutable)
            count = count + 1
            return;


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
