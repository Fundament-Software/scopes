#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Array
    =====

    Provides mutable array types that store their elements on the heap rather
    than in registers or the stack.

using import struct

# declare void @llvm.memcpy.p0i8.p0i8.i64(i8* <dest>, i8* <src>,
                                        i64 <len>, i1 <isvolatile>)
let llvm.memcpy.p0i8.p0i8.i64 =
    extern 'llvm.memcpy.p0i8.p0i8.i64
        function void (mutable rawstring) rawstring i64 bool

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

typedef Array < Struct
typedef FixedArray < Array
typedef GrowingArray < Array

""""The abstract supertype of both `FixedArray` and `GrowingArray` which
    supplies methods shared by both implementations.
typedef+ Array

    """"Implements support for the `as` operator. Arrays can be cast to
        `Generator`, or directly passed to `for`.
    inline __as (cls T)
        static-if (T == Generator) array-generator

    inline reverse (self)
        Generator
            inline () (deref self._count)
            inline (i) (i > 0:usize)
            inline (i) (self @ (i - 1:usize))
            inline (i) (i - 1:usize)

    """"Implements support for pointer casts, to pass the array to C functions
        for example.
    inline __imply (cls T)
        static-match T
        case pointer
            inline (self) (self._items as cls.PointerType)
        case voidstar
            inline (self) (self._items as voidstar)
        case cls.PointerType
            inline (self) (self._items as cls.PointerType)
        default ()

    inline __typecall (cls element-type capacity)
        """"Construct a mutable array type of `element-type` with a variable or
            fixed maximum capacity.

            If `capacity` is defined, then it specifies the maximum number
            of array elements permitted. If it is undefined, then an initial
            capacity of 16 elements is assumed, which is doubled whenever
            it is exceeded, allowing for an indefinite number of elements.
        static-assert (cls == Array)
        static-if (none? capacity)
            GrowingArray element-type
        else
            FixedArray element-type capacity

    """"Implements support for the `countof` operator. Returns the current
        number of elements stored in `self` as a value of `usize` type.
    inline __countof (self)
        deref self._count

    """"Implements support for the `@` operator. Returns a view reference to the
        element at `index` of array `self`.
    fn __@ (self index)
        let index = (index as usize)
        assert (index < self._count) "index out of bounds"
        self._items @ index

    fn last (self)
        assert (self._count > 0) "empty array has no last element"
        self._items @ (self._count - 1:usize)

    @@ memo
    inline gen-sort (key)
        let key =
            static-if (none? key)
                inline (x) x
            else key

        fn siftDown (items start end ...)
            loop (root = start)
                if ((iLeftChild root) > end)
                    break;
                let child = (iLeftChild root)
                let v_root = (items @ root)
                let k_root = (key v_root ...)
                let v_child = (items @ child)
                let k_child = (key v_child ...)
                inline step2 (iswap v_swap k_swap)
                    if (iswap == root)
                        break;
                    swap v_root v_swap
                    repeat iswap
                inline step1 (iswap v_swap k_swap)
                    let child1 = (child + 1:i64)
                    if (child1 <= end)
                        let v_child1 = (items @ child1)
                        let k_child1 = (key v_child1 ...)
                        if (k_swap < k_child1)
                            step2 child1 v_child1 k_child1
                    step2 iswap v_swap k_swap
                if (k_root < k_child)
                    step1 child v_child k_child
                else
                    step1 root v_root k_root

        fn "sort-array" (items count ...)
            let count-1 = (count - 1:i64)

            # heapify
            loop (start = (iParent count-1))
                if (start < 0:i64)
                    break;
                siftDown items start count-1 ...
                repeat (start - 1:i64)

            loop (end = count-1)
                if (end <= 0:i64)
                    break;
                let v_0 = (items @ 0)
                let v_end = (items @ end)
                swap v_0 v_end
                let end = (end - 1:i64)
                siftDown items 0:i64 end ...
                repeat end

    """"Sort elements of array `self` from smallest to largest, either using
        the `<` operator supplied by the element type, or by using the key
        supplied by the callable `key`, which is expected to return a comparable
        value for each element value supplied.
    inline sort (self key ...)
        (gen-sort key) (deref self._items) ((deref self._count) as i64) ...

    fn append-slots (self n)
        let idx = (deref self._count)
        let new-count = (idx + n)
        'reserve self new-count
        self._count = new-count
        self._items @ idx

    """"Append `value` as an element to the array `self` and return a reference
        to the new element. When the `array` is of `GrowingArray` type, this
        operation will transparently resize the array's storage.
    fn append (self value)
        let dest = (append-slots self 1:usize)
        assign (imply value ((typeof self) . ElementType)) dest
        dest

    """"Construct a new element with arguments `args...` directly in a newly
        assigned slot of array `self`. When the `array` is of `GrowingArray`
        type, this operation will transparently resize the array's storage.
    inline emplace-append (self args...)
        let dest = (append-slots self 1:usize)
        let value = (((typeof self) . ElementType) args...)
        assign value dest
        dest

    """"Construct a new element with arguments `args...` directly in a newly
        assigned slot of array `self`. When the `array` is of `GrowingArray`
        type, this operation will transparently resize the array's storage.
    inline emplace-append-many (self size args...)
        let dest = (& (append-slots self size))
        for idx in (range size)
            let value = (((typeof self) . ElementType) args...)
            assign value (dest @ idx)
        dest

    """"Insert `value` at `index` into the array `self` and return a reference
        to the new element. When the `array` is of `GrowingArray` type, this
        operation will transparently resize the array's storage.
        This operation offsets the index of each following element by 1.
        If index is omitted, `insert` operates like `append`.
    define insert
        fn _insert (self value index)
            let count = (deref self._count)
            assert (index <= count) "insertion index out of bounds"
            append-slots self 1:usize
            let items = self._items
            for i in (rrange index count)
                assign (dupe (items @ i)) (items @ (i + 1))
            let slot = (self._items @ index)
            assign value slot
            slot
        inline... insert
        case (self, value)
            append self value
        case (self, value, index)
            _insert self value index

    """"Remove element with highest index from array `self` and return it.
    fn pop (self)
        let &count = self._count
        assert (&count > 0) "can't pop from empty array"
        &count -= 1
        let idx = (deref &count)
        dupe (deref (self._items @ idx))

    """"Remove element at index from array `self` and return it.
        This operation offsets the index of each following element by -1.
    fn remove (self index)
        let &count = self._count
        assert (index < &count) "can't remove from empty array"
        &count -= 1
        let items = self._items
        let result =
            dupe (deref (items @ index))
        for i in (range index &count)
            assign (dupe (items @ (i + 1))) (items @ i)
        result

    """"Clear the array and reset its element count to zero. This will drop
        all elements that have been previously contained by the array.
    fn clear (self)
        for idx in (range (deref self._count))
            __drop (self._items @ idx)
        self._count = 0:usize
        return;

    """"Resize the array to the specified count. Items are appended or removed
        to meet the desired count.
    fn resize (self count args...)
        let count = (count as usize)
        if (self._count < count)
            let T = (typeof self)
            delta := count - self._count
            emplace-append-many self delta args...
        else
            offset := self._count - count
            for idx in (range offset (deref self._count))
                __drop (self._items @ idx)
            self._count = count

    """"Implements support for freeing the array's memory when it goes out
        of scope.
    fn __drop (self)
        returning void
        for idx in (range (deref self._count))
            __drop (self._items @ idx)
        free self._items

    """"Safely swap the contents of two indices.
    fn swap (self a b)
        swap (self._items @ a) (self._items @ b)

    unlet gen-sort append-slots

""""The supertype and constructor for arrays of fixed size.

    To construct a new fixed array type:

        :::scopes
        FixedArray element-type capacity

    Instantiate a new array with mutable memory:

        :::scopes
        local new-array : (FixedArray element-type capacity)
