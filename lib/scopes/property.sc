#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""property
    ========

    Provides a property accessor for types.

typedef Property

@@ memo
inline property (getter setter)
    @@ memo
    inline property-type (T key)
        typedef (.. (tostring T) "." (tostring key)) < Property :: (storageof T)

            define value-type
                fn test-getter (value)
                    getter value
                let F = (static-typify test-getter T)
                unqualified (returnof (typeof F))

            inline get (self)
                getter (bitcast (view self) T)

            inline __imply (cls destT)
                static-if (destT == immutable) get
                elseif (imply? value-type destT)
                    inline (self)
                        imply (get self) destT

            inline __unpack (self)
                unpack (get self)

            inline __typecall (cls value)
                bitcast (view value) cls

            inline __getattr (self key)
                getattr (get self) key

            inline __toptr (self)
                & (get self)

            inline __toref (self)
                @ (get self)

            inline __methodcall (name self ...)
                name (get self) ...

            inline __= (cls destT)
                inline (self value)
                    setter (bitcast (view self) T) value

    Accessor
        inline "generate-property" (value key)
            (property-type (typeof value) key) value

do
    let property Property
    locals;
