*spice*{.property} `format`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.format "Permalink to this definition"){.headerlink} {#scopes.spice.format}

:   format
    ======
    
    Support for agnostic string formatting through `format`.
    
    See the following examples:
    
        :::scopes
        using import format
        using import String
    
        # prints Hello World!
        print
            .. (format "{} {}!" "Hello" "World")
    
        # prints fizzbuzzbuzzfizzfizz
        print
            String (format "{0}buzz{1}fizz{0}" "fizz" "buzz")
    
        # prints * Joana (42)
        print
            vvv String
            format "* {name} ({age})"
                name = "Joana"
                age = 42

