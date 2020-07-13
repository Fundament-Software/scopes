testing
=======

The testing module simplifies writing and running tests in an ad-hoc
fashion.

.. type:: One

   
   this type is used for discovering leaks and double frees. It holds an integer
   value as well as a pointer to a single reference on the heap which is 1 as
   long as the object exists, otherwise 0. The refcount is leaked in
   order to not cause segfaults when a double free occurs.
   
   In addition, a global refcounter is updated which can be checked for balance.

   .. inline:: (__!= cls T)
   .. inline:: (__< cls T)
   .. inline:: (__<= cls T)
   .. inline:: (__== cls T)
   .. inline:: (__> cls T)
   .. inline:: (__>= cls T)
   .. fn:: (__drop self)
   .. fn:: (__repr self)
   .. inline:: (__typecall cls value)
   .. fn:: (check self)
   .. fn:: (refcount)
   .. fn:: (reset-refcount)
   .. fn:: (test-refcount-balanced)
   .. fn:: (value self)
.. sugar:: (features ...)
.. sugar:: (test ...)
.. sugar:: (test-compiler-error ...)
.. sugar:: (test-error ...)
.. sugar:: (test-modules ...)
