Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array
.. typefn:: (Array '__typecall cls element-type capacity opts...)
   
   Construct a mutable array type of ``element-type`` with a variable or
   fixed maximum capacity.
   
   If ``capacity`` is defined, then it specifies the maximum number
   of array elements permitted. If it is undefined, then an initial
   capacity of 16 elements is assumed, which is doubled whenever
   it is exceeded, allowing for an indefinite number of elements.
.. reftypefn:: (Array '__new self args...)
.. type:: FixedArray
.. reftypefn:: (FixedArray 'clear self)
.. reftypefn:: (FixedArray 'reserve self count)
.. reftypefn:: (FixedArray 'append self value)
.. reftypefn:: (FixedArray 'emplace-append self args...)
.. reftypefn:: (FixedArray 'sort self key)
.. type:: GrowingArray
.. reftypefn:: (GrowingArray 'clear self)
.. reftypefn:: (GrowingArray 'reserve self count)
.. reftypefn:: (GrowingArray 'append self value)
.. reftypefn:: (GrowingArray 'emplace-append self args...)
.. reftypefn:: (GrowingArray 'sort self key)
