glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

.. type:: gsampler

   ``gsampler`` 
.. type:: gsampler1D

   ``gsampler1D`` < ``gsampler`` 
.. builtin:: (gsampler1D.texture-samples ...)
.. builtin:: (gsampler1D.texture-levels ...)
.. type:: gsampler1DArray

   ``gsampler1DArray`` < ``gsampler`` 
.. builtin:: (gsampler1DArray.texture-samples ...)
.. builtin:: (gsampler1DArray.texture-levels ...)
.. type:: gsampler2D

   ``gsampler2D`` < ``gsampler`` 
.. builtin:: (gsampler2D.texture-samples ...)
.. builtin:: (gsampler2D.texture-levels ...)
.. type:: gsampler2DArray

   ``gsampler2DArray`` < ``gsampler`` 
.. builtin:: (gsampler2DArray.texture-samples ...)
.. builtin:: (gsampler2DArray.texture-levels ...)
.. type:: gsampler2DMS

   ``gsampler2DMS`` < ``gsampler`` 
.. builtin:: (gsampler2DMS.texture-samples ...)
.. builtin:: (gsampler2DMS.texture-levels ...)
.. type:: gsampler2DMSArray

   ``gsampler2DMSArray`` < ``gsampler`` 
.. builtin:: (gsampler2DMSArray.texture-samples ...)
.. builtin:: (gsampler2DMSArray.texture-levels ...)
.. type:: gsampler2DRect

   ``gsampler2DRect`` < ``gsampler`` 
.. builtin:: (gsampler2DRect.texture-samples ...)
.. builtin:: (gsampler2DRect.texture-levels ...)
.. type:: gsampler3D

   ``gsampler3D`` < ``gsampler`` 
.. builtin:: (gsampler3D.texture-samples ...)
.. builtin:: (gsampler3D.texture-levels ...)
.. type:: gsamplerBuffer

   ``gsamplerBuffer`` < ``gsampler`` 
.. builtin:: (gsamplerBuffer.texture-samples ...)
.. builtin:: (gsamplerBuffer.texture-levels ...)
.. type:: gsamplerCube

   ``gsamplerCube`` < ``gsampler`` 
.. builtin:: (gsamplerCube.texture-samples ...)
.. builtin:: (gsamplerCube.texture-levels ...)
.. type:: gsamplerCubeArray

   ``gsamplerCubeArray`` < ``gsampler`` 
.. builtin:: (gsamplerCubeArray.texture-samples ...)
.. builtin:: (gsamplerCubeArray.texture-levels ...)
.. type:: isampler1D

   ``isampler1D`` < ``gsampler1D$3`` 
.. type:: isampler1DArray

   ``isampler1DArray`` < ``gsampler1DArray$3`` 
.. type:: isampler2D

   ``isampler2D`` < ``gsampler2D$3`` 
.. type:: isampler2DArray

   ``isampler2DArray`` < ``gsampler2DArray$3`` 
.. type:: isampler2DMS

   ``isampler2DMS`` < ``gsampler2DMS$3`` 
.. type:: isampler2DMSArray

   ``isampler2DMSArray`` < ``gsampler2DMSArray$3`` 
.. type:: isampler2DRect

   ``isampler2DRect`` < ``gsampler2DRect$3`` 
.. type:: isampler3D

   ``isampler3D`` < ``gsampler3D$3`` 
.. type:: isamplerBuffer

   ``isamplerBuffer`` < ``gsamplerBuffer$3`` 
.. type:: isamplerCube

   ``isamplerCube`` < ``gsamplerCube$3`` 
.. type:: isamplerCubeArray

   ``isamplerCubeArray`` < ``gsamplerCubeArray$3`` 
.. type:: sampler1D

   ``sampler1D`` < ``gsampler1D$2`` 
.. type:: sampler1DArray

   ``sampler1DArray`` < ``gsampler1DArray$2`` 
.. type:: sampler2D

   ``sampler2D`` < ``gsampler2D$2`` 
.. type:: sampler2DArray

   ``sampler2DArray`` < ``gsampler2DArray$2`` 
.. type:: sampler2DMS

   ``sampler2DMS`` < ``gsampler2DMS$2`` 
.. type:: sampler2DMSArray

   ``sampler2DMSArray`` < ``gsampler2DMSArray$2`` 
.. type:: sampler2DRect

   ``sampler2DRect`` < ``gsampler2DRect$2`` 
.. type:: sampler3D

   ``sampler3D`` < ``gsampler3D$2`` 
.. type:: samplerBuffer

   ``samplerBuffer`` < ``gsamplerBuffer$2`` 
.. type:: samplerCube

   ``samplerCube`` < ``gsamplerCube$2`` 
.. type:: samplerCubeArray

   ``samplerCubeArray`` < ``gsamplerCubeArray$2`` 
.. type:: usampler1D

   ``usampler1D`` < ``gsampler1D$4`` 
.. type:: usampler1DArray

   ``usampler1DArray`` < ``gsampler1DArray$4`` 
.. type:: usampler2D

   ``usampler2D`` < ``gsampler2D$4`` 
.. type:: usampler2DArray

   ``usampler2DArray`` < ``gsampler2DArray$4`` 
.. type:: usampler2DMS

   ``usampler2DMS`` < ``gsampler2DMS$4`` 
.. type:: usampler2DMSArray

   ``usampler2DMSArray`` < ``gsampler2DMSArray$4`` 
.. type:: usampler2DRect

   ``usampler2DRect`` < ``gsampler2DRect$4`` 
.. type:: usampler3D

   ``usampler3D`` < ``gsampler3D$4`` 
.. type:: usamplerBuffer

   ``usamplerBuffer`` < ``gsamplerBuffer$4`` 
.. type:: usamplerCube

   ``usamplerCube`` < ``gsamplerCube$4`` 
.. type:: usamplerCubeArray

   ``usamplerCubeArray`` < ``gsamplerCubeArray$4`` 
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

   ``u32<-(vec2)``
.. compiledfn:: (unpackHalf2x16 ...)

   ``vec2<-(u32)``