typedef+ FixedArray
    let parent-type = this-type

    @@ memo
    inline gen-fixed-array-type (element-type capacity)
        static-assert ((typeof element-type) == type)
        static-assert ((typeof capacity) == i32)
        let parent-type = this-type
        struct
            .. "<FixedArray "
                tostring element-type
                " x "
                tostring capacity
                ">"
            \ < parent-type

            _items : (mutable pointer element-type)
            _count : usize

            let
                ElementType = element-type
                PointerType = (pointer element-type)
                Capacity = capacity

    inline __typecall (cls opts...)
        static-if (cls == this-type)
            let element-type capacity = opts...
            gen-fixed-array-type element-type (capacity as i32)
        else
            Struct.__typecall cls
                _items = (malloc-array cls.ElementType cls.Capacity)
                _count = 0:usize

    """"Implements support for the `repr` operation.
    fn __repr (self)
        ..
            "[count="
            repr self._count
            " items="
            repr self._items
            "]"

    """"Returns the maximum capacity of array `self`, which is fixed.
    inline capacity (self)
        (typeof self) . Capacity

    """"Internally used by the type. Ensures that array `self` can hold at least
        `count` elements. A fixed array will raise an assertion when its
        capacity has been exceeded.
    fn reserve (self count)
        let T = (typeof self)
        assert (count <= T.Capacity) "capacity exceeded"

    unlet gen-fixed-array-type parent-type



let DEFAULT_CAPACITY = (1:usize << 2:usize)

""""The supertype and constructor for arrays of growing size. New instances
    have a default capacity of 4, and grow by a factor of 2.7 each time their
    capacity is exceeded.

    To construct a new growing array type:

        :::scopes
        GrowingArray element-type

    Instantiate a new array with mutable memory:

        :::scopes
        local new-array : (GrowingArray element-type) [(capacity = ...)]
