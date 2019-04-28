Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   An opaque type of supertype `Struct`.

   .. fn:: (__@ self index)
      
      Implements support for the `@` operator. Returns a view reference to the
      element at `index` of array `self`.
   
   .. spice:: (__as cls T)
   
      Implements support for the `as` operator. Arrays can be cast to
      `Generator`, or directly passed to `for`.
   .. inline:: (__countof self)
      
      Implements support for the `countof` operator. Returns the current
      number of elements stored in `self` as a value of `usize` type.
   .. inline:: (__drop self)
      
      Implements support for freeing the array's memory when it goes out
      of scope.
   .. inline:: (__typecall cls element-type capacity)
      
      Construct a mutable array type of ``element-type`` with a variable or
      fixed maximum capacity.
      
      If ``capacity`` is defined, then it specifies the maximum number
      of array elements permitted. If it is undefined, then an initial
      capacity of 16 elements is assumed, which is doubled whenever
      it is exceeded, allowing for an indefinite number of elements.
   .. fn:: (append self value)
      
      Append `value` as an element to the array `self` and return a reference
      to the new element. When the `array` is of `GrowingArray` type, this
      operation will transparently resize the array's storage.
   .. fn:: (clear self)
      
      Clear the array and reset its element count to zero. This will drop
      all elements that have been previously contained by the array.
   .. inline:: (emplace-append self args...)
      
      Construct a new element with arguments `args...` directly in a newly
      assigned slot of array `self`. When the `array` is of `GrowingArray`
      type, this operation will transparently resize the array's storage.
   .. inline:: (sort self key)
      
      Sort elements of array `self` from smallest to largest, either using
      the `<` operator supplied by the element type, or by using the key
      supplied by the callable `key`, which is expected to return a comparable
      value for each element value supplied.
.. type:: FixedArray

   An opaque type of supertype `Array`.

   .. fn:: (__repr self)
      
      Implements support for the `repr` operation.
   .. inline:: (__typecall cls opts...)
   .. inline:: (capacity self)
      
      Returns the maximum capacity of array `self`, which is fixed.
   .. fn:: (reserve self count)
      
      Internally used by the type. Ensures that array `self` can hold at least
      `count` elements. A fixed array will raise an assertion when its
      capacity has been exceeded.
.. type:: GrowingArray

   An opaque type of supertype `Array`.

   .. fn:: (__repr self)
      
      Implements support for the `repr` operation.
   .. inline:: (__typecall cls opts...)
   .. inline:: (capacity self)
      
      Returns the current maximum capacity of array `self`.
   .. fn:: (reserve self count)
      
      Internally used by the type. Ensures that array `self` can hold at least
      `count` elements. A growing array will always attempt to comply.
