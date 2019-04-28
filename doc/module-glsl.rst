glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

.. type:: gsampler

   An opaque type.

.. type:: gsampler1D

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler1DArray

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler2D

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler2DArray

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler2DMS

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler2DMSArray

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler2DRect

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsampler3D

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsamplerBuffer

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsamplerCube

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: gsamplerCubeArray

   An opaque type of supertype `gsampler`.

   .. builtin:: (texture-levels ...)
   .. builtin:: (texture-samples ...)
.. type:: isampler1D

   A plain type of supertype `gsampler1D$3` and of storage type `<SampledImage <Image ivec4 '1D sampled 'Unknown>>`.

.. type:: isampler1DArray

   A plain type of supertype `gsampler1DArray$3` and of storage type `<SampledImage <Image ivec4 '1D array sampled 'Unknown>>`.

.. type:: isampler2D

   A plain type of supertype `gsampler2D$3` and of storage type `<SampledImage <Image ivec4 '2D sampled 'Unknown>>`.

.. type:: isampler2DArray

   A plain type of supertype `gsampler2DArray$3` and of storage type `<SampledImage <Image ivec4 '2D array sampled 'Unknown>>`.

.. type:: isampler2DMS

   A plain type of supertype `gsampler2DMS$3` and of storage type `<SampledImage <Image ivec4 '2D ms sampled 'Unknown>>`.

.. type:: isampler2DMSArray

   A plain type of supertype `gsampler2DMSArray$3` and of storage type `<SampledImage <Image ivec4 '2D array ms sampled 'Unknown>>`.

.. type:: isampler2DRect

   A plain type of supertype `gsampler2DRect$3` and of storage type `<SampledImage <Image ivec4 'Rect sampled 'Unknown>>`.

.. type:: isampler3D

   A plain type of supertype `gsampler3D$3` and of storage type `<SampledImage <Image ivec4 '3D sampled 'Unknown>>`.

.. type:: isamplerBuffer

   A plain type of supertype `gsamplerBuffer$3` and of storage type `<SampledImage <Image ivec4 'Buffer sampled 'Unknown>>`.

.. type:: isamplerCube

   A plain type of supertype `gsamplerCube$3` and of storage type `<SampledImage <Image ivec4 'Cube sampled 'Unknown>>`.

.. type:: isamplerCubeArray

   A plain type of supertype `gsamplerCubeArray$3` and of storage type `<SampledImage <Image ivec4 'Cube array sampled 'Unknown>>`.

.. type:: sampler1D

   A plain type of supertype `gsampler1D$2` and of storage type `<SampledImage <Image vec4 '1D sampled 'Unknown>>`.

.. type:: sampler1DArray

   A plain type of supertype `gsampler1DArray$2` and of storage type `<SampledImage <Image vec4 '1D array sampled 'Unknown>>`.

.. type:: sampler2D

   A plain type of supertype `gsampler2D$2` and of storage type `<SampledImage <Image vec4 '2D sampled 'Unknown>>`.

.. type:: sampler2DArray

   A plain type of supertype `gsampler2DArray$2` and of storage type `<SampledImage <Image vec4 '2D array sampled 'Unknown>>`.

.. type:: sampler2DMS

   A plain type of supertype `gsampler2DMS$2` and of storage type `<SampledImage <Image vec4 '2D ms sampled 'Unknown>>`.

.. type:: sampler2DMSArray

   A plain type of supertype `gsampler2DMSArray$2` and of storage type `<SampledImage <Image vec4 '2D array ms sampled 'Unknown>>`.

.. type:: sampler2DRect

   A plain type of supertype `gsampler2DRect$2` and of storage type `<SampledImage <Image vec4 'Rect sampled 'Unknown>>`.

.. type:: sampler3D

   A plain type of supertype `gsampler3D$2` and of storage type `<SampledImage <Image vec4 '3D sampled 'Unknown>>`.

.. type:: samplerBuffer

   A plain type of supertype `gsamplerBuffer$2` and of storage type `<SampledImage <Image vec4 'Buffer sampled 'Unknown>>`.

.. type:: samplerCube

   A plain type of supertype `gsamplerCube$2` and of storage type `<SampledImage <Image vec4 'Cube sampled 'Unknown>>`.

.. type:: samplerCubeArray

   A plain type of supertype `gsamplerCubeArray$2` and of storage type `<SampledImage <Image vec4 'Cube array sampled 'Unknown>>`.

