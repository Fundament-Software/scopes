<style type="text/css" rel="stylesheet">body { counter-reset: chapter 7; }</style>

Array
=====

Provides mutable array types that store their elements on the heap rather
than in registers or the stack.

*type*{.property} `Array`{.descname} [](#scopes.type.Array "Permalink to this definition"){.headerlink} {#scopes.type.Array}

:   An opaque type of supertype `Struct`.

    *inline*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Array.inline.__== "Permalink to this definition"){.headerlink} {#scopes.Array.inline.__==}

    :   

    *fn*{.property} `__@`{.descname} (*&ensp;self index&ensp;*)[](#scopes.Array.fn.__@ "Permalink to this definition"){.headerlink} {#scopes.Array.fn.__@}

    :   Implements support for the `@` operator. Returns a view reference to the
        element at `index` of array `self`.

    *inline*{.property} `__as`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.Array.inline.__as "Permalink to this definition"){.headerlink} {#scopes.Array.inline.__as}

    :   Implements support for the `as` operator. Arrays can be cast to
        `Generator`, or directly passed to `for`.

    *fn*{.property} `__copy`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.fn.__copy "Permalink to this definition"){.headerlink} {#scopes.Array.fn.__copy}

    :   Implements support for the `copy` operation.

    *inline*{.property} `__countof`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.inline.__countof "Permalink to this definition"){.headerlink} {#scopes.Array.inline.__countof}

    :   Implements support for the `countof` operator. Returns the current
        number of elements stored in `self` as a value of `usize` type.

    *fn*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.fn.__drop "Permalink to this definition"){.headerlink} {#scopes.Array.fn.__drop}

    :   Implements support for freeing the array's memory when it goes out
        of scope.

    *inline*{.property} `__imply`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.Array.inline.__imply "Permalink to this definition"){.headerlink} {#scopes.Array.inline.__imply}

    :   Implements support for pointer casts, to pass the array to C functions
        for example.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls element-type capacity&ensp;*)[](#scopes.Array.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Array.inline.__typecall}

    :   Construct a mutable array type of `element-type` with a variable or
        fixed maximum capacity.
        
        If `capacity` is defined, then it specifies the maximum number
        of array elements permitted. If it is undefined, then an initial
        capacity of 16 elements is assumed, which is doubled whenever
        it is exceeded, allowing for an indefinite number of elements.

    *fn*{.property} `append`{.descname} (*&ensp;self value&ensp;*)[](#scopes.Array.fn.append "Permalink to this definition"){.headerlink} {#scopes.Array.fn.append}

    :   Append `value` as an element to the array `self` and return a reference
        to the new element. When the `array` is of `GrowingArray` type, this
        operation will transparently resize the array's storage.

    *fn*{.property} `clear`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.fn.clear "Permalink to this definition"){.headerlink} {#scopes.Array.fn.clear}

    :   Clear the array and reset its element count to zero. This will drop
        all elements that have been previously contained by the array.

    *inline*{.property} `emplace-append`{.descname} (*&ensp;self args...&ensp;*)[](#scopes.Array.inline.emplace-append "Permalink to this definition"){.headerlink} {#scopes.Array.inline.emplace-append}

    :   Construct a new element with arguments `args...` directly in a newly
        assigned slot of array `self`. When the `array` is of `GrowingArray`
        type, this operation will transparently resize the array's storage.

    *inline*{.property} `emplace-append-many`{.descname} (*&ensp;self size args...&ensp;*)[](#scopes.Array.inline.emplace-append-many "Permalink to this definition"){.headerlink} {#scopes.Array.inline.emplace-append-many}

    :   Construct a new element with arguments `args...` directly in a newly
        assigned slot of array `self`. When the `array` is of `GrowingArray`
        type, this operation will transparently resize the array's storage.

    *type*{.property} `insert`{.descname} [](#scopes.Array.type.insert "Permalink to this definition"){.headerlink} {#scopes.Array.type.insert}

    :   Insert `value` at `index` into the array `self` and return a reference
        to the new element. When the `array` is of `GrowingArray` type, this
        operation will transparently resize the array's storage.
        This operation offsets the index of each following element by 1.
        If index is omitted, `insert` operates like `append`.

    *fn*{.property} `last`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.fn.last "Permalink to this definition"){.headerlink} {#scopes.Array.fn.last}

    :   

    *fn*{.property} `pop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Array.fn.pop "Permalink to this definition"){.headerlink} {#scopes.Array.fn.pop}

    :   Remove element with highest index from array `self` and return it.

    *inline*{.property} `predicated-sort`{.descname} (*&ensp;self predicate ...&ensp;*)[](#scopes.Array.inline.predicated-sort "Permalink to this definition"){.headerlink} {#scopes.Array.inline.predicated-sort}

    :   Sort elements of array `self` from smallest to largest, either using
        the `<` operator supplied by the element type, or by using the predicate
        supplied by the callable `predicate`, which takes two arguments and
        is expected to return true if the first argument is smaller than the
        second one.

    *fn*{.property} `remove`{.descname} (*&ensp;self index&ensp;*)[](#scopes.Array.fn.remove "Permalink to this definition"){.headerlink} {#scopes.Array.fn.remove}

    :   Remove element at index from array `self` and return it.
        This operation offsets the index of each following element by -1.

    *fn*{.property} `resize`{.descname} (*&ensp;self count args...&ensp;*)[](#scopes.Array.fn.resize "Permalink to this definition"){.headerlink} {#scopes.Array.fn.resize}

    :   Resize the array to the specified count. Items are appended or removed
        to meet the desired count.

    *inline*{.property} `sort`{.descname} (*&ensp;self key ...&ensp;*)[](#scopes.Array.inline.sort "Permalink to this definition"){.headerlink} {#scopes.Array.inline.sort}

    :   Sort elements of array `self` from smallest to largest, either using
        the `<` operator supplied by the element type, or by using the key
        supplied by the callable `key`, which is expected to return a comparable
        value for each element value supplied.

    *fn*{.property} `swap`{.descname} (*&ensp;self a b&ensp;*)[](#scopes.Array.fn.swap "Permalink to this definition"){.headerlink} {#scopes.Array.fn.swap}

    :   Safely swap the contents of two indices.

*type*{.property} `FixedArray`{.descname} [](#scopes.type.FixedArray "Permalink to this definition"){.headerlink} {#scopes.type.FixedArray}

:   An opaque type of supertype `Array`.

    *fn*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.FixedArray.fn.__repr "Permalink to this definition"){.headerlink} {#scopes.FixedArray.fn.__repr}

    :   Implements support for the `repr` operation.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls opts...&ensp;*)[](#scopes.FixedArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.FixedArray.inline.__typecall}

    :   

    *inline*{.property} `capacity`{.descname} (*&ensp;self&ensp;*)[](#scopes.FixedArray.inline.capacity "Permalink to this definition"){.headerlink} {#scopes.FixedArray.inline.capacity}

    :   Returns the maximum capacity of array `self`, which is fixed.

    *fn*{.property} `reserve`{.descname} (*&ensp;self count&ensp;*)[](#scopes.FixedArray.fn.reserve "Permalink to this definition"){.headerlink} {#scopes.FixedArray.fn.reserve}

    :   Internally used by the type. Ensures that array `self` can hold at least
        `count` elements. A fixed array will raise an assertion when its
        capacity has been exceeded.

*type*{.property} `GrowingArray`{.descname} [](#scopes.type.GrowingArray "Permalink to this definition"){.headerlink} {#scopes.type.GrowingArray}

:   An opaque type of supertype `Array`.

    *fn*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.GrowingArray.fn.__repr "Permalink to this definition"){.headerlink} {#scopes.GrowingArray.fn.__repr}

    :   Implements support for the `repr` operation.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls opts...&ensp;*)[](#scopes.GrowingArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.GrowingArray.inline.__typecall}

    :   

    *inline*{.property} `capacity`{.descname} (*&ensp;self&ensp;*)[](#scopes.GrowingArray.inline.capacity "Permalink to this definition"){.headerlink} {#scopes.GrowingArray.inline.capacity}

    :   Returns the current maximum capacity of array `self`.

    *fn*{.property} `reserve`{.descname} (*&ensp;self count&ensp;*)[](#scopes.GrowingArray.fn.reserve "Permalink to this definition"){.headerlink} {#scopes.GrowingArray.fn.reserve}

    :   Internally used by the type. Ensures that array `self` can hold at least
        `count` elements. A growing array will always attempt to comply.

