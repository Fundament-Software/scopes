
sugar-quote
    -1.0 1.0 +1.0 1.0 -1.0:f32 1.0:f32 +1.0:f32 1.0:f32
    -0x30 0x30 +0x30 0x30
    -0b10 0b01 +0x30 0x30
    -20 20 +20 20
    -1e-5 1e-5 +1e-5 1e-5

do
    assert
        ==
            sugar-quote
                1 2; 3 4
            '((1 2) (3 4))

    assert
        ==
            sugar-quote
                1 2; 3
            '((1 2) 3)

    assert
        ==
            sugar-quote
                1 2; 3; 4 5
            '((1 2) (3) (4 5))

true
