glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

.. type:: gsampler

   An opaque type.
.. type:: gsampler1D

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler1D.texture-samples ...)
.. builtin:: (gsampler1D.texture-levels ...)
.. type:: gsampler1DArray

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler1DArray.texture-samples ...)
.. builtin:: (gsampler1DArray.texture-levels ...)
.. type:: gsampler2D

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler2D.texture-samples ...)
.. builtin:: (gsampler2D.texture-levels ...)
.. type:: gsampler2DArray

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler2DArray.texture-samples ...)
.. builtin:: (gsampler2DArray.texture-levels ...)
.. type:: gsampler2DMS

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler2DMS.texture-samples ...)
.. builtin:: (gsampler2DMS.texture-levels ...)
.. type:: gsampler2DMSArray

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler2DMSArray.texture-samples ...)
.. builtin:: (gsampler2DMSArray.texture-levels ...)
.. type:: gsampler2DRect

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler2DRect.texture-samples ...)
.. builtin:: (gsampler2DRect.texture-levels ...)
.. type:: gsampler3D

   An opaque type of supertype `gsampler`.
.. builtin:: (gsampler3D.texture-samples ...)
.. builtin:: (gsampler3D.texture-levels ...)
.. type:: gsamplerBuffer

   An opaque type of supertype `gsampler`.
.. builtin:: (gsamplerBuffer.texture-samples ...)
.. builtin:: (gsamplerBuffer.texture-levels ...)
.. type:: gsamplerCube

   An opaque type of supertype `gsampler`.
.. builtin:: (gsamplerCube.texture-samples ...)
.. builtin:: (gsamplerCube.texture-levels ...)
.. type:: gsamplerCubeArray

   An opaque type of supertype `gsampler`.
.. builtin:: (gsamplerCubeArray.texture-samples ...)
.. builtin:: (gsamplerCubeArray.texture-levels ...)
.. type:: isampler1D

   An opaque type of supertype `gsampler1D$3`.
.. type:: isampler1DArray

   An opaque type of supertype `gsampler1DArray$3`.
.. type:: isampler2D

   An opaque type of supertype `gsampler2D$3`.
.. type:: isampler2DArray

   An opaque type of supertype `gsampler2DArray$3`.
.. type:: isampler2DMS

   An opaque type of supertype `gsampler2DMS$3`.
.. type:: isampler2DMSArray

   An opaque type of supertype `gsampler2DMSArray$3`.
.. type:: isampler2DRect

   An opaque type of supertype `gsampler2DRect$3`.
.. type:: isampler3D

   An opaque type of supertype `gsampler3D$3`.
.. type:: isamplerBuffer

   An opaque type of supertype `gsamplerBuffer$3`.
.. type:: isamplerCube

   An opaque type of supertype `gsamplerCube$3`.
.. type:: isamplerCubeArray

   An opaque type of supertype `gsamplerCubeArray$3`.
.. type:: sampler1D

   An opaque type of supertype `gsampler1D$2`.
.. type:: sampler1DArray

   An opaque type of supertype `gsampler1DArray$2`.
.. type:: sampler2D

   An opaque type of supertype `gsampler2D$2`.
.. type:: sampler2DArray

   An opaque type of supertype `gsampler2DArray$2`.
.. type:: sampler2DMS

   An opaque type of supertype `gsampler2DMS$2`.
.. type:: sampler2DMSArray

   An opaque type of supertype `gsampler2DMSArray$2`.
.. type:: sampler2DRect

   An opaque type of supertype `gsampler2DRect$2`.
.. type:: sampler3D

   An opaque type of supertype `gsampler3D$2`.
.. type:: samplerBuffer

   An opaque type of supertype `gsamplerBuffer$2`.
.. type:: samplerCube

   An opaque type of supertype `gsamplerCube$2`.
.. type:: samplerCubeArray

   An opaque type of supertype `gsamplerCubeArray$2`.
.. type:: usampler1D

   An opaque type of supertype `gsampler1D$4`.
.. type:: usampler1DArray

   An opaque type of supertype `gsampler1DArray$4`.
.. type:: usampler2D

   An opaque type of supertype `gsampler2D$4`.
.. type:: usampler2DArray

   An opaque type of supertype `gsampler2DArray$4`.
.. type:: usampler2DMS

   An opaque type of supertype `gsampler2DMS$4`.
.. type:: usampler2DMSArray

   An opaque type of supertype `gsampler2DMSArray$4`.
.. type:: usampler2DRect

   An opaque type of supertype `gsampler2DRect$4`.
.. type:: usampler3D

   An opaque type of supertype `gsampler3D$4`.
.. type:: usamplerBuffer

   An opaque type of supertype `gsamplerBuffer$4`.
.. type:: usamplerCube

   An opaque type of supertype `gsamplerCube$4`.
.. type:: usamplerCubeArray

   An opaque type of supertype `gsamplerCubeArray$4`.
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
