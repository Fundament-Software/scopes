
# use http://falutin.net/iching/ to learn more

include
    import C
""""#include <stdlib.h>
    #include <time.h>

C.srand
    u32
        C.time null

fn random ()
    (C.rand) / (i32 C.RAND_MAX)

fn print-hexagram (hexagram moving)
    for i in (range 6)
        let tag = (0x20 >> i)
        let moving? = ((moving & tag) != 0)
        if (hexagram & tag)
            if moving?
                print "=========== o"
            else
                print "==========="
        else
            if moving?
                print "====   ==== o"
            else
                print "====   ===="

fn get-text (hexagram)
    switch hexagram
    case 0b111111
        """"1. Ch'ien / The Creative
    case 0b000000
        """"2. K'un / The Receptive
    case 0b010001
        """"3. Chun / Difficulty at the Beginning
    case 0b100010
        """"4. Mêng / Youthful Folly
    case 0b010111
        """"5. Hsü / Waiting
    case 0b111010
        """"6. Sung / Conflict
    case 0b000010
        """"7. Shih / The Army
    case 0b010000
        """"8. Pi / Holding Together
    case 0b110111
        """"9. Hsiao Ch'u / Taming Power of the Small
    case 0b111011
        """"10. Lü / Treading
    case 0b000111
        """"11. T'ai / Peace
    case 0b111000
        """"12. P'i / Standstill
    case 0b111101
        """"13. T'ung Jên / Fellowship with Men
    case 0b101111
        """"14. Ta Yu / Possession in Great Measure
    case 0b000100
        """"15. Ch'ien / Modesty
    case 0b001000
        """"16. Yü / Enthusiasm
    case 0b011001
        """"17. Sui / Following
    case 0b100110
        """"18. Ku / Work on what has been spoiled
    case 0b000011
        """"19. Lin / Approach
    case 0b110000
        """"20. Kuan / Contemplation
    case 0b101001
        """"21. Shih Ho / Biting Through
    case 0b100101
        """"22. Pi / Grace
    case 0b100000
        """"23. Po / Splitting Apart
    case 0b000001
        """"24. Fu / Return
    case 0b111001
        """"25. Wu Wang / Innocence
    case 0b100111
        """"26. Ta Ch'u / Taming Power of the Great
    case 0b100001
        """"27. I / Corners of the Mouth
    case 0b011110
        """"28. Ta Kuo / Preponderance of the Great
    case 0b010010
        """"29. K'an / The Abysmal
    case 0b101101
        """"30. Li / The Clinging, Fire
    case 0b011100
        """"31. Hsien / Influence
    case 0b001110
        """"32. Hêng / Duration
    case 0b111100
        """"33. Tun / Retreat
    case 0b001111
        """"34. Ta Chuang / The Power of the Great
    case 0b101000
        """"35. Chin / Progress
    case 0b000101
        """"36. Ming I / Darkening of the light
    case 0b110101
        """"37. Chia Jên / The Family
    case 0b101011
        """"38. K'uei / Opposition
    case 0b010100
        """"39. Chien / Obstruction
    case 0b001010
        """"40. Hsieh / Deliverance
    case 0b100011
        """"41. Sun / Decrease
    case 0b110001
        """"42. I / Increase
    case 0b011111
        """"43. Kuai / Break-through
    case 0b111110
        """"44. Kou / Coming to Meet
    case 0b011000
        """"45. Ts'ui / Gathering Together
    case 0b000110
        """"46. Shêng / Pushing Upward
    case 0b011010
        """"47. K'un / Oppression
    case 0b010110
        """"48. Ching / The Well
    case 0b011101
        """"49. Ko / Revolution
    case 0b101110
        """"50. Ting / The Cauldron
    case 0b001001
        """"51. Chên / The Arousing
    case 0b100100
        """"52. Kên / Keeping Still, Mountain
    case 0b110100
        """"53. Chien / Development
    case 0b001011
        """"54. Kuei Mei/The Marrying Maiden
    case 0b001101
        """"55. Fêng / Abundance
    case 0b101100
        """"56. Lü / The Wanderer
    case 0b110110
        """"57. Sun / The Gentle
    case 0b011011
        """"58. Tui / The Joyous, Lake
    case 0b110010
        """"59. Huan / Dispersion
    case 0b010011
        """"60. Chieh / Limitation
    case 0b110011
        """"61. Chung Fu / Inner Truth
    case 0b001100
        """"62. Hsiao Kuo / Preponderance of the Small
    case 0b010101
        """"63. Chi Chi / After Completion
    case 0b101010
        """"64. Wei Chi / Before Completion
    default
        .. "unknown hexagram: " (bin hexagram)

let ctpop =
    extern 'llvm.ctpop.i32 (function i32 i32)

fn print-moving-lines (hexagram moving)
    # alfred huang rules
        https://en.wikibooks.org/wiki/I_Ching/The_Moving_Line
    let count = (ctpop moving)
    if (count == 6)
        print "all lines change"
        return;
    fn print-line-changed (i)
        print "change in line" (i + 1) "(counting bottom to top)"
    fn print-nth-result (moving n)
        local found = 0
        for i in (range 6)
            let tag = (1 << i)
            if (moving & tag)
                found += 1
                if (found == n)
                    print-line-changed i
                    return;
    if (count == 1)
        print-nth-result moving 1
    elseif (count == 2)
        # If there are two moving lines - one Yin and the other Yang - consult only the Yin moving line
        moving-yin := moving & (hexagram ^ 63)
        let count-yin = (ctpop moving-yin)
        if (count-yin == 1)
            print-nth-result moving-yin 1
            return;
        # If the two moving lines are both Yin or both Yang, consult the lower one
        print-nth-result moving 1
    elseif (count == 3)
        # If there are three moving lines, consult only the middle one
        print-nth-result moving 2
    elseif (count == 4)
        # If there are four moving lines, consult only the upper of the two non-moving lines
        let moving = (moving ^ 63)
        print-nth-result moving 2
    elseif (count == 5)
        # If there are five moving lines, consult only the other, non-moving line
        let moving = (moving ^ 63)
        print-nth-result moving 1


let hexagram moving =
    fold (hexagram moving = 0 0) for i in (range 6)
        let c =
            random;
        let tag = (0x20 >> i)
        if (c < 0.25)
            _ hexagram moving
        elseif (c < 0.5)
            _ hexagram
                moving | tag
        elseif (c < 0.75)
            _
                hexagram | tag
                moving
        else
            _
                hexagram | tag
                moving | tag

#let hexagram moving = 0b100111 0b11111

print-hexagram hexagram moving
print;
print
    get-text hexagram
if (moving != 0)
    print-moving-lines hexagram moving
    print;
    let hexagram = (hexagram ^ moving)
    print-hexagram hexagram 0
    print;
    print
        get-text hexagram
