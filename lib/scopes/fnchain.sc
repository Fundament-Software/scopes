
#-------------------------------------------------------------------------------
# compile time function chaining
#-------------------------------------------------------------------------------

define fnchain (typename "fnchain" (fn ()))

typefn fnchain 'clear (self)
    assert (not (type== self fnchain))
    assert (constant? self)
    typefn self '__apply-type (self args...)
    self
typefn fnchain 'append (self f)
    assert (not (type== self fnchain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__apply-type
    typefn self '__apply-type (self args...)
        oldfn self args...
        f args...
    self
typefn fnchain 'prepend (self f)
    assert (not (type== self fnchain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__apply-type
    typefn self '__apply-type (self args...)
        f args...
        oldfn self args...
    self

typefn! fnchain '__apply-type (cls name)
    let T = (typename (.. "<fnchain " name ">"))
    set-typename-super! T cls
    typefn T '__apply-type (cls args...)
    T

fnchain
