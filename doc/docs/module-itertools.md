<style type="text/css" rel="stylesheet">body { counter-reset: chapter 15; }</style>

itertools
=========

itertools provides various utilities which simplify the composition of
generators and collectors.

*define*{.property} `drain`{.descname} [](#scopes.define.drain "Permalink to this definition"){.headerlink} {#scopes.define.drain}

:   A constant of type `Collector`.

*inline*{.property} `->>`{.descname} (*&ensp;generator collector...&ensp;*)[](#scopes.inline.->> "Permalink to this definition"){.headerlink} {#scopes.inline.->>}

:   

*inline*{.property} `cascade`{.descname} (*&ensp;collector...&ensp;*)[](#scopes.inline.cascade "Permalink to this definition"){.headerlink} {#scopes.inline.cascade}

:   two collectors:
    every time a is full, b collects a and a is reset
    when b ends, the remainder of a is collected

*inline*{.property} `cat`{.descname} (*&ensp;coll&ensp;*)[](#scopes.inline.cat "Permalink to this definition"){.headerlink} {#scopes.inline.cat}

:   treat input as a generator and forward its arguments individually

*inline*{.property} `collect`{.descname} (*&ensp;coll&ensp;*)[](#scopes.inline.collect "Permalink to this definition"){.headerlink} {#scopes.inline.collect}

:   run collector until full and return the result

*inline*{.property} `demux`{.descname} (*&ensp;init-value f collector...&ensp;*)[](#scopes.inline.demux "Permalink to this definition"){.headerlink} {#scopes.inline.demux}

:   

*inline*{.property} `each`{.descname} (*&ensp;generator collector&ensp;*)[](#scopes.inline.each "Permalink to this definition"){.headerlink} {#scopes.inline.each}

:   fold output from generator into collector

*inline*{.property} `filter`{.descname} (*&ensp;f coll&ensp;*)[](#scopes.inline.filter "Permalink to this definition"){.headerlink} {#scopes.inline.filter}

:   

*inline*{.property} `flatten`{.descname} (*&ensp;coll&ensp;*)[](#scopes.inline.flatten "Permalink to this definition"){.headerlink} {#scopes.inline.flatten}

:   collect variadic input as individual single items

*inline*{.property} `gate`{.descname} (*&ensp;f a b&ensp;*)[](#scopes.inline.gate "Permalink to this definition"){.headerlink} {#scopes.inline.gate}

:   if f is true, collect input in a, otherwise collect in b
    when both are full, output both
    until then, new input for full containers is discarded

*inline*{.property} `imap`{.descname} (*&ensp;gen f&ensp;*)[](#scopes.inline.imap "Permalink to this definition"){.headerlink} {#scopes.inline.imap}

:   

*inline*{.property} `ipair`{.descname} (*&ensp;gen N&ensp;*)[](#scopes.inline.ipair "Permalink to this definition"){.headerlink} {#scopes.inline.ipair}

:   generate one variadic argument from N generated arguments

*inline*{.property} `limit`{.descname} (*&ensp;f coll&ensp;*)[](#scopes.inline.limit "Permalink to this definition"){.headerlink} {#scopes.inline.limit}

:   

*inline*{.property} `map`{.descname} (*&ensp;f coll&ensp;*)[](#scopes.inline.map "Permalink to this definition"){.headerlink} {#scopes.inline.map}

:   

*inline*{.property} `mux`{.descname} (*&ensp;collector...&ensp;*)[](#scopes.inline.mux "Permalink to this definition"){.headerlink} {#scopes.inline.mux}

:   send input into multiple collectors which each fork the target collector

*inline*{.property} `permutate-range`{.descname} (*&ensp;...&ensp;*)[](#scopes.inline.permutate-range "Permalink to this definition"){.headerlink} {#scopes.inline.permutate-range}

:   Return a generator that iterates all permutations of the range from 0
    to `n`, where `n` must be smaller than 256, and returns a vector of
    `element-type` for each iteration. If `element-type` is omitted, the
    default element type will be i32.
    
    The generator will perform `n!` iterations to complete.

*inline*{.property} `reduce`{.descname} (*&ensp;init f&ensp;*)[](#scopes.inline.reduce "Permalink to this definition"){.headerlink} {#scopes.inline.reduce}

:   

*inline*{.property} `take`{.descname} (*&ensp;n coll&ensp;*)[](#scopes.inline.take "Permalink to this definition"){.headerlink} {#scopes.inline.take}

:   limit collector to output n items

*spice*{.property} `compose`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.compose "Permalink to this definition"){.headerlink} {#scopes.spice.compose}

:   

*spice*{.property} `join`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.join "Permalink to this definition"){.headerlink} {#scopes.spice.join}

:   

*spice*{.property} `span`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.span "Permalink to this definition"){.headerlink} {#scopes.spice.span}

:   

*spice*{.property} `zip`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.zip "Permalink to this definition"){.headerlink} {#scopes.spice.zip}

:   

