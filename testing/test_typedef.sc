
# define opaque type MyType
typedef MyType

assert ((typeof MyType) == type)

# define MyHandleType as borrow checked handle of 32-bit integer storage type
typedef MyHandleType :: i32

assert (('storageof MyHandleType) == i32)

# define MyIntType as plain 32-bit integer type with integer supertype
typedef MyIntType : i32 < integer

assert (('storageof MyIntType) == i32)
assert (MyIntType < integer)

# define opaque supertype
typedef MyTupleSuperType

# define subtype and add new methods
typedef MyTupleType < MyTupleSuperType : (tuple i32 i32)
    # in this context, `this-type` is bound to the type we are defining

    # constructor
    @@ ast-quote
    method inline '__typecall (cls x y)
        bitcast
            tupleof (imply x i32) (imply y i32)
            this-type

    # accessor
    method inline 'get (self)
        return
            extractvalue self 0
            extractvalue self 1

run-stage;

let val = (MyTupleType 1 2)
let u v = ('get val)
assert ((u == 1) and (v == 2))



true