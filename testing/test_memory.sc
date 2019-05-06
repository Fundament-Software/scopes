struct Handle
    _handle : voidstar

    # constructor
    inline __typecall (cls handle)
        super-type.__typecall cls handle

    # destructor
    inline __drop (self)
        print "destroying handle"
        free self._handle
        return;

do
    Handle (malloc i32)

;