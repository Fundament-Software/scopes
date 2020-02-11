
using import testing

using import struct
using import property

struct IntOrFloat
    float : f32

    int :=
        property
            inline "getter" (self)
                self.float as i32
            inline "setter" (self value)
                self.float = value as f32

local iof = (IntOrFloat 3.0)

print iof.float
print iof.int
test (iof.float == 3.0)
test (iof.int == 3)
iof.int = 11
test (iof.float == 11.0)
test (iof.int == 11)

;