.. type:: usampler1D

   A plain type of supertype `gsampler1D$4` and of storage type `<SampledImage <Image uvec4 '1D sampled 'Unknown>>`.

.. type:: usampler1DArray

   A plain type of supertype `gsampler1DArray$4` and of storage type `<SampledImage <Image uvec4 '1D array sampled 'Unknown>>`.

.. type:: usampler2D

   A plain type of supertype `gsampler2D$4` and of storage type `<SampledImage <Image uvec4 '2D sampled 'Unknown>>`.

.. type:: usampler2DArray

   A plain type of supertype `gsampler2DArray$4` and of storage type `<SampledImage <Image uvec4 '2D array sampled 'Unknown>>`.

.. type:: usampler2DMS

   A plain type of supertype `gsampler2DMS$4` and of storage type `<SampledImage <Image uvec4 '2D ms sampled 'Unknown>>`.

.. type:: usampler2DMSArray

   A plain type of supertype `gsampler2DMSArray$4` and of storage type `<SampledImage <Image uvec4 '2D array ms sampled 'Unknown>>`.

.. type:: usampler2DRect

   A plain type of supertype `gsampler2DRect$4` and of storage type `<SampledImage <Image uvec4 'Rect sampled 'Unknown>>`.

.. type:: usampler3D

   A plain type of supertype `gsampler3D$4` and of storage type `<SampledImage <Image uvec4 '3D sampled 'Unknown>>`.

.. type:: usamplerBuffer

   A plain type of supertype `gsamplerBuffer$4` and of storage type `<SampledImage <Image uvec4 'Buffer sampled 'Unknown>>`.

.. type:: usamplerCube

   A plain type of supertype `gsamplerCube$4` and of storage type `<SampledImage <Image uvec4 'Cube sampled 'Unknown>>`.

.. type:: usamplerCubeArray

   A plain type of supertype `gsamplerCubeArray$4` and of storage type `<SampledImage <Image uvec4 'Cube array sampled 'Unknown>>`.

.. inline:: (iimage1D format)
.. inline:: (iimage1DArray format)
.. inline:: (iimage2D format)
.. inline:: (iimage2DArray format)
.. inline:: (iimage2DMS format)
.. inline:: (iimage2DMSArray format)
.. inline:: (iimage2DRect format)
.. inline:: (iimage3D format)
.. inline:: (iimageBuffer format)
.. inline:: (iimageCube format)
.. inline:: (iimageCubeArray format)
.. inline:: (image1D format)
.. inline:: (image1DArray format)
.. inline:: (image2D format)
.. inline:: (image2DArray format)
.. inline:: (image2DMS format)
.. inline:: (image2DMSArray format)
.. inline:: (image2DRect format)
.. inline:: (image3D format)
.. inline:: (imageBuffer format)
.. inline:: (imageCube format)
.. inline:: (imageCubeArray format)
.. inline:: (imageLoad image coord)
.. inline:: (imageStore image coord data)
.. inline:: (local_size x y z)
.. inline:: (texelFetch sampler P ...)
.. inline:: (texelFetchOffset sampler P lod offset)
.. inline:: (texture sampler P ...)
.. inline:: (textureGather sampler P ...)
.. inline:: (textureLod sampler P lod)
.. inline:: (textureOffset sampler P offset ...)
.. inline:: (textureProj sampler P ...)
.. inline:: (textureQueryLevels sampler)
.. inline:: (textureQueryLod sampler P)
.. inline:: (textureSamples sampler)
.. inline:: (textureSize sampler ...)
.. inline:: (uimage1D format)
.. inline:: (uimage1DArray format)
.. inline:: (uimage2D format)
.. inline:: (uimage2DArray format)
.. inline:: (uimage2DMS format)
.. inline:: (uimage2DMSArray format)
.. inline:: (uimage2DRect format)
.. inline:: (uimage3D format)
.. inline:: (uimageBuffer format)
.. inline:: (uimageCube format)
.. inline:: (uimageCubeArray format)
.. sugar:: (buffer ...)
.. sugar:: (in ...)
.. sugar:: (inout ...)
.. sugar:: (out ...)
.. sugar:: (uniform ...)
.. compiledfn:: (packHalf2x16 ...)

   An external function of type ``u32<-(vec2)``.
.. compiledfn:: (unpackHalf2x16 ...)

   An external function of type ``vec2<-(u32)``.
