
using import testing

# define opaque type MyType
typedef MyType

assert ((typeof MyType) == type)

# define MyHandleType as borrow checked handle of 32-bit integer storage type
typedef MyHandleType :: i32

assert (('storageof MyHandleType) == i32)

# define MyIntType as plain 32-bit integer type with integer supertype
typedef MyIntType < integer : i32

assert (('storageof MyIntType) == i32)
assert (MyIntType < integer)

let RuntimeType =
    typedef "RuntimeType" < integer : i32
        inline func () true

assert (not (constant? RuntimeType))

assert-compiler-error
    'func RuntimeType

# define opaque supertype
typedef MyTupleSuperType

# define subtype and add new methods
typedef MyTupleType < MyTupleSuperType : (tuple i32 i32)
    # constructor
    inline __typecall (cls x y)
        bitcast
            tupleof (imply x i32) (imply y i32)
            this-type

    # accessor
    inline get (self)
        return
            extractvalue self 0
            extractvalue self 1

# recursive definition
typedef SelfRefType < tuple : (tuple i32 (pointer this-type))
static-assert (constant? SelfRefType)

assert (('superof SelfRefType) == tuple)
assert (('storageof SelfRefType) == (tuple.type i32 (pointer SelfRefType)))

let val = (MyTupleType 1 2)
let u v = ('get val)
assert ((u == 1) and (v == 2))

run-stage;

assert ('func RuntimeType)


true