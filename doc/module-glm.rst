glm
===

The `glm` module exports the basic vector and matrix types as well as
related arithmetic operations which mimic the features available to shaders
written in the GL shader language.

.. type:: bmat2

   ``bmat2x2`` < ``mat-type`` : ``[bvec2 x 2]`` 
.. type:: bmat2x2

   ``bmat2x2`` < ``mat-type`` : ``[bvec2 x 2]`` 
.. type:: bmat2x3

   ``bmat2x3`` < ``mat-type`` : ``[bvec3 x 2]`` 
.. type:: bmat2x4

   ``bmat2x4`` < ``mat-type`` : ``[bvec4 x 2]`` 
.. type:: bmat3

   ``bmat3x3`` < ``mat-type`` : ``[bvec3 x 3]`` 
.. type:: bmat3x2

   ``bmat3x2`` < ``mat-type`` : ``[bvec2 x 3]`` 
.. type:: bmat3x3

   ``bmat3x3`` < ``mat-type`` : ``[bvec3 x 3]`` 
.. type:: bmat3x4

   ``bmat3x4`` < ``mat-type`` : ``[bvec4 x 3]`` 
.. type:: bmat4

   ``bmat4x4`` < ``mat-type`` : ``[bvec4 x 4]`` 
.. type:: bmat4x2

   ``bmat4x2`` < ``mat-type`` : ``[bvec2 x 4]`` 
.. type:: bmat4x3

   ``bmat4x3`` < ``mat-type`` : ``[bvec3 x 4]`` 
.. type:: bmat4x4

   ``bmat4x4`` < ``mat-type`` : ``[bvec4 x 4]`` 
.. type:: bvec2

   ``bvec2`` < ``vec-type`` : ``<bool x 2>`` 
.. type:: bvec3

   ``bvec3`` < ``vec-type`` : ``<bool x 3>`` 
.. type:: bvec4

   ``bvec4`` < ``vec-type`` : ``<bool x 4>`` 
.. type:: dmat2

   ``dmat2x2`` < ``mat-type`` : ``[dvec2 x 2]`` 
.. type:: dmat2x2

   ``dmat2x2`` < ``mat-type`` : ``[dvec2 x 2]`` 
.. type:: dmat2x3

   ``dmat2x3`` < ``mat-type`` : ``[dvec3 x 2]`` 
.. type:: dmat2x4

   ``dmat2x4`` < ``mat-type`` : ``[dvec4 x 2]`` 
.. type:: dmat3

   ``dmat3x3`` < ``mat-type`` : ``[dvec3 x 3]`` 
.. type:: dmat3x2

   ``dmat3x2`` < ``mat-type`` : ``[dvec2 x 3]`` 
.. type:: dmat3x3

   ``dmat3x3`` < ``mat-type`` : ``[dvec3 x 3]`` 
.. type:: dmat3x4

   ``dmat3x4`` < ``mat-type`` : ``[dvec4 x 3]`` 
.. type:: dmat4

   ``dmat4x4`` < ``mat-type`` : ``[dvec4 x 4]`` 
.. type:: dmat4x2

   ``dmat4x2`` < ``mat-type`` : ``[dvec2 x 4]`` 
.. type:: dmat4x3

   ``dmat4x3`` < ``mat-type`` : ``[dvec3 x 4]`` 
.. type:: dmat4x4

   ``dmat4x4`` < ``mat-type`` : ``[dvec4 x 4]`` 
.. type:: dvec2

   ``dvec2`` < ``vec-type`` : ``<f64 x 2>`` 
.. type:: dvec3

   ``dvec3`` < ``vec-type`` : ``<f64 x 3>`` 
.. type:: dvec4

   ``dvec4`` < ``vec-type`` : ``<f64 x 4>`` 
.. type:: imat2

   ``imat2x2`` < ``mat-type`` : ``[ivec2 x 2]`` 
.. type:: imat2x2

   ``imat2x2`` < ``mat-type`` : ``[ivec2 x 2]`` 
.. type:: imat2x3

   ``imat2x3`` < ``mat-type`` : ``[ivec3 x 2]`` 
.. type:: imat2x4

   ``imat2x4`` < ``mat-type`` : ``[ivec4 x 2]`` 
.. type:: imat3

   ``imat3x3`` < ``mat-type`` : ``[ivec3 x 3]`` 
.. type:: imat3x2

   ``imat3x2`` < ``mat-type`` : ``[ivec2 x 3]`` 
.. type:: imat3x3

   ``imat3x3`` < ``mat-type`` : ``[ivec3 x 3]`` 
