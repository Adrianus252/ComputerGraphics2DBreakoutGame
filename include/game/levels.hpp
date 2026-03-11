#pragma once
#include <array>
#include <cstddef>

// 10 predefined level layouts with 10 columns and 11 rows
//   '0' represents empty space, '1'..'9' represent colored bricks
//   '1' blue, '2' light green, '3' purple, '4' red, '5' orange, '6' light blue, '7' yellow, '8' green, '9' brown

namespace BreakoutLevels {

    static inline const std::array<std::array<const char*, 11>, 10> layouts = {

        // Level 1: Full Wall - Solid rectangular pattern filling the top half
        std::array<const char*, 11>{
            "1111111111",
            "2222222222",
            "3333333333",
            "4444444444",
            "5555555555",
            "6666666666",
            "7777777777",
            "8888888888",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 2: Zig-Zag - Alternating pattern
        std::array<const char*, 11>{
            "1010101010",
            "0101010101",
            "6060606060",
            "0606060606",
            "6060606060",
            "0606060606",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 3: Stripes - Horizontal stripes with gaps
        std::array<const char*, 11>{
            "1111111111",
            "0000000000",
            "3333333333",
            "0000000000",
            "5555555555",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 4: Smiley - Happy face 
        std::array<const char*, 11>{
            "0011111100",
            "0200000020",
            "0203003020",
            "0200000020",
            "0205555020",
            "0066666600",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 5: Skull - Skull Head
        std::array<const char*, 11>{
            "0055555500",
            "0555555550",
            "5500550055",
            "5555555555",
            "5555505555",
            "0555555550",
            "0505050505",
            "0505050505",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 6: Diamond - Diamond shape
        std::array<const char*, 11>{
            "0000660000",
            "0006666000",
            "0066222600",
            "0662222260",
            "6622222226",
            "0662222260",
            "0066222600",
            "0006666000",
            "0000660000",
            "0000000000",
            "0000000000"
        },
        // Level 7: Scatter - Diagonal scattered pattern
        std::array<const char*, 11>{
            "3710291054",
            "8402650792",
            "1063907340",
            "0524089106",
            "9370145068",
            "6208360471",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 8: Pillars - Four corner and a center block
        std::array<const char*, 11>{
            "1100000033",
            "1100000033",
            "0004444000",
            "0004444000",
            "5500000099",
            "5500000099",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 9: Bars - Vertical bars pattern with alternating heights
        std::array<const char*, 11>{
            "1020302010",
            "1020302010",
            "1020302010",
            "0404050404",
            "0404050404",
            "0404050404",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000",
            "0000000000"
        },
        // Level 10: Plus - Plus/cross symbol pattern
        std::array<const char*, 11>{
            "0000330000",
            "0000330000",
            "0033333300",
            "0033333300",
            "0033333300",
            "0033333300",
            "0033333300",
            "0000330000",
            "0000330000",
            "0000000000",
            "0000000000"
        }
    };

    // Level Names Array: used in the level select menu and game UI to display the current level
    static inline const std::array<const char*, 10> names = {
        "Full Wall",
        "Zig-Zag",
        "Stripes",
        "Smiley",
        "Skull",
        "Diamond",
        "Scatter",
        "Pillars",
        "Bars",
        "Plus"
    };
}
