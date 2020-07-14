<style type="text/css" rel="stylesheet">body { counter-reset: chapter 9; }</style>

Capture
=======

A capture is a runtime closure that transparently captures (hence the name)
runtime values outside of the function.

*type*{.property} `Capture`{.descname} [](#scopes.type.Capture "Permalink to this definition"){.headerlink} {#scopes.type.Capture}

:   An opaque type.

    *inline*{.property} `__call`{.descname} (*&ensp;self args...&ensp;*)[](#scopes.Capture.inline.__call "Permalink to this definition"){.headerlink} {#scopes.Capture.inline.__call}

    :   

    *inline*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Capture.inline.__drop "Permalink to this definition"){.headerlink} {#scopes.Capture.inline.__drop}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls args...&ensp;*)[](#scopes.Capture.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Capture.inline.__typecall}

    :   

    *inline*{.property} `function`{.descname} (*&ensp;return-type param-types...&ensp;*)[](#scopes.Capture.inline.function "Permalink to this definition"){.headerlink} {#scopes.Capture.inline.function}

    :   

    *inline*{.property} `make-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.Capture.inline.make-type "Permalink to this definition"){.headerlink} {#scopes.Capture.inline.make-type}

    :   

*type*{.property} `CaptureTemplate`{.descname} [](#scopes.type.CaptureTemplate "Permalink to this definition"){.headerlink} {#scopes.type.CaptureTemplate}

:   An opaque type.

    *inline*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.CaptureTemplate.inline.__drop "Permalink to this definition"){.headerlink} {#scopes.CaptureTemplate.inline.__drop}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.CaptureTemplate.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.CaptureTemplate.spice.__imply}

    :   

    *inline*{.property} `build-instance`{.descname} (*&ensp;self f&ensp;*)[](#scopes.CaptureTemplate.inline.build-instance "Permalink to this definition"){.headerlink} {#scopes.CaptureTemplate.inline.build-instance}

    :   

    *inline*{.property} `instance`{.descname} (*&ensp;self types...&ensp;*)[](#scopes.CaptureTemplate.inline.instance "Permalink to this definition"){.headerlink} {#scopes.CaptureTemplate.inline.instance}

    :   

    *inline*{.property} `typify-function`{.descname} (*&ensp;cls types...&ensp;*)[](#scopes.CaptureTemplate.inline.typify-function "Permalink to this definition"){.headerlink} {#scopes.CaptureTemplate.inline.typify-function}

    :   

*type*{.property} `SpiceCapture`{.descname} [](#scopes.type.SpiceCapture "Permalink to this definition"){.headerlink} {#scopes.type.SpiceCapture}

:   An opaque type.

*sugar*{.property} (`capture`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.capture "Permalink to this definition"){.headerlink} {#scopes.sugar.capture}

:   

*sugar*{.property} (`decorate-capture`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-capture "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-capture}

:   

*sugar*{.property} (`spice-capture`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.spice-capture "Permalink to this definition"){.headerlink} {#scopes.sugar.spice-capture}

:   

