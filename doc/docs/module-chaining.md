<style type="text/css" rel="stylesheet">body { counter-reset: chapter 10; }</style>

chaining
========

chaining provides the `-->` operator, which allows the nesting of
expressions by chaining them in a sequence.

*sugar*{.property} (`-->`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.--> "Permalink to this definition"){.headerlink} {#scopes.sugar.-->}

:   Expands a processing chain into nested expressions so that each expression
    is passed as a tailing argument to the following expression.
    
    `__` can be used as a placeholder token to position the previous expression.
    
    Example:
    
        :::scopes
        --> x
            f
            g
            h 2 __
            k
    
    Expands to:
    
        :::scopes
        k
            h 2
                g
                    f x

