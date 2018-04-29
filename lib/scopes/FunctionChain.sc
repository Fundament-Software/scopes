
#-------------------------------------------------------------------------------
# compile time function chaining
#-------------------------------------------------------------------------------

let FunctionChain = (typename "FunctionChain")

typefn FunctionChain 'clear (self)
    assert (not (type== self FunctionChain))
    assert (constant? self)
    typefn self '__apply-type (self args...)
    self
typefn FunctionChain 'append (self f)
    assert (not (type== self FunctionChain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__apply-type
    typefn self '__apply-type (self args...)
        oldfn self args...
        f args...
    self
typefn FunctionChain 'prepend (self f)
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
    list let name '= (list FunctionChain (name as Syntax as Symbol as string))

do
    let FunctionChain fnchain
    locals;
