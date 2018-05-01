glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

.. type:: XVarBridgeType
.. type:: XVarType
.. type:: gsampler1D
.. typefn:: (gsampler1D 'fetch args...)
.. type:: gsampler1DArray
.. typefn:: (gsampler1DArray 'fetch args...)
.. type:: gsampler2D
.. typefn:: (gsampler2D 'fetch args...)
.. type:: gsampler2DArray
.. typefn:: (gsampler2DArray 'fetch args...)
.. type:: gsampler2DMS
.. typefn:: (gsampler2DMS 'fetch args...)
.. type:: gsampler2DMSArray
.. typefn:: (gsampler2DMSArray 'fetch args...)
.. type:: gsampler2DRect
.. typefn:: (gsampler2DRect 'fetch args...)
.. type:: gsampler3D
.. typefn:: (gsampler3D 'fetch args...)
.. type:: gsamplerBuffer
.. typefn:: (gsamplerBuffer 'fetch args...)
.. type:: gsamplerCube
.. typefn:: (gsamplerCube 'fetch args...)
.. type:: gsamplerCubeArray
.. typefn:: (gsamplerCubeArray 'fetch args...)
.. type:: isampler1D
.. type:: isampler1DArray
.. type:: isampler2D
.. type:: isampler2DArray
.. type:: isampler2DMS
.. type:: isampler2DMSArray
.. type:: isampler2DRect
.. type:: isampler3D
.. type:: isamplerBuffer
.. type:: isamplerCube
.. type:: isamplerCubeArray
.. type:: sampler
.. type:: sampler1D
.. type:: sampler1DArray
.. type:: sampler2D
.. type:: sampler2DArray
.. type:: sampler2DMS
.. type:: sampler2DMSArray
.. type:: sampler2DRect
.. type:: sampler3D
.. type:: samplerBuffer
.. type:: samplerCube
.. type:: samplerCubeArray
.. type:: usampler1D
.. type:: usampler1DArray
.. type:: usampler2D
.. type:: usampler2DArray
.. type:: usampler2DMS
.. type:: usampler2DMSArray
.. type:: usampler2DRect
.. type:: usampler3D
.. type:: usamplerBuffer
.. type:: usamplerCube
.. type:: usamplerCubeArray
.. fn:: (iimage1D format)
.. fn:: (iimage1DArray format)
.. fn:: (iimage2D format)
.. fn:: (iimage2DArray format)
.. fn:: (iimage2DMS format)
.. fn:: (iimage2DMSArray format)
.. fn:: (iimage2DRect format)
.. fn:: (iimage3D format)
.. fn:: (iimageBuffer format)
.. fn:: (iimageCube format)
.. fn:: (iimageCubeArray format)
.. fn:: (image1D format)
.. fn:: (image1DArray format)
.. fn:: (image2D format)
.. fn:: (image2DArray format)
.. fn:: (image2DMS format)
.. fn:: (image2DMSArray format)
.. fn:: (image2DRect format)
.. fn:: (image3D format)
.. fn:: (imageBuffer format)
.. fn:: (imageCube format)
.. fn:: (imageCubeArray format)
.. fn:: (imageLoad image coord)
.. fn:: (imageStore image coord data)
.. fn:: (local_size x y z)
.. fn:: (texelFetch sampler ...)
.. fn:: (uimage1D format)
.. fn:: (uimage1DArray format)
.. fn:: (uimage2D format)
.. fn:: (uimage2DArray format)
.. fn:: (uimage2DMS format)
.. fn:: (uimage2DMSArray format)
.. fn:: (uimage2DRect format)
.. fn:: (uimage3D format)
.. fn:: (uimageBuffer format)
.. fn:: (uimageCube format)
.. fn:: (uimageCubeArray format)
.. macro:: (xvar ...)
