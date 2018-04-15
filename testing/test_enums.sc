
do
    enum test-enum
        X
        Y = 5
        Z
        W = 25
        Q
        R

    assert ((superof test-enum) == CEnum)
    assert ((storageof test-enum) == i32)

    assert ((typeof test-enum.X) == test-enum)
    assert (test-enum.X == 0)
    assert (test-enum.Y == 5)
    assert (test-enum.Z == 6)
    assert (test-enum.W == 25)
    assert (test-enum.Q == 26)
    assert (test-enum.R == 27)

do
    let T =
        enum (do "test-enum") X
            Y = 5
            Z
            W = -1
            'Q
            R

    assert
        (superof T) == CEnum
    assert
        (storageof T) == i32

    assert ((typeof T.X) == T)
    assert
        T.X == 0
    assert
        T.Y == 5
    assert
        T.Z == 6
    assert
        T.W == -1
    assert
        T.Q == 0
    assert
        T.R == 1
