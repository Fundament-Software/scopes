
using import Map

local map : (Map Symbol i32)
inline iter-pairs1 (f)
    f 'A 101
    f 'B 303
    f 'C 606
    f 'D 909
    f 'E 11
    f 'F 22
    f 'G 33
    f 'H 44
inline iter-pairs2 (f)
    f 'I 111
    f 'J 333
    f 'K 666
    f 'L 999
    f 'M 1111
    f 'N 2222
    f 'O 3333
    f 'P 4444
inline iter-pairs (f)
    iter-pairs1 f
    iter-pairs2 f

#print (countof map)
iter-pairs
    inline (key value)
        'set map key value
'dump map
iter-pairs
    inline (key value)
        if (('getdefault map key -1) == -1)
            print key value
            error "key missing"
iter-pairs1
    inline (key value)
        'discard map key
iter-pairs1
    inline (key value)
        if (('getdefault map key -1) != -1)
            print key value
            error "key not deleted"
iter-pairs2
    inline (key value)
        if (('getdefault map key -1) == -1)
            print key value
            error "key missing"
iter-pairs2
    inline (key value)
        'discard map key
iter-pairs2
    inline (key value)
        if (('getdefault map key -1) != -1)
            print key value
            error "key not deleted"

;


