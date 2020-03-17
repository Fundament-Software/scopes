#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Option
    ======

    Provides a value type that can be undefined

using import .enum

spice option-rimply (other-cls cls T)
    if (other-cls == Nothing)
        return `(inline none-converter () (cls.None))
    other-cls as:= type
    T as:= type
    let conv = (imply-converter other-cls T false)
    if (operator-valid? conv)
        return `(inline (self) (cls.Some (conv self)))
    return `()

run-stage;

let extract-payload = Enum.unsafe-extract-payload

typedef UnwrapError : (tuple)
    inline __typecall (cls)
        bitcast none this-type

@@ memo
inline Option (T)
    T := (unqualified T)

    enum (.. "<Option " (tostring T) ">")
        None
        Some : T

        inline... __typecall
        case (cls : type,)
            this-type.None;
        case (cls : type, value)
            # follow same rules as assignment
            imply value this-type

        inline __tobool (self)
            dispatch self
            case None () false
            default true

        inline swap (self newvalue)
            let value = (deref (dupe self))
            assign (imply newvalue this-type) self
            value

        inline __imply (cls other-cls)
            static-if (T == bool)
                __tobool

        inline __rimply (other-cls cls)
            option-rimply other-cls cls T

        fn force-unwrap (self)
            assert self "unwrapping empty Option failed"
            extract-payload self T

        fn unwrap (self)
            if (not self)
                raise (UnwrapError)
            extract-payload self T

do
    let Option UnwrapError
    locals;
