Capture
=======

A capture is a runtime closure that transparently captures (hence the name)
runtime values outside of the function.

.. type:: Capture

   An opaque type.

   .. inline:: (__call self args...)
   .. inline:: (__drop self)
   .. inline:: (__typecall cls args...)
   .. inline:: (function return-type param-types...)
   .. inline:: (make-type ...)
.. type:: CaptureTemplate

   An opaque type.

   .. inline:: (__drop self)
   .. spice:: (__imply ...)
   .. inline:: (build-instance self f)
   .. inline:: (instance self types...)
   .. inline:: (typify-function cls types...)
.. type:: SpiceCapture

   An opaque type.

.. sugar:: (capture ...)
.. sugar:: (decorate-capture ...)
.. sugar:: (spice-capture ...)
