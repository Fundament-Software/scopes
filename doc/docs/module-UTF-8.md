<style type="text/css" rel="stylesheet">body { counter-reset: chapter 26; }</style>

UTF-8
=====

This module provides UTF-8 encoder and decoder collectors, as well as
a UTF-8 aware `char` function.

*inline*{.property} `decoder`{.descname} (*&ensp;coll&ensp;*)[](#scopes.inline.decoder "Permalink to this definition"){.headerlink} {#scopes.inline.decoder}

:   Convert a char character stream as UTF-8 codepoints of type i32.
    Invalid bytes are forwarded as negative numbers; negating the number
    yields the offending byte character.

*inline*{.property} `encoder`{.descname} (*&ensp;coll&ensp;*)[](#scopes.inline.encoder "Permalink to this definition"){.headerlink} {#scopes.inline.encoder}

:   Convert an integer codepoint to char bytes;
    the collector forwards a byte at a time.

*spice*{.property} `char32`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.char32 "Permalink to this definition"){.headerlink} {#scopes.spice.char32}

:   

