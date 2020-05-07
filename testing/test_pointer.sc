
using import testing

# testing pointer cast

local value = 1
local value2 = 2

# reference to pointer
let value = &value

# implicitly convert stack pointer to universal/heap pointer
let value = (imply value @i32)
# implicitly convert pointer to void pointer
let anyvalue = (imply value voidstar)

# explicitly convert pointer to another pointer
value := value as @f32

# explicitly convert pointer to intptr and back
ivalue := value as intptr
vvalue := ivalue as voidstar
test (vvalue == value)
test (vvalue != &value2)

;