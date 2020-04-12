#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""property
    ========

    Provides a property accessor for types

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

            inline __imply (cls destT)
                static-if (imply? value-type destT)
                    inline (self)
                        imply (getter (bitcast self T)) destT

            inline __unpack (self)
                unpack (getter (bitcast self T))

            inline __typecall (cls value)
                bitcast (view value) cls

            inline __getattr (self key)
                getattr (getter (bitcast self T)) key

            inline __= (cls destT)
                inline (self value)
                    setter (bitcast self T) value

    Accessor
        inline "generate-property" (value key)
            (property-type (typeof value) key) value

do
    let property Property
    locals;
