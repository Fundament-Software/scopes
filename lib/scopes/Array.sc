#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Array
    =====

    Exports a configurable type for a mutable array that stores its elements
    on the heap rather than in registers or the stack.

typedef Array < CStruct
typedef FixedArray < Array
typedef GrowingArray < Array

run-stage;

fn swap (a b)
    local t = b
    b = a
    a = t
    return;

fn iParent (i)
    (i - 1:i64) // 2

fn iLeftChild (i)
    2:i64 * i + 1:i64
fn iRightChild (i)
    2:i64 * i + 2:i64

do
    let this-type = Array

    @@ box-cast
    method '__as (cls T self)
        if (T == Generator)
            ast-quote
                Generator
                    label (fdone i)
                        if (i >= self._count)
                            fdone;
                        else
                            _ (i + 1:usize) (self @ i)
                    0:usize
        else
            compiler-error! "unsupported type"

    method inline '__countof (self)
        deref self._count

    method inline '__@ (self index)
        let index = (index as usize)
        assert (index < self._count) "index out of bounds"
        getelementptr self._items index

    #typefn& T 'sort (self key)
        let key =
            if (none? key)
                fn (x) x
            else key
        assert (constant? key)

        let count = ((deref self._count) as i64)
        let items = (deref self._items)

        inline siftDown (start end)
            loop (root) = start
            if ((iLeftChild root) <= end)
                let child = (iLeftChild root)
                let v_root = ((getelementptr items root) as ref)
                let k_root = (key v_root)
                let v_child = ((getelementptr items child) as ref)
                let k_child = (key v_child)
                label step2 (iswap v_swap k_swap)
                    if (iswap == root)
                        return;
                    swap v_root v_swap
                    repeat iswap
                label step1 (iswap v_swap k_swap)
                    let child1 = (child + 1:i64)
                    if (child1 <= end)
                        let v_child1 = ((getelementptr items child1) as ref)
                        let k_child1 = (key v_child1)
                        if (k_swap < k_child1)
                            step2 child1 v_child1 k_child1
                    step2 iswap v_swap k_swap
                if (k_root < k_child)
                    step1 child v_child k_child
                else
                    step1 root v_root k_root

        let count-1 = (count - 1:i64)

        inline heapify ()
            loop (start) =
                iParent count-1
            if (start >= 0:i64)
                siftDown start count-1
                repeat (start - 1:i64)

        heapify;
        loop (end) = count-1
        if (end > 0:i64)
            let v_0 = ((getelementptr items 0) as ref)
            let v_end = ((getelementptr items end) as ref)
            swap v_0 v_end
            let end = (end - 1:i64)
            siftDown 0:i64 end
            repeat end

    inline append-slots (self n)
        let idx = (deref self._count)
        let new-count = (idx + n)
        'reserve self new-count
        self._count = new-count
        getelementptr self._items idx

    #typefn& T 'emplace-append (self args...)
        let dest = (append-slots self 1:usize)
        construct dest args...
        dest

    #typefn& T 'append (self value)
        let dest = (append-slots self 1:usize)
        copy-construct dest value
        dest

    #method 'clear (self)
        destruct-array (deref self._count) ((deref self._items) as ref)
        self._count = 0:usize
        return;

    return;


do
    let this-type = FixedArray

    method '__typecall (element-type capacity memory)
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

    method& '__move (self other)
        assert ((typeof& self) == (typeof& other))
        self._items = other._items
        self._count = other._count

    method& '__delete (self)
        destructor self self._items
        'free memory (deref self._items)


do
    let this-type = GrowingArray


method 'reserve (self count)
    let T = (typeof self)
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

#define-common-array-methods GrowingArray
#typefn& GrowingArray 'reserve (self count)
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

#let DEFAULT_CAPACITY = (1:usize << 2:usize)
#fn VariableMutableArray (element-type memory)
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

        method& '__move (self other)
            assert ((typeof& self) == (typeof& other))
            self._items = other._items
            self._count = other._count
            self._capacity = other._capacity

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
    if (cls == Array)
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
    else
        compiler-error! "arrays can not be constructed as immutable"

do
    let Array FixedArray GrowingArray
    locals;

