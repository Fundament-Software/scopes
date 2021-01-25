#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""String
    ======

    Provides a string type that manages a mutable byte buffer of varying size
    on the heap. Strings are guaranteed to be zero-terminated.

using import struct

# declare void @llvm.memcpy.p0i8.p0i8.i64(i8* <dest>, i8* <src>,
                                        i64 <len>, i1 <isvolatile>)
let llvm.memcpy.p0i8.p0i8.i64 =
    extern 'llvm.memcpy.p0i8.p0i8.i64
        function void (mutable rawstring) rawstring i64 bool
# declare void @llvm.memset.p0i8.i64(i8* <dest>, i8 <val>,
                                   i64 <len>, i1 <isvolatile>)
let llvm.memset.p0i8.i64 =
    extern 'llvm.memset.p0i8.i64
        function void (mutable rawstring) i8 i64 bool

inline string-generator (self)
    Generator
        inline () 0:usize
        inline (i) (i < self._count)
        inline (i) (self @ i)
        inline (i) (i + 1:usize)

typedef StringBase < Struct
typedef FixedString < StringBase
typedef GrowingString < StringBase

fn zero-terminated-length (value)
    let ZE = (nullof (elementof (typeof value)))
    # count length
    let len =
        loop (i = 0)
            let c = (value @ i)
            if (c == ZE)
                break i
            i + 1

