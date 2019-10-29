
spice try-call-context (next symbol env obj)
    let T = ('typeof obj)
    let arg =
        try ('@ T (symbol as Symbol))
        except (err)
            return `(next symbol env)
    spice-quote
        inline (args...)
            arg obj args...

run-stage;

inline with-context (obj)
    inline (next symbol env)
        try-call-context next symbol env obj

using import struct

struct TestObj
    x = 303
    y = 606
    z = 909

    fn summary (self)
        + self.x self.y self.z


do
    let testobj = (TestObj)

    chain-typed-symbol-handler
        with-context testobj

    # prints 303 606 909
    assert (('summary testobj) == (summary))
