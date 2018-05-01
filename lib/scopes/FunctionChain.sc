""""FunctionChain
    =============

    A function chain implements a compile-time observer pattern that allows
    a module to call back into dependent modules in a decoupled way.

#-------------------------------------------------------------------------------
# compile time function chaining
#-------------------------------------------------------------------------------

let FunctionChain = (typename "FunctionChain")

typefn FunctionChain 'clear (self)
    """"Clear the function chain. When the function chain is applied next,
        no functions will be called.
    assert (not (type== self FunctionChain))
    assert (constant? self)
    typefn self '__apply-type (self args...)
    self
typefn FunctionChain 'append (self f)
    """"Append function `f` to function chain. When the function chain is called,
        `f` will be called last. The return value of `f` will be ignored.
    assert (not (type== self FunctionChain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__apply-type
    typefn self '__apply-type (self args...)
        oldfn self args...
        f args...
    self
typefn FunctionChain 'prepend (self f)
    """"Prepend function `f` to function chain. When the function chain is called,
        `f` will be called first. The return value of `f` will be ignored.
    assert (not (type== self FunctionChain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__apply-type
    typefn self '__apply-type (self args...)
        f args...
        oldfn self args...
    self

typefn! FunctionChain '__apply-type (cls name)
    let T = (typename (.. "<FunctionChain " name ">"))
    set-typename-super! T cls
    typefn T '__apply-type (cls args...)
    T

define-macro fnchain
    let name = (decons args)
    list let name '=
        list FunctionChain
            list (do ..)
                'module-name
                "."
                name as Syntax as Symbol as string

do
    let FunctionChain fnchain
    locals;
