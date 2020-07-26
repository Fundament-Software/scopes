<style type="text/css" rel="stylesheet">body { counter-reset: chapter 25; }</style>

testing
=======

The testing module simplifies writing and running tests in an ad-hoc
fashion.

*type*{.property} `One`{.descname} [](#scopes.type.One "Permalink to this definition"){.headerlink} {#scopes.type.One}

:   This type is used for discovering leaks and double frees. It holds an
    integer value as well as a pointer to a single reference on the heap which
    is 1 as long as the object exists, otherwise 0. The refcount is leaked in
    order to not cause segfaults when a double free occurs.
    
    In addition, a global refcounter is updated which can be checked for
    balance.

    *inline*{.property} `__!=`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__!= "Permalink to this definition"){.headerlink} {#scopes.One.inline.__!=}

    :   

    *inline*{.property} `__<`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__< "Permalink to this definition"){.headerlink} {#scopes.One.inline.__<}

    :   

    *inline*{.property} `__<=`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__<= "Permalink to this definition"){.headerlink} {#scopes.One.inline.__<=}

    :   

    *inline*{.property} `__==`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__== "Permalink to this definition"){.headerlink} {#scopes.One.inline.__==}

    :   

    *inline*{.property} `__>`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__> "Permalink to this definition"){.headerlink} {#scopes.One.inline.__>}

    :   

    *inline*{.property} `__>=`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.One.inline.__>= "Permalink to this definition"){.headerlink} {#scopes.One.inline.__>=}

    :   

    *fn*{.property} `__drop`{.descname} (*&ensp;self&ensp;*)[](#scopes.One.fn.__drop "Permalink to this definition"){.headerlink} {#scopes.One.fn.__drop}

    :   

    *fn*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.One.fn.__repr "Permalink to this definition"){.headerlink} {#scopes.One.fn.__repr}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls value&ensp;*)[](#scopes.One.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.One.inline.__typecall}

    :   

    *fn*{.property} `check`{.descname} (*&ensp;self&ensp;*)[](#scopes.One.fn.check "Permalink to this definition"){.headerlink} {#scopes.One.fn.check}

    :   

    *fn*{.property} `refcount`{.descname} ()[](#scopes.One.fn.refcount "Permalink to this definition"){.headerlink} {#scopes.One.fn.refcount}

    :   

    *fn*{.property} `reset-refcount`{.descname} ()[](#scopes.One.fn.reset-refcount "Permalink to this definition"){.headerlink} {#scopes.One.fn.reset-refcount}

    :   

    *fn*{.property} `test-refcount-balanced`{.descname} ()[](#scopes.One.fn.test-refcount-balanced "Permalink to this definition"){.headerlink} {#scopes.One.fn.test-refcount-balanced}

    :   

    *fn*{.property} `value`{.descname} (*&ensp;self&ensp;*)[](#scopes.One.fn.value "Permalink to this definition"){.headerlink} {#scopes.One.fn.value}

    :   

*sugar*{.property} (`features`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.features "Permalink to this definition"){.headerlink} {#scopes.sugar.features}

:   A feature matrix that tests 2-d permutations
    
    Usage:
    
        :::scopes
        features    B1  B2  B3 ...
            ---
            A1      Y   N   Y
            A2      N   Y   N
            A3      Y   N   Q
    
    Will expand to:
    
        :::scopes
        do
            Y A1 B1; N A1 B2; Y A1 B3
            N A2 B1; Y A2 B2; N A2 B3
            Y A3 B1; N A3 B2; Q A3 B3

*sugar*{.property} (`test`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.test "Permalink to this definition"){.headerlink} {#scopes.sugar.test}

:   

*sugar*{.property} (`test-compiler-error`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.test-compiler-error "Permalink to this definition"){.headerlink} {#scopes.sugar.test-compiler-error}

:   

*sugar*{.property} (`test-error`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.test-error "Permalink to this definition"){.headerlink} {#scopes.sugar.test-error}

:   

*sugar*{.property} (`test-modules`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.test-modules "Permalink to this definition"){.headerlink} {#scopes.sugar.test-modules}

:   