.. type:: imat3x4

   ``imat3x4`` < ``mat-type`` : ``[ivec4 x 3]`` 
.. type:: imat4

   ``imat4x4`` < ``mat-type`` : ``[ivec4 x 4]`` 
.. type:: imat4x2

   ``imat4x2`` < ``mat-type`` : ``[ivec2 x 4]`` 
.. type:: imat4x3

   ``imat4x3`` < ``mat-type`` : ``[ivec3 x 4]`` 
.. type:: imat4x4

   ``imat4x4`` < ``mat-type`` : ``[ivec4 x 4]`` 
.. type:: ivec2

   ``ivec2`` < ``vec-type`` : ``<i32 x 2>`` 
.. type:: ivec3

   ``ivec3`` < ``vec-type`` : ``<i32 x 3>`` 
.. type:: ivec4

   ``ivec4`` < ``vec-type`` : ``<i32 x 4>`` 
.. type:: mat-type

   ``mat-type`` < ``immutable`` 
.. spice:: (mat-type.__typecall ...)
.. spice:: (mat-type.row ...)
.. type:: mat2

   ``mat2x2`` < ``mat-type`` : ``[vec2 x 2]`` 
.. type:: mat2x2

   ``mat2x2`` < ``mat-type`` : ``[vec2 x 2]`` 
.. type:: mat2x3

   ``mat2x3`` < ``mat-type`` : ``[vec3 x 2]`` 
.. type:: mat2x4

   ``mat2x4`` < ``mat-type`` : ``[vec4 x 2]`` 
.. type:: mat3

   ``mat3x3`` < ``mat-type`` : ``[vec3 x 3]`` 
.. type:: mat3x2

   ``mat3x2`` < ``mat-type`` : ``[vec2 x 3]`` 
.. type:: mat3x3

   ``mat3x3`` < ``mat-type`` : ``[vec3 x 3]`` 
.. type:: mat3x4

   ``mat3x4`` < ``mat-type`` : ``[vec4 x 3]`` 
.. type:: mat4

   ``mat4x4`` < ``mat-type`` : ``[vec4 x 4]`` 
.. type:: mat4x2

   ``mat4x2`` < ``mat-type`` : ``[vec2 x 4]`` 
.. type:: mat4x3

   ``mat4x3`` < ``mat-type`` : ``[vec3 x 4]`` 
.. type:: mat4x4

   ``mat4x4`` < ``mat-type`` : ``[vec4 x 4]`` 
.. type:: umat2

   ``umat2x2`` < ``mat-type`` : ``[uvec2 x 2]`` 
.. type:: umat2x2

   ``umat2x2`` < ``mat-type`` : ``[uvec2 x 2]`` 
.. type:: umat2x3

   ``umat2x3`` < ``mat-type`` : ``[uvec3 x 2]`` 
.. type:: umat2x4

   ``umat2x4`` < ``mat-type`` : ``[uvec4 x 2]`` 
.. type:: umat3

   ``umat3x3`` < ``mat-type`` : ``[uvec3 x 3]`` 
.. type:: umat3x2

   ``umat3x2`` < ``mat-type`` : ``[uvec2 x 3]`` 
.. type:: umat3x3

   ``umat3x3`` < ``mat-type`` : ``[uvec3 x 3]`` 
.. type:: umat3x4

   ``umat3x4`` < ``mat-type`` : ``[uvec4 x 3]`` 
.. type:: umat4

   ``umat4x4`` < ``mat-type`` : ``[uvec4 x 4]`` 
.. type:: umat4x2

   ``umat4x2`` < ``mat-type`` : ``[uvec2 x 4]`` 
.. type:: umat4x3

   ``umat4x3`` < ``mat-type`` : ``[uvec3 x 4]`` 
.. type:: umat4x4

   ``umat4x4`` < ``mat-type`` : ``[uvec4 x 4]`` 
.. type:: uvec2

   ``uvec2`` < ``vec-type`` : ``<u32 x 2>`` 
.. type:: uvec3

   ``uvec3`` < ``vec-type`` : ``<u32 x 3>`` 
.. type:: uvec4

   ``uvec4`` < ``vec-type`` : ``<u32 x 4>`` 
.. type:: vec-type

   ``vec-type`` < ``immutable`` 
.. spice:: (vec-type.__typecall ...)
.. type:: vec2

   ``vec2`` < ``vec-type`` : ``<f32 x 2>`` 
.. type:: vec3

   ``vec3`` < ``vec-type`` : ``<f32 x 3>`` 
.. type:: vec4

   ``vec4`` < ``vec-type`` : ``<f32 x 4>`` 
.. fn:: (dot u v)
.. spice:: (mix ...)
