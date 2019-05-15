
using import testing
using import enum

spice make-enum (val)
    spice-quote
        enum [(.. (val as string) "-enum")] plain
            X
            Y = 5
            Z
            W = -1
            tag 'Q
            R

run-stage;

do
    enum Mode plain 
        \ Notch Low High Band Peak Count

    print Mode.Notch
    print Mode.Low
    print Mode.Band

do
    enum test-enum plain
        X
        Y = 5
        Z
        W = 25
        Q
        R

    test (test-enum.Y | test-enum.W == 29)

    test ((superof test-enum) == CEnum)
    test ((storageof test-enum) == i32)

    test ((typeof test-enum.X) == test-enum)
    test (test-enum.X == 0)
    test (test-enum.Y == 5)
    test (test-enum.Z == 6)
    test (test-enum.W == 25)
    test (test-enum.Q == 26)
    test (test-enum.R == 27)

    test ((test-enum.R != test-enum.R) == false)
    test ((test-enum.R == test-enum.R) == true)
    test (not (test-enum.X == test-enum.Y))
    test (test-enum.X != test-enum.Y)

do
    let T =
        make-enum "test2"

    test
        (superof T) == CEnum
    test
        (storageof T) == i32

    test ((typeof T.X) == T)
    test
        T.X == 0
    test
        T.Y == 5
    test
        T.Z == 6
    test
        T.W == -1
    test
        T.Q == 0
    test
        T.R == 1
    
    print T.Q T.X

do
    # sum | tagged enum type
    enum sum1
        # classic tag
        Empty
        # tag name : field type
        Byte : i8
        Tuple2xi32 : (tuple i32 i32)
        TupleXYi : (tuple (x = i32) (y = i32) (z = i32))
        Tuple2xf32 : (tuple f32 f32)
        Message : string

    test
        (superof sum1) == Enum

    test ((typeof sum1.Empty) == sum1)
    test (constant? sum1.Empty)

    fn dispatch-sum1 (val)
        dispatch val
        case Empty ()
            print "got empty!"
        case Byte (x)
            print "got byte" x
        case Tuple2xi32 (x y)
            print "got Tuple2xi32" x y
        case TupleXYi (z y x)
            print "got TupleXYi" x y z
        case Tuple2xf32 (x y)
            print "got Tuple2xf32" x y
        case Message (x)
            print "got Message" x
        default
            print "default!"

    dispatch-sum1
        sum1.Byte 120
    dispatch-sum1
        sum1.Tuple2xi32 10 20
    dispatch-sum1
        sum1.TupleXYi (y = 20) (x = 10)
    dispatch-sum1 
        sum1.Message "hello"

;