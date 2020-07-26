<style type="text/css" rel="stylesheet">body { counter-reset: chapter 21; }</style>

Set
===

This module implements mathematical sets using hashtables.

*type*{.property} `Set`{.descname} [](#scopes.type.Set "Permalink to this definition"){.headerlink} {#scopes.type.Set}

:   An opaque type of supertype `Struct`.

    *inline*{.property} `__as`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.Set.inline.__as "Permalink to this definition"){.headerlink} {#scopes.Set.inline.__as}

    :   

    *inline*{.property} `__countof`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.inline.__countof "Permalink to this definition"){.headerlink} {#scopes.Set.inline.__countof}

    :   

    *fn*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.fn.__drop "Permalink to this definition"){.headerlink} {#scopes.Set.fn.__drop}

    :   

    *inline*{.property} `__rin`{.descname} (*&ensp;...&ensp;*)[](#scopes.Set.inline.__rin "Permalink to this definition"){.headerlink} {#scopes.Set.inline.__rin}

    :   

    *inline*{.property} `__tobool`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.inline.__tobool "Permalink to this definition"){.headerlink} {#scopes.Set.inline.__tobool}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls opts...&ensp;*)[](#scopes.Set.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Set.inline.__typecall}

    :   

    *fn*{.property} `clear`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.fn.clear "Permalink to this definition"){.headerlink} {#scopes.Set.fn.clear}

    :   

    *fn*{.property} `discard`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Set.fn.discard "Permalink to this definition"){.headerlink} {#scopes.Set.fn.discard}

    :   Erases a key -> value association from the map; if the map does not
        contain this key, nothing happens.

    *fn*{.property} `dump`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.fn.dump "Permalink to this definition"){.headerlink} {#scopes.Set.fn.dump}

    :   

    *fn*{.property} `get`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Set.fn.get "Permalink to this definition"){.headerlink} {#scopes.Set.fn.get}

    :   Returns the value associated with key or raises an error.

    *fn*{.property} `getdefault`{.descname} (*&ensp;self key value&ensp;*)[](#scopes.Set.fn.getdefault "Permalink to this definition"){.headerlink} {#scopes.Set.fn.getdefault}

    :   Returns the value associated with key or value if the map does not
        contain the key.

    *fn*{.property} `in?`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Set.fn.in? "Permalink to this definition"){.headerlink} {#scopes.Set.fn.in?}

    :   

    *fn*{.property} `insert`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Set.fn.insert "Permalink to this definition"){.headerlink} {#scopes.Set.fn.insert}

    :   Inserts a new key into set.

    *fn*{.property} `pop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.fn.pop "Permalink to this definition"){.headerlink} {#scopes.Set.fn.pop}

    :   Discards an arbitrary key from the set and returns the discarded key.

    *fn*{.property} `terseness`{.descname} (*&ensp;self&ensp;*)[](#scopes.Set.fn.terseness "Permalink to this definition"){.headerlink} {#scopes.Set.fn.terseness}

    :   Computes the hashmap load as a normal between 0.0 and 1.0.

