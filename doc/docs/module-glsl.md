<style type="text/css" rel="stylesheet">body { counter-reset: chapter 15; }</style>

glsl
====

The `glsl` module exports bridge symbols that make it possible to define
and access external variables for shader programs.

*define*{.property} `gl_ClipDistance`{.descname} [](#scopes.define.gl_ClipDistance "Permalink to this definition"){.headerlink} {#scopes.define.gl_ClipDistance}

:   A constant of type `(array f32)`.

*define*{.property} `gl_FragCoord`{.descname} [](#scopes.define.gl_FragCoord "Permalink to this definition"){.headerlink} {#scopes.define.gl_FragCoord}

:   A constant of type `vec4`.

*define*{.property} `gl_FragDepth`{.descname} [](#scopes.define.gl_FragDepth "Permalink to this definition"){.headerlink} {#scopes.define.gl_FragDepth}

:   A constant of type `f32`.

*define*{.property} `gl_GlobalInvocationID`{.descname} [](#scopes.define.gl_GlobalInvocationID "Permalink to this definition"){.headerlink} {#scopes.define.gl_GlobalInvocationID}

:   A constant of type `uvec3`.

*define*{.property} `gl_InstanceID`{.descname} [](#scopes.define.gl_InstanceID "Permalink to this definition"){.headerlink} {#scopes.define.gl_InstanceID}

:   A constant of type `i32`.

*define*{.property} `gl_InstanceIndex`{.descname} [](#scopes.define.gl_InstanceIndex "Permalink to this definition"){.headerlink} {#scopes.define.gl_InstanceIndex}

:   A constant of type `i32`.

*define*{.property} `gl_LocalInvocationID`{.descname} [](#scopes.define.gl_LocalInvocationID "Permalink to this definition"){.headerlink} {#scopes.define.gl_LocalInvocationID}

:   A constant of type `uvec3`.

*define*{.property} `gl_LocalInvocationIndex`{.descname} [](#scopes.define.gl_LocalInvocationIndex "Permalink to this definition"){.headerlink} {#scopes.define.gl_LocalInvocationIndex}

:   A constant of type `u32`.

*define*{.property} `gl_NumWorkGroups`{.descname} [](#scopes.define.gl_NumWorkGroups "Permalink to this definition"){.headerlink} {#scopes.define.gl_NumWorkGroups}

:   A constant of type `uvec3`.

*define*{.property} `gl_PointSize`{.descname} [](#scopes.define.gl_PointSize "Permalink to this definition"){.headerlink} {#scopes.define.gl_PointSize}

:   A constant of type `f32`.

*define*{.property} `gl_Position`{.descname} [](#scopes.define.gl_Position "Permalink to this definition"){.headerlink} {#scopes.define.gl_Position}

:   A constant of type `vec4`.

*define*{.property} `gl_PrimitiveID`{.descname} [](#scopes.define.gl_PrimitiveID "Permalink to this definition"){.headerlink} {#scopes.define.gl_PrimitiveID}

:   A constant of type `i32`.

*define*{.property} `gl_VertexID`{.descname} [](#scopes.define.gl_VertexID "Permalink to this definition"){.headerlink} {#scopes.define.gl_VertexID}

:   A constant of type `i32`.

*define*{.property} `gl_VertexIndex`{.descname} [](#scopes.define.gl_VertexIndex "Permalink to this definition"){.headerlink} {#scopes.define.gl_VertexIndex}

:   A constant of type `i32`.

*define*{.property} `gl_WorkGroupID`{.descname} [](#scopes.define.gl_WorkGroupID "Permalink to this definition"){.headerlink} {#scopes.define.gl_WorkGroupID}

:   A constant of type `uvec3`.

*define*{.property} `gl_WorkGroupSize`{.descname} [](#scopes.define.gl_WorkGroupSize "Permalink to this definition"){.headerlink} {#scopes.define.gl_WorkGroupSize}

:   A constant of type `uvec3`.

*define*{.property} `r16`{.descname} [](#scopes.define.r16 "Permalink to this definition"){.headerlink} {#scopes.define.r16}

:   A constant of type `Symbol`.

*define*{.property} `r16_snorm`{.descname} [](#scopes.define.r16_snorm "Permalink to this definition"){.headerlink} {#scopes.define.r16_snorm}

:   A constant of type `Symbol`.

*define*{.property} `r16f`{.descname} [](#scopes.define.r16f "Permalink to this definition"){.headerlink} {#scopes.define.r16f}

:   A constant of type `Symbol`.

*define*{.property} `r16i`{.descname} [](#scopes.define.r16i "Permalink to this definition"){.headerlink} {#scopes.define.r16i}

:   A constant of type `Symbol`.

*define*{.property} `r16ui`{.descname} [](#scopes.define.r16ui "Permalink to this definition"){.headerlink} {#scopes.define.r16ui}

:   A constant of type `Symbol`.

*define*{.property} `r32`{.descname} [](#scopes.define.r32 "Permalink to this definition"){.headerlink} {#scopes.define.r32}

:   A constant of type `Symbol`.

*define*{.property} `r32f`{.descname} [](#scopes.define.r32f "Permalink to this definition"){.headerlink} {#scopes.define.r32f}

:   A constant of type `Symbol`.

*define*{.property} `r32i`{.descname} [](#scopes.define.r32i "Permalink to this definition"){.headerlink} {#scopes.define.r32i}

:   A constant of type `Symbol`.

*define*{.property} `r32ui`{.descname} [](#scopes.define.r32ui "Permalink to this definition"){.headerlink} {#scopes.define.r32ui}

:   A constant of type `Symbol`.

*define*{.property} `r8`{.descname} [](#scopes.define.r8 "Permalink to this definition"){.headerlink} {#scopes.define.r8}

:   A constant of type `Symbol`.

*define*{.property} `r8_snorm`{.descname} [](#scopes.define.r8_snorm "Permalink to this definition"){.headerlink} {#scopes.define.r8_snorm}

:   A constant of type `Symbol`.

*define*{.property} `r8i`{.descname} [](#scopes.define.r8i "Permalink to this definition"){.headerlink} {#scopes.define.r8i}

:   A constant of type `Symbol`.

*define*{.property} `r8ui`{.descname} [](#scopes.define.r8ui "Permalink to this definition"){.headerlink} {#scopes.define.r8ui}

:   A constant of type `Symbol`.

*define*{.property} `rg16`{.descname} [](#scopes.define.rg16 "Permalink to this definition"){.headerlink} {#scopes.define.rg16}

:   A constant of type `Symbol`.

*define*{.property} `rg16_snorm`{.descname} [](#scopes.define.rg16_snorm "Permalink to this definition"){.headerlink} {#scopes.define.rg16_snorm}

:   A constant of type `Symbol`.

*define*{.property} `rg16f`{.descname} [](#scopes.define.rg16f "Permalink to this definition"){.headerlink} {#scopes.define.rg16f}

:   A constant of type `Symbol`.

*define*{.property} `rg16i`{.descname} [](#scopes.define.rg16i "Permalink to this definition"){.headerlink} {#scopes.define.rg16i}

:   A constant of type `Symbol`.

*define*{.property} `rg16ui`{.descname} [](#scopes.define.rg16ui "Permalink to this definition"){.headerlink} {#scopes.define.rg16ui}

:   A constant of type `Symbol`.

*define*{.property} `rg32`{.descname} [](#scopes.define.rg32 "Permalink to this definition"){.headerlink} {#scopes.define.rg32}

:   A constant of type `Symbol`.

*define*{.property} `rg32f`{.descname} [](#scopes.define.rg32f "Permalink to this definition"){.headerlink} {#scopes.define.rg32f}

:   A constant of type `Symbol`.

*define*{.property} `rg32i`{.descname} [](#scopes.define.rg32i "Permalink to this definition"){.headerlink} {#scopes.define.rg32i}

:   A constant of type `Symbol`.

*define*{.property} `rg32ui`{.descname} [](#scopes.define.rg32ui "Permalink to this definition"){.headerlink} {#scopes.define.rg32ui}

:   A constant of type `Symbol`.

*define*{.property} `rg8`{.descname} [](#scopes.define.rg8 "Permalink to this definition"){.headerlink} {#scopes.define.rg8}

:   A constant of type `Symbol`.

*define*{.property} `rg8_snorm`{.descname} [](#scopes.define.rg8_snorm "Permalink to this definition"){.headerlink} {#scopes.define.rg8_snorm}

:   A constant of type `Symbol`.

*define*{.property} `rg8i`{.descname} [](#scopes.define.rg8i "Permalink to this definition"){.headerlink} {#scopes.define.rg8i}

:   A constant of type `Symbol`.

*define*{.property} `rg8ui`{.descname} [](#scopes.define.rg8ui "Permalink to this definition"){.headerlink} {#scopes.define.rg8ui}

:   A constant of type `Symbol`.

*define*{.property} `rgba16`{.descname} [](#scopes.define.rgba16 "Permalink to this definition"){.headerlink} {#scopes.define.rgba16}

:   A constant of type `Symbol`.

*define*{.property} `rgba16_snorm`{.descname} [](#scopes.define.rgba16_snorm "Permalink to this definition"){.headerlink} {#scopes.define.rgba16_snorm}

:   A constant of type `Symbol`.

*define*{.property} `rgba16f`{.descname} [](#scopes.define.rgba16f "Permalink to this definition"){.headerlink} {#scopes.define.rgba16f}

:   A constant of type `Symbol`.

*define*{.property} `rgba16i`{.descname} [](#scopes.define.rgba16i "Permalink to this definition"){.headerlink} {#scopes.define.rgba16i}

:   A constant of type `Symbol`.

*define*{.property} `rgba16ui`{.descname} [](#scopes.define.rgba16ui "Permalink to this definition"){.headerlink} {#scopes.define.rgba16ui}

:   A constant of type `Symbol`.

*define*{.property} `rgba32`{.descname} [](#scopes.define.rgba32 "Permalink to this definition"){.headerlink} {#scopes.define.rgba32}

:   A constant of type `Symbol`.

*define*{.property} `rgba32f`{.descname} [](#scopes.define.rgba32f "Permalink to this definition"){.headerlink} {#scopes.define.rgba32f}

:   A constant of type `Symbol`.

*define*{.property} `rgba32i`{.descname} [](#scopes.define.rgba32i "Permalink to this definition"){.headerlink} {#scopes.define.rgba32i}

:   A constant of type `Symbol`.

*define*{.property} `rgba32ui`{.descname} [](#scopes.define.rgba32ui "Permalink to this definition"){.headerlink} {#scopes.define.rgba32ui}

:   A constant of type `Symbol`.

*define*{.property} `rgba8`{.descname} [](#scopes.define.rgba8 "Permalink to this definition"){.headerlink} {#scopes.define.rgba8}

:   A constant of type `Symbol`.

*define*{.property} `rgba8_snorm`{.descname} [](#scopes.define.rgba8_snorm "Permalink to this definition"){.headerlink} {#scopes.define.rgba8_snorm}

:   A constant of type `Symbol`.

*define*{.property} `rgba8i`{.descname} [](#scopes.define.rgba8i "Permalink to this definition"){.headerlink} {#scopes.define.rgba8i}

:   A constant of type `Symbol`.

*define*{.property} `rgba8ui`{.descname} [](#scopes.define.rgba8ui "Permalink to this definition"){.headerlink} {#scopes.define.rgba8ui}

:   A constant of type `Symbol`.

*type*{.property} `DispatchIndirectCommand`{.descname} [](#scopes.type.DispatchIndirectCommand "Permalink to this definition"){.headerlink} {#scopes.type.DispatchIndirectCommand}

:   A plain type of supertype `CStruct` and of storage type `(tuple (num_groups_x = u32) (num_groups_y = u32) (num_groups_z = u32))`.

*type*{.property} `DrawArraysIndirectCommand`{.descname} [](#scopes.type.DrawArraysIndirectCommand "Permalink to this definition"){.headerlink} {#scopes.type.DrawArraysIndirectCommand}

:   A plain type of supertype `CStruct` and of storage type `(tuple (count = u32) (instanceCount = u32) (first = u32) (baseInstance = u32))`.

*type*{.property} `ceil`{.descname} [](#scopes.type.ceil "Permalink to this definition"){.headerlink} {#scopes.type.ceil}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `dFdx`{.descname} [](#scopes.type.dFdx "Permalink to this definition"){.headerlink} {#scopes.type.dFdx}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `dFdy`{.descname} [](#scopes.type.dFdy "Permalink to this definition"){.headerlink} {#scopes.type.dFdy}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `findLSB`{.descname} [](#scopes.type.findLSB "Permalink to this definition"){.headerlink} {#scopes.type.findLSB}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `fract`{.descname} [](#scopes.type.fract "Permalink to this definition"){.headerlink} {#scopes.type.fract}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `fwidth`{.descname} [](#scopes.type.fwidth "Permalink to this definition"){.headerlink} {#scopes.type.fwidth}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `gsampler`{.descname} [](#scopes.type.gsampler "Permalink to this definition"){.headerlink} {#scopes.type.gsampler}

:   An opaque type.

*type*{.property} `gsampler1D`{.descname} [](#scopes.type.gsampler1D "Permalink to this definition"){.headerlink} {#scopes.type.gsampler1D}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler1D.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler1D.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler1D.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler1D.builtin.texture-samples}

    :   

*type*{.property} `gsampler1DArray`{.descname} [](#scopes.type.gsampler1DArray "Permalink to this definition"){.headerlink} {#scopes.type.gsampler1DArray}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler1DArray.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler1DArray.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler1DArray.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler1DArray.builtin.texture-samples}

    :   

*type*{.property} `gsampler2D`{.descname} [](#scopes.type.gsampler2D "Permalink to this definition"){.headerlink} {#scopes.type.gsampler2D}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2D.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler2D.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2D.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler2D.builtin.texture-samples}

    :   

*type*{.property} `gsampler2DArray`{.descname} [](#scopes.type.gsampler2DArray "Permalink to this definition"){.headerlink} {#scopes.type.gsampler2DArray}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DArray.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler2DArray.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DArray.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler2DArray.builtin.texture-samples}

    :   

*type*{.property} `gsampler2DMS`{.descname} [](#scopes.type.gsampler2DMS "Permalink to this definition"){.headerlink} {#scopes.type.gsampler2DMS}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DMS.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler2DMS.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DMS.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler2DMS.builtin.texture-samples}

    :   

*type*{.property} `gsampler2DMSArray`{.descname} [](#scopes.type.gsampler2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.gsampler2DMSArray}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DMSArray.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler2DMSArray.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DMSArray.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler2DMSArray.builtin.texture-samples}

    :   

*type*{.property} `gsampler2DRect`{.descname} [](#scopes.type.gsampler2DRect "Permalink to this definition"){.headerlink} {#scopes.type.gsampler2DRect}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DRect.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler2DRect.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler2DRect.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler2DRect.builtin.texture-samples}

    :   

*type*{.property} `gsampler3D`{.descname} [](#scopes.type.gsampler3D "Permalink to this definition"){.headerlink} {#scopes.type.gsampler3D}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler3D.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsampler3D.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsampler3D.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsampler3D.builtin.texture-samples}

    :   

*type*{.property} `gsamplerBuffer`{.descname} [](#scopes.type.gsamplerBuffer "Permalink to this definition"){.headerlink} {#scopes.type.gsamplerBuffer}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerBuffer.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsamplerBuffer.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerBuffer.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsamplerBuffer.builtin.texture-samples}

    :   

*type*{.property} `gsamplerCube`{.descname} [](#scopes.type.gsamplerCube "Permalink to this definition"){.headerlink} {#scopes.type.gsamplerCube}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerCube.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsamplerCube.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerCube.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsamplerCube.builtin.texture-samples}

    :   

*type*{.property} `gsamplerCubeArray`{.descname} [](#scopes.type.gsamplerCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.gsamplerCubeArray}

:   An opaque type of supertype `gsampler`.

    *builtin*{.property} `texture-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerCubeArray.builtin.texture-levels "Permalink to this definition"){.headerlink} {#scopes.gsamplerCubeArray.builtin.texture-levels}

    :   

    *builtin*{.property} `texture-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.gsamplerCubeArray.builtin.texture-samples "Permalink to this definition"){.headerlink} {#scopes.gsamplerCubeArray.builtin.texture-samples}

    :   

*type*{.property} `isampler1D`{.descname} [](#scopes.type.isampler1D "Permalink to this definition"){.headerlink} {#scopes.type.isampler1D}

:   A plain type of supertype `gsampler1D$3` and of storage type `<SampledImage <Image ivec4 '1D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler1D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler1D.inline.__typecall}

    :   

*type*{.property} `isampler1DArray`{.descname} [](#scopes.type.isampler1DArray "Permalink to this definition"){.headerlink} {#scopes.type.isampler1DArray}

:   A plain type of supertype `gsampler1DArray$3` and of storage type `<SampledImage <Image ivec4 '1D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler1DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler1DArray.inline.__typecall}

    :   

*type*{.property} `isampler2D`{.descname} [](#scopes.type.isampler2D "Permalink to this definition"){.headerlink} {#scopes.type.isampler2D}

:   A plain type of supertype `gsampler2D$3` and of storage type `<SampledImage <Image ivec4 '2D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler2D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler2D.inline.__typecall}

    :   

*type*{.property} `isampler2DArray`{.descname} [](#scopes.type.isampler2DArray "Permalink to this definition"){.headerlink} {#scopes.type.isampler2DArray}

:   A plain type of supertype `gsampler2DArray$3` and of storage type `<SampledImage <Image ivec4 '2D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler2DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler2DArray.inline.__typecall}

    :   

*type*{.property} `isampler2DMS`{.descname} [](#scopes.type.isampler2DMS "Permalink to this definition"){.headerlink} {#scopes.type.isampler2DMS}

:   A plain type of supertype `gsampler2DMS$3` and of storage type `<SampledImage <Image ivec4 '2D ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler2DMS.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler2DMS.inline.__typecall}

    :   

*type*{.property} `isampler2DMSArray`{.descname} [](#scopes.type.isampler2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.isampler2DMSArray}

:   A plain type of supertype `gsampler2DMSArray$3` and of storage type `<SampledImage <Image ivec4 '2D array ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler2DMSArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler2DMSArray.inline.__typecall}

    :   

*type*{.property} `isampler2DRect`{.descname} [](#scopes.type.isampler2DRect "Permalink to this definition"){.headerlink} {#scopes.type.isampler2DRect}

:   A plain type of supertype `gsampler2DRect$3` and of storage type `<SampledImage <Image ivec4 'Rect sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler2DRect.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler2DRect.inline.__typecall}

    :   

*type*{.property} `isampler3D`{.descname} [](#scopes.type.isampler3D "Permalink to this definition"){.headerlink} {#scopes.type.isampler3D}

:   A plain type of supertype `gsampler3D$3` and of storage type `<SampledImage <Image ivec4 '3D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isampler3D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isampler3D.inline.__typecall}

    :   

*type*{.property} `isamplerBuffer`{.descname} [](#scopes.type.isamplerBuffer "Permalink to this definition"){.headerlink} {#scopes.type.isamplerBuffer}

:   A plain type of supertype `gsamplerBuffer$3` and of storage type `<SampledImage <Image ivec4 'Buffer sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isamplerBuffer.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isamplerBuffer.inline.__typecall}

    :   

*type*{.property} `isamplerCube`{.descname} [](#scopes.type.isamplerCube "Permalink to this definition"){.headerlink} {#scopes.type.isamplerCube}

:   A plain type of supertype `gsamplerCube$3` and of storage type `<SampledImage <Image ivec4 'Cube sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isamplerCube.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isamplerCube.inline.__typecall}

    :   

*type*{.property} `isamplerCubeArray`{.descname} [](#scopes.type.isamplerCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.isamplerCubeArray}

:   A plain type of supertype `gsamplerCubeArray$3` and of storage type `<SampledImage <Image ivec4 'Cube array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.isamplerCubeArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.isamplerCubeArray.inline.__typecall}

    :   

*type*{.property} `itexture1D`{.descname} [](#scopes.type.itexture1D "Permalink to this definition"){.headerlink} {#scopes.type.itexture1D}

:   A plain type labeled `<Image ivec4 '1D sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '1D sampled 'Unknown>`.

*type*{.property} `itexture1DArray`{.descname} [](#scopes.type.itexture1DArray "Permalink to this definition"){.headerlink} {#scopes.type.itexture1DArray}

:   A plain type labeled `<Image ivec4 '1D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '1D array sampled 'Unknown>`.

*type*{.property} `itexture2D`{.descname} [](#scopes.type.itexture2D "Permalink to this definition"){.headerlink} {#scopes.type.itexture2D}

:   A plain type labeled `<Image ivec4 '2D sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '2D sampled 'Unknown>`.

*type*{.property} `itexture2DArray`{.descname} [](#scopes.type.itexture2DArray "Permalink to this definition"){.headerlink} {#scopes.type.itexture2DArray}

:   A plain type labeled `<Image ivec4 '2D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '2D array sampled 'Unknown>`.

*type*{.property} `itexture2DMS`{.descname} [](#scopes.type.itexture2DMS "Permalink to this definition"){.headerlink} {#scopes.type.itexture2DMS}

:   A plain type labeled `<Image ivec4 '2D ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '2D ms sampled 'Unknown>`.

*type*{.property} `itexture2DMSArray`{.descname} [](#scopes.type.itexture2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.itexture2DMSArray}

:   A plain type labeled `<Image ivec4 '2D array ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '2D array ms sampled 'Unknown>`.

*type*{.property} `itexture2DRect`{.descname} [](#scopes.type.itexture2DRect "Permalink to this definition"){.headerlink} {#scopes.type.itexture2DRect}

:   A plain type labeled `<Image ivec4 'Rect sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 'Rect sampled 'Unknown>`.

*type*{.property} `itexture3D`{.descname} [](#scopes.type.itexture3D "Permalink to this definition"){.headerlink} {#scopes.type.itexture3D}

:   A plain type labeled `<Image ivec4 '3D sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 '3D sampled 'Unknown>`.

*type*{.property} `itextureBuffer`{.descname} [](#scopes.type.itextureBuffer "Permalink to this definition"){.headerlink} {#scopes.type.itextureBuffer}

:   A plain type labeled `<Image ivec4 'Buffer sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 'Buffer sampled 'Unknown>`.

*type*{.property} `itextureCube`{.descname} [](#scopes.type.itextureCube "Permalink to this definition"){.headerlink} {#scopes.type.itextureCube}

:   A plain type labeled `<Image ivec4 'Cube sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 'Cube sampled 'Unknown>`.

*type*{.property} `itextureCubeArray`{.descname} [](#scopes.type.itextureCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.itextureCubeArray}

:   A plain type labeled `<Image ivec4 'Cube array sampled 'Unknown>` of supertype `Image` and of storage type `<Image ivec4 'Cube array sampled 'Unknown>`.

*type*{.property} `sampler`{.descname} [](#scopes.type.sampler "Permalink to this definition"){.headerlink} {#scopes.type.sampler}

:   A plain type labeled `Sampler` of supertype `immutable` and of storage type `Sampler`.

*type*{.property} `sampler1D`{.descname} [](#scopes.type.sampler1D "Permalink to this definition"){.headerlink} {#scopes.type.sampler1D}

:   A plain type of supertype `gsampler1D$2` and of storage type `<SampledImage <Image vec4 '1D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler1D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler1D.inline.__typecall}

    :   

*type*{.property} `sampler1DArray`{.descname} [](#scopes.type.sampler1DArray "Permalink to this definition"){.headerlink} {#scopes.type.sampler1DArray}

:   A plain type of supertype `gsampler1DArray$2` and of storage type `<SampledImage <Image vec4 '1D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler1DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler1DArray.inline.__typecall}

    :   

*type*{.property} `sampler2D`{.descname} [](#scopes.type.sampler2D "Permalink to this definition"){.headerlink} {#scopes.type.sampler2D}

:   A plain type of supertype `gsampler2D$2` and of storage type `<SampledImage <Image vec4 '2D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler2D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler2D.inline.__typecall}

    :   

*type*{.property} `sampler2DArray`{.descname} [](#scopes.type.sampler2DArray "Permalink to this definition"){.headerlink} {#scopes.type.sampler2DArray}

:   A plain type of supertype `gsampler2DArray$2` and of storage type `<SampledImage <Image vec4 '2D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler2DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler2DArray.inline.__typecall}

    :   

*type*{.property} `sampler2DMS`{.descname} [](#scopes.type.sampler2DMS "Permalink to this definition"){.headerlink} {#scopes.type.sampler2DMS}

:   A plain type of supertype `gsampler2DMS$2` and of storage type `<SampledImage <Image vec4 '2D ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler2DMS.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler2DMS.inline.__typecall}

    :   

*type*{.property} `sampler2DMSArray`{.descname} [](#scopes.type.sampler2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.sampler2DMSArray}

:   A plain type of supertype `gsampler2DMSArray$2` and of storage type `<SampledImage <Image vec4 '2D array ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler2DMSArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler2DMSArray.inline.__typecall}

    :   

*type*{.property} `sampler2DRect`{.descname} [](#scopes.type.sampler2DRect "Permalink to this definition"){.headerlink} {#scopes.type.sampler2DRect}

:   A plain type of supertype `gsampler2DRect$2` and of storage type `<SampledImage <Image vec4 'Rect sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler2DRect.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler2DRect.inline.__typecall}

    :   

*type*{.property} `sampler3D`{.descname} [](#scopes.type.sampler3D "Permalink to this definition"){.headerlink} {#scopes.type.sampler3D}

:   A plain type of supertype `gsampler3D$2` and of storage type `<SampledImage <Image vec4 '3D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.sampler3D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.sampler3D.inline.__typecall}

    :   

*type*{.property} `samplerBuffer`{.descname} [](#scopes.type.samplerBuffer "Permalink to this definition"){.headerlink} {#scopes.type.samplerBuffer}

:   A plain type of supertype `gsamplerBuffer$2` and of storage type `<SampledImage <Image vec4 'Buffer sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.samplerBuffer.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.samplerBuffer.inline.__typecall}

    :   

*type*{.property} `samplerCube`{.descname} [](#scopes.type.samplerCube "Permalink to this definition"){.headerlink} {#scopes.type.samplerCube}

:   A plain type of supertype `gsamplerCube$2` and of storage type `<SampledImage <Image vec4 'Cube sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.samplerCube.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.samplerCube.inline.__typecall}

    :   

*type*{.property} `samplerCubeArray`{.descname} [](#scopes.type.samplerCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.samplerCubeArray}

:   A plain type of supertype `gsamplerCubeArray$2` and of storage type `<SampledImage <Image vec4 'Cube array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.samplerCubeArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.samplerCubeArray.inline.__typecall}

    :   

*type*{.property} `smoothstep`{.descname} [](#scopes.type.smoothstep "Permalink to this definition"){.headerlink} {#scopes.type.smoothstep}

:   An opaque type of supertype `OverloadedFunction`.

*type*{.property} `texture1D`{.descname} [](#scopes.type.texture1D "Permalink to this definition"){.headerlink} {#scopes.type.texture1D}

:   A plain type labeled `<Image vec4 '1D sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '1D sampled 'Unknown>`.

*type*{.property} `texture1DArray`{.descname} [](#scopes.type.texture1DArray "Permalink to this definition"){.headerlink} {#scopes.type.texture1DArray}

:   A plain type labeled `<Image vec4 '1D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '1D array sampled 'Unknown>`.

*type*{.property} `texture2D`{.descname} [](#scopes.type.texture2D "Permalink to this definition"){.headerlink} {#scopes.type.texture2D}

:   A plain type labeled `<Image vec4 '2D sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '2D sampled 'Unknown>`.

*type*{.property} `texture2DArray`{.descname} [](#scopes.type.texture2DArray "Permalink to this definition"){.headerlink} {#scopes.type.texture2DArray}

:   A plain type labeled `<Image vec4 '2D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '2D array sampled 'Unknown>`.

*type*{.property} `texture2DMS`{.descname} [](#scopes.type.texture2DMS "Permalink to this definition"){.headerlink} {#scopes.type.texture2DMS}

:   A plain type labeled `<Image vec4 '2D ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '2D ms sampled 'Unknown>`.

*type*{.property} `texture2DMSArray`{.descname} [](#scopes.type.texture2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.texture2DMSArray}

:   A plain type labeled `<Image vec4 '2D array ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '2D array ms sampled 'Unknown>`.

*type*{.property} `texture2DRect`{.descname} [](#scopes.type.texture2DRect "Permalink to this definition"){.headerlink} {#scopes.type.texture2DRect}

:   A plain type labeled `<Image vec4 'Rect sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 'Rect sampled 'Unknown>`.

*type*{.property} `texture3D`{.descname} [](#scopes.type.texture3D "Permalink to this definition"){.headerlink} {#scopes.type.texture3D}

:   A plain type labeled `<Image vec4 '3D sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 '3D sampled 'Unknown>`.

*type*{.property} `textureBuffer`{.descname} [](#scopes.type.textureBuffer "Permalink to this definition"){.headerlink} {#scopes.type.textureBuffer}

:   A plain type labeled `<Image vec4 'Buffer sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 'Buffer sampled 'Unknown>`.

*type*{.property} `textureCube`{.descname} [](#scopes.type.textureCube "Permalink to this definition"){.headerlink} {#scopes.type.textureCube}

:   A plain type labeled `<Image vec4 'Cube sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 'Cube sampled 'Unknown>`.

*type*{.property} `textureCubeArray`{.descname} [](#scopes.type.textureCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.textureCubeArray}

:   A plain type labeled `<Image vec4 'Cube array sampled 'Unknown>` of supertype `Image` and of storage type `<Image vec4 'Cube array sampled 'Unknown>`.

*type*{.property} `usampler1D`{.descname} [](#scopes.type.usampler1D "Permalink to this definition"){.headerlink} {#scopes.type.usampler1D}

:   A plain type of supertype `gsampler1D$4` and of storage type `<SampledImage <Image uvec4 '1D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler1D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler1D.inline.__typecall}

    :   

*type*{.property} `usampler1DArray`{.descname} [](#scopes.type.usampler1DArray "Permalink to this definition"){.headerlink} {#scopes.type.usampler1DArray}

:   A plain type of supertype `gsampler1DArray$4` and of storage type `<SampledImage <Image uvec4 '1D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler1DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler1DArray.inline.__typecall}

    :   

*type*{.property} `usampler2D`{.descname} [](#scopes.type.usampler2D "Permalink to this definition"){.headerlink} {#scopes.type.usampler2D}

:   A plain type of supertype `gsampler2D$4` and of storage type `<SampledImage <Image uvec4 '2D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler2D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler2D.inline.__typecall}

    :   

*type*{.property} `usampler2DArray`{.descname} [](#scopes.type.usampler2DArray "Permalink to this definition"){.headerlink} {#scopes.type.usampler2DArray}

:   A plain type of supertype `gsampler2DArray$4` and of storage type `<SampledImage <Image uvec4 '2D array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler2DArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler2DArray.inline.__typecall}

    :   

*type*{.property} `usampler2DMS`{.descname} [](#scopes.type.usampler2DMS "Permalink to this definition"){.headerlink} {#scopes.type.usampler2DMS}

:   A plain type of supertype `gsampler2DMS$4` and of storage type `<SampledImage <Image uvec4 '2D ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler2DMS.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler2DMS.inline.__typecall}

    :   

*type*{.property} `usampler2DMSArray`{.descname} [](#scopes.type.usampler2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.usampler2DMSArray}

:   A plain type of supertype `gsampler2DMSArray$4` and of storage type `<SampledImage <Image uvec4 '2D array ms sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler2DMSArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler2DMSArray.inline.__typecall}

    :   

*type*{.property} `usampler2DRect`{.descname} [](#scopes.type.usampler2DRect "Permalink to this definition"){.headerlink} {#scopes.type.usampler2DRect}

:   A plain type of supertype `gsampler2DRect$4` and of storage type `<SampledImage <Image uvec4 'Rect sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler2DRect.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler2DRect.inline.__typecall}

    :   

*type*{.property} `usampler3D`{.descname} [](#scopes.type.usampler3D "Permalink to this definition"){.headerlink} {#scopes.type.usampler3D}

:   A plain type of supertype `gsampler3D$4` and of storage type `<SampledImage <Image uvec4 '3D sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usampler3D.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usampler3D.inline.__typecall}

    :   

*type*{.property} `usamplerBuffer`{.descname} [](#scopes.type.usamplerBuffer "Permalink to this definition"){.headerlink} {#scopes.type.usamplerBuffer}

:   A plain type of supertype `gsamplerBuffer$4` and of storage type `<SampledImage <Image uvec4 'Buffer sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usamplerBuffer.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usamplerBuffer.inline.__typecall}

    :   

*type*{.property} `usamplerCube`{.descname} [](#scopes.type.usamplerCube "Permalink to this definition"){.headerlink} {#scopes.type.usamplerCube}

:   A plain type of supertype `gsamplerCube$4` and of storage type `<SampledImage <Image uvec4 'Cube sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usamplerCube.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usamplerCube.inline.__typecall}

    :   

*type*{.property} `usamplerCubeArray`{.descname} [](#scopes.type.usamplerCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.usamplerCubeArray}

:   A plain type of supertype `gsamplerCubeArray$4` and of storage type `<SampledImage <Image uvec4 'Cube array sampled 'Unknown>>`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls _texture _sampler&ensp;*)[](#scopes.usamplerCubeArray.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.usamplerCubeArray.inline.__typecall}

    :   

*type*{.property} `utexture1D`{.descname} [](#scopes.type.utexture1D "Permalink to this definition"){.headerlink} {#scopes.type.utexture1D}

:   A plain type labeled `<Image uvec4 '1D sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '1D sampled 'Unknown>`.

*type*{.property} `utexture1DArray`{.descname} [](#scopes.type.utexture1DArray "Permalink to this definition"){.headerlink} {#scopes.type.utexture1DArray}

:   A plain type labeled `<Image uvec4 '1D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '1D array sampled 'Unknown>`.

*type*{.property} `utexture2D`{.descname} [](#scopes.type.utexture2D "Permalink to this definition"){.headerlink} {#scopes.type.utexture2D}

:   A plain type labeled `<Image uvec4 '2D sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '2D sampled 'Unknown>`.

*type*{.property} `utexture2DArray`{.descname} [](#scopes.type.utexture2DArray "Permalink to this definition"){.headerlink} {#scopes.type.utexture2DArray}

:   A plain type labeled `<Image uvec4 '2D array sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '2D array sampled 'Unknown>`.

*type*{.property} `utexture2DMS`{.descname} [](#scopes.type.utexture2DMS "Permalink to this definition"){.headerlink} {#scopes.type.utexture2DMS}

:   A plain type labeled `<Image uvec4 '2D ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '2D ms sampled 'Unknown>`.

*type*{.property} `utexture2DMSArray`{.descname} [](#scopes.type.utexture2DMSArray "Permalink to this definition"){.headerlink} {#scopes.type.utexture2DMSArray}

:   A plain type labeled `<Image uvec4 '2D array ms sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '2D array ms sampled 'Unknown>`.

*type*{.property} `utexture2DRect`{.descname} [](#scopes.type.utexture2DRect "Permalink to this definition"){.headerlink} {#scopes.type.utexture2DRect}

:   A plain type labeled `<Image uvec4 'Rect sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 'Rect sampled 'Unknown>`.

*type*{.property} `utexture3D`{.descname} [](#scopes.type.utexture3D "Permalink to this definition"){.headerlink} {#scopes.type.utexture3D}

:   A plain type labeled `<Image uvec4 '3D sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 '3D sampled 'Unknown>`.

*type*{.property} `utextureBuffer`{.descname} [](#scopes.type.utextureBuffer "Permalink to this definition"){.headerlink} {#scopes.type.utextureBuffer}

:   A plain type labeled `<Image uvec4 'Buffer sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 'Buffer sampled 'Unknown>`.

*type*{.property} `utextureCube`{.descname} [](#scopes.type.utextureCube "Permalink to this definition"){.headerlink} {#scopes.type.utextureCube}

:   A plain type labeled `<Image uvec4 'Cube sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 'Cube sampled 'Unknown>`.

*type*{.property} `utextureCubeArray`{.descname} [](#scopes.type.utextureCubeArray "Permalink to this definition"){.headerlink} {#scopes.type.utextureCubeArray}

:   A plain type labeled `<Image uvec4 'Cube array sampled 'Unknown>` of supertype `Image` and of storage type `<Image uvec4 'Cube array sampled 'Unknown>`.

*inline*{.property} `atomicAdd`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicAdd "Permalink to this definition"){.headerlink} {#scopes.inline.atomicAdd}

:   

*inline*{.property} `atomicAnd`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicAnd "Permalink to this definition"){.headerlink} {#scopes.inline.atomicAnd}

:   

*inline*{.property} `atomicCompSwap`{.descname} (*&ensp;mem compare data&ensp;*)[](#scopes.inline.atomicCompSwap "Permalink to this definition"){.headerlink} {#scopes.inline.atomicCompSwap}

:   

*inline*{.property} `atomicExchange`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicExchange "Permalink to this definition"){.headerlink} {#scopes.inline.atomicExchange}

:   

*inline*{.property} `atomicMax`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicMax "Permalink to this definition"){.headerlink} {#scopes.inline.atomicMax}

:   

*inline*{.property} `atomicMin`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicMin "Permalink to this definition"){.headerlink} {#scopes.inline.atomicMin}

:   

*inline*{.property} `atomicOr`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicOr "Permalink to this definition"){.headerlink} {#scopes.inline.atomicOr}

:   

*inline*{.property} `atomicXor`{.descname} (*&ensp;mem data&ensp;*)[](#scopes.inline.atomicXor "Permalink to this definition"){.headerlink} {#scopes.inline.atomicXor}

:   

*inline*{.property} `barrier`{.descname} ()[](#scopes.inline.barrier "Permalink to this definition"){.headerlink} {#scopes.inline.barrier}

:   

*inline*{.property} `groupMemoryBarrier`{.descname} ()[](#scopes.inline.groupMemoryBarrier "Permalink to this definition"){.headerlink} {#scopes.inline.groupMemoryBarrier}

:   

*inline*{.property} `iimage1D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage1D "Permalink to this definition"){.headerlink} {#scopes.inline.iimage1D}

:   

*inline*{.property} `iimage1DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage1DArray "Permalink to this definition"){.headerlink} {#scopes.inline.iimage1DArray}

:   

*inline*{.property} `iimage2D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage2D "Permalink to this definition"){.headerlink} {#scopes.inline.iimage2D}

:   

*inline*{.property} `iimage2DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage2DArray "Permalink to this definition"){.headerlink} {#scopes.inline.iimage2DArray}

:   

*inline*{.property} `iimage2DMS`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage2DMS "Permalink to this definition"){.headerlink} {#scopes.inline.iimage2DMS}

:   

*inline*{.property} `iimage2DMSArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage2DMSArray "Permalink to this definition"){.headerlink} {#scopes.inline.iimage2DMSArray}

:   

*inline*{.property} `iimage2DRect`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage2DRect "Permalink to this definition"){.headerlink} {#scopes.inline.iimage2DRect}

:   

*inline*{.property} `iimage3D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimage3D "Permalink to this definition"){.headerlink} {#scopes.inline.iimage3D}

:   

*inline*{.property} `iimageBuffer`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimageBuffer "Permalink to this definition"){.headerlink} {#scopes.inline.iimageBuffer}

:   

*inline*{.property} `iimageCube`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimageCube "Permalink to this definition"){.headerlink} {#scopes.inline.iimageCube}

:   

*inline*{.property} `iimageCubeArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.iimageCubeArray "Permalink to this definition"){.headerlink} {#scopes.inline.iimageCubeArray}

:   

*inline*{.property} `image1D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image1D "Permalink to this definition"){.headerlink} {#scopes.inline.image1D}

:   

*inline*{.property} `image1DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image1DArray "Permalink to this definition"){.headerlink} {#scopes.inline.image1DArray}

:   

*inline*{.property} `image2D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image2D "Permalink to this definition"){.headerlink} {#scopes.inline.image2D}

:   

*inline*{.property} `image2DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image2DArray "Permalink to this definition"){.headerlink} {#scopes.inline.image2DArray}

:   

*inline*{.property} `image2DMS`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image2DMS "Permalink to this definition"){.headerlink} {#scopes.inline.image2DMS}

:   

*inline*{.property} `image2DMSArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image2DMSArray "Permalink to this definition"){.headerlink} {#scopes.inline.image2DMSArray}

:   

*inline*{.property} `image2DRect`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image2DRect "Permalink to this definition"){.headerlink} {#scopes.inline.image2DRect}

:   

*inline*{.property} `image3D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.image3D "Permalink to this definition"){.headerlink} {#scopes.inline.image3D}

:   

*inline*{.property} `imageBuffer`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.imageBuffer "Permalink to this definition"){.headerlink} {#scopes.inline.imageBuffer}

:   

*inline*{.property} `imageCube`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.imageCube "Permalink to this definition"){.headerlink} {#scopes.inline.imageCube}

:   

*inline*{.property} `imageCubeArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.imageCubeArray "Permalink to this definition"){.headerlink} {#scopes.inline.imageCubeArray}

:   

*inline*{.property} `imageLoad`{.descname} (*&ensp;image coord&ensp;*)[](#scopes.inline.imageLoad "Permalink to this definition"){.headerlink} {#scopes.inline.imageLoad}

:   

*inline*{.property} `imageSize`{.descname} (*&ensp;image&ensp;*)[](#scopes.inline.imageSize "Permalink to this definition"){.headerlink} {#scopes.inline.imageSize}

:   

*inline*{.property} `imageStore`{.descname} (*&ensp;image coord data&ensp;*)[](#scopes.inline.imageStore "Permalink to this definition"){.headerlink} {#scopes.inline.imageStore}

:   

*inline*{.property} `local_size`{.descname} (*&ensp;x y z&ensp;*)[](#scopes.inline.local_size "Permalink to this definition"){.headerlink} {#scopes.inline.local_size}

:   

*inline*{.property} `memoryBarrier`{.descname} ()[](#scopes.inline.memoryBarrier "Permalink to this definition"){.headerlink} {#scopes.inline.memoryBarrier}

:   

*inline*{.property} `memoryBarrierBuffer`{.descname} ()[](#scopes.inline.memoryBarrierBuffer "Permalink to this definition"){.headerlink} {#scopes.inline.memoryBarrierBuffer}

:   

*inline*{.property} `memoryBarrierImage`{.descname} ()[](#scopes.inline.memoryBarrierImage "Permalink to this definition"){.headerlink} {#scopes.inline.memoryBarrierImage}

:   

*inline*{.property} `memoryBarrierShared`{.descname} ()[](#scopes.inline.memoryBarrierShared "Permalink to this definition"){.headerlink} {#scopes.inline.memoryBarrierShared}

:   

*inline*{.property} `texelFetch`{.descname} (*&ensp;sampler P ...&ensp;*)[](#scopes.inline.texelFetch "Permalink to this definition"){.headerlink} {#scopes.inline.texelFetch}

:   

*inline*{.property} `texelFetchOffset`{.descname} (*&ensp;sampler P lod offset&ensp;*)[](#scopes.inline.texelFetchOffset "Permalink to this definition"){.headerlink} {#scopes.inline.texelFetchOffset}

:   

*inline*{.property} `texture`{.descname} (*&ensp;sampler P ...&ensp;*)[](#scopes.inline.texture "Permalink to this definition"){.headerlink} {#scopes.inline.texture}

:   

*inline*{.property} `textureGather`{.descname} (*&ensp;sampler P ...&ensp;*)[](#scopes.inline.textureGather "Permalink to this definition"){.headerlink} {#scopes.inline.textureGather}

:   

*inline*{.property} `textureLod`{.descname} (*&ensp;sampler P lod&ensp;*)[](#scopes.inline.textureLod "Permalink to this definition"){.headerlink} {#scopes.inline.textureLod}

:   

*inline*{.property} `textureOffset`{.descname} (*&ensp;sampler P offset ...&ensp;*)[](#scopes.inline.textureOffset "Permalink to this definition"){.headerlink} {#scopes.inline.textureOffset}

:   

*inline*{.property} `textureProj`{.descname} (*&ensp;sampler P ...&ensp;*)[](#scopes.inline.textureProj "Permalink to this definition"){.headerlink} {#scopes.inline.textureProj}

:   

*inline*{.property} `textureQueryLevels`{.descname} (*&ensp;sampler&ensp;*)[](#scopes.inline.textureQueryLevels "Permalink to this definition"){.headerlink} {#scopes.inline.textureQueryLevels}

:   

*inline*{.property} `textureQueryLod`{.descname} (*&ensp;sampler P&ensp;*)[](#scopes.inline.textureQueryLod "Permalink to this definition"){.headerlink} {#scopes.inline.textureQueryLod}

:   

*inline*{.property} `textureSamples`{.descname} (*&ensp;sampler&ensp;*)[](#scopes.inline.textureSamples "Permalink to this definition"){.headerlink} {#scopes.inline.textureSamples}

:   

*inline*{.property} `textureSize`{.descname} (*&ensp;sampler ...&ensp;*)[](#scopes.inline.textureSize "Permalink to this definition"){.headerlink} {#scopes.inline.textureSize}

:   

*inline*{.property} `uimage1D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage1D "Permalink to this definition"){.headerlink} {#scopes.inline.uimage1D}

:   

*inline*{.property} `uimage1DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage1DArray "Permalink to this definition"){.headerlink} {#scopes.inline.uimage1DArray}

:   

*inline*{.property} `uimage2D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage2D "Permalink to this definition"){.headerlink} {#scopes.inline.uimage2D}

:   

*inline*{.property} `uimage2DArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage2DArray "Permalink to this definition"){.headerlink} {#scopes.inline.uimage2DArray}

:   

*inline*{.property} `uimage2DMS`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage2DMS "Permalink to this definition"){.headerlink} {#scopes.inline.uimage2DMS}

:   

*inline*{.property} `uimage2DMSArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage2DMSArray "Permalink to this definition"){.headerlink} {#scopes.inline.uimage2DMSArray}

:   

*inline*{.property} `uimage2DRect`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage2DRect "Permalink to this definition"){.headerlink} {#scopes.inline.uimage2DRect}

:   

*inline*{.property} `uimage3D`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimage3D "Permalink to this definition"){.headerlink} {#scopes.inline.uimage3D}

:   

*inline*{.property} `uimageBuffer`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimageBuffer "Permalink to this definition"){.headerlink} {#scopes.inline.uimageBuffer}

:   

*inline*{.property} `uimageCube`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimageCube "Permalink to this definition"){.headerlink} {#scopes.inline.uimageCube}

:   

*inline*{.property} `uimageCubeArray`{.descname} (*&ensp;format&ensp;*)[](#scopes.inline.uimageCubeArray "Permalink to this definition"){.headerlink} {#scopes.inline.uimageCubeArray}

:   

*sugar*{.property} (`buffer`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.buffer "Permalink to this definition"){.headerlink} {#scopes.sugar.buffer}

:   

*sugar*{.property} (`fragment_depth`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.fragment_depth "Permalink to this definition"){.headerlink} {#scopes.sugar.fragment_depth}

:   

*sugar*{.property} (`in`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.in "Permalink to this definition"){.headerlink} {#scopes.sugar.in}

:   

*sugar*{.property} (`inout`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.inout "Permalink to this definition"){.headerlink} {#scopes.sugar.inout}

:   

*sugar*{.property} (`inout-geometry`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.inout-geometry "Permalink to this definition"){.headerlink} {#scopes.sugar.inout-geometry}

:   

*sugar*{.property} (`input_primitive`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.input_primitive "Permalink to this definition"){.headerlink} {#scopes.sugar.input_primitive}

:   

*sugar*{.property} (`out`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.out "Permalink to this definition"){.headerlink} {#scopes.sugar.out}

:   

*sugar*{.property} (`output_primitive`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.output_primitive "Permalink to this definition"){.headerlink} {#scopes.sugar.output_primitive}

:   

*sugar*{.property} (`shared`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.shared "Permalink to this definition"){.headerlink} {#scopes.sugar.shared}

:   

*sugar*{.property} (`uniform`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.uniform "Permalink to this definition"){.headerlink} {#scopes.sugar.uniform}

:   

*compiledfn*{.property} `EmitVertex`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.EmitVertex "Permalink to this definition"){.headerlink} {#scopes.compiledfn.EmitVertex}

:   An external function of type `(void <-: ())`.

*compiledfn*{.property} `EndPrimitive`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.EndPrimitive "Permalink to this definition"){.headerlink} {#scopes.compiledfn.EndPrimitive}

:   An external function of type `(void <-: ())`.

*compiledfn*{.property} `packHalf2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.packHalf2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.packHalf2x16}

:   An external function of type `(u32 <-: (vec2))`.

*compiledfn*{.property} `packSnorm2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.packSnorm2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.packSnorm2x16}

:   An external function of type `(u32 <-: (vec2))`.

*compiledfn*{.property} `packSnorm4x8`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.packSnorm4x8 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.packSnorm4x8}

:   An external function of type `(u32 <-: (vec4))`.

*compiledfn*{.property} `packUnorm2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.packUnorm2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.packUnorm2x16}

:   An external function of type `(u32 <-: (vec2))`.

*compiledfn*{.property} `packUnorm4x8`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.packUnorm4x8 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.packUnorm4x8}

:   An external function of type `(u32 <-: (vec4))`.

*compiledfn*{.property} `unpackHalf2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.unpackHalf2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.unpackHalf2x16}

:   An external function of type `(vec2 <-: (u32))`.

*compiledfn*{.property} `unpackSnorm2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.unpackSnorm2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.unpackSnorm2x16}

:   An external function of type `(vec2 <-: (u32))`.

*compiledfn*{.property} `unpackSnorm4x8`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.unpackSnorm4x8 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.unpackSnorm4x8}

:   An external function of type `(vec4 <-: (u32))`.

*compiledfn*{.property} `unpackUnorm2x16`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.unpackUnorm2x16 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.unpackUnorm2x16}

:   An external function of type `(vec2 <-: (u32))`.

*compiledfn*{.property} `unpackUnorm4x8`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.unpackUnorm4x8 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.unpackUnorm4x8}

:   An external function of type `(vec4 <-: (u32))`.

