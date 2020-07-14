<style type="text/css" rel="stylesheet">body { counter-reset: chapter 13; }</style>

glm
===

The `glm` module exports the basic vector and matrix types as well as
related arithmetic operations which mimic the features available to shaders
written in the GL shader language.

*type*{.property} `bmat2`{.descname} [](#scopes.type.bmat2 "Permalink to this definition"){.headerlink} {#scopes.type.bmat2}

:   A plain type labeled `bmat2x2` of supertype `mat-type` and of storage type `(matrix bool 2 2)`.

*type*{.property} `bmat2x2`{.descname} [](#scopes.type.bmat2x2 "Permalink to this definition"){.headerlink} {#scopes.type.bmat2x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 2)`.

*type*{.property} `bmat2x3`{.descname} [](#scopes.type.bmat2x3 "Permalink to this definition"){.headerlink} {#scopes.type.bmat2x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 3)`.

*type*{.property} `bmat2x4`{.descname} [](#scopes.type.bmat2x4 "Permalink to this definition"){.headerlink} {#scopes.type.bmat2x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 2 4)`.

*type*{.property} `bmat3`{.descname} [](#scopes.type.bmat3 "Permalink to this definition"){.headerlink} {#scopes.type.bmat3}

:   A plain type labeled `bmat3x3` of supertype `mat-type` and of storage type `(matrix bool 3 3)`.

*type*{.property} `bmat3x2`{.descname} [](#scopes.type.bmat3x2 "Permalink to this definition"){.headerlink} {#scopes.type.bmat3x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 2)`.

*type*{.property} `bmat3x3`{.descname} [](#scopes.type.bmat3x3 "Permalink to this definition"){.headerlink} {#scopes.type.bmat3x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 3)`.

*type*{.property} `bmat3x4`{.descname} [](#scopes.type.bmat3x4 "Permalink to this definition"){.headerlink} {#scopes.type.bmat3x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 3 4)`.

*type*{.property} `bmat4`{.descname} [](#scopes.type.bmat4 "Permalink to this definition"){.headerlink} {#scopes.type.bmat4}

:   A plain type labeled `bmat4x4` of supertype `mat-type` and of storage type `(matrix bool 4 4)`.

*type*{.property} `bmat4x2`{.descname} [](#scopes.type.bmat4x2 "Permalink to this definition"){.headerlink} {#scopes.type.bmat4x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 2)`.

*type*{.property} `bmat4x3`{.descname} [](#scopes.type.bmat4x3 "Permalink to this definition"){.headerlink} {#scopes.type.bmat4x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 3)`.

*type*{.property} `bmat4x4`{.descname} [](#scopes.type.bmat4x4 "Permalink to this definition"){.headerlink} {#scopes.type.bmat4x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix bool 4 4)`.

*type*{.property} `bvec2`{.descname} [](#scopes.type.bvec2 "Permalink to this definition"){.headerlink} {#scopes.type.bvec2}

:   A plain type of supertype `gvec2` and of storage type `(vector bool 2)`.

*type*{.property} `bvec3`{.descname} [](#scopes.type.bvec3 "Permalink to this definition"){.headerlink} {#scopes.type.bvec3}

:   A plain type of supertype `gvec3` and of storage type `(vector bool 3)`.

*type*{.property} `bvec4`{.descname} [](#scopes.type.bvec4 "Permalink to this definition"){.headerlink} {#scopes.type.bvec4}

:   A plain type of supertype `gvec4` and of storage type `(vector bool 4)`.

*type*{.property} `dmat2`{.descname} [](#scopes.type.dmat2 "Permalink to this definition"){.headerlink} {#scopes.type.dmat2}

:   A plain type labeled `dmat2x2` of supertype `mat-type` and of storage type `(matrix f64 2 2)`.

*type*{.property} `dmat2x2`{.descname} [](#scopes.type.dmat2x2 "Permalink to this definition"){.headerlink} {#scopes.type.dmat2x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 2)`.

*type*{.property} `dmat2x3`{.descname} [](#scopes.type.dmat2x3 "Permalink to this definition"){.headerlink} {#scopes.type.dmat2x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 3)`.

*type*{.property} `dmat2x4`{.descname} [](#scopes.type.dmat2x4 "Permalink to this definition"){.headerlink} {#scopes.type.dmat2x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 2 4)`.

*type*{.property} `dmat3`{.descname} [](#scopes.type.dmat3 "Permalink to this definition"){.headerlink} {#scopes.type.dmat3}

:   A plain type labeled `dmat3x3` of supertype `mat-type` and of storage type `(matrix f64 3 3)`.

*type*{.property} `dmat3x2`{.descname} [](#scopes.type.dmat3x2 "Permalink to this definition"){.headerlink} {#scopes.type.dmat3x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 2)`.

*type*{.property} `dmat3x3`{.descname} [](#scopes.type.dmat3x3 "Permalink to this definition"){.headerlink} {#scopes.type.dmat3x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 3)`.

*type*{.property} `dmat3x4`{.descname} [](#scopes.type.dmat3x4 "Permalink to this definition"){.headerlink} {#scopes.type.dmat3x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 3 4)`.

*type*{.property} `dmat4`{.descname} [](#scopes.type.dmat4 "Permalink to this definition"){.headerlink} {#scopes.type.dmat4}

:   A plain type labeled `dmat4x4` of supertype `mat-type` and of storage type `(matrix f64 4 4)`.

*type*{.property} `dmat4x2`{.descname} [](#scopes.type.dmat4x2 "Permalink to this definition"){.headerlink} {#scopes.type.dmat4x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 2)`.

*type*{.property} `dmat4x3`{.descname} [](#scopes.type.dmat4x3 "Permalink to this definition"){.headerlink} {#scopes.type.dmat4x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 3)`.

*type*{.property} `dmat4x4`{.descname} [](#scopes.type.dmat4x4 "Permalink to this definition"){.headerlink} {#scopes.type.dmat4x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f64 4 4)`.

*type*{.property} `dvec2`{.descname} [](#scopes.type.dvec2 "Permalink to this definition"){.headerlink} {#scopes.type.dvec2}

:   A plain type of supertype `gvec2` and of storage type `(vector f64 2)`.

*type*{.property} `dvec3`{.descname} [](#scopes.type.dvec3 "Permalink to this definition"){.headerlink} {#scopes.type.dvec3}

:   A plain type of supertype `gvec3` and of storage type `(vector f64 3)`.

*type*{.property} `dvec4`{.descname} [](#scopes.type.dvec4 "Permalink to this definition"){.headerlink} {#scopes.type.dvec4}

:   A plain type of supertype `gvec4` and of storage type `(vector f64 4)`.

*type*{.property} `gvec2`{.descname} [](#scopes.type.gvec2 "Permalink to this definition"){.headerlink} {#scopes.type.gvec2}

:   An opaque type of supertype `vec-type`.

*type*{.property} `gvec3`{.descname} [](#scopes.type.gvec3 "Permalink to this definition"){.headerlink} {#scopes.type.gvec3}

:   An opaque type of supertype `vec-type`.

*type*{.property} `gvec4`{.descname} [](#scopes.type.gvec4 "Permalink to this definition"){.headerlink} {#scopes.type.gvec4}

:   An opaque type of supertype `vec-type`.

*type*{.property} `imat2`{.descname} [](#scopes.type.imat2 "Permalink to this definition"){.headerlink} {#scopes.type.imat2}

:   A plain type labeled `imat2x2` of supertype `mat-type` and of storage type `(matrix i32 2 2)`.

*type*{.property} `imat2x2`{.descname} [](#scopes.type.imat2x2 "Permalink to this definition"){.headerlink} {#scopes.type.imat2x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 2)`.

*type*{.property} `imat2x3`{.descname} [](#scopes.type.imat2x3 "Permalink to this definition"){.headerlink} {#scopes.type.imat2x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 3)`.

*type*{.property} `imat2x4`{.descname} [](#scopes.type.imat2x4 "Permalink to this definition"){.headerlink} {#scopes.type.imat2x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 2 4)`.

*type*{.property} `imat3`{.descname} [](#scopes.type.imat3 "Permalink to this definition"){.headerlink} {#scopes.type.imat3}

:   A plain type labeled `imat3x3` of supertype `mat-type` and of storage type `(matrix i32 3 3)`.

*type*{.property} `imat3x2`{.descname} [](#scopes.type.imat3x2 "Permalink to this definition"){.headerlink} {#scopes.type.imat3x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 2)`.

*type*{.property} `imat3x3`{.descname} [](#scopes.type.imat3x3 "Permalink to this definition"){.headerlink} {#scopes.type.imat3x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 3)`.

*type*{.property} `imat3x4`{.descname} [](#scopes.type.imat3x4 "Permalink to this definition"){.headerlink} {#scopes.type.imat3x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 3 4)`.

*type*{.property} `imat4`{.descname} [](#scopes.type.imat4 "Permalink to this definition"){.headerlink} {#scopes.type.imat4}

:   A plain type labeled `imat4x4` of supertype `mat-type` and of storage type `(matrix i32 4 4)`.

*type*{.property} `imat4x2`{.descname} [](#scopes.type.imat4x2 "Permalink to this definition"){.headerlink} {#scopes.type.imat4x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 2)`.

*type*{.property} `imat4x3`{.descname} [](#scopes.type.imat4x3 "Permalink to this definition"){.headerlink} {#scopes.type.imat4x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 3)`.

*type*{.property} `imat4x4`{.descname} [](#scopes.type.imat4x4 "Permalink to this definition"){.headerlink} {#scopes.type.imat4x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix i32 4 4)`.

*type*{.property} `ivec2`{.descname} [](#scopes.type.ivec2 "Permalink to this definition"){.headerlink} {#scopes.type.ivec2}

:   A plain type of supertype `gvec2` and of storage type `(vector i32 2)`.

*type*{.property} `ivec3`{.descname} [](#scopes.type.ivec3 "Permalink to this definition"){.headerlink} {#scopes.type.ivec3}

:   A plain type of supertype `gvec3` and of storage type `(vector i32 3)`.

*type*{.property} `ivec4`{.descname} [](#scopes.type.ivec4 "Permalink to this definition"){.headerlink} {#scopes.type.ivec4}

:   A plain type of supertype `gvec4` and of storage type `(vector i32 4)`.

*type*{.property} `mat-type`{.descname} [](#scopes.type.mat-type "Permalink to this definition"){.headerlink} {#scopes.type.mat-type}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.__* "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.__*}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.__== "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.__==}

    :   

    *inline*{.property} `__@`{.descname} (*&ensp;self index&ensp;*)[](#scopes.mat-type.inline.__@ "Permalink to this definition"){.headerlink} {#scopes.mat-type.inline.__@}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.__as "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.__as}

    :   

    *spice*{.property} `__r*`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.__r* "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.__r*}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.__typecall}

    :   

    *inline*{.property} `__unpack`{.descname} (*&ensp;self&ensp;*)[](#scopes.mat-type.inline.__unpack "Permalink to this definition"){.headerlink} {#scopes.mat-type.inline.__unpack}

    :   

    *spice*{.property} `row`{.descname} (*&ensp;...&ensp;*)[](#scopes.mat-type.spice.row "Permalink to this definition"){.headerlink} {#scopes.mat-type.spice.row}

    :   

*type*{.property} `mat2`{.descname} [](#scopes.type.mat2 "Permalink to this definition"){.headerlink} {#scopes.type.mat2}

:   A plain type labeled `mat2x2` of supertype `mat-type` and of storage type `(matrix f32 2 2)`.

*type*{.property} `mat2x2`{.descname} [](#scopes.type.mat2x2 "Permalink to this definition"){.headerlink} {#scopes.type.mat2x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 2)`.

*type*{.property} `mat2x3`{.descname} [](#scopes.type.mat2x3 "Permalink to this definition"){.headerlink} {#scopes.type.mat2x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 3)`.

*type*{.property} `mat2x4`{.descname} [](#scopes.type.mat2x4 "Permalink to this definition"){.headerlink} {#scopes.type.mat2x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 2 4)`.

*type*{.property} `mat3`{.descname} [](#scopes.type.mat3 "Permalink to this definition"){.headerlink} {#scopes.type.mat3}

:   A plain type labeled `mat3x3` of supertype `mat-type` and of storage type `(matrix f32 3 3)`.

*type*{.property} `mat3x2`{.descname} [](#scopes.type.mat3x2 "Permalink to this definition"){.headerlink} {#scopes.type.mat3x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 2)`.

*type*{.property} `mat3x3`{.descname} [](#scopes.type.mat3x3 "Permalink to this definition"){.headerlink} {#scopes.type.mat3x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 3)`.

*type*{.property} `mat3x4`{.descname} [](#scopes.type.mat3x4 "Permalink to this definition"){.headerlink} {#scopes.type.mat3x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 3 4)`.

*type*{.property} `mat4`{.descname} [](#scopes.type.mat4 "Permalink to this definition"){.headerlink} {#scopes.type.mat4}

:   A plain type labeled `mat4x4` of supertype `mat-type` and of storage type `(matrix f32 4 4)`.

*type*{.property} `mat4x2`{.descname} [](#scopes.type.mat4x2 "Permalink to this definition"){.headerlink} {#scopes.type.mat4x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 2)`.

*type*{.property} `mat4x3`{.descname} [](#scopes.type.mat4x3 "Permalink to this definition"){.headerlink} {#scopes.type.mat4x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 3)`.

*type*{.property} `mat4x4`{.descname} [](#scopes.type.mat4x4 "Permalink to this definition"){.headerlink} {#scopes.type.mat4x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix f32 4 4)`.

*type*{.property} `umat2`{.descname} [](#scopes.type.umat2 "Permalink to this definition"){.headerlink} {#scopes.type.umat2}

:   A plain type labeled `umat2x2` of supertype `mat-type` and of storage type `(matrix u32 2 2)`.

*type*{.property} `umat2x2`{.descname} [](#scopes.type.umat2x2 "Permalink to this definition"){.headerlink} {#scopes.type.umat2x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 2)`.

*type*{.property} `umat2x3`{.descname} [](#scopes.type.umat2x3 "Permalink to this definition"){.headerlink} {#scopes.type.umat2x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 3)`.

*type*{.property} `umat2x4`{.descname} [](#scopes.type.umat2x4 "Permalink to this definition"){.headerlink} {#scopes.type.umat2x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 2 4)`.

*type*{.property} `umat3`{.descname} [](#scopes.type.umat3 "Permalink to this definition"){.headerlink} {#scopes.type.umat3}

:   A plain type labeled `umat3x3` of supertype `mat-type` and of storage type `(matrix u32 3 3)`.

*type*{.property} `umat3x2`{.descname} [](#scopes.type.umat3x2 "Permalink to this definition"){.headerlink} {#scopes.type.umat3x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 2)`.

*type*{.property} `umat3x3`{.descname} [](#scopes.type.umat3x3 "Permalink to this definition"){.headerlink} {#scopes.type.umat3x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 3)`.

*type*{.property} `umat3x4`{.descname} [](#scopes.type.umat3x4 "Permalink to this definition"){.headerlink} {#scopes.type.umat3x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 3 4)`.

*type*{.property} `umat4`{.descname} [](#scopes.type.umat4 "Permalink to this definition"){.headerlink} {#scopes.type.umat4}

:   A plain type labeled `umat4x4` of supertype `mat-type` and of storage type `(matrix u32 4 4)`.

*type*{.property} `umat4x2`{.descname} [](#scopes.type.umat4x2 "Permalink to this definition"){.headerlink} {#scopes.type.umat4x2}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 2)`.

*type*{.property} `umat4x3`{.descname} [](#scopes.type.umat4x3 "Permalink to this definition"){.headerlink} {#scopes.type.umat4x3}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 3)`.

*type*{.property} `umat4x4`{.descname} [](#scopes.type.umat4x4 "Permalink to this definition"){.headerlink} {#scopes.type.umat4x4}

:   A plain type of supertype `mat-type` and of storage type `(matrix u32 4 4)`.

*type*{.property} `uvec2`{.descname} [](#scopes.type.uvec2 "Permalink to this definition"){.headerlink} {#scopes.type.uvec2}

:   A plain type of supertype `gvec2` and of storage type `(vector u32 2)`.

*type*{.property} `uvec3`{.descname} [](#scopes.type.uvec3 "Permalink to this definition"){.headerlink} {#scopes.type.uvec3}

:   A plain type of supertype `gvec3` and of storage type `(vector u32 3)`.

*type*{.property} `uvec4`{.descname} [](#scopes.type.uvec4 "Permalink to this definition"){.headerlink} {#scopes.type.uvec4}

:   A plain type of supertype `gvec4` and of storage type `(vector u32 4)`.

*type*{.property} `vec-type`{.descname} [](#scopes.type.vec-type "Permalink to this definition"){.headerlink} {#scopes.type.vec-type}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__%`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__% "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__%}

    :   

    *spice*{.property} `__&`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__& "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__&}

    :   

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__* "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__*}

    :   

    *spice*{.property} `__**`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__** "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__**}

    :   

    *spice*{.property} `__+`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__+ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__+}

    :   

    *spice*{.property} `__-`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__- "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__-}

    :   

    *spice*{.property} `__/`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__/ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__/}

    :   

    *spice*{.property} `__//`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__// "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__//}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__< "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__<}

    :   

    *spice*{.property} `__<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__<< "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__<<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__== "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__> "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__>=}

    :   

    *spice*{.property} `__>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__>> "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__>>}

    :   

    *inline*{.property} `__@`{.descname} (*&ensp;self i&ensp;*)[](#scopes.vec-type.inline.__@ "Permalink to this definition"){.headerlink} {#scopes.vec-type.inline.__@}

    :   

    *spice*{.property} `__^`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__^ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__^}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__as "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__as}

    :   

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__getattr}

    :   

    *inline*{.property} `__neg`{.descname} (*&ensp;self&ensp;*)[](#scopes.vec-type.inline.__neg "Permalink to this definition"){.headerlink} {#scopes.vec-type.inline.__neg}

    :   

    *spice*{.property} `__r%`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r% "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r%}

    :   

    *spice*{.property} `__r&`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r& "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r&}

    :   

    *spice*{.property} `__r*`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r* "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r*}

    :   

    *spice*{.property} `__r**`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r** "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r**}

    :   

    *spice*{.property} `__r+`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r+ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r+}

    :   

    *spice*{.property} `__r-`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r- "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r-}

    :   

    *spice*{.property} `__r/`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r/ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r/}

    :   

    *spice*{.property} `__r//`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r// "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r//}

    :   

    *spice*{.property} `__r<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r< "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r<}

    :   

    *spice*{.property} `__r<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r<< "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r<<}

    :   

    *spice*{.property} `__r<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r<= "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r<=}

    :   

    *spice*{.property} `__r>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r> "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r>}

    :   

    *spice*{.property} `__r>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r>= "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r>=}

    :   

    *spice*{.property} `__r>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r>> "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r>>}

    :   

    *spice*{.property} `__r^`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r^ "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r^}

    :   

    *inline*{.property} `__rcp`{.descname} (*&ensp;self&ensp;*)[](#scopes.vec-type.inline.__rcp "Permalink to this definition"){.headerlink} {#scopes.vec-type.inline.__rcp}

    :   

    *spice*{.property} `__rimply`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__rimply "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__rimply}

    :   

    *spice*{.property} `__r|`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__r| "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__r|}

    :   

    *spice*{.property} `__static-rimply`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__static-rimply "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__static-rimply}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__typecall}

    :   

    *inline*{.property} `__unpack`{.descname} (*&ensp;self&ensp;*)[](#scopes.vec-type.inline.__unpack "Permalink to this definition"){.headerlink} {#scopes.vec-type.inline.__unpack}

    :   

    *spice*{.property} `__|`{.descname} (*&ensp;...&ensp;*)[](#scopes.vec-type.spice.__| "Permalink to this definition"){.headerlink} {#scopes.vec-type.spice.__|}

    :   

*type*{.property} `vec2`{.descname} [](#scopes.type.vec2 "Permalink to this definition"){.headerlink} {#scopes.type.vec2}

:   A plain type of supertype `gvec2` and of storage type `(vector f32 2)`.

*type*{.property} `vec3`{.descname} [](#scopes.type.vec3 "Permalink to this definition"){.headerlink} {#scopes.type.vec3}

:   A plain type of supertype `gvec3` and of storage type `(vector f32 3)`.

*type*{.property} `vec4`{.descname} [](#scopes.type.vec4 "Permalink to this definition"){.headerlink} {#scopes.type.vec4}

:   A plain type of supertype `gvec4` and of storage type `(vector f32 4)`.

*inline*{.property} `dot`{.descname} (*&ensp;u v&ensp;*)[](#scopes.inline.dot "Permalink to this definition"){.headerlink} {#scopes.inline.dot}

:   

*spice*{.property} `mix`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.mix "Permalink to this definition"){.headerlink} {#scopes.spice.mix}

:   

