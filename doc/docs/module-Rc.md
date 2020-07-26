<style type="text/css" rel="stylesheet">body { counter-reset: chapter 20; }</style>

Rc
==

A reference counted value that is dropped when all users are dropped. This
module provides a strong reference type `Rc`, as well as a weak reference
type `Weak`.

*type*{.property} `Rc`{.descname} [](#scopes.type.Rc "Permalink to this definition"){.headerlink} {#scopes.type.Rc}

:   An opaque type of supertype `ReferenceCounted`.

    *inline*{.property} `__=`{.descname} (*&ensp;selfT otherT&ensp;*)[](#scopes.Rc.inline.__= "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__=}

    :   

    *inline*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.inline.__== "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__==}

    :   

    *inline*{.property} `__@`{.descname} (*&ensp;self keys...&ensp;*)[](#scopes.Rc.inline.__@ "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__@}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.spice.__as "Permalink to this definition"){.headerlink} {#scopes.Rc.spice.__as}

    :   

    *inline*{.property} `__call`{.descname} (*&ensp;self ...&ensp;*)[](#scopes.Rc.inline.__call "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__call}

    :   

    *inline*{.property} `__countof`{.descname} (*&ensp;self&ensp;*)[](#scopes.Rc.inline.__countof "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__countof}

    :   

    *inline*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Rc.inline.__drop "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__drop}

    :   

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.Rc.spice.__getattr}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.Rc.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.Rc.spice.__imply}

    :   

    *spice*{.property} `__methodcall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.spice.__methodcall "Permalink to this definition"){.headerlink} {#scopes.Rc.spice.__methodcall}

    :   

    *inline*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.Rc.inline.__repr "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.__repr}

    :   

    *spice*{.property} `__static-imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.Rc.spice.__static-imply "Permalink to this definition"){.headerlink} {#scopes.Rc.spice.__static-imply}

    :   

    *inline*{.property} `make-cast-op`{.descname} (*&ensp;f const?&ensp;*)[](#scopes.Rc.inline.make-cast-op "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.make-cast-op}

    :   

    *inline*{.property} `new`{.descname} (*&ensp;T args...&ensp;*)[](#scopes.Rc.inline.new "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.new}

    :   

    *inline*{.property} `wrap`{.descname} (*&ensp;value&ensp;*)[](#scopes.Rc.inline.wrap "Permalink to this definition"){.headerlink} {#scopes.Rc.inline.wrap}

    :   

*type*{.property} `UpgradeError`{.descname} [](#scopes.type.UpgradeError "Permalink to this definition"){.headerlink} {#scopes.type.UpgradeError}

:   A plain type of storage type `(tuple )`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls&ensp;*)[](#scopes.UpgradeError.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.UpgradeError.inline.__typecall}

    :   

*type*{.property} `Weak`{.descname} [](#scopes.type.Weak "Permalink to this definition"){.headerlink} {#scopes.type.Weak}

:   An opaque type of supertype `ReferenceCounted`.

    *inline*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Weak.inline.__== "Permalink to this definition"){.headerlink} {#scopes.Weak.inline.__==}

    :   

    *inline*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Weak.inline.__drop "Permalink to this definition"){.headerlink} {#scopes.Weak.inline.__drop}

    :   

    *inline*{.property} `__rimply`{.descname} (*&ensp;T cls&ensp;*)[](#scopes.Weak.inline.__rimply "Permalink to this definition"){.headerlink} {#scopes.Weak.inline.__rimply}

    :   

    *fn*{.property} `_drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Weak.fn._drop "Permalink to this definition"){.headerlink} {#scopes.Weak.fn._drop}

    :   

    *fn*{.property} `force-upgrade`{.descname} (*&ensp;self&ensp;*)[](#scopes.Weak.fn.force-upgrade "Permalink to this definition"){.headerlink} {#scopes.Weak.fn.force-upgrade}

    :   

    *fn*{.property} `upgrade`{.descname} (*&ensp;self&ensp;*)[](#scopes.Weak.fn.upgrade "Permalink to this definition"){.headerlink} {#scopes.Weak.fn.upgrade}

    :   

