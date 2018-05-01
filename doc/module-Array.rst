Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array
.. typefn:: (Array '__apply-type cls element-type capacity)
   
   Construct a mutable array type of ``element-type`` with a variable or
   fixed maximum capacity.
   
   If ``capacity`` is defined, then it specifies the maximum number
   of array elements permitted. If it is undefined, then an initial
   capacity of 16 elements is assumed, which is doubled whenever
   it is exceeded, allowing for an indefinite number of elements.
.. type:: FixedArray
.. reftypefn:: (FixedArray 'append self value)
.. reftypefn:: (FixedArray 'sort self key)
.. reftypefn:: (FixedArray 'emplace-append self args...)
.. type:: GrowingArray
.. reftypefn:: (GrowingArray 'append self value)
.. reftypefn:: (GrowingArray 'sort self key)
.. reftypefn:: (GrowingArray 'emplace-append self args...)
