
using import Map

local map : (Map Symbol i32)
'set map 'A 101
'set map 'B 303
'set map 'C 606
'set map 'D 909
'set map 'E 11
'set map 'F 22
'set map 'G 33
'set map 'H 44
'set map 'I 111
'set map 'J 333
'set map 'K 666
'set map 'L 999
'set map 'M 1111
'set map 'N 2222
'set map 'O 3333
'set map 'P 4444
#print (countof map)
'dump map
inline test-key (key)
    print key ('getdefault map key -1)
test-key 'A
test-key 'B
test-key 'C

;

