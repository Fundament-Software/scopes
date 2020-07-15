<style type="text/css" rel="stylesheet">body { counter-reset: chapter 16; }</style>

Map
===

This module implements a key -> value store using hashtables.

*type*{.property} `Map`{.descname} [](#scopes.type.Map "Permalink to this definition"){.headerlink} {#scopes.type.Map}

:   An opaque type of supertype `Struct`.

    *inline*{.property} `__as`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.Map.inline.__as "Permalink to this definition"){.headerlink} {#scopes.Map.inline.__as}

    :   

    *inline*{.property} `__countof`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.inline.__countof "Permalink to this definition"){.headerlink} {#scopes.Map.inline.__countof}

    :   

    *fn*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.fn.__drop "Permalink to this definition"){.headerlink} {#scopes.Map.fn.__drop}

    :   

    *inline*{.property} `__rin`{.descname} (*&ensp;...&ensp;*)[](#scopes.Map.inline.__rin "Permalink to this definition"){.headerlink} {#scopes.Map.inline.__rin}

    :   

    *inline*{.property} `__tobool`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.inline.__tobool "Permalink to this definition"){.headerlink} {#scopes.Map.inline.__tobool}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls opts...&ensp;*)[](#scopes.Map.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Map.inline.__typecall}

    :   

    *fn*{.property} `clear`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.fn.clear "Permalink to this definition"){.headerlink} {#scopes.Map.fn.clear}

    :   

    *fn*{.property} `discard`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Map.fn.discard "Permalink to this definition"){.headerlink} {#scopes.Map.fn.discard}

    :   erases a key -> value association from the map; if the map
        does not contain this key, nothing happens.

    *fn*{.property} `dump`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.fn.dump "Permalink to this definition"){.headerlink} {#scopes.Map.fn.dump}

    :   

    *fn*{.property} `get`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Map.fn.get "Permalink to this definition"){.headerlink} {#scopes.Map.fn.get}

    :   returns the value associated with key or raises an error

    *fn*{.property} `getdefault`{.descname} (*&ensp;self key value&ensp;*)[](#scopes.Map.fn.getdefault "Permalink to this definition"){.headerlink} {#scopes.Map.fn.getdefault}

    :   returns the value associated with key or raises an error

    *fn*{.property} `in?`{.descname} (*&ensp;self key&ensp;*)[](#scopes.Map.fn.in? "Permalink to this definition"){.headerlink} {#scopes.Map.fn.in?}

    :   

    *fn*{.property} `set`{.descname} (*&ensp;self key value&ensp;*)[](#scopes.Map.fn.set "Permalink to this definition"){.headerlink} {#scopes.Map.fn.set}

    :   inserts a new key -> value association into map; key can be the
        output of any custom hash function. If the key already exists,
        it will be updated.

    *fn*{.property} `terseness`{.descname} (*&ensp;self&ensp;*)[](#scopes.Map.fn.terseness "Permalink to this definition"){.headerlink} {#scopes.Map.fn.terseness}

    :   computes the hashmap load as a normal between 0.0 and 1.0

*type*{.property} `MapError`{.descname} [](#scopes.type.MapError "Permalink to this definition"){.headerlink} {#scopes.type.MapError}

:   An unique type of supertype `Enum` and of storage type `(tuple u8 (tuple (vector i8 1)))`.

    *inline*{.property} `__==`{.descname} (*&ensp;A B&ensp;*)[](#scopes.MapError.inline.__== "Permalink to this definition"){.headerlink} {#scopes.MapError.inline.__==}

    :   

    *fn*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.MapError.fn.__drop "Permalink to this definition"){.headerlink} {#scopes.MapError.fn.__drop}

    :   

    *fn*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.MapError.fn.__hash "Permalink to this definition"){.headerlink} {#scopes.MapError.fn.__hash}

    :   

    *fn*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.MapError.fn.__repr "Permalink to this definition"){.headerlink} {#scopes.MapError.fn.__repr}

    :   

