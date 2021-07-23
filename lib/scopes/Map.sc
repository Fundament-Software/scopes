#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Map
    ===

    This module implements a key -> value store using hashtables.

# generic Hashmap implementing Robin Hood Hashing, see
     http://sebastiansylvan.com/2013/05/08/robin-hood-hashing-should-be-your-default-hash-table-implementation/

using import enum
using import struct

# declare void @llvm.memcpy.p0i8.p0i8.i64(i8* <dest>, i8* <src>,
                                        i64 <len>, i1 <isvolatile>)
let llvm.memcpy.p0i8.p0i8.i64 =
    extern 'llvm.memcpy.p0i8.p0i8.i64
        function void (mutable rawstring) rawstring i64 bool

fn... addpos
case (a : u64, b : u64, mask : u64)
    (a + b) & mask

fn... nextpos
case (k : u64, mask : u64)
    addpos k 1:u64 mask

fn... prevpos
case (k : u64, mask : u64)
    addpos k mask mask

fn... keydistance
case (a : u64, b : u64, mask : u64)
    (b + (mask + 1:u64) - (a & mask)) & mask

fn... keypos
case (k : u64, mask : u64)
    k & mask

enum MapError
    KeyNotFound

typedef Map < Struct
    let MinCapacity = 16:u64
    let MinMask = (MinCapacity - 1:u64)
    let BitfieldType = u64

    @@ memo
    inline gen-type (key-type value-type hash-function)
        let parent-type = this-type
        let hash-function =
            static-if (none? hash-function) hash
            else hash-function
        struct (.. "<Map " (tostring key-type) "=" (tostring value-type) ">") < parent-type
            let KeyType = key-type
            let ValueType = value-type
            let HashFunction = hash-function

            _valid : (mutable pointer BitfieldType)
            _keys : (mutable pointer KeyType)
            _values : (mutable pointer ValueType)
            _count : u64
            _mask : u64
            _capacity : u64

    fn set-slot (self idx)
        let flag = (1:u64 << (idx % 64:u64))
        let ofs = (idx // 64:u64)
        self._valid @ ofs |= flag
        ;

    fn unset-slot (self idx)
        let flag = (1:u64 << (idx % 64:u64))
        let ofs = (idx // 64:u64)
        self._valid @ ofs &= (~ flag)
        ;

    fn valid-slot? (self idx)
        let flag = (1:u64 << (idx % 64:u64))
        let ofs = (idx // 64:u64)
        ((self._valid @ ofs) & flag) == flag

    fn terseness (self)
        """"Computes the hashmap load as a normal between 0.0 and 1.0.
        self._count / (self._mask + 1:u64)

    inline insert_entry (self key keyhash value mask)
        let hash = ((typeof self) . HashFunction)
        let mask =
            static-if (none? mask) (deref self._mask)
            else mask
        assert (self._count <= mask) "map full"
        local key = key
        local keyhash = keyhash
        local value = value
        let capacity = (mask + 1:u64)
        let pos = (keypos keyhash mask)
        loop (i dist = 0:u64 0:u64)
            if (i == capacity)
                break;
            let index = (addpos pos i mask)
            if (valid-slot? self index) # already occupied
                let pos_key = (self._keys @ index)
                let pos_keyhash = ((hash (deref pos_key)) as u64)
                let pd = (keydistance pos_keyhash index mask)
                repeat (i + 1:u64)
                    + 1:u64
                        if (dist > pd)
                            # swap out
                            let pos_value = (self._values @ index)
                            swap pos_key (view key)
                            swap pos_value (view value)
                            keyhash = pos_keyhash
                            dupe pd
                        else
                            dist
            else # free
                set-slot self index
                assign key (self._keys @ index)
                assign value (self._values @ index)
                self._count += 1
                break;

    inline erase_pos (self pos mask)
        let hash = ((typeof self) . HashFunction)
        let mask =
            static-if (none? mask) self._mask
            else mask
        let capacity = (mask + 1:u64)
        let key value =
            dupe (deref (self._keys @ pos))
            dupe (deref (self._values @ pos))
        label done
            loop (i = 1:u64)
                if (i == capacity)
                    merge done
                let index = (addpos pos i mask)
                let index_prev = (prevpos index mask)
                let atkey = (self._keys @ index)
                let atvalue = (self._values @ index)
                let prev_key = (self._keys @ index_prev)
                let prev_value = (self._values @ index_prev)
                let pd = (keydistance ((hash atkey) as u64) index mask)
                if ((pd == 0) or (not (valid-slot? self index)))
                    unset-slot self index_prev
                    merge done
                swap atkey prev_key
                swap atvalue prev_value
                i + 1:u64
        self._count = self._count - 1:u32
        _ key value

    inline lookup (self key keyhash successf failf mask)
        """"Finds the index and address of an entry associated with key or
            invokes label failf on failure.
        let hash = ((typeof self) . HashFunction)
        let mask =
            static-if (none? mask) (deref self._mask)
            else mask
        loop (pos dist = (keypos keyhash mask) 0:u64)
            if (not (valid-slot? self pos))
                return (failf)
            let pos_key = (deref (self._keys @ pos))
            if (pos_key == key)
                return (successf pos)
            elseif (dist > (keydistance ((hash pos_key) as u64) pos mask))
                return (failf)
            repeat (nextpos pos mask) (dist + 1:u64)

    fn rehash (self newmask)
        let hash = ((typeof self) . HashFunction)
        let oldmask = (deref self._mask)
        self._mask = newmask
        let mask =
            max oldmask newmask
        # try simplest thing: reinsert slots not at their correct position
        for i in (range 0:u64 (mask + 1:u64))
            if (not (valid-slot? self i))
                continue;
            let key value = (self._keys @ i) (self._values @ i)
            let keyhash = ((hash key) as u64)
            lookup self key keyhash
                inline "ok" (idx)
                    assert (idx == i)
                inline "fail" ()
                    unset-slot self i
                    self._count -= 1:u64
                    # extract as new uniques
                    let key = (dupe (deref key))
                    let value = (dupe (deref value))
                    insert_entry self key keyhash value
                newmask

    fn reserve (self new-capacity)
        let cls = (typeof self)
        let capacity = (deref self._capacity)
        let old-valid = (deref self._valid)
        let old-keys = (deref self._keys)
        let old-values = (deref self._values)
        let validsize = ((capacity + 63:u64) // 64:u64)
        let new-validsize = ((new-capacity + 63:u64) // 64:u64)
        let new-valid = (malloc-array BitfieldType new-validsize)
        let new-keys = (malloc-array cls.KeyType new-capacity)
        let new-values = (malloc-array cls.ValueType new-capacity)
        llvm.memcpy.p0i8.p0i8.i64
            bitcast (view new-valid) (mutable rawstring)
            bitcast (view old-valid) rawstring
            (validsize * (sizeof BitfieldType)) as i64
            false
        llvm.memcpy.p0i8.p0i8.i64
            bitcast (view new-keys) (mutable rawstring)
            bitcast (view old-keys) rawstring
            (capacity * (sizeof cls.KeyType)) as i64
            false
        llvm.memcpy.p0i8.p0i8.i64
            bitcast (view new-values) (mutable rawstring)
            bitcast (view old-values) rawstring
            (capacity * (sizeof cls.ValueType)) as i64
            false
        for i in (range validsize new-validsize)
            new-valid @ i = 0:u64
        free old-valid
        free old-keys
        free old-values
        assign new-valid self._valid
        assign new-keys self._keys
        assign new-values self._values
        self._capacity = new-capacity
        return;

    fn auto-rehash (self)
        let l = (terseness self)
        let maxmask = (self._capacity - 1:u64)
        if (l >= 0.9)
            if (self._mask >= maxmask)
                # we must expand capacity
                reserve self (self._capacity << 1:u64)
            # expand
            rehash self ((self._mask << 1:u64) + 1:u64)
        elseif ((l <= 0.225) and (self._mask > MinCapacity))
            # compact
            rehash self (self._mask >> 1:u64)

    fn clear (self)
        for i in (range 0:u64 (self._mask + 1:u64))
            if (valid-slot? self i)
                unset-slot self i
                __drop (self._keys @ i)
                __drop (self._values @ i)
        self._count = 0:u64
        self._mask = MinMask
        return;

    fn set (self key value)
        """"Inserts a new key -> value association into map; key can be the
            output of any custom hash function. If the key already exists,
            it will be updated.
        let hash = ((typeof self) . HashFunction)
        let keyhash = ((hash key) as u64)
        lookup self key keyhash
            inline "ok" (idx)
                self._values @ idx = value
                return;
            inline "fail" ()
                insert_entry self key keyhash value
                auto-rehash self
                return;

    inline key-value-generator (self)
        inline next (i)
            let fin = (deref self._mask)
            loop (i = i)
                let i = (i + 1:u64)
                if (i > fin)
                    break i
                if (valid-slot? self i)
                    break i
                i
        Generator
            inline ()
                if (valid-slot? self 0:u64) 0:u64
                else (next 0:u64)
            inline (i) (i <= self._mask)
            inline (i)
                _ (deref (self._keys @ i)) (deref (self._values @ i))
            next

    inline __as (cls T)
        static-if (T == Generator)
            key-value-generator
        else
            ;

    fn in? (self key)
        let hash = ((typeof self) . HashFunction)
        lookup self key ((hash key) as u64)
            inline "ok" (idx) true
            inline "fail" () false

    @@ memo
    inline __rin (elemT cls)
        let KeyType = cls.KeyType
        static-if (imply? elemT KeyType)
            inline (key self)
                in? self (imply key KeyType)

    fn getdefault (self key value)
        """"Returns the value associated with key or raises an error.
        let hash = ((typeof self) . HashFunction)
        lookup self key ((hash key) as u64)
            inline "ok" (idx)
                return (deref (self._values @ idx))
            inline "fail" ()
                return (view value)

    fn get (self key)
        """"Returns the value associated with key or raises an error.
        let hash = ((typeof self) . HashFunction)
        lookup self key ((hash key) as u64)
            inline "ok" (idx)
                return (self._values @ idx)
            inline "fail" ()
                raise (MapError.KeyNotFound)

    fn discard (self key)
        """"Erases a key -> value association from the map; if the map does not
            contain this key, nothing happens.
        let hash = ((typeof self) . HashFunction)
        lookup self key ((hash key) as u64)
            inline "ok" (idx)
                erase_pos self idx
                auto-rehash self
                return;
            inline "fail" ()
                return;

    fn pop (self key)
        """"Erases a key -> value association from the map and pops the old value
        let hash = ((typeof self) . HashFunction)
        lookup self key ((hash key) as u64)
            inline "ok" (idx)
                let k v = (erase_pos self idx)
                auto-rehash self
                return v
            inline "fail" ()
                raise (MapError.KeyNotFound)

    inline __tobool (self)
        self._count != 0:usize

    inline __countof (self)
        (deref self._count) as usize

    fn __drop (self)
        for i in (range 0:u64 (self._mask + 1:u64))
            if (valid-slot? self i)
                __drop (self._keys @ i)
                __drop (self._values @ i)
        free self._valid
        free self._keys
        free self._values

    fn __copy (self)
        local other : (typeof self)
        for k v in self
            'set other (copy k) (copy v)
        other

    inline __typecall (cls opts...)
        static-if (cls == this-type)
            let key-type value-type function-type = opts...
            gen-type key-type value-type function-type
        else
            let numsets = ((MinCapacity + 63:u64) // 64:u64)
            let validset = (malloc-array BitfieldType numsets)
            for i in (range 0:u64 numsets)
                validset @ i = 0:u64
            let self =
                Struct.__typecall cls
                    _valid = validset
                    _keys = (malloc-array cls.KeyType MinCapacity)
                    _values = (malloc-array cls.ValueType MinCapacity)
                    _count = 0:usize
                    _mask = MinMask
                    _capacity = MinCapacity
            self

    fn dump (self)
        for i in (range 0:u64 (self._mask + 1:u64))
            if (valid-slot? self i)
                print i (self._keys @ i) "=" (self._values @ i)
            else
                print i "<empty>"
        print "terseness" (terseness self) "mask" self._mask "count" self._count


    unlet unset-slot rehash auto-rehash lookup insert_entry reserve
        \ erase_pos key-value-generator gen-type set-slot valid-slot?

do
    let Map MapError
    locals;


