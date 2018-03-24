
fn assert-reference (self)
    assert ((typeof self) < reference) "array must be reference"

fn define-common-array-methods (T)
    typefn T 'delete (self)
        assert-reference self
        let items = (load self.items)
        #do
            # TODO: call destructor on all items
            let count =
                self.count as immutable
            let loop (i) = (unconst 0:usize)
            if (i < count)
                let ptr = (getelementptr items i)
                # delete ptr
                loop (i + 1:usize)
        free items

    typefn T 'as& (self T)
        if (T == Generator)
            Generator
                label (fret fdone i)
                    if (i >= self.count)
                        fdone;
                    else
                        fret (i + 1:usize) (self @ i)
                unconst 0:usize
        else
            return;

    typefn T 'countof (self)
        self.count

    typefn T 'countof& (self)
        self.count as immutable

    typefn T '@& (self index)
        let index = (index as usize)
        assert ((index < self.count) & (index >= 0:usize)) "index out of bounds"
        'from-pointer reference
            getelementptr (load self.items) index
    return;


fn FixedMutableArray (element-type capacity)
    let arrayT =
        pointer element-type 'mutable

    struct
        .. "<MutableArray "
            string-repr element-type
            " x "
            string-repr (i32 capacity)
            ">"
        count : usize
        items : arrayT

        define-common-array-methods this-struct

        method 'apply-type (cls)
            CStruct.apply-type cls
                count = 0:usize
                items = (malloc-array element-type capacity)

        method 'append (self value)
            assert-reference self
            let count = self.count
            assert (count < capacity) "capacity exceeded"
            store
                value as immutable
                getelementptr (load self.items) (count as immutable)
            count = count + 1
            return;


let DEFAULT_CAPACITY = (1:usize << 4:usize)
fn VariableMutableArray (element-type)
    let arrayT =
        pointer element-type 'mutable

    fn grow (self)
        let new-capacity =
            self.capacity << 1:usize
        let count =
            self.count as immutable
        let old-items = (load self.items)
        let new-items =
            malloc-array element-type new-capacity
        let loop (i) = (unconst 0:usize)
        if (i < count)
            let sptr = (getelementptr old-items i)
            let dptr = (getelementptr new-items i)
            store (load sptr) dptr
            loop (i + 1:usize)
        self.items = new-items
        self.capacity = new-capacity
        free old-items
        return;

    struct
        .. "<MutableArray "
            string-repr element-type
            ">"
        capacity : usize
        count : usize
        items : arrayT

        define-common-array-methods this-struct

        method 'apply-type (cls capacity)
            let capacity =
                if (none? capacity) DEFAULT_CAPACITY
                else capacity
            CStruct.apply-type cls
                capacity = capacity
                count = 0:usize
                items = (malloc-array element-type capacity)

        method 'append (self value)
            assert-reference self
            let count = self.count
            if (count == self.capacity)
                grow self
            store
                value as immutable
                getelementptr (load self.items) (count as immutable)
            count = count + 1
            return;

fn MutableArray (element-type capacity)
    """"Construct a mutable array type of ``element-type`` with a variable or
        fixed maximum capacity.

        If ``capacity`` is defined, then it specifies the maximum number
        of array elements permitted. If it is undefined, then an initial
        capacity of 16 elements is assumed, which is doubled whenever
        it is exceeded, allowing for an indefinite number of elements.
    if (none? capacity)
        VariableMutableArray element-type
    else
        assert (constant? capacity) "capacity must be constant"
        FixedMutableArray element-type (capacity as usize)
