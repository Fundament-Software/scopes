
# we build a new integer type at runtime from scratch
let T = (typename "T")
'set-plain-storage T i32

# constructor can't be typed because T is a variable outside function scope.
fn constructor (cls x)
    bitcast x T

# this pseudoquotes(!) a function - we're actually not declaring constructor2
    right away, but instead generate the API calls that build a new function
    object at runtime. T is constant at runtime and can be properly inlined.
fn! constructor2 (cls x)
    bitcast x T

# we assign constructor2 to T at runtime, and now our type is complete
'set-symbol T '__typecall constructor2

# at this point we can't call constructor2 yet because it's not been generated
    and compiled yet.

# run the code until here, and update the namespace with the evaluated values
run-stage;

# we can now construct T and invoke constructor2
print (T 5)

