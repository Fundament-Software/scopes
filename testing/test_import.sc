
using import testing

import .submod
assert (submod == true)

from (import "glm") let vec3
# glm should not be in local scope
test-compiler-error glm

dump vec3