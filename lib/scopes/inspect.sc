
inline block-generator (self)
    count := sc_block_instruction_count self
    Generator
        inline () 0
        inline (i) (i < count)
        inline (i) (sc_block_get_instruction self i)
        inline (i) (+ i 1)

type+ SCILBlock
    inline __imply (cls T)
        static-if (T == Generator) block-generator

    __countof := sc_block_instruction_count
    terminator := sc_block_terminator

type+ Value
    inline icall-args (self)
        count := sc_icall_argcount self
        Generator
            inline () 0
            inline (i) (i < count)
            inline (i) (sc_icall_getarg self i)
            inline (i) (+ i 1)

    function-get-body := sc_function_get_body

    icall-callee := sc_icall_callee
    icall-argcount := sc_icall_argcount
    icall-getarg := sc_icall_getarg
    icall-exception-body := sc_icall_exception_body
    icall-exception-type := sc_icall_exception_type

do
    locals;
