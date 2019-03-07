
assert ('match? "^(t.st)?(t.st)?(t.st)?$" "tisttosttust")
assert (not ('match? "^(t.st)?(t.st)?(t.st)?$" "tisttozt"))

let s = "^(gl(.+)|GL(.+))$"
loop (i = 0)
    if (i < 20000)
        assert ('match? s "GL_UNIFORM_BUFFER_EXT")
        assert ('match? s "GL_CURRENT_MATRIX_INDEX_ARB")
        assert ('match? s "GL_DOT4_ATI")
        repeat (i + 1)
    break;

