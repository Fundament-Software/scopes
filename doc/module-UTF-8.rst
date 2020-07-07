UTF-8
=====

This module provides UTF-8 encoder and decoder collectors, as well as
an UTF-8 aware `char` function.

.. inline:: (decoder coll)
   
   convert a i8 character stream as UTF-8 codepoints of type i32.
   invalid bytes are forwarded as negative numbers; negating the number
   yields the offending byte character.
.. inline:: (encoder coll)
   
   convert an integer codepoint to i8 bytes.
   the collector forwards a byte at a time.
.. spice:: (char ...)
