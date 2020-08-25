
using import testing

# define opaque type MyType
typedef MyType

test ((typeof MyType) == type)

# define MyHandleType as borrow checked handle of 32-bit integer storage type
typedef MyHandleType :: i32

test (('storageof MyHandleType) == i32)

# define MyIntType as plain 32-bit integer type with integer supertype
typedef MyIntType < integer : i32

test (('storageof MyIntType) == i32)
test (MyIntType < integer)

# define MyIntCopy as 32-bit integer type with the same supertype
typedef MyIntCopy <:: i32
test (('storageof MyIntCopy) == i32)
test (MyIntCopy < integer)

let name = "RuntimeType"
let RuntimeType =
    @@ spice-quote
    typedef [name] < [(opaque "new")] : i32
        inline func () true

test (not (constant? RuntimeType))

test-compiler-error
    'func RuntimeType

typedef K
    let x y = (decons '(1 2 3))
    let keys... = (decons '(1 2 3))

# define opaque supertype
typedef MyTupleSuperType

# define subtype and add new methods
typedef MyTupleType < MyTupleSuperType : (tuple i32 i32)
    # constructor
    inline __typecall (cls x y)
        bitcast
            tupleof (imply x i32) (imply y i32)
            this-type

    static-assert (super-type == MyTupleSuperType)

    # trivial accessor implementation
    let get =
        Accessor
            inline (self key)
                _
                    extractvalue self 0
                    extractvalue self 1

# recursive definition
typedef SelfRefType < tuple : (tuple i32 (pointer this-type))
static-assert (constant? SelfRefType)

test (('superof SelfRefType) == tuple)
test (('storageof SelfRefType) == (tuple.type i32 (pointer SelfRefType)))

do
    let val = (MyTupleType 1 2)
    let u v = val.get
    assert ((u == 1) and (v == 2))

run-stage;

print K.x K.y K.keys...

print RuntimeType (superof RuntimeType)
test ('func RuntimeType)

spice make-type (name u v)
    @@ spice-quote
    typedef [(name as string)]
        let x = u
        let y = v

run-stage;

let T = (make-type "test" 1 2)
print T T.x T.y


typedef+ i32
    static-assert (super-type == integer)

true