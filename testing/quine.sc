(do (syntax-extend (set-scope-symbol! syntax-scope (quote source) (list-load module-path)) syntax-scope) (print (slice (Any-string (Any (source as Syntax as list))) 1 -1)))
