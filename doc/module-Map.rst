Map
===

This module implements a key -> value store using hashtables.

.. type:: Map

   An opaque type of supertype `Struct`.

   .. inline:: (__as cls T)
   .. inline:: (__countof self)
   .. fn:: (__drop self)
   .. inline:: (__rin ...)
   .. inline:: (__tobool self)
   .. inline:: (__typecall cls opts...)
   .. fn:: (clear self)
   .. fn:: (discard self key)
      
      erases a key -> value association from the map; if the map
      does not contain this key, nothing happens.
   .. fn:: (dump self)
   .. fn:: (get self key)
      
      returns the value associated with key or raises an error
   .. fn:: (getdefault self key value)
      
      returns the value associated with key or raises an error
   .. fn:: (in? self key)
   .. fn:: (set self key value)
      
      inserts a new key -> value association into map; key can be the
      output of any custom hash function. If the key already exists,
      it will be updated.
   .. fn:: (terseness self)
      
      computes the hashmap load as a normal between 0.0 and 1.0
.. type:: MapError

   An unique type of supertype `Enum` and of storage type `(tuple u8 (tuple (vector i8 1)))`.

