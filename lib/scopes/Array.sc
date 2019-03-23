#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Array
    =====

    Exports a configurable type for a mutable array that stores its elements
    on the heap rather than in registers or the stack.

typedef Array
typedef FixedArray
typedef GrowingArray

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

inline array-generator (self)
    Generator
        inline () 0:usize
        inline (i) (i < self._count)
        inline (i) (self @ i)
        inline (i) (i + 1:usize)

typedef Array < CStruct

    @@ box-cast
    fn __as (cls T self)
        if (T == Generator)
            `(array-generator self)
        else
            compiler-error! "unsupported type"

    inline __countof (self)
        deref self._count

    fn __@ (self index)
        let index = (index as usize)
        assert (index < self._count) "index out of bounds"
        self._items @ index

    inline gen-sort (key)
        let key =
            static-if (none? key)
                inline (x) x
            else key

        fn siftDown (items start end)
            loop (root = start)
                if ((iLeftChild root) > end)
                    break;
                let child = (iLeftChild root)
                let v_root = (items @ root)
                let k_root = (key v_root)
                let v_child = (items @ child)
                let k_child = (key v_child)
                inline step2 (iswap v_swap k_swap)
                    if (iswap == root)
                        break;
                    swap v_root v_swap
                    repeat iswap
                inline step1 (iswap v_swap k_swap)
                    let child1 = (child + 1:i64)
                    if (child1 <= end)
                        let v_child1 = (items @ child1)
                        let k_child1 = (key v_child1)
                        if (k_swap < k_child1)
                            step2 child1 v_child1 k_child1
                    step2 iswap v_swap k_swap
                if (k_root < k_child)
                    step1 child v_child k_child
                else
                    step1 root v_root k_root

        fn "sort-array" (items count)
            let count-1 = (count - 1:i64)

            # heapify
            loop (start = (iParent count-1))
                if (start < 0:i64)
                    break;
                siftDown items start count-1
                repeat (start - 1:i64)

            loop (end = count-1)
                if (end <= 0:i64)
                    break;
                let v_0 = (items @ 0)
                let v_end = (items @ end)
                swap v_0 v_end
                let end = (end - 1:i64)
                siftDown items 0:i64 end
                repeat end

    inline sort (self key)
        (gen-sort key) (deref self._items) ((deref self._count) as i64)

    fn append-slots (self n)
        let idx = (deref self._count)
        let new-count = (idx + n)
        'reserve self new-count
        self._count = new-count
        self._items @ idx

    inline emplace-append (self args...)
        let dest = (append-slots self 1:usize)
        __init dest args...
        dest

    fn append (self value)
        let dest = (append-slots self 1:usize)
        __init-copy dest value
        dest

    fn clear (self)
        for idx in (range (deref self._count))
            __delete (self._items @ idx)
        self._count = 0:usize
        return;

    fn __delete (self)
        clear self
        free self._items


typedef FixedArray < Array
    fn __typecall (cls element-type capacity)
        let arrayT =
            'mutable (pointer element-type)

        struct
            .. "<FixedArray "
                'string element-type
                " x "
                tostring (i32 capacity)
                ">"
            \ < FixedArray

            _items : arrayT
            _count : usize

            let
                ElementType = element-type
                Capacity = capacity

    fn __repr (self)
        ..
            "[count="
            repr self._count
            " items="
            repr self._items
            "]"

    inline capacity (self)
        (typeof self) . Capacity

    fn __init (self)
        let T = (typeof self)
        self._items = (malloc-array T.ElementType T.Capacity)
        self._count = 0:usize

    #@@ box-binary-op
    #fn __= (selfT otherT self other)
        if false
            return `[]
        compiler-error! "unsupported type"

    #fn __init-copy (self other)
        assert ((typeof self) == (typeof other))
        self._items = other._items
        self._count = other._count

    fn reserve (self count)
        let T = (typeof self)
        assert (count <= T.Capacity) "capacity exceeded"


let DEFAULT_CAPACITY = (1:usize << 2:usize)

typedef GrowingArray < Array

    fn __typecall (cls element-type)
        let arrayT = ('mutable (pointer element-type))

        struct
            .. "<GrowingArray "
                'string element-type
                ">"
            \ < GrowingArray
            _items : arrayT
            _count : usize
            _capacity : usize

            let
                ElementType = element-type

    fn nearest-capacity (capacity count)
        loop (new-capacity = capacity)
            if (new-capacity < count)
                repeat (new-capacity * 27:usize // 10:usize)
            break new-capacity

    @@ spice-quote
    fn __init (self opts...)
        let T = (typeof self)
        let capacity =
            nearest-capacity DEFAULT_CAPACITY
                (va-option capacity opts... DEFAULT_CAPACITY) as usize
        self._items = (malloc-array T.ElementType capacity)
        self._count = 0:usize
        self._capacity = capacity

    fn __repr (self)
        ..
            "[count="
            repr self._count
            " capacity="
            repr self._capacity
            " items="
            repr self._items
            "]"

    inline capacity (self)
        deref self._capacity

    fn reserve (self count)
        if (count <= self._capacity)
            return;
        # multiply capacity by 2.7 (roughly e) until we can carry the desired
            count
        let new-capacity =
            nearest-capacity (deref self._capacity) count
        let T = (typeof self)
        let count = (deref self._count)
        let old-items = (deref self._items)
        let new-items = (malloc-array T.ElementType new-capacity)
        for i in (range count)
            new-items @ i = old-items @ i
        free old-items
        self._items = new-items
        self._capacity = new-capacity
        return;

#typefn Array '__typecall (cls element-type capacity opts...)
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

