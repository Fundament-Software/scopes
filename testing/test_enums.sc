
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
        # classic unit tag
        Empty
        # tag name : argument types
        Byte : i8
        # compile time expression
        tag 'Tuple2xi32 (tuple i32 i32)
        TupleXYi :
            x = i32; y = i32; z = i32
        Tuple2xf32 : f32 f32
        Message : string

    test
        (superof sum1) == Enum

    # unit tags must also be instantiated
    test ((typeof (sum1.Empty)) == sum1)
    test (not (constant? (sum1.Empty)))

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

    using sum1
    dispatch-sum1
        Byte 120
    dispatch-sum1
        Tuple2xi32 10 20
    dispatch-sum1
        TupleXYi (y = 20) (x = 10)
    dispatch-sum1
        Message "hello"

do
    # plain enums have i32 storage by default
    enum Implicit plain
        A

    enum Explicit : i32
        A

    test ((storageof Implicit) == (storageof Explicit))

    # only types allowed
    test-compiler-error
        do
            let z = 0
            enum OddlyTyped : z
                A

    # the type after `:` must be a storage type
    test-compiler-error
        enum NonStorageEnum : Implicit
            A

    # enum storage type must be of integer type
    test-compiler-error
        enum PointerTypeEnum : rawstring
            A

    enum ByteEnum : i8
        a   = 0x61
        # ...
        EOS = 0x00

    test ((sizeof ByteEnum.a) == 1:usize)
    let str = ("abcde" as rawstring)
    let count =
        loop (char-count = 0:usize)
            let c = (str @ char-count)
            if (c == ByteEnum.EOS)
                break char-count
            else
                _ (char-count + 1)
    test (count == 5:usize)

do
    enum A
        a : i32
        b

    test (A.a.Literal == 0)
    test (A.a.Name == 'a)
    test (A.b.Literal == 1)
    test (A.b.Name == 'b)

    dump
        arrayof A
            A.a 0
            A.b;

do
    # forward declaration
    enum Options

    # test if payload type has holes
    enum Options
        A : i32 i64 i64
        B : i64 (vector i32 4)

    fn testval (x y z)
        let rad =
            Options.A x y z

        #do
            local radmem = rad
            let ptr = (bitcast &radmem (pointer u8))
            for k in (range (sizeof rad))
                if ((k % 4) == 0)
                    io-write! "|"
                io-write! (hex (deref (ptr @ k)))
                io-write! " "
            io-write! "\n"

        do
            dispatch rad
            case A (a b c)
                test (a == x)
                test (b == y) # fails
                test (c == z)
            default;

    testval 0x1234 0x5678 0xabcd

do
    # previously, duplicated tags would be innaccessible (except for the last defined one); now
      it causes an error.
    test-compiler-error
        enum DupedEnum
            A
            A : i32 i32
            B
            C
    test-compiler-error
        enum DupedCEnum plain
            A
            B
            B
            C
            B
            C

do
    # comparing and hashing unique enums
    enum Atom
        Empty1
        Empty2
        Number : i32 i32
        Var : Symbol

    test ((Atom.Empty1) == (Atom.Empty1))
    test ((Atom.Empty1) != (Atom.Empty2))
    test ((Atom.Number 1 2) != (Atom.Number 2 2))
    test ((Atom.Number 1 3) == (Atom.Number 1 3))

    test ((hash (Atom.Empty1)) == (hash (Atom.Empty1)))
    test ((hash (Atom.Empty1)) != (hash (Atom.Empty2)))
    test ((hash (Atom.Number 1 2)) != (hash (Atom.Number 2 2)))
    test ((hash (Atom.Number 1 3)) == (hash (Atom.Number 1 3)))

;

