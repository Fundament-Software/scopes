glm
===

The `glm` module exports the basic vector and matrix types as well as
related arithmetic operations which mimic the features available to shaders
written in the GL shader language.

.. type:: bmat2

   A plain type labeled ``bmat2x2`` of supertype `mat-type` and of storage type `(matrix bool 2 2)`.

.. type:: bmat2x2

   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 2)`.

.. type:: bmat2x3

   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 3)`.

.. type:: bmat2x4

   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 4)`.

.. type:: bmat3

   A plain type labeled ``bmat3x3`` of supertype `mat-type` and of storage type `(matrix bool 3 3)`.

.. type:: bmat3x2

   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 2)`.

.. type:: bmat3x3

   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 3)`.

.. type:: bmat3x4

   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 4)`.

.. type:: bmat4

   A plain type labeled ``bmat4x4`` of supertype `mat-type` and of storage type `(matrix bool 4 4)`.

.. type:: bmat4x2

   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 2)`.

.. type:: bmat4x3

   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 3)`.

.. type:: bmat4x4

   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 4)`.

.. type:: bvec2

   A plain type of supertype `gvec2` and of storage type `(vector bool 2)`.

.. type:: bvec3

   A plain type of supertype `gvec3` and of storage type `(vector bool 3)`.

.. type:: bvec4

   A plain type of supertype `gvec4` and of storage type `(vector bool 4)`.

.. type:: dmat2

   A plain type labeled ``dmat2x2`` of supertype `mat-type` and of storage type `(matrix f64 2 2)`.

.. type:: dmat2x2

   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 2)`.

.. type:: dmat2x3

   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 3)`.

.. type:: dmat2x4

   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 4)`.

.. type:: dmat3

   A plain type labeled ``dmat3x3`` of supertype `mat-type` and of storage type `(matrix f64 3 3)`.

.. type:: dmat3x2

   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 2)`.

.. type:: dmat3x3

   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 3)`.

.. type:: dmat3x4

   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 4)`.

.. type:: dmat4

   A plain type labeled ``dmat4x4`` of supertype `mat-type` and of storage type `(matrix f64 4 4)`.

.. type:: dmat4x2

   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 2)`.

.. type:: dmat4x3

   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 3)`.

.. type:: dmat4x4

   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 4)`.

.. type:: dvec2

   A plain type of supertype `gvec2` and of storage type `(vector f64 2)`.

.. type:: dvec3

   A plain type of supertype `gvec3` and of storage type `(vector f64 3)`.

.. type:: dvec4

   A plain type of supertype `gvec4` and of storage type `(vector f64 4)`.

.. type:: gvec2

   An opaque type of supertype `vec-type`.

.. type:: gvec3

   An opaque type of supertype `vec-type`.

.. type:: gvec4

   An opaque type of supertype `vec-type`.

.. type:: imat2

   A plain type labeled ``imat2x2`` of supertype `mat-type` and of storage type `(matrix i32 2 2)`.

.. type:: imat2x2

   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 2)`.

.. type:: imat2x3

   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 3)`.

.. type:: imat2x4

   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 4)`.

.. type:: imat3

   A plain type labeled ``imat3x3`` of supertype `mat-type` and of storage type `(matrix i32 3 3)`.

.. type:: imat3x2

   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 2)`.

.. type:: imat3x3

   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 3)`.

.. type:: imat3x4

   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 4)`.

.. type:: imat4

   A plain type labeled ``imat4x4`` of supertype `mat-type` and of storage type `(matrix i32 4 4)`.

.. type:: imat4x2

   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 2)`.

.. type:: imat4x3

   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 3)`.

.. type:: imat4x4

   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 4)`.

.. type:: ivec2

   A plain type of supertype `gvec2` and of storage type `(vector i32 2)`.

.. type:: ivec3

   A plain type of supertype `gvec3` and of storage type `(vector i32 3)`.

.. type:: ivec4

   A plain type of supertype `gvec4` and of storage type `(vector i32 4)`.

.. type:: mat-type

   An opaque type of supertype `immutable`.

   .. spice:: (__* ...)
   .. spice:: (__== ...)
   .. inline:: (__@ self index)
   .. spice:: (__as ...)
   .. spice:: (__r* ...)
   .. spice:: (__typecall ...)
   .. inline:: (__unpack self)
   .. spice:: (row ...)
.. type:: mat2

   A plain type labeled ``mat2x2`` of supertype `mat-type` and of storage type `(matrix f32 2 2)`.

.. type:: mat2x2

   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 2)`.

.. type:: mat2x3

   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 3)`.

.. type:: mat2x4

   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 4)`.

.. type:: mat3

   A plain type labeled ``mat3x3`` of supertype `mat-type` and of storage type `(matrix f32 3 3)`.

.. type:: mat3x2

   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 2)`.

.. type:: mat3x3

   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 3)`.

.. type:: mat3x4

   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 4)`.

.. type:: mat4

   A plain type labeled ``mat4x4`` of supertype `mat-type` and of storage type `(matrix f32 4 4)`.

.. type:: mat4x2

   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 2)`.

.. type:: mat4x3

   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 3)`.

.. type:: mat4x4

   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 4)`.

.. type:: umat2

   A plain type labeled ``umat2x2`` of supertype `mat-type` and of storage type `(matrix u32 2 2)`.

.. type:: umat2x2

   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 2)`.

.. type:: umat2x3

   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 3)`.

.. type:: umat2x4

   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 4)`.

.. type:: umat3

   A plain type labeled ``umat3x3`` of supertype `mat-type` and of storage type `(matrix u32 3 3)`.

.. type:: umat3x2

   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 2)`.

.. type:: umat3x3

   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 3)`.

.. type:: umat3x4

   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 4)`.

.. type:: umat4

   A plain type labeled ``umat4x4`` of supertype `mat-type` and of storage type `(matrix u32 4 4)`.

.. type:: umat4x2

   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 2)`.

.. type:: umat4x3

   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 3)`.

.. type:: umat4x4

   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 4)`.

.. type:: uvec2

   A plain type of supertype `gvec2` and of storage type `(vector u32 2)`.

.. type:: uvec3

   A plain type of supertype `gvec3` and of storage type `(vector u32 3)`.

.. type:: uvec4

   A plain type of supertype `gvec4` and of storage type `(vector u32 4)`.

.. type:: vec-type

   An opaque type of supertype `immutable`.

   .. spice:: (__% ...)
   .. spice:: (__& ...)
   .. spice:: (__* ...)
   .. spice:: (__** ...)
   .. spice:: (__+ ...)
   .. spice:: (__- ...)
   .. spice:: (__/ ...)
   .. spice:: (__// ...)
   .. spice:: (__< ...)
   .. spice:: (__<< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. spice:: (__>> ...)
   .. inline:: (__@ self i)
   .. spice:: (__^ ...)
   .. spice:: (__as ...)
   .. spice:: (__getattr ...)
   .. inline:: (__neg self)
   .. spice:: (__r% ...)
   .. spice:: (__r& ...)
   .. spice:: (__r* ...)
   .. spice:: (__r** ...)
   .. spice:: (__r+ ...)
   .. spice:: (__r- ...)
   .. spice:: (__r/ ...)
   .. spice:: (__r// ...)
   .. spice:: (__r< ...)
   .. spice:: (__r<< ...)
   .. spice:: (__r<= ...)
   .. spice:: (__r> ...)
   .. spice:: (__r>= ...)
   .. spice:: (__r>> ...)
   .. spice:: (__r^ ...)
   .. inline:: (__rcp self)
   .. spice:: (__rimply ...)
   .. spice:: (__r| ...)
   .. spice:: (__static-rimply ...)
   .. spice:: (__typecall ...)
   .. inline:: (__unpack self)
   .. spice:: (__| ...)
.. type:: vec2

   A plain type of supertype `gvec2` and of storage type `(vector f32 2)`.

.. type:: vec3

   A plain type of supertype `gvec3` and of storage type `(vector f32 3)`.

.. type:: vec4

   A plain type of supertype `gvec4` and of storage type `(vector f32 4)`.

.. inline:: (dot u v)
.. spice:: (mix ...)