typedef+ GrowingArray
    let parent-type = this-type

    @@ memo
    inline gen-growing-array-type (element-type)
        static-assert ((typeof element-type) == type)
        let parent-type = this-type
        struct
            .. "<GrowingArray "
                tostring element-type
                ">"
            \ < parent-type
            _items : (mutable pointer element-type)
            _count : usize
            _capacity : usize

            let
                ElementType = element-type
                PointerType = (pointer element-type)

    fn nearest-capacity (capacity count)
        loop (new-capacity = capacity)
            if (new-capacity < count)
                repeat (new-capacity * 27:usize // 10:usize)
            break new-capacity

    inline __typecall (cls opts...)
        static-if (cls == this-type)
            gen-growing-array-type opts...
        else
            let capacity =
                nearest-capacity DEFAULT_CAPACITY
                    (va-option capacity opts... DEFAULT_CAPACITY) as usize
            Struct.__typecall cls
                _items = (malloc-array cls.ElementType capacity)
                _count = 0:usize
                _capacity = capacity

    """"Implements support for the `repr` operation.
    fn __repr (self)
        ..
            "[count="
            repr self._count
            " capacity="
            repr self._capacity
            " items="
            repr self._items
            "]"

    """"Returns the current maximum capacity of array `self`.
    inline capacity (self)
        deref self._capacity

    """"Internally used by the type. Ensures that array `self` can hold at least
        `count` elements. A growing array will always attempt to comply.
    fn reserve (self count)
        if (count <= self._capacity)
            return;
        do
            # multiply capacity by 2.7 (roughly e) until we can carry the desired
                count
            let new-capacity =
                nearest-capacity (deref self._capacity) count
            let T = (typeof self)
            let count = (deref self._count)
            let old-items = (deref self._items)
            let new-items = (malloc-array T.ElementType new-capacity)
            llvm.memcpy.p0i8.p0i8.i64
                bitcast (view new-items) (mutable rawstring)
                bitcast old-items rawstring
                (count * (sizeof T.ElementType)) as i64
                false
            free old-items
            assign new-items self._items
            self._capacity = new-capacity
        return;

    unlet gen-growing-array-type parent-type nearest-capacity

do
    let Array FixedArray GrowingArray
    locals;