fn copy-from-memory (self other count)
    let cls = (typeof self)
    let ZE = cls.ZeroElement
    let olditems = ('internal-reserve self count)
    let items = self._items
    loop (i = 0)
        if (i == count)
            items @ i = ZE
            self._count = i
            break;
        items @ i = other @ i
        i + 1
    free olditems

fn join-from-memory (self other count)
    local self = (copy self)
    let cls = (typeof self)
    let ZE = cls.ZeroElement
    let start = (countof self)
    let totalcount = (start + count)
    'reserve self totalcount
    let items = self._items
    loop (i j = 0 start)
        if (i == count)
            items @ j = ZE
            self._count = j
            break;
        items @ j = other @ i
        _ (i + 1) (j + 1)
    self

fn compare-strings== (self other count)
    let cls = (typeof self)
    let lcount = (countof self)
    if (lcount != count) false
    else
        let items = self._items
        loop (i = 0)
            if (i == count)
                break true
            let a = (deref (items @ i))
            let b = (deref (other @ i))
            if (a != b)
                break false
            else
                i + 1

inline gen-compare-strings (pred< pred= pred>)
    fn compare-strings (self other count)
        let cls = (typeof self)
        let lcount = (countof self)
        let items = self._items
        loop (i = 0)
            if (i == lcount)
                if (i == count)
                    break pred=
                else
                    break pred<
            elseif (i == count)
                break pred>
            let a = (deref (items @ i))
            let b = (deref (other @ i))
            if (a < b)
                break pred<
            elseif (a > b)
                break pred>
            else
                i + 1
let
    compare-strings> = (gen-compare-strings false false true)
    compare-strings< = (gen-compare-strings true false false)

inline string-binary-op (f superf)
    @@ memo
    inline (cls T)
        let ET = cls.ElementType
        static-if ((T == (pointer ET)) or (T == (mutable pointer ET)))
            inline (self other)
                f self other (zero-terminated-length other)
        elseif ((ET == i8) and (T == string))
            inline (self other)
                f self (other as rawstring) (countof other)
        elseif (not none? superf)
            superf cls T

inline inplace-string-binary-op (f superf)
    let nextf = (string-binary-op f superf)
    @@ memo
    inline (cls T)
        let ET = cls.ElementType
        static-if (cls == T)
            inline (self other)
                f self (other as rawstring) (countof other)
        else
            nextf cls T

inline inverted-string-binary-op (f)
    @@ memo
    inline (cls T)
        let f = (f cls T)
        static-if (not none? f)
            inline (self other)
                not f self other

""""The abstract supertype of both `FixedString` and `GrowingString` which
    supplies methods shared by both implementations.
typedef+ StringBase

    """"Implements support for the `as` operator. Strings can be cast to
        `Generator`, or directly passed to `for`.
    inline __as (cls T)
        static-if (T == Generator) string-generator
        elseif ((cls.ElementType == i8) and (T == string))
            inline (self)
                string self._items self._count

    inline __ras (T cls)
        static-if ((cls.ElementType == i8) and (T == string)) cls

    inline reverse (self)
        Generator
            inline () (deref self._count)
            inline (i) (i > 0:usize)
            inline (i) (self @ (i - 1:usize))
            inline (i) (i - 1:usize)

    """"Implements support for pointer casts, to pass the string to C functions
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

    inline __static-rimply (T cls)
        let ET = cls.ElementType
        static-if ((ET == i8) and (T == string)) cls

    inline __typecall (cls element-type capacity)
        """"Construct a mutable string type of `element-type` with a variable or
            fixed maximum capacity.

            If `capacity` is defined, then it specifies the maximum number
            of string elements permitted. If it is undefined, then an initial
            capacity of 16 elements is assumed, which is doubled whenever
            it is exceeded, allowing for an indefinite number of elements.
        static-assert (cls == StringBase)
        static-if (none? capacity)
            GrowingString element-type
        else
            FixedString element-type capacity

    let
        __= = (string-binary-op copy-from-memory super-type.__=)
        __.. = (inplace-string-binary-op join-from-memory)

        __== = (inplace-string-binary-op compare-strings==)
        __> = (inplace-string-binary-op compare-strings>)
        __< = (inplace-string-binary-op compare-strings<)

    let
        __!= = (inverted-string-binary-op __==)
        __<= = (inverted-string-binary-op __>)
        __>= = (inverted-string-binary-op __<)

    fn __copy (self)
        let cls = (typeof self)
        local result : cls
        copy-from-memory result (self as rawstring) (countof self)
        result

    """"Implements support for the `countof` operator. Returns the current
        number of elements stored in `self` as a value of `usize` type.
    inline __countof (self)
        deref self._count

    """"Implements support for the `@` operator. Returns a view reference to the
        element at `index` of string `self`.
    fn __@ (self index)
        let index = (index as usize)
        assert (index <= self._count) "index out of bounds"
        self._items @ index

    fn __hash (self)
        hash.from-bytes (self as rawstring) (countof self)

    fn last (self)
        assert (self._count > 0) "empty string has no last element"
        self._items @ (self._count - 1:usize)

    fn append-slots (self n)
        let idx = (deref self._count)
        let new-count = (idx + n)
        'reserve self new-count
        self._count = new-count
        self._items @ idx

    """"Append `value` as an element to the string `self` and return a reference
        to the new element. When the array is of `GrowingString` type, this
        operation will transparently resize the string's storage.
    fn append (self value)
        let dest = (append-slots self 1:usize)
        assign (imply value ((typeof self) . ElementType)) dest
        ;

    inline... append (self, value : (typematch T < StringBase))
        let cls = (typeof self)
        static-assert (cls == (typeof value))
        let count = (countof value)
        let ptr = (append-slots self (countof value))
        llvm.memcpy.p0i8.p0i8.i64
            bitcast (& ptr) (mutable rawstring)
            & (value @ 0)
            (count * (sizeof cls.ElementType)) as i64
            false
        ;
    case (self, value : string)
        let cls = (typeof self)
        static-assert (cls.ElementType == i8)
        let count = (countof value)
        let ptr = (append-slots self (countof value))
        llvm.memcpy.p0i8.p0i8.i64
            bitcast (& ptr) (mutable rawstring)
            value as rawstring
            (count * (sizeof cls.ElementType)) as i64
            false
        ;
    case using append

    """"Construct a new element with arguments `args...` directly in a newly
        assigned slot of string `self`. When the string is of `GrowingString`
        type, this operation will transparently resize the string's storage.
    inline emplace-append (self args...)
        let dest = (append-slots self 1:usize)
        let value = (((typeof self) . ElementType) args...)
        assign value dest
        dest

    """"Construct a new element with arguments `args...` directly in a newly
        assigned slot of string `self`. When the string is of `GrowingString`
        type, this operation will transparently resize the string's storage.
    inline emplace-append-many (self size args...)
        let dest = (& (append-slots self size))
        for idx in (range size)
            let value = (((typeof self) . ElementType) args...)
            assign value (dest @ idx)
        dest

    """"Insert `value` at `index` into the string `self` and return a reference
        to the new element. When the string is of `GrowingString` type, this
        operation will transparently resize the string's storage.
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

    """"Remove element with highest index from string `self` and return it.
    fn pop (self)
        let &count = self._count
        assert (&count > 0) "can't pop from empty string"
        &count -= 1
        let idx = (deref &count)
        let result = (dupe (deref (self._items @ idx)))
        store ((typeof self) . ZeroElement) (getelementptr self._items idx)
        result

    """"Remove element at index from string `self` and return it.
        This operation offsets the index of each following element by -1.
    fn remove (self index)
        let &count = self._count
        assert (index < &count) "can't pop from empty string"
        &count -= 1
        let items = self._items
        let result =
            dupe (deref (items @ index))
        for i in (range index &count)
            assign (dupe (items @ (i + 1))) (items @ i)
        store ((typeof self) . ZeroElement) (getelementptr self._items &count)
        result

    """"Clear the string and reset its element count to zero. This will drop
        all elements that have been previously contained by the string.
    fn clear (self)
        let cls = (typeof self)
        offset := (deref self._count)
        self._count = 0:usize
        # null remainder of memory
        llvm.memset.p0i8.i64
            bitcast self._items (mutable rawstring)
            0:i8
            (offset * (sizeof cls.ElementType)) as i64
            false
        return;

    """"Resize the array to the specified count. Items are apppend or removed
        to meet the desired count.
    fn resize (self count args...)
        let cls = (typeof self)
        let count = (count as usize)
        if (self._count < count)
            let T = (typeof self)
            delta := count - self._count
            emplace-append-many self delta args...
        else
            offset := self._count - count
            self._count = count
            # null remainder of memory
            llvm.memset.p0i8.i64
                bitcast (getelementptr self._items count) (mutable rawstring)
                0:i8
                (offset * (sizeof cls.ElementType)) as i64
                false

    fn reserve (self count)
        free ('internal-reserve self count)

    """"Implements support for freeing the string's memory when it goes out
        of scope.
    inline __drop (self)
        free self._items

    """"Safely swap the contents of two indices.
    fn swap (self a b)
        swap (self._items @ a) (self._items @ b)

    unlet append-slots

""""The supertype and constructor for strings of fixed size.

    To construct a new fixed string type:

        :::scopes
        FixedString element-type capacity

    Instantiate a new string with mutable memory:

        :::scopes
        local new-string : (FixedString element-type capacity)
typedef+ FixedString
    let parent-type = this-type

    @@ memo
    inline gen-fixed-string-type (element-type capacity)
        static-assert ((typeof element-type) == type)
        static-assert ((typeof capacity) == i32)
        static-assert (capacity >= 0)
        let parent-type = this-type
        struct
            .. "<FixedString "
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
                ZeroElement = (nullof element-type)
                Capacity = (capacity + 1)

    inline __typecall (cls opts...)
        static-if (cls == this-type)
            let element-type capacity = opts...
            gen-fixed-string-type element-type (capacity as i32)
        else
            let ET = cls.ElementType
            let items = (malloc-array ET cls.Capacity)
            store cls.ZeroElement items
            Struct.__typecall cls
                _items = items
                _count = 0:usize

    """"Implements support for the `repr` operation.
    fn __repr (self)
        let cls = (typeof self)
        if (cls.ElementType == i8)
            string self._items self._count
        else
            ..
                "[count="
                repr self._count
                " items="
                repr self._items
                "]"

    """"Returns the maximum capacity of string `self`, which is fixed.
    inline capacity (self)
        (typeof self) . Capacity

    """"Internally used by the type. Ensures that string `self` can hold at least
        `count` elements. A fixed string will raise an assertion when its
        capacity has been exceeded.
    fn internal-reserve (self count)
        let T = (typeof self)
        assert (count < T.Capacity) "capacity exceeded"
        nullof (typeof self._items)

    unlet gen-fixed-string-type parent-type



let DEFAULT_CAPACITY = (1:usize << 2:usize)

""""The supertype and constructor for strings of growing size. New instances
    have a default capacity of 4, and grow by a factor of 2.7 each time their
    capacity is exceeded.

    To construct a new growing string type:

        :::scopes
        GrowingString element-type

    Instantiate a new string with mutable memory:

        :::scopes
        local new-string : (GrowingString element-type) [(capacity = ...)]
typedef+ GrowingString
    let parent-type = this-type

    @@ memo
    inline gen-growing-string-type (element-type)
        static-assert ((typeof element-type) == type)
        let parent-type = this-type
        struct
            .. "<GrowingString "
                tostring element-type
                ">"
            \ < parent-type
            _items : (mutable pointer element-type)
            _count : usize
            _capacity : usize

            let
                ElementType = element-type
                PointerType = (pointer element-type)
                ZeroElement = (nullof element-type)

    fn nearest-capacity (capacity count)
        loop (new-capacity = capacity)
            if (new-capacity < count)
                repeat (new-capacity * 27:usize // 10:usize)
            break new-capacity

    @@ memo
    inline from-arguments (cls)
        from cls let PointerType
        inline... string-constructor
        case (s : string,)
            this-function (s as rawstring) (countof s)
        case (data : PointerType, count : usize)
            let capacity =
                nearest-capacity DEFAULT_CAPACITY (count + 1)
            let ET = cls.ElementType
            let items = (malloc-array ET capacity)
            llvm.memcpy.p0i8.p0i8.i64
                bitcast (view items) (mutable rawstring)
                bitcast data rawstring
                (count * (sizeof cls.ElementType)) as i64
                false
            # null remainder of memory
            llvm.memset.p0i8.i64
                bitcast (getelementptr (view items) count) (mutable rawstring)
                0:i8
                ((capacity - count) * (sizeof ET)) as i64
                false
            Struct.__typecall cls
                _items = items
                _count = count
                _capacity = capacity
        #case (data : PointerType,)
            this-function data (zero-terminated-length data)
        case (capacity : usize = DEFAULT_CAPACITY,)
            let capacity =
                nearest-capacity DEFAULT_CAPACITY (capacity + 1)
            let ET = cls.ElementType
            let items = (malloc-array ET capacity)
            llvm.memset.p0i8.i64
                bitcast items (mutable rawstring)
                0:i8
                (capacity * (sizeof ET)) as i64
                false
            Struct.__typecall cls
                _items = items
                _count = 0:usize
                _capacity = capacity

    inline __typecall (cls opts...)
        static-if (cls == this-type)
            gen-growing-string-type opts...
        else
            (from-arguments cls) opts...

    unlet from-arguments

    """"Implements support for the `repr` operation.
    fn __repr (self)
        let cls = (typeof self)
        if (cls.ElementType == i8)
            string self._items self._count
        else
            ..
                "[count="
                repr self._count
                " capacity="
                repr self._capacity
                " items="
                repr self._items
                "]"

    """"Returns the current maximum capacity of string `self`, which includes
        the trailing zero.
    inline capacity (self)
        deref self._capacity

    """"Internally used by the type. Ensures that string `self` can hold at least
        `count` elements. A growing string will always attempt to comply.
    fn internal-reserve (self count)
        let cls = (typeof self)
        if (count >= self._capacity)
            # multiply capacity by 2.7 (roughly e) until we can carry the desired
                count (plus trailing zero)
            let new-capacity =
                nearest-capacity (deref self._capacity) (count + 1)
            let T = (typeof self)
            let count = (deref self._count)
            let old-items = (deref self._items)
            let new-items = (malloc-array T.ElementType new-capacity)
            llvm.memcpy.p0i8.p0i8.i64
                bitcast (view new-items) (mutable rawstring)
                bitcast old-items rawstring
                (count * (sizeof T.ElementType)) as i64
                false
            # null remainder of memory
            llvm.memset.p0i8.i64
                bitcast (getelementptr (view new-items) count) (mutable rawstring)
                0:i8
                ((new-capacity - count) * (sizeof T.ElementType)) as i64
                false
            assign new-items self._items
            self._capacity = new-capacity
            dupe old-items
        else
            nullof (typeof self._items)

    unlet gen-growing-string-type parent-type nearest-capacity

do
    #let StringBase FixedString GrowingString
    let String = (GrowingString i8)
    locals;
