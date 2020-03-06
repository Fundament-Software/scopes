#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Option
    ======

    Provides a value type that can be undefined

using import .enum

let extract-payload = Enum.unsafe-extract-payload

typedef UnwrapError : (tuple)
    inline __typecall (cls)
        bitcast none this-type

@@ memo
inline Option (T)
    enum (.. "Option<" (tostring T) ">")
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
            static-if (other-cls == Nothing)
                inline ()
                    this-type.None;
            else (imply? other-cls T)
                inline (self)
                    this-type.Some self

        inline unwrap (self)
            assert self "unwrapping empty Option failed"
            extract-payload self T

        inline try-unwrap (self)
            if self
                extract-payload self T
            else
                raise (UnwrapError)


do
    let Option
    locals;
