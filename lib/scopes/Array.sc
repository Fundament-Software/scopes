
""""Array
    =====

    Exports a configurable type for a mutable array that stores its elements
    on the heap rather than in registers or the stack.

let Array = (typename "Array")
set-typename-super! Array CStruct
let FixedArray = (typename "FixedArray")
let GrowingArray = (typename "GrowingArray")
set-typename-super! FixedArray Array
set-typename-super! GrowingArray Array

fn define-common-array-methods (T ensure-capacity)
    fn destructor (self items)
        let T = (@ (typeof self))
        let element-type = T.ElementType
        let element-destructor = (type@& element-type '__delete)
        if (none? element-destructor)
            return;
        let count =
            self.count as immutable
        let loop (i) = (unconst 0:usize)
        if (i < count)
            let ptr = (items @ i)
            element-destructor ptr
            loop (i + 1:usize)

    typefn& T '__delete (self)
        destructor self self.items
        free (load self.items)

    typefn& T '__as (self T)
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

    typefn& T '__countof (self)
        self.count as immutable

    typefn& T '__@ (self index)
        let index = (index as usize)
        assert ((index < self.count) & (index >= 0:usize)) "index out of bounds"
        reference.from-pointer
            getelementptr (load self.items) index

    typefn& T 'sort (self key)
        let count = (self.count as immutable)
        let items = (load self.items)
        let outer () =
        let inner (i swapped) = (unconst 1:usize) (unconst false)
        if (i < count)
            let a =
                reference.from-pointer
                    getelementptr items (sub i 1:usize)
            let b =
                reference.from-pointer
                    getelementptr items i
            let a-key =
                if (none? key) a
                else (key a)
            let b-key =
                if (none? key) b
                else (key b)
            let swapped =
                if (a-key > b-key)
                    let t = (local 'copy b)
                    b = a
                    a = t
                    true
                else
                    swapped
            inner (add i 1:usize) swapped
        elseif swapped
            outer;
        return;

    fn append-slot (self)
        let count = self.count
        ensure-capacity self count
        let idx = (count as immutable)
        count = count + 1
        return
            reference.from-pointer
                getelementptr (load self.items) idx

    typefn& T 'emplace-append (self args...)
        let element-type = ((@ (typeof self)) . ElementType)
        let dest = (append-slot self)
        store
            element-type args...
            dest
        dest

    typefn& T 'append (self value)
        let dest = (append-slot self)
        dest = value
        dest

    return;

define-common-array-methods FixedArray
    fn "ensure-fixed-capacity" (self count)
        let T = (@ (typeof self))
        assert (count < T.capacity) "capacity exceeded"

fn grow (self)
    let T = (@ (typeof self))
    let element-type = T.ElementType
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

define-common-array-methods GrowingArray
    fn "ensure-variable-capacity" (self count)
        if (count == self.capacity)
            grow self

fn FixedMutableArray (element-type capacity)
    let arrayT =
        pointer element-type 'mutable

    struct
        .. "<Array "
            string-repr element-type
            " x "
            string-repr (i32 capacity)
            ">"
        items : arrayT
        count : usize

        set-type-symbol! this-struct 'ElementType element-type
        set-type-symbol! this-struct 'capacity capacity
        set-typename-super! this-struct FixedArray

        method '__repr (self)
            ..
                "[count="
                repr self.count
                " items="
                repr self.items
                "]"

        method '__typecall (cls)
            CStruct.__typecall cls
                count = 0:usize
                items = (malloc-array element-type capacity)

let DEFAULT_CAPACITY = (1:usize << 4:usize)
fn VariableMutableArray (element-type)
    let arrayT =
        pointer element-type 'mutable

    struct
        .. "<Array "
            string-repr element-type
            ">"
        items : arrayT
        count : usize
        capacity : usize

        set-type-symbol! this-struct 'ElementType element-type
        set-typename-super! this-struct GrowingArray

        method '__repr (self)
            ..
                "[count="
                repr self.count
                " capacity="
                repr self.capacity
                " items="
                repr self.items
                "]"

        method '__typecall (cls capacity)
            let capacity =
                if (none? capacity) DEFAULT_CAPACITY
                else capacity
            CStruct.__typecall cls
                capacity = capacity
                count = 0:usize
                items = (malloc-array element-type capacity)

typefn Array '__typecall (cls element-type capacity)
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

do
    let Array FixedArray GrowingArray
    locals;

