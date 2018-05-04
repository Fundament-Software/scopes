
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
                    if (i >= self._count)
                        fdone;
                    else
                        fret (i + 1:usize) (self @ i)
                unconst 0:usize
        else
            return;

    typefn& T '__countof (self)
        deref self._count

    typefn& T '__@ (self index)
        let index = (index as usize)
        assert ((index < self._count) & (index >= 0:usize)) "index out of bounds"
        (getelementptr (deref self._items) index) as ref

    typefn& T 'sort (self key)
        let count = (deref self._count)
        let items = (deref self._items)
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

    fn append-slots (self n)
        let idx = (deref self._count)
        let new-count =
            idx +
                do
                    if (none? n) 1
                    else n
        'reserve self new-count
        self._count = new-count
        (getelementptr (deref self._items) idx) as ref

    typefn& T 'emplace-append (self args...)
        let dest = (append-slots self)
        construct dest args...
        dest

    typefn& T 'append (self value)
        let dest = (append-slots self)
        copy-construct dest value
        dest

    typefn& T 'clear (self)
        destruct-array (deref self._count) ((deref self._items) as ref)
        self._count = 0:usize
        return;

    return;

define-common-array-methods FixedArray
typefn& FixedArray 'reserve (self count)
    let T = (typeof& self)
    assert (count <= T.Capacity) "capacity exceeded"

fn destructor (self items)
    let T = (@ (typeof self))
    let element-type = T.ElementType
    let element-destructor ok = (type@& element-type '__delete)
    if (not ok)
        return;
    let count =
        deref self._count
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
        _items : arrayT
        _count : usize

        set-type-symbol! this-struct 'ElementType element-type
        set-type-symbol! this-struct 'Capacity capacity
        set-typename-super! this-struct FixedArray
        set-type-symbol! this-struct 'Memory memory

        method& '__repr (self)
            ..
                "[count="
                repr self._count
                " items="
                repr self._items
                "]"

        method& 'capacity (self) capacity

        method& '__new (self)
            self._items = ('allocate-array memory element-type capacity)
            self._count = 0:usize

        method& '__copy (self other)
            let count = (countof other)
            assert (count <= capacity)
                .. "cannot construct value of type " (repr (typeof& self))
                    \ " from value of type " (repr (typeof& other))
                    " because of insufficient capacity ("
                    \ (repr count) " > " (repr capacity) ")"
            self._items = ('allocate-array memory element-type capacity)
            self._count = count
            copy-construct-array count
                (deref self._items) as ref
                (deref other._items) as ref
            return;

        method& '__delete (self)
            destructor self self._items
            'free memory (deref self._items)

define-common-array-methods GrowingArray
typefn& GrowingArray 'reserve (self count)
    if (count < self._capacity)
        return;
    let loop (new-capacity) = (deref self._capacity)
    if (new-capacity < count)
        loop (new-capacity * 27:usize // 10:usize)
    let T = (@ (typeof self))
    let memory = T.Memory
    let element-type = T.ElementType
    # multiply capacity by 2.7 (roughly e)
    let count = (deref self._count)
    let old-items = ((deref self._items) as ref)
    let new-items =
        ('allocate-array memory element-type new-capacity) as ref
    move-construct-array count new-items old-items
    self._items = new-items
    self._capacity = new-capacity
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
        _items : arrayT
        _count : usize
        _capacity : usize

        set-type-symbol! this-struct 'ElementType element-type
        set-typename-super! this-struct GrowingArray
        set-type-symbol! this-struct 'Memory memory

        method& '__repr (self)
            ..
                "[count="
                repr self._count
                " capacity="
                repr self._capacity
                " items="
                repr self._items
                "]"

        method& 'capacity (self)
            deref self._capacity

        method& '__new (self opts...)
            let capacity = (va@ 'capacity opts...)
            let capacity =
                if (none? capacity) DEFAULT_CAPACITY
                else capacity
            self._items = ('allocate-array memory element-type capacity)
            self._count = 0:usize
            self._capacity = capacity

        method& '__copy (self other)
            let capacity = ('capacity other)
            let count = (countof other)
            self._items = ('allocate-array memory element-type capacity)
            self._count = count
            self._capacity = capacity
            copy-construct-array count
                (deref self._items) as ref
                (deref other._items) as ref
            return;

        method& '__delete (self)
            destructor self self._items
            'free memory (deref self._items)

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

