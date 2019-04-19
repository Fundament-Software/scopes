Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   
   The abstract supertype of both `FixedArray` and `GrowingArray` which
   supplies methods shared by both implementations.

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

   
   The supertype and constructor for arrays of fixed size.
   
   To construct a new fixed array type::
   
       FixedArray element-type capacity
   
   Instantiate a new array with mutable memory::
   
       local new-array : (FixedArray element-type capacity)

   .. fn:: (__repr self)
      
      Implements support for the `repr` operation.
   .. inline:: (capacity self)
      
      Returns the maximum capacity of array `self`, which is fixed.
   .. fn:: (reserve self count)
      
      Internally used by the type. Ensures that array `self` can hold at least
      `count` elements. A fixed array will raise an assertion when its
      capacity has been exceeded.
.. type:: GrowingArray

   
   The supertype and constructor for arrays of growing size. New instances
   have a default capacity of 4, and grow by factor 2.7 each time their
   capacity is exceeded.
   
   To construct a new growing array type::
   
       GrowingArray element-type
   
   Instantiate a new array with mutable memory::
   
       local new-array : (GrowingArray element-type) [(capacity = ...)]

   .. fn:: (__repr self)
      
      Implements support for the `repr` operation.
   .. inline:: (capacity self)
      
      Returns the current maximum capacity of array `self`.
   .. fn:: (reserve self count)
      
      Internally used by the type. Ensures that array `self` can hold at least
      `count` elements. A growing array will always attempt to comply.
