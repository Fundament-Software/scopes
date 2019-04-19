itertools
=========

itertools provides various utilities which simplify the composition of
generators and collectors.

.. define:: drain

   ``Collector``
.. inline:: (->> generator collector...)
.. inline:: (cascade collector...)
   
   two collectors:
   every time a is full, b collects a and a is reset
   when b ends, the remainder of a is collected
.. inline:: (cat coll)
   
   treat input as a generator and forward its arguments individually
.. inline:: (collect coll)
   
   run collector until full and return the result
.. inline:: (demux init-value f collector...)
.. inline:: (each generator collector)
   
   fold output from generator into collector
.. inline:: (filter f coll)
.. inline:: (flatten coll)
   
   collect variadic input as individual single items
.. inline:: (gate f a b)
   
   if f is true, collect input in a, otherwise collect in b
   when both are full, output both
   until then, new input for full containers is discarded
.. inline:: (imap gen f)
.. inline:: (limit f coll)
.. inline:: (map f coll)
.. inline:: (mux collector...)
   
   send input into multiple collectors which each fork the target collector
.. inline:: (mux1 c1 c2 coll)
   
   send input into two collectors which fork the target collector
.. inline:: (reduce init f)
.. inline:: (take n coll)
   
   limit collector to output n items
.. spice:: (compose ...)
.. spice:: (span ...)
.. spice:: (unpack-bitdim ...)
.. spice:: (unpack-dim ...)
.. spice:: (zip ...)
