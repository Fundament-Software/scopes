glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

.. define:: gl_ClipDistance

   A constant of type `[f32 x ?]`.
.. define:: gl_FragCoord

   A constant of type `vec4`.
.. define:: gl_FragDepth

   A constant of type `f32`.
.. define:: gl_GlobalInvocationID

   A constant of type `uvec3`.
.. define:: gl_InstanceID

   A constant of type `i32`.
.. define:: gl_InstanceIndex

   A constant of type `i32`.
.. define:: gl_LocalInvocationID

   A constant of type `uvec3`.
.. define:: gl_LocalInvocationIndex

   A constant of type `u32`.
.. define:: gl_NumWorkGroups

   A constant of type `uvec3`.
.. define:: gl_PointSize

   A constant of type `f32`.
.. define:: gl_Position

   A constant of type `vec4`.
.. define:: gl_VertexID

   A constant of type `i32`.
.. define:: gl_VertexIndex

   A constant of type `i32`.
.. define:: gl_WorkGroupID

   A constant of type `uvec3`.
.. define:: gl_WorkGroupSize

   A constant of type `uvec3`.
.. define:: r16

   A constant of type `Symbol`.
.. define:: r16_snorm

   A constant of type `Symbol`.
.. define:: r16f

   A constant of type `Symbol`.
.. define:: r16i

   A constant of type `Symbol`.
.. define:: r16ui

   A constant of type `Symbol`.
.. define:: r32

   A constant of type `Symbol`.
.. define:: r32f

   A constant of type `Symbol`.
.. define:: r32i

   A constant of type `Symbol`.
.. define:: r32ui

   A constant of type `Symbol`.
.. define:: r8

   A constant of type `Symbol`.
.. define:: r8_snorm

   A constant of type `Symbol`.
.. define:: r8i

   A constant of type `Symbol`.
.. define:: r8ui

   A constant of type `Symbol`.
.. define:: rg16

   A constant of type `Symbol`.
.. define:: rg16_snorm

   A constant of type `Symbol`.
.. define:: rg16f

   A constant of type `Symbol`.
.. define:: rg16i

   A constant of type `Symbol`.
.. define:: rg16ui

   A constant of type `Symbol`.
.. define:: rg32

   A constant of type `Symbol`.
.. define:: rg32f

   A constant of type `Symbol`.
.. define:: rg32i

   A constant of type `Symbol`.
.. define:: rg32ui

   A constant of type `Symbol`.
.. define:: rg8

   A constant of type `Symbol`.
.. define:: rg8_snorm

   A constant of type `Symbol`.
.. define:: rg8i

   A constant of type `Symbol`.
.. define:: rg8ui

   A constant of type `Symbol`.
.. define:: rgba16

   A constant of type `Symbol`.
.. define:: rgba16_snorm

   A constant of type `Symbol`.
.. define:: rgba16f

   A constant of type `Symbol`.
.. define:: rgba16i

   A constant of type `Symbol`.
.. define:: rgba16ui

   A constant of type `Symbol`.
.. define:: rgba32

   A constant of type `Symbol`.
.. define:: rgba32f

   A constant of type `Symbol`.
.. define:: rgba32i

   A constant of type `Symbol`.
.. define:: rgba32ui

   A constant of type `Symbol`.
.. define:: rgba8

   A constant of type `Symbol`.
.. define:: rgba8_snorm

   A constant of type `Symbol`.
.. define:: rgba8i

   A constant of type `Symbol`.
.. define:: rgba8ui

   A constant of type `Symbol`.
.. type:: DispatchIndirectCommand

   A plain type of supertype `CStruct` and of storage type `{num_groups_x=u32 num_groups_y=u32 num_groups_z=u32}`.

.. type:: DrawArraysIndirectCommand

   A plain type of supertype `CStruct` and of storage type `{count=u32 instanceCount=u32 first=u32 baseInstance=u32}`.

.. type:: ceil

   An opaque type of supertype `OverloadedFunction`.

.. type:: dFdx

   An opaque type of supertype `OverloadedFunction`.

.. type:: dFdy

   An opaque type of supertype `OverloadedFunction`.

.. type:: findLSB

   An opaque type of supertype `OverloadedFunction`.

.. type:: fract

   An opaque type of supertype `OverloadedFunction`.

.. type:: fwidth

   An opaque type of supertype `OverloadedFunction`.

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

.. type:: smoothstep

   An opaque type of supertype `OverloadedFunction`.

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

.. inline:: (atomicAdd mem data)
.. inline:: (atomicAnd mem data)
.. inline:: (atomicCompSwap mem compare data)
.. inline:: (atomicExchange mem data)
.. inline:: (atomicMax mem data)
.. inline:: (atomicMin mem data)
.. inline:: (atomicOr mem data)
.. inline:: (atomicXor mem data)
.. inline:: (barrier)
.. inline:: (groupMemoryBarrier)
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
.. inline:: (memoryBarrier)
.. inline:: (memoryBarrierBuffer)
.. inline:: (memoryBarrierImage)
.. inline:: (memoryBarrierShared)
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
.. sugar:: (fragment_depth ...)
.. sugar:: (in ...)
.. sugar:: (inout ...)
.. sugar:: (inout-geometry ...)
.. sugar:: (input_primitive ...)
.. sugar:: (out ...)
.. sugar:: (output_primitive ...)
.. sugar:: (shared ...)
.. sugar:: (uniform ...)
.. compiledfn:: (EmitVertex ...)

   An external function of type ``void<-()``.
.. compiledfn:: (EndPrimitive ...)

   An external function of type ``void<-()``.
.. compiledfn:: (packHalf2x16 ...)

   An external function of type ``u32<-(vec2)``.
.. compiledfn:: (packSnorm2x16 ...)

   An external function of type ``u32<-(vec2)``.
.. compiledfn:: (packSnorm4x8 ...)

   An external function of type ``u32<-(vec4)``.
.. compiledfn:: (packUnorm2x16 ...)

   An external function of type ``u32<-(vec2)``.
.. compiledfn:: (packUnorm4x8 ...)

   An external function of type ``u32<-(vec4)``.
.. compiledfn:: (unpackHalf2x16 ...)

   An external function of type ``vec2<-(u32)``.
.. compiledfn:: (unpackSnorm2x16 ...)

   An external function of type ``vec2<-(u32)``.
.. compiledfn:: (unpackSnorm4x8 ...)

   An external function of type ``vec4<-(u32)``.
.. compiledfn:: (unpackUnorm2x16 ...)

   An external function of type ``vec2<-(u32)``.
.. compiledfn:: (unpackUnorm4x8 ...)

   An external function of type ``vec4<-(u32)``.
