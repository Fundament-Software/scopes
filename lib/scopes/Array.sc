
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

fn define-common-array-methods (T)
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
        (getelementptr (load self.items) index) as ref

    typefn& T 'sort (self key)
        let count = (self.count as immutable)
        let items = (load self.items)
        let outer () =
        let inner (i swapped) = (unconst 1:usize) (unconst false)
        if (i < count)
            let a =
                (getelementptr items (sub i 1:usize)) as ref
            let b =
                (getelementptr items i) as ref
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
        let idx = (count as immutable)
        count += 1
        'reserve self (deref count)
        (getelementptr (deref self.items) idx) as ref

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
typefn& FixedArray 'reserve (self count)
    let T = (@ (typeof self))
    assert (count <= T.capacity) "capacity exceeded"

fn destructor (self items)
    let T = (@ (typeof self))
    let element-type = T.ElementType
    let element-destructor ok = (type@& element-type '__delete)
    if (not ok)
        return;
    let count =
        self.count as immutable
    let loop (i) = (unconst 0:usize)
    if (i < count)
        let ptr = (items @ i)
        element-destructor ptr
        loop (i + 1:usize)

fn FixedMutableArray (element-type capacity memory)
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
        set-type-symbol! this-struct 'Memory memory

        method '__repr (self)
            ..
                "[count="
                repr self.count
                " items="
                repr self.items
                "]"

        method& '__new (self)
            self.items = ('allocate-array memory element-type capacity)
            self.count = 0:usize

        method& '__delete (self)
            destructor self self.items
            'free memory (deref self.items)

define-common-array-methods GrowingArray
typefn& GrowingArray 'reserve (self count)
    if (count < self.capacity)
        return;
    let T = (@ (typeof self))
    let memory = T.Memory
    let element-type = T.ElementType
    # multiply capacity by 2.7 (roughly e)
    let new-capacity =
        self.capacity * 27:usize // 10:usize
    let count = (deref self.count)
    let old-items = ((deref self.items) as ref)
    let new-items =
        ('allocate-array memory element-type new-capacity) as ref
    move-construct-array count old-items new-items
    self.items = new-items
    self.capacity = new-capacity
    'free memory old-items
    return;

let DEFAULT_CAPACITY = (1:usize << 2:usize)
fn VariableMutableArray (element-type memory)
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
        set-type-symbol! this-struct 'Memory memory

        method '__repr (self)
            ..
                "[count="
                repr self.count
                " capacity="
                repr self.capacity
                " items="
                repr self.items
                "]"

        method& '__new (self opts...)
            let capacity = (va@ 'capacity opts...)
            let capacity =
                if (none? capacity) DEFAULT_CAPACITY
                else capacity
            self.items = ('allocate-array memory element-type capacity)
            self.count = 0:usize
            self.capacity = capacity

        method& '__delete (self)
            destructor self self.items
            'free memory (deref self.items)

typefn Array '__typecall (cls element-type capacity opts...)
    """"Construct a mutable array type of ``element-type`` with a variable or
        fixed maximum capacity.

        If ``capacity`` is defined, then it specifies the maximum number
        of array elements permitted. If it is undefined, then an initial
        capacity of 16 elements is assumed, which is doubled whenever
        it is exceeded, allowing for an indefinite number of elements.
    let memory =
        va@ 'memory opts...
    let memory =
        if (none? memory) HeapMemory
        else memory
    if (none? capacity)
        VariableMutableArray element-type memory
    else
        assert (constant? capacity) "capacity must be constant"
        FixedMutableArray element-type (capacity as usize) memory

do
    let Array FixedArray GrowingArray
    locals;

