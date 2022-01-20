#ifndef __TRACY_HPP__
#define __TRACY_HPP__


/*** Start of inlined file: TracyColor.hpp ***/
#ifndef __TRACYCOLOR_HPP__
#define __TRACYCOLOR_HPP__

namespace tracy
{
struct Color
{
enum ColorType
{
    Snow = 0xfffafa,
    GhostWhite = 0xf8f8ff,
    WhiteSmoke = 0xf5f5f5,
    Gainsboro = 0xdcdcdc,
    FloralWhite = 0xfffaf0,
    OldLace = 0xfdf5e6,
    Linen = 0xfaf0e6,
    AntiqueWhite = 0xfaebd7,
    PapayaWhip = 0xffefd5,
    BlanchedAlmond = 0xffebcd,
    Bisque = 0xffe4c4,
    PeachPuff = 0xffdab9,
    NavajoWhite = 0xffdead,
    Moccasin = 0xffe4b5,
    Cornsilk = 0xfff8dc,
    Ivory = 0xfffff0,
    LemonChiffon = 0xfffacd,
    Seashell = 0xfff5ee,
    Honeydew = 0xf0fff0,
    MintCream = 0xf5fffa,
    Azure = 0xf0ffff,
    AliceBlue = 0xf0f8ff,
    Lavender = 0xe6e6fa,
    LavenderBlush = 0xfff0f5,
    MistyRose = 0xffe4e1,
    White = 0xffffff,
    Black = 0x000000,
    DarkSlateGray = 0x2f4f4f,
    DarkSlateGrey = 0x2f4f4f,
    DimGray = 0x696969,
    DimGrey = 0x696969,
    SlateGray = 0x708090,
    SlateGrey = 0x708090,
    LightSlateGray = 0x778899,
    LightSlateGrey = 0x778899,
    Gray = 0xbebebe,
    Grey = 0xbebebe,
    X11Gray = 0xbebebe,
    X11Grey = 0xbebebe,
    WebGray = 0x808080,
    WebGrey = 0x808080,
    LightGrey = 0xd3d3d3,
    LightGray = 0xd3d3d3,
    MidnightBlue = 0x191970,
    Navy = 0x000080,
    NavyBlue = 0x000080,
    CornflowerBlue = 0x6495ed,
    DarkSlateBlue = 0x483d8b,
    SlateBlue = 0x6a5acd,
    MediumSlateBlue = 0x7b68ee,
    LightSlateBlue = 0x8470ff,
    MediumBlue = 0x0000cd,
    RoyalBlue = 0x4169e1,
    Blue = 0x0000ff,
    DodgerBlue = 0x1e90ff,
    DeepSkyBlue = 0x00bfff,
    SkyBlue = 0x87ceeb,
    LightSkyBlue = 0x87cefa,
    SteelBlue = 0x4682b4,
    LightSteelBlue = 0xb0c4de,
    LightBlue = 0xadd8e6,
    PowderBlue = 0xb0e0e6,
    PaleTurquoise = 0xafeeee,
    DarkTurquoise = 0x00ced1,
    MediumTurquoise = 0x48d1cc,
    Turquoise = 0x40e0d0,
    Cyan = 0x00ffff,
    Aqua = 0x00ffff,
    LightCyan = 0xe0ffff,
    CadetBlue = 0x5f9ea0,
    MediumAquamarine = 0x66cdaa,
    Aquamarine = 0x7fffd4,
    DarkGreen = 0x006400,
    DarkOliveGreen = 0x556b2f,
    DarkSeaGreen = 0x8fbc8f,
    SeaGreen = 0x2e8b57,
    MediumSeaGreen = 0x3cb371,
    LightSeaGreen = 0x20b2aa,
    PaleGreen = 0x98fb98,
    SpringGreen = 0x00ff7f,
    LawnGreen = 0x7cfc00,
    Green = 0x00ff00,
    Lime = 0x00ff00,
    X11Green = 0x00ff00,
    WebGreen = 0x008000,
    Chartreuse = 0x7fff00,
    MediumSpringGreen = 0x00fa9a,
    GreenYellow = 0xadff2f,
    LimeGreen = 0x32cd32,
    YellowGreen = 0x9acd32,
    ForestGreen = 0x228b22,
    OliveDrab = 0x6b8e23,
    DarkKhaki = 0xbdb76b,
    Khaki = 0xf0e68c,
    PaleGoldenrod = 0xeee8aa,
    LightGoldenrodYellow = 0xfafad2,
    LightYellow = 0xffffe0,
    Yellow = 0xffff00,
    Gold = 0xffd700,
    LightGoldenrod = 0xeedd82,
    Goldenrod = 0xdaa520,
    DarkGoldenrod = 0xb8860b,
    RosyBrown = 0xbc8f8f,
    IndianRed = 0xcd5c5c,
    SaddleBrown = 0x8b4513,
    Sienna = 0xa0522d,
    Peru = 0xcd853f,
    Burlywood = 0xdeb887,
    Beige = 0xf5f5dc,
    Wheat = 0xf5deb3,
    SandyBrown = 0xf4a460,
    Tan = 0xd2b48c,
    Chocolate = 0xd2691e,
    Firebrick = 0xb22222,
    Brown = 0xa52a2a,
    DarkSalmon = 0xe9967a,
    Salmon = 0xfa8072,
    LightSalmon = 0xffa07a,
    Orange = 0xffa500,
    DarkOrange = 0xff8c00,
    Coral = 0xff7f50,
    LightCoral = 0xf08080,
    Tomato = 0xff6347,
    OrangeRed = 0xff4500,
    Red = 0xff0000,
    HotPink = 0xff69b4,
    DeepPink = 0xff1493,
    Pink = 0xffc0cb,
    LightPink = 0xffb6c1,
    PaleVioletRed = 0xdb7093,
    Maroon = 0xb03060,
    X11Maroon = 0xb03060,
    WebMaroon = 0x800000,
    MediumVioletRed = 0xc71585,
    VioletRed = 0xd02090,
    Magenta = 0xff00ff,
    Fuchsia = 0xff00ff,
    Violet = 0xee82ee,
    Plum = 0xdda0dd,
    Orchid = 0xda70d6,
    MediumOrchid = 0xba55d3,
    DarkOrchid = 0x9932cc,
    DarkViolet = 0x9400d3,
    BlueViolet = 0x8a2be2,
    Purple = 0xa020f0,
    X11Purple = 0xa020f0,
    WebPurple = 0x800080,
    MediumPurple = 0x9370db,
    Thistle = 0xd8bfd8,
    Snow1 = 0xfffafa,
    Snow2 = 0xeee9e9,
    Snow3 = 0xcdc9c9,
    Snow4 = 0x8b8989,
    Seashell1 = 0xfff5ee,
    Seashell2 = 0xeee5de,
    Seashell3 = 0xcdc5bf,
    Seashell4 = 0x8b8682,
    AntiqueWhite1 = 0xffefdb,
    AntiqueWhite2 = 0xeedfcc,
    AntiqueWhite3 = 0xcdc0b0,
    AntiqueWhite4 = 0x8b8378,
    Bisque1 = 0xffe4c4,
    Bisque2 = 0xeed5b7,
    Bisque3 = 0xcdb79e,
    Bisque4 = 0x8b7d6b,
    PeachPuff1 = 0xffdab9,
    PeachPuff2 = 0xeecbad,
    PeachPuff3 = 0xcdaf95,
    PeachPuff4 = 0x8b7765,
    NavajoWhite1 = 0xffdead,
    NavajoWhite2 = 0xeecfa1,
    NavajoWhite3 = 0xcdb38b,
    NavajoWhite4 = 0x8b795e,
    LemonChiffon1 = 0xfffacd,
    LemonChiffon2 = 0xeee9bf,
    LemonChiffon3 = 0xcdc9a5,
    LemonChiffon4 = 0x8b8970,
    Cornsilk1 = 0xfff8dc,
    Cornsilk2 = 0xeee8cd,
    Cornsilk3 = 0xcdc8b1,
    Cornsilk4 = 0x8b8878,
    Ivory1 = 0xfffff0,
    Ivory2 = 0xeeeee0,
    Ivory3 = 0xcdcdc1,
    Ivory4 = 0x8b8b83,
    Honeydew1 = 0xf0fff0,
    Honeydew2 = 0xe0eee0,
    Honeydew3 = 0xc1cdc1,
    Honeydew4 = 0x838b83,
    LavenderBlush1 = 0xfff0f5,
    LavenderBlush2 = 0xeee0e5,
    LavenderBlush3 = 0xcdc1c5,
    LavenderBlush4 = 0x8b8386,
    MistyRose1 = 0xffe4e1,
    MistyRose2 = 0xeed5d2,
    MistyRose3 = 0xcdb7b5,
    MistyRose4 = 0x8b7d7b,
    Azure1 = 0xf0ffff,
    Azure2 = 0xe0eeee,
    Azure3 = 0xc1cdcd,
    Azure4 = 0x838b8b,
    SlateBlue1 = 0x836fff,
    SlateBlue2 = 0x7a67ee,
    SlateBlue3 = 0x6959cd,
    SlateBlue4 = 0x473c8b,
    RoyalBlue1 = 0x4876ff,
    RoyalBlue2 = 0x436eee,
    RoyalBlue3 = 0x3a5fcd,
    RoyalBlue4 = 0x27408b,
    Blue1 = 0x0000ff,
    Blue2 = 0x0000ee,
    Blue3 = 0x0000cd,
    Blue4 = 0x00008b,
    DodgerBlue1 = 0x1e90ff,
    DodgerBlue2 = 0x1c86ee,
    DodgerBlue3 = 0x1874cd,
    DodgerBlue4 = 0x104e8b,
    SteelBlue1 = 0x63b8ff,
    SteelBlue2 = 0x5cacee,
    SteelBlue3 = 0x4f94cd,
    SteelBlue4 = 0x36648b,
    DeepSkyBlue1 = 0x00bfff,
    DeepSkyBlue2 = 0x00b2ee,
    DeepSkyBlue3 = 0x009acd,
    DeepSkyBlue4 = 0x00688b,
    SkyBlue1 = 0x87ceff,
    SkyBlue2 = 0x7ec0ee,
    SkyBlue3 = 0x6ca6cd,
    SkyBlue4 = 0x4a708b,
    LightSkyBlue1 = 0xb0e2ff,
    LightSkyBlue2 = 0xa4d3ee,
    LightSkyBlue3 = 0x8db6cd,
    LightSkyBlue4 = 0x607b8b,
    SlateGray1 = 0xc6e2ff,
    SlateGray2 = 0xb9d3ee,
    SlateGray3 = 0x9fb6cd,
    SlateGray4 = 0x6c7b8b,
    LightSteelBlue1 = 0xcae1ff,
    LightSteelBlue2 = 0xbcd2ee,
    LightSteelBlue3 = 0xa2b5cd,
    LightSteelBlue4 = 0x6e7b8b,
    LightBlue1 = 0xbfefff,
    LightBlue2 = 0xb2dfee,
    LightBlue3 = 0x9ac0cd,
    LightBlue4 = 0x68838b,
    LightCyan1 = 0xe0ffff,
    LightCyan2 = 0xd1eeee,
    LightCyan3 = 0xb4cdcd,
    LightCyan4 = 0x7a8b8b,
    PaleTurquoise1 = 0xbbffff,
    PaleTurquoise2 = 0xaeeeee,
    PaleTurquoise3 = 0x96cdcd,
    PaleTurquoise4 = 0x668b8b,
    CadetBlue1 = 0x98f5ff,
    CadetBlue2 = 0x8ee5ee,
    CadetBlue3 = 0x7ac5cd,
    CadetBlue4 = 0x53868b,
    Turquoise1 = 0x00f5ff,
    Turquoise2 = 0x00e5ee,
    Turquoise3 = 0x00c5cd,
    Turquoise4 = 0x00868b,
    Cyan1 = 0x00ffff,
    Cyan2 = 0x00eeee,
    Cyan3 = 0x00cdcd,
    Cyan4 = 0x008b8b,
    DarkSlateGray1 = 0x97ffff,
    DarkSlateGray2 = 0x8deeee,
    DarkSlateGray3 = 0x79cdcd,
    DarkSlateGray4 = 0x528b8b,
    Aquamarine1 = 0x7fffd4,
    Aquamarine2 = 0x76eec6,
    Aquamarine3 = 0x66cdaa,
    Aquamarine4 = 0x458b74,
    DarkSeaGreen1 = 0xc1ffc1,
    DarkSeaGreen2 = 0xb4eeb4,
    DarkSeaGreen3 = 0x9bcd9b,
    DarkSeaGreen4 = 0x698b69,
    SeaGreen1 = 0x54ff9f,
    SeaGreen2 = 0x4eee94,
    SeaGreen3 = 0x43cd80,
    SeaGreen4 = 0x2e8b57,
    PaleGreen1 = 0x9aff9a,
    PaleGreen2 = 0x90ee90,
    PaleGreen3 = 0x7ccd7c,
    PaleGreen4 = 0x548b54,
    SpringGreen1 = 0x00ff7f,
    SpringGreen2 = 0x00ee76,
    SpringGreen3 = 0x00cd66,
    SpringGreen4 = 0x008b45,
    Green1 = 0x00ff00,
    Green2 = 0x00ee00,
    Green3 = 0x00cd00,
    Green4 = 0x008b00,
    Chartreuse1 = 0x7fff00,
    Chartreuse2 = 0x76ee00,
    Chartreuse3 = 0x66cd00,
    Chartreuse4 = 0x458b00,
    OliveDrab1 = 0xc0ff3e,
    OliveDrab2 = 0xb3ee3a,
    OliveDrab3 = 0x9acd32,
    OliveDrab4 = 0x698b22,
    DarkOliveGreen1 = 0xcaff70,
    DarkOliveGreen2 = 0xbcee68,
    DarkOliveGreen3 = 0xa2cd5a,
    DarkOliveGreen4 = 0x6e8b3d,
    Khaki1 = 0xfff68f,
    Khaki2 = 0xeee685,
    Khaki3 = 0xcdc673,
    Khaki4 = 0x8b864e,
    LightGoldenrod1 = 0xffec8b,
    LightGoldenrod2 = 0xeedc82,
    LightGoldenrod3 = 0xcdbe70,
    LightGoldenrod4 = 0x8b814c,
    LightYellow1 = 0xffffe0,
    LightYellow2 = 0xeeeed1,
    LightYellow3 = 0xcdcdb4,
    LightYellow4 = 0x8b8b7a,
    Yellow1 = 0xffff00,
    Yellow2 = 0xeeee00,
    Yellow3 = 0xcdcd00,
    Yellow4 = 0x8b8b00,
    Gold1 = 0xffd700,
    Gold2 = 0xeec900,
    Gold3 = 0xcdad00,
    Gold4 = 0x8b7500,
    Goldenrod1 = 0xffc125,
    Goldenrod2 = 0xeeb422,
    Goldenrod3 = 0xcd9b1d,
    Goldenrod4 = 0x8b6914,
    DarkGoldenrod1 = 0xffb90f,
    DarkGoldenrod2 = 0xeead0e,
    DarkGoldenrod3 = 0xcd950c,
    DarkGoldenrod4 = 0x8b6508,
    RosyBrown1 = 0xffc1c1,
    RosyBrown2 = 0xeeb4b4,
    RosyBrown3 = 0xcd9b9b,
    RosyBrown4 = 0x8b6969,
    IndianRed1 = 0xff6a6a,
    IndianRed2 = 0xee6363,
    IndianRed3 = 0xcd5555,
    IndianRed4 = 0x8b3a3a,
    Sienna1 = 0xff8247,
    Sienna2 = 0xee7942,
    Sienna3 = 0xcd6839,
    Sienna4 = 0x8b4726,
    Burlywood1 = 0xffd39b,
    Burlywood2 = 0xeec591,
    Burlywood3 = 0xcdaa7d,
    Burlywood4 = 0x8b7355,
    Wheat1 = 0xffe7ba,
    Wheat2 = 0xeed8ae,
    Wheat3 = 0xcdba96,
    Wheat4 = 0x8b7e66,
    Tan1 = 0xffa54f,
    Tan2 = 0xee9a49,
    Tan3 = 0xcd853f,
    Tan4 = 0x8b5a2b,
    Chocolate1 = 0xff7f24,
    Chocolate2 = 0xee7621,
    Chocolate3 = 0xcd661d,
    Chocolate4 = 0x8b4513,
    Firebrick1 = 0xff3030,
    Firebrick2 = 0xee2c2c,
    Firebrick3 = 0xcd2626,
    Firebrick4 = 0x8b1a1a,
    Brown1 = 0xff4040,
    Brown2 = 0xee3b3b,
    Brown3 = 0xcd3333,
    Brown4 = 0x8b2323,
    Salmon1 = 0xff8c69,
    Salmon2 = 0xee8262,
    Salmon3 = 0xcd7054,
    Salmon4 = 0x8b4c39,
    LightSalmon1 = 0xffa07a,
    LightSalmon2 = 0xee9572,
    LightSalmon3 = 0xcd8162,
    LightSalmon4 = 0x8b5742,
    Orange1 = 0xffa500,
    Orange2 = 0xee9a00,
    Orange3 = 0xcd8500,
    Orange4 = 0x8b5a00,
    DarkOrange1 = 0xff7f00,
    DarkOrange2 = 0xee7600,
    DarkOrange3 = 0xcd6600,
    DarkOrange4 = 0x8b4500,
    Coral1 = 0xff7256,
    Coral2 = 0xee6a50,
    Coral3 = 0xcd5b45,
    Coral4 = 0x8b3e2f,
    Tomato1 = 0xff6347,
    Tomato2 = 0xee5c42,
    Tomato3 = 0xcd4f39,
    Tomato4 = 0x8b3626,
    OrangeRed1 = 0xff4500,
    OrangeRed2 = 0xee4000,
    OrangeRed3 = 0xcd3700,
    OrangeRed4 = 0x8b2500,
    Red1 = 0xff0000,
    Red2 = 0xee0000,
    Red3 = 0xcd0000,
    Red4 = 0x8b0000,
    DeepPink1 = 0xff1493,
    DeepPink2 = 0xee1289,
    DeepPink3 = 0xcd1076,
    DeepPink4 = 0x8b0a50,
    HotPink1 = 0xff6eb4,
    HotPink2 = 0xee6aa7,
    HotPink3 = 0xcd6090,
    HotPink4 = 0x8b3a62,
    Pink1 = 0xffb5c5,
    Pink2 = 0xeea9b8,
    Pink3 = 0xcd919e,
    Pink4 = 0x8b636c,
    LightPink1 = 0xffaeb9,
    LightPink2 = 0xeea2ad,
    LightPink3 = 0xcd8c95,
    LightPink4 = 0x8b5f65,
    PaleVioletRed1 = 0xff82ab,
    PaleVioletRed2 = 0xee799f,
    PaleVioletRed3 = 0xcd6889,
    PaleVioletRed4 = 0x8b475d,
    Maroon1 = 0xff34b3,
    Maroon2 = 0xee30a7,
    Maroon3 = 0xcd2990,
    Maroon4 = 0x8b1c62,
    VioletRed1 = 0xff3e96,
    VioletRed2 = 0xee3a8c,
    VioletRed3 = 0xcd3278,
    VioletRed4 = 0x8b2252,
    Magenta1 = 0xff00ff,
    Magenta2 = 0xee00ee,
    Magenta3 = 0xcd00cd,
    Magenta4 = 0x8b008b,
    Orchid1 = 0xff83fa,
    Orchid2 = 0xee7ae9,
    Orchid3 = 0xcd69c9,
    Orchid4 = 0x8b4789,
    Plum1 = 0xffbbff,
    Plum2 = 0xeeaeee,
    Plum3 = 0xcd96cd,
    Plum4 = 0x8b668b,
    MediumOrchid1 = 0xe066ff,
    MediumOrchid2 = 0xd15fee,
    MediumOrchid3 = 0xb452cd,
    MediumOrchid4 = 0x7a378b,
    DarkOrchid1 = 0xbf3eff,
    DarkOrchid2 = 0xb23aee,
    DarkOrchid3 = 0x9a32cd,
    DarkOrchid4 = 0x68228b,
    Purple1 = 0x9b30ff,
    Purple2 = 0x912cee,
    Purple3 = 0x7d26cd,
    Purple4 = 0x551a8b,
    MediumPurple1 = 0xab82ff,
    MediumPurple2 = 0x9f79ee,
    MediumPurple3 = 0x8968cd,
    MediumPurple4 = 0x5d478b,
    Thistle1 = 0xffe1ff,
    Thistle2 = 0xeed2ee,
    Thistle3 = 0xcdb5cd,
    Thistle4 = 0x8b7b8b,
    Gray0 = 0x000000,
    Grey0 = 0x000000,
    Gray1 = 0x030303,
    Grey1 = 0x030303,
    Gray2 = 0x050505,
    Grey2 = 0x050505,
    Gray3 = 0x080808,
    Grey3 = 0x080808,
    Gray4 = 0x0a0a0a,
    Grey4 = 0x0a0a0a,
    Gray5 = 0x0d0d0d,
    Grey5 = 0x0d0d0d,
    Gray6 = 0x0f0f0f,
    Grey6 = 0x0f0f0f,
    Gray7 = 0x121212,
    Grey7 = 0x121212,
    Gray8 = 0x141414,
    Grey8 = 0x141414,
    Gray9 = 0x171717,
    Grey9 = 0x171717,
    Gray10 = 0x1a1a1a,
    Grey10 = 0x1a1a1a,
    Gray11 = 0x1c1c1c,
    Grey11 = 0x1c1c1c,
    Gray12 = 0x1f1f1f,
    Grey12 = 0x1f1f1f,
    Gray13 = 0x212121,
    Grey13 = 0x212121,
    Gray14 = 0x242424,
    Grey14 = 0x242424,
    Gray15 = 0x262626,
    Grey15 = 0x262626,
    Gray16 = 0x292929,
    Grey16 = 0x292929,
    Gray17 = 0x2b2b2b,
    Grey17 = 0x2b2b2b,
    Gray18 = 0x2e2e2e,
    Grey18 = 0x2e2e2e,
    Gray19 = 0x303030,
    Grey19 = 0x303030,
    Gray20 = 0x333333,
    Grey20 = 0x333333,
    Gray21 = 0x363636,
    Grey21 = 0x363636,
    Gray22 = 0x383838,
    Grey22 = 0x383838,
    Gray23 = 0x3b3b3b,
    Grey23 = 0x3b3b3b,
    Gray24 = 0x3d3d3d,
    Grey24 = 0x3d3d3d,
    Gray25 = 0x404040,
    Grey25 = 0x404040,
    Gray26 = 0x424242,
    Grey26 = 0x424242,
    Gray27 = 0x454545,
    Grey27 = 0x454545,
    Gray28 = 0x474747,
    Grey28 = 0x474747,
    Gray29 = 0x4a4a4a,
    Grey29 = 0x4a4a4a,
    Gray30 = 0x4d4d4d,
    Grey30 = 0x4d4d4d,
    Gray31 = 0x4f4f4f,
    Grey31 = 0x4f4f4f,
    Gray32 = 0x525252,
    Grey32 = 0x525252,
    Gray33 = 0x545454,
    Grey33 = 0x545454,
    Gray34 = 0x575757,
    Grey34 = 0x575757,
    Gray35 = 0x595959,
    Grey35 = 0x595959,
    Gray36 = 0x5c5c5c,
    Grey36 = 0x5c5c5c,
    Gray37 = 0x5e5e5e,
    Grey37 = 0x5e5e5e,
    Gray38 = 0x616161,
    Grey38 = 0x616161,
    Gray39 = 0x636363,
    Grey39 = 0x636363,
    Gray40 = 0x666666,
    Grey40 = 0x666666,
    Gray41 = 0x696969,
    Grey41 = 0x696969,
    Gray42 = 0x6b6b6b,
    Grey42 = 0x6b6b6b,
    Gray43 = 0x6e6e6e,
    Grey43 = 0x6e6e6e,
    Gray44 = 0x707070,
    Grey44 = 0x707070,
    Gray45 = 0x737373,
    Grey45 = 0x737373,
    Gray46 = 0x757575,
    Grey46 = 0x757575,
    Gray47 = 0x787878,
    Grey47 = 0x787878,
    Gray48 = 0x7a7a7a,
    Grey48 = 0x7a7a7a,
    Gray49 = 0x7d7d7d,
    Grey49 = 0x7d7d7d,
    Gray50 = 0x7f7f7f,
    Grey50 = 0x7f7f7f,
    Gray51 = 0x828282,
    Grey51 = 0x828282,
    Gray52 = 0x858585,
    Grey52 = 0x858585,
    Gray53 = 0x878787,
    Grey53 = 0x878787,
    Gray54 = 0x8a8a8a,
    Grey54 = 0x8a8a8a,
    Gray55 = 0x8c8c8c,
    Grey55 = 0x8c8c8c,
    Gray56 = 0x8f8f8f,
    Grey56 = 0x8f8f8f,
    Gray57 = 0x919191,
    Grey57 = 0x919191,
    Gray58 = 0x949494,
    Grey58 = 0x949494,
    Gray59 = 0x969696,
    Grey59 = 0x969696,
    Gray60 = 0x999999,
    Grey60 = 0x999999,
    Gray61 = 0x9c9c9c,
    Grey61 = 0x9c9c9c,
    Gray62 = 0x9e9e9e,
    Grey62 = 0x9e9e9e,
    Gray63 = 0xa1a1a1,
    Grey63 = 0xa1a1a1,
    Gray64 = 0xa3a3a3,
    Grey64 = 0xa3a3a3,
    Gray65 = 0xa6a6a6,
    Grey65 = 0xa6a6a6,
    Gray66 = 0xa8a8a8,
    Grey66 = 0xa8a8a8,
    Gray67 = 0xababab,
    Grey67 = 0xababab,
    Gray68 = 0xadadad,
    Grey68 = 0xadadad,
    Gray69 = 0xb0b0b0,
    Grey69 = 0xb0b0b0,
    Gray70 = 0xb3b3b3,
    Grey70 = 0xb3b3b3,
    Gray71 = 0xb5b5b5,
    Grey71 = 0xb5b5b5,
    Gray72 = 0xb8b8b8,
    Grey72 = 0xb8b8b8,
    Gray73 = 0xbababa,
    Grey73 = 0xbababa,
    Gray74 = 0xbdbdbd,
    Grey74 = 0xbdbdbd,
    Gray75 = 0xbfbfbf,
    Grey75 = 0xbfbfbf,
    Gray76 = 0xc2c2c2,
    Grey76 = 0xc2c2c2,
    Gray77 = 0xc4c4c4,
    Grey77 = 0xc4c4c4,
    Gray78 = 0xc7c7c7,
    Grey78 = 0xc7c7c7,
    Gray79 = 0xc9c9c9,
    Grey79 = 0xc9c9c9,
    Gray80 = 0xcccccc,
    Grey80 = 0xcccccc,
    Gray81 = 0xcfcfcf,
    Grey81 = 0xcfcfcf,
    Gray82 = 0xd1d1d1,
    Grey82 = 0xd1d1d1,
    Gray83 = 0xd4d4d4,
    Grey83 = 0xd4d4d4,
    Gray84 = 0xd6d6d6,
    Grey84 = 0xd6d6d6,
    Gray85 = 0xd9d9d9,
    Grey85 = 0xd9d9d9,
    Gray86 = 0xdbdbdb,
    Grey86 = 0xdbdbdb,
    Gray87 = 0xdedede,
    Grey87 = 0xdedede,
    Gray88 = 0xe0e0e0,
    Grey88 = 0xe0e0e0,
    Gray89 = 0xe3e3e3,
    Grey89 = 0xe3e3e3,
    Gray90 = 0xe5e5e5,
    Grey90 = 0xe5e5e5,
    Gray91 = 0xe8e8e8,
    Grey91 = 0xe8e8e8,
    Gray92 = 0xebebeb,
    Grey92 = 0xebebeb,
    Gray93 = 0xededed,
    Grey93 = 0xededed,
    Gray94 = 0xf0f0f0,
    Grey94 = 0xf0f0f0,
    Gray95 = 0xf2f2f2,
    Grey95 = 0xf2f2f2,
    Gray96 = 0xf5f5f5,
    Grey96 = 0xf5f5f5,
    Gray97 = 0xf7f7f7,
    Grey97 = 0xf7f7f7,
    Gray98 = 0xfafafa,
    Grey98 = 0xfafafa,
    Gray99 = 0xfcfcfc,
    Grey99 = 0xfcfcfc,
    Gray100 = 0xffffff,
    Grey100 = 0xffffff,
    DarkGrey = 0xa9a9a9,
    DarkGray = 0xa9a9a9,
    DarkBlue = 0x00008b,
    DarkCyan = 0x008b8b,
    DarkMagenta = 0x8b008b,
    DarkRed = 0x8b0000,
    LightGreen = 0x90ee90,
    Crimson = 0xdc143c,
    Indigo = 0x4b0082,
    Olive = 0x808000,
    RebeccaPurple = 0x663399,
    Silver = 0xc0c0c0,
    Teal = 0x008080,
};
};
}

#endif

/*** End of inlined file: TracyColor.hpp ***/


/*** Start of inlined file: TracySystem.hpp ***/
#ifndef __TRACYSYSTEM_HPP__
#define __TRACYSYSTEM_HPP__

#include <stdint.h>


/*** Start of inlined file: TracyApi.h ***/
#ifndef __TRACYAPI_H__
#define __TRACYAPI_H__

#if defined _WIN32 || defined __CYGWIN__
#  if defined TRACY_EXPORTS
#    define TRACY_API __declspec(dllexport)
#  elif defined TRACY_IMPORTS
#    define TRACY_API __declspec(dllimport)
#  else
#    define TRACY_API
#  endif
#else
#  define TRACY_API __attribute__((visibility("default")))
#endif

#endif    // __TRACYAPI_H__

/*** End of inlined file: TracyApi.h ***/

namespace tracy
{

namespace detail
{
TRACY_API uint64_t GetThreadHandleImpl();
}

#ifdef TRACY_ENABLE
TRACY_API uint64_t GetThreadHandle();
#else
static inline uint64_t GetThreadHandle()
{
    return detail::GetThreadHandleImpl();
}
#endif

TRACY_API void SetThreadName( const char* name );
TRACY_API const char* GetThreadName( uint64_t id );

TRACY_API const char* GetEnvVar(const char* name);

}

#endif

/*** End of inlined file: TracySystem.hpp ***/

#ifndef TRACY_ENABLE

#define ZoneNamed(x,y)
#define ZoneNamedN(x,y,z)
#define ZoneNamedC(x,y,z)
#define ZoneNamedNC(x,y,z,w)

#define ZoneTransient(x,y)
#define ZoneTransientN(x,y,z)

#define ZoneScoped
#define ZoneScopedN(x)
#define ZoneScopedC(x)
#define ZoneScopedNC(x,y)

#define ZoneText(x,y)
#define ZoneTextV(x,y,z)
#define ZoneName(x,y)
#define ZoneNameV(x,y,z)
#define ZoneColor(x)
#define ZoneColorV(x,y)
#define ZoneValue(x)
#define ZoneValueV(x,y)
#define ZoneIsActive false
#define ZoneIsActiveV(x) false

#define FrameMark
#define FrameMarkNamed(x)
#define FrameMarkStart(x)
#define FrameMarkEnd(x)

#define FrameImage(x,y,z,w,a)

#define TracyLockable( type, varname ) type varname;
#define TracyLockableN( type, varname, desc ) type varname;
#define TracySharedLockable( type, varname ) type varname;
#define TracySharedLockableN( type, varname, desc ) type varname;
#define LockableBase( type ) type
#define SharedLockableBase( type ) type
#define LockMark(x) (void)x;
#define LockableName(x,y,z);

#define TracyPlot(x,y)
#define TracyPlotConfig(x,y)

#define TracyMessage(x,y)
#define TracyMessageL(x)
#define TracyMessageC(x,y,z)
#define TracyMessageLC(x,y)
#define TracyAppInfo(x,y)

#define TracyAlloc(x,y)
#define TracyFree(x)
#define TracySecureAlloc(x,y)
#define TracySecureFree(x)

#define TracyAllocN(x,y,z)
#define TracyFreeN(x,y)
#define TracySecureAllocN(x,y,z)
#define TracySecureFreeN(x,y)

#define ZoneNamedS(x,y,z)
#define ZoneNamedNS(x,y,z,w)
#define ZoneNamedCS(x,y,z,w)
#define ZoneNamedNCS(x,y,z,w,a)

#define ZoneTransientS(x,y,z)
#define ZoneTransientNS(x,y,z,w)

#define ZoneScopedS(x)
#define ZoneScopedNS(x,y)
#define ZoneScopedCS(x,y)
#define ZoneScopedNCS(x,y,z)

#define TracyAllocS(x,y,z)
#define TracyFreeS(x,y)
#define TracySecureAllocS(x,y,z)
#define TracySecureFreeS(x,y)

#define TracyAllocNS(x,y,z,w)
#define TracyFreeNS(x,y,z)
#define TracySecureAllocNS(x,y,z,w)
#define TracySecureFreeNS(x,y,z)

#define TracyMessageS(x,y,z)
#define TracyMessageLS(x,y)
#define TracyMessageCS(x,y,z,w)
#define TracyMessageLCS(x,y,z)

#define TracyParameterRegister(x)
#define TracyParameterSetup(x,y,z,w)
#define TracyIsConnected false

#else

#include <string.h>


/*** Start of inlined file: TracyLock.hpp ***/
#ifndef __TRACYLOCK_HPP__
#define __TRACYLOCK_HPP__

#include <atomic>
#include <limits>


/*** Start of inlined file: TracyAlign.hpp ***/
#ifndef __TRACYALIGN_HPP__
#define __TRACYALIGN_HPP__

#include <string.h>


/*** Start of inlined file: TracyForceInline.hpp ***/
#ifndef __TRACYFORCEINLINE_HPP__
#define __TRACYFORCEINLINE_HPP__

#if defined(__GNUC__)
#  define tracy_force_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#  define tracy_force_inline __forceinline
#else
#  define tracy_force_inline inline
#endif

#if defined(__GNUC__)
#  define tracy_no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
#  define tracy_no_inline __declspec(noinline)
#else
#  define tracy_no_inline
#endif

#endif

/*** End of inlined file: TracyForceInline.hpp ***/

namespace tracy
{

template<typename T>
tracy_force_inline T MemRead( const void* ptr )
{
    T val;
    memcpy( &val, ptr, sizeof( T ) );
    return val;
}

template<typename T>
tracy_force_inline void MemWrite( void* ptr, T val )
{
    memcpy( ptr, &val, sizeof( T ) );
}

}

#endif

/*** End of inlined file: TracyAlign.hpp ***/


/*** Start of inlined file: TracyProfiler.hpp ***/
#ifndef __TRACYPROFILER_HPP__
#define __TRACYPROFILER_HPP__

#include <assert.h>
#include <atomic>
#include <stdint.h>
#include <string.h>
#include <time.h>


/*** Start of inlined file: tracy_concurrentqueue.h ***/
// Provides a C++11 implementation of a multi-producer, multi-consumer lock-free queue.
// An overview, including benchmark results, is provided here:
//     http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
// The full design is also described in excruciating detail at:
//    http://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue

// Simplified BSD license:
// Copyright (c) 2013-2016, Cameron Desrochers.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice, this list of
// conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice, this list of
// conditions and the following disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once


/*** Start of inlined file: TracyAlloc.hpp ***/
#ifndef __TRACYALLOC_HPP__
#define __TRACYALLOC_HPP__

#include <stdlib.h>

#ifdef TRACY_ENABLE


/*** Start of inlined file: tracy_rpmalloc.hpp ***/
#pragma once

#include <stddef.h>

namespace tracy
{

#if defined(__clang__) || defined(__GNUC__)
# define RPMALLOC_EXPORT __attribute__((visibility("default")))
# define RPMALLOC_ALLOCATOR
# define RPMALLOC_ATTRIB_MALLOC __attribute__((__malloc__))
# if defined(__clang_major__) && (__clang_major__ < 4)
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count, size)
# else
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size) __attribute__((alloc_size(size)))
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count, size)  __attribute__((alloc_size(count, size)))
# endif
# define RPMALLOC_CDECL
#elif defined(_MSC_VER)
# define RPMALLOC_EXPORT
# define RPMALLOC_ALLOCATOR __declspec(allocator) __declspec(restrict)
# define RPMALLOC_ATTRIB_MALLOC
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count,size)
# define RPMALLOC_CDECL __cdecl
#else
# define RPMALLOC_EXPORT
# define RPMALLOC_ALLOCATOR
# define RPMALLOC_ATTRIB_MALLOC
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count,size)
# define RPMALLOC_CDECL
#endif

//! Define RPMALLOC_CONFIGURABLE to enable configuring sizes
#ifndef RPMALLOC_CONFIGURABLE
#define RPMALLOC_CONFIGURABLE 0
#endif

//! Flag to rpaligned_realloc to not preserve content in reallocation
#define RPMALLOC_NO_PRESERVE    1

typedef struct rpmalloc_global_statistics_t {
	//! Current amount of virtual memory mapped, all of which might not have been committed (only if ENABLE_STATISTICS=1)
	size_t mapped;
	//! Peak amount of virtual memory mapped, all of which might not have been committed (only if ENABLE_STATISTICS=1)
	size_t mapped_peak;
	//! Current amount of memory in global caches for small and medium sizes (<32KiB)
	size_t cached;
	//! Current amount of memory allocated in huge allocations, i.e larger than LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
	size_t huge_alloc;
	//! Peak amount of memory allocated in huge allocations, i.e larger than LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
	size_t huge_alloc_peak;
	//! Total amount of memory mapped since initialization (only if ENABLE_STATISTICS=1)
	size_t mapped_total;
	//! Total amount of memory unmapped since initialization  (only if ENABLE_STATISTICS=1)
	size_t unmapped_total;
} rpmalloc_global_statistics_t;

typedef struct rpmalloc_thread_statistics_t {
	//! Current number of bytes available in thread size class caches for small and medium sizes (<32KiB)
	size_t sizecache;
	//! Current number of bytes available in thread span caches for small and medium sizes (<32KiB)
	size_t spancache;
	//! Total number of bytes transitioned from thread cache to global cache (only if ENABLE_STATISTICS=1)
	size_t thread_to_global;
	//! Total number of bytes transitioned from global cache to thread cache (only if ENABLE_STATISTICS=1)
	size_t global_to_thread;
	//! Per span count statistics (only if ENABLE_STATISTICS=1)
	struct {
		//! Currently used number of spans
		size_t current;
		//! High water mark of spans used
		size_t peak;
		//! Number of spans transitioned to global cache
		size_t to_global;
		//! Number of spans transitioned from global cache
		size_t from_global;
		//! Number of spans transitioned to thread cache
		size_t to_cache;
		//! Number of spans transitioned from thread cache
		size_t from_cache;
		//! Number of spans transitioned to reserved state
		size_t to_reserved;
		//! Number of spans transitioned from reserved state
		size_t from_reserved;
		//! Number of raw memory map calls (not hitting the reserve spans but resulting in actual OS mmap calls)
		size_t map_calls;
	} span_use[32];
	//! Per size class statistics (only if ENABLE_STATISTICS=1)
	struct {
		//! Current number of allocations
		size_t alloc_current;
		//! Peak number of allocations
		size_t alloc_peak;
		//! Total number of allocations
		size_t alloc_total;
		//! Total number of frees
		size_t free_total;
		//! Number of spans transitioned to cache
		size_t spans_to_cache;
		//! Number of spans transitioned from cache
		size_t spans_from_cache;
		//! Number of spans transitioned from reserved state
		size_t spans_from_reserved;
		//! Number of raw memory map calls (not hitting the reserve spans but resulting in actual OS mmap calls)
		size_t map_calls;
	} size_use[128];
} rpmalloc_thread_statistics_t;

typedef struct rpmalloc_config_t {
	//! Map memory pages for the given number of bytes. The returned address MUST be
	//  aligned to the rpmalloc span size, which will always be a power of two.
	//  Optionally the function can store an alignment offset in the offset variable
	//  in case it performs alignment and the returned pointer is offset from the
	//  actual start of the memory region due to this alignment. The alignment offset
	//  will be passed to the memory unmap function. The alignment offset MUST NOT be
	//  larger than 65535 (storable in an uint16_t), if it is you must use natural
	//  alignment to shift it into 16 bits. If you set a memory_map function, you
	//  must also set a memory_unmap function or else the default implementation will
	//  be used for both.
	void* (*memory_map)(size_t size, size_t* offset);
	//! Unmap the memory pages starting at address and spanning the given number of bytes.
	//  If release is set to non-zero, the unmap is for an entire span range as returned by
	//  a previous call to memory_map and that the entire range should be released. The
	//  release argument holds the size of the entire span range. If release is set to 0,
	//  the unmap is a partial decommit of a subset of the mapped memory range.
	//  If you set a memory_unmap function, you must also set a memory_map function or
	//  else the default implementation will be used for both.
	void (*memory_unmap)(void* address, size_t size, size_t offset, size_t release);
	//! Size of memory pages. The page size MUST be a power of two. All memory mapping
	//  requests to memory_map will be made with size set to a multiple of the page size.
	//  Used if RPMALLOC_CONFIGURABLE is defined to 1, otherwise system page size is used.
	size_t page_size;
	//! Size of a span of memory blocks. MUST be a power of two, and in [4096,262144]
	//  range (unless 0 - set to 0 to use the default span size). Used if RPMALLOC_CONFIGURABLE
	//  is defined to 1.
	size_t span_size;
	//! Number of spans to map at each request to map new virtual memory blocks. This can
	//  be used to minimize the system call overhead at the cost of virtual memory address
	//  space. The extra mapped pages will not be written until actually used, so physical
	//  committed memory should not be affected in the default implementation. Will be
	//  aligned to a multiple of spans that match memory page size in case of huge pages.
	size_t span_map_count;
	//! Enable use of large/huge pages. If this flag is set to non-zero and page size is
	//  zero, the allocator will try to enable huge pages and auto detect the configuration.
	//  If this is set to non-zero and page_size is also non-zero, the allocator will
	//  assume huge pages have been configured and enabled prior to initializing the
	//  allocator.
	//  For Windows, see https://docs.microsoft.com/en-us/windows/desktop/memory/large-page-support
	//  For Linux, see https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt
	int enable_huge_pages;
} rpmalloc_config_t;

//! Initialize allocator with default configuration
TRACY_API int
rpmalloc_initialize(void);

//! Initialize allocator with given configuration
RPMALLOC_EXPORT int
rpmalloc_initialize_config(const rpmalloc_config_t* config);

//! Get allocator configuration
RPMALLOC_EXPORT const rpmalloc_config_t*
rpmalloc_config(void);

//! Finalize allocator
TRACY_API void
rpmalloc_finalize(void);

//! Initialize allocator for calling thread
TRACY_API void
rpmalloc_thread_initialize(void);

//! Finalize allocator for calling thread
TRACY_API void
rpmalloc_thread_finalize(void);

//! Perform deferred deallocations pending for the calling thread heap
RPMALLOC_EXPORT void
rpmalloc_thread_collect(void);

//! Query if allocator is initialized for calling thread
RPMALLOC_EXPORT int
rpmalloc_is_thread_initialized(void);

//! Get per-thread statistics
RPMALLOC_EXPORT void
rpmalloc_thread_statistics(rpmalloc_thread_statistics_t* stats);

//! Get global statistics
RPMALLOC_EXPORT void
rpmalloc_global_statistics(rpmalloc_global_statistics_t* stats);

//! Dump all statistics in human readable format to file (should be a FILE*)
RPMALLOC_EXPORT void
rpmalloc_dump_statistics(void* file);

//! Allocate a memory block of at least the given size
TRACY_API RPMALLOC_ALLOCATOR void*
rpmalloc(size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(1);

//! Free the given memory block
TRACY_API void
rpfree(void* ptr);

//! Allocate a memory block of at least the given size and zero initialize it
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpcalloc(size_t num, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2);

//! Reallocate the given block to at least the given size
TRACY_API RPMALLOC_ALLOCATOR void*
rprealloc(void* ptr, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Reallocate the given block to at least the given size and alignment,
//  with optional control flags (see RPMALLOC_NO_PRESERVE).
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpaligned_realloc(void* ptr, size_t alignment, size_t size, size_t oldsize, unsigned int flags) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpaligned_alloc(size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpmemalign(size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT int
rpposix_memalign(void **memptr, size_t alignment, size_t size);

//! Query the usable size of the given memory block (from given pointer to the end of block)
RPMALLOC_EXPORT size_t
rpmalloc_usable_size(void* ptr);

}

/*** End of inlined file: tracy_rpmalloc.hpp ***/

#endif

namespace tracy
{

#ifdef TRACY_ENABLE
TRACY_API void InitRpmalloc();
#endif

static inline void* tracy_malloc( size_t size )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    return rpmalloc( size );
#else
    return malloc( size );
#endif
}

static inline void* tracy_malloc_fast( size_t size )
{
#ifdef TRACY_ENABLE
    return rpmalloc( size );
#else
    return malloc( size );
#endif
}

static inline void tracy_free( void* ptr )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    rpfree( ptr );
#else
    free( ptr );
#endif
}

static inline void tracy_free_fast( void* ptr )
{
#ifdef TRACY_ENABLE
    rpfree( ptr );
#else
    free( ptr );
#endif
}

static inline void* tracy_realloc( void* ptr, size_t size )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    return rprealloc( ptr, size );
#else
    return realloc( ptr, size );
#endif
}

}

#endif

/*** End of inlined file: TracyAlloc.hpp ***/

#if defined(__GNUC__)
// Disable -Wconversion warnings (spuriously triggered when Traits::size_t and
// Traits::index_t are set to < 32 bits, causing integer promotion, causing warnings
// upon assigning any computed values)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#include <atomic>		// Requires C++11. Sorry VS2010.
#include <cassert>
#include <cstddef>              // for max_align_t
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <limits>
#include <climits>		// for CHAR_BIT
#include <array>
#include <thread>		// partly for __WINPTHREADS_VERSION if on MinGW-w64 w/ POSIX threading

namespace tracy
{

// Compiler-specific likely/unlikely hints
namespace moodycamel { namespace details {
#if defined(__GNUC__)
	inline bool cqLikely(bool x) { return __builtin_expect((x), true); }
	inline bool cqUnlikely(bool x) { return __builtin_expect((x), false); }
#else
	inline bool cqLikely(bool x) { return x; }
	inline bool cqUnlikely(bool x) { return x; }
#endif
} }

namespace
{
    // to avoid MSVC warning 4127: conditional expression is constant
    template <bool>
    struct compile_time_condition
    {
        static const bool value = false;
    };
    template <>
    struct compile_time_condition<true>
    {
        static const bool value = true;
    };
}

namespace moodycamel {
namespace details {
	template<typename T>
	struct const_numeric_max {
		static_assert(std::is_integral<T>::value, "const_numeric_max can only be used with integers");
		static const T value = std::numeric_limits<T>::is_signed
			? (static_cast<T>(1) << (sizeof(T) * CHAR_BIT - 1)) - static_cast<T>(1)
			: static_cast<T>(-1);
	};

#if defined(__GLIBCXX__)
	typedef ::max_align_t std_max_align_t;      // libstdc++ forgot to add it to std:: for a while
#else
	typedef std::max_align_t std_max_align_t;   // Others (e.g. MSVC) insist it can *only* be accessed via std::
#endif

	// Some platforms have incorrectly set max_align_t to a type with <8 bytes alignment even while supporting
	// 8-byte aligned scalar values (*cough* 32-bit iOS). Work around this with our own union. See issue #64.
	typedef union {
		std_max_align_t x;
		long long y;
		void* z;
	} max_align_t;
}

// Default traits for the ConcurrentQueue. To change some of the
// traits without re-implementing all of them, inherit from this
// struct and shadow the declarations you wish to be different;
// since the traits are used as a template type parameter, the
// shadowed declarations will be used where defined, and the defaults
// otherwise.
struct ConcurrentQueueDefaultTraits
{
	// General-purpose size type. std::size_t is strongly recommended.
	typedef std::size_t size_t;

	// The type used for the enqueue and dequeue indices. Must be at least as
	// large as size_t. Should be significantly larger than the number of elements
	// you expect to hold at once, especially if you have a high turnover rate;
	// for example, on 32-bit x86, if you expect to have over a hundred million
	// elements or pump several million elements through your queue in a very
	// short space of time, using a 32-bit type *may* trigger a race condition.
	// A 64-bit int type is recommended in that case, and in practice will
	// prevent a race condition no matter the usage of the queue. Note that
	// whether the queue is lock-free with a 64-int type depends on the whether
	// std::atomic<std::uint64_t> is lock-free, which is platform-specific.
	typedef std::size_t index_t;

	// Internally, all elements are enqueued and dequeued from multi-element
	// blocks; this is the smallest controllable unit. If you expect few elements
	// but many producers, a smaller block size should be favoured. For few producers
	// and/or many elements, a larger block size is preferred. A sane default
	// is provided. Must be a power of 2.
	static const size_t BLOCK_SIZE = 64*1024;

	// For explicit producers (i.e. when using a producer token), the block is
	// checked for being empty by iterating through a list of flags, one per element.
	// For large block sizes, this is too inefficient, and switching to an atomic
	// counter-based approach is faster. The switch is made for block sizes strictly
	// larger than this threshold.
	static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;

	// How many full blocks can be expected for a single explicit producer? This should
	// reflect that number's maximum for optimal performance. Must be a power of 2.
	static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 32;

	// Controls the number of items that an explicit consumer (i.e. one with a token)
	// must consume before it causes all consumers to rotate and move on to the next
	// internal queue.
	static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;

	// The maximum number of elements (inclusive) that can be enqueued to a sub-queue.
	// Enqueue operations that would cause this limit to be surpassed will fail. Note
	// that this limit is enforced at the block level (for performance reasons), i.e.
	// it's rounded up to the nearest block size.
	static const size_t MAX_SUBQUEUE_SIZE = details::const_numeric_max<size_t>::value;

	// Memory allocation can be customized if needed.
	// malloc should return nullptr on failure, and handle alignment like std::malloc.
#if defined(malloc) || defined(free)
	// Gah, this is 2015, stop defining macros that break standard code already!
	// Work around malloc/free being special macros:
	static inline void* WORKAROUND_malloc(size_t size) { return malloc(size); }
	static inline void WORKAROUND_free(void* ptr) { return free(ptr); }
	static inline void* (malloc)(size_t size) { return WORKAROUND_malloc(size); }
	static inline void (free)(void* ptr) { return WORKAROUND_free(ptr); }
#else
	static inline void* malloc(size_t size) { return tracy::tracy_malloc(size); }
	static inline void free(void* ptr) { return tracy::tracy_free(ptr); }
#endif
};

// When producing or consuming many elements, the most efficient way is to:
//    1) Use one of the bulk-operation methods of the queue with a token
//    2) Failing that, use the bulk-operation methods without a token
//    3) Failing that, create a token and use that with the single-item methods
//    4) Failing that, use the single-parameter methods of the queue
// Having said that, don't create tokens willy-nilly -- ideally there should be
// a maximum of one token per thread (of each kind).
struct ProducerToken;
struct ConsumerToken;

template<typename T, typename Traits> class ConcurrentQueue;

namespace details
{
	struct ConcurrentQueueProducerTypelessBase
	{
		ConcurrentQueueProducerTypelessBase* next;
		std::atomic<bool> inactive;
		ProducerToken* token;
        uint64_t threadId;

		ConcurrentQueueProducerTypelessBase()
			: next(nullptr), inactive(false), token(nullptr), threadId(0)
		{
		}
	};

	template<typename T>
	static inline bool circular_less_than(T a, T b)
	{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4554)
#endif
		static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "circular_less_than is intended to be used only with unsigned integer types");
		return static_cast<T>(a - b) > static_cast<T>(static_cast<T>(1) << static_cast<T>(sizeof(T) * CHAR_BIT - 1));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	}

	template<typename U>
	static inline char* align_for(char* ptr)
	{
		const std::size_t alignment = std::alignment_of<U>::value;
		return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
	}

	template<typename T>
	static inline T ceil_to_pow_2(T x)
	{
		static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "ceil_to_pow_2 is intended to be used only with unsigned integer types");

		// Adapted from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		--x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		for (std::size_t i = 1; i < sizeof(T); i <<= 1) {
			x |= x >> (i << 3);
		}
		++x;
		return x;
	}

	template<typename T>
	static inline void swap_relaxed(std::atomic<T>& left, std::atomic<T>& right)
	{
		T temp = std::move(left.load(std::memory_order_relaxed));
		left.store(std::move(right.load(std::memory_order_relaxed)), std::memory_order_relaxed);
		right.store(std::move(temp), std::memory_order_relaxed);
	}

	template<typename T>
	static inline T const& nomove(T const& x)
	{
		return x;
	}

	template<bool Enable>
	struct nomove_if
	{
		template<typename T>
		static inline T const& eval(T const& x)
		{
			return x;
		}
	};

	template<>
	struct nomove_if<false>
	{
		template<typename U>
		static inline auto eval(U&& x)
			-> decltype(std::forward<U>(x))
		{
			return std::forward<U>(x);
		}
	};

	template<typename It>
	static inline auto deref_noexcept(It& it) noexcept -> decltype(*it)
	{
		return *it;
	}

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
	template<typename T> struct is_trivially_destructible : std::is_trivially_destructible<T> { };
#else
	template<typename T> struct is_trivially_destructible : std::has_trivial_destructor<T> { };
#endif

	template<typename T> struct static_is_lock_free_num { enum { value = 0 }; };
	template<> struct static_is_lock_free_num<signed char> { enum { value = ATOMIC_CHAR_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<short> { enum { value = ATOMIC_SHORT_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<int> { enum { value = ATOMIC_INT_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<long> { enum { value = ATOMIC_LONG_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<long long> { enum { value = ATOMIC_LLONG_LOCK_FREE }; };
	template<typename T> struct static_is_lock_free : static_is_lock_free_num<typename std::make_signed<T>::type> {  };
	template<> struct static_is_lock_free<bool> { enum { value = ATOMIC_BOOL_LOCK_FREE }; };
	template<typename U> struct static_is_lock_free<U*> { enum { value = ATOMIC_POINTER_LOCK_FREE }; };
}

struct ProducerToken
{
	template<typename T, typename Traits>
	explicit ProducerToken(ConcurrentQueue<T, Traits>& queue);

	ProducerToken(ProducerToken&& other) noexcept
		: producer(other.producer)
	{
		other.producer = nullptr;
		if (producer != nullptr) {
			producer->token = this;
		}
	}

	inline ProducerToken& operator=(ProducerToken&& other) noexcept
	{
		swap(other);
		return *this;
	}

	void swap(ProducerToken& other) noexcept
	{
		std::swap(producer, other.producer);
		if (producer != nullptr) {
			producer->token = this;
		}
		if (other.producer != nullptr) {
			other.producer->token = &other;
		}
	}

	// A token is always valid unless:
	//     1) Memory allocation failed during construction
	//     2) It was moved via the move constructor
	//        (Note: assignment does a swap, leaving both potentially valid)
	//     3) The associated queue was destroyed
	// Note that if valid() returns true, that only indicates
	// that the token is valid for use with a specific queue,
	// but not which one; that's up to the user to track.
	inline bool valid() const { return producer != nullptr; }

	~ProducerToken()
	{
		if (producer != nullptr) {
			producer->token = nullptr;
			producer->inactive.store(true, std::memory_order_release);
		}
	}

	// Disable copying and assignment
	ProducerToken(ProducerToken const&) = delete;
	ProducerToken& operator=(ProducerToken const&) = delete;

private:
	template<typename T, typename Traits> friend class ConcurrentQueue;

protected:
	details::ConcurrentQueueProducerTypelessBase* producer;
};

struct ConsumerToken
{
	template<typename T, typename Traits>
	explicit ConsumerToken(ConcurrentQueue<T, Traits>& q);

	ConsumerToken(ConsumerToken&& other) noexcept
		: initialOffset(other.initialOffset), lastKnownGlobalOffset(other.lastKnownGlobalOffset), itemsConsumedFromCurrent(other.itemsConsumedFromCurrent), currentProducer(other.currentProducer), desiredProducer(other.desiredProducer)
	{
	}

	inline ConsumerToken& operator=(ConsumerToken&& other) noexcept
	{
		swap(other);
		return *this;
	}

	void swap(ConsumerToken& other) noexcept
	{
		std::swap(initialOffset, other.initialOffset);
		std::swap(lastKnownGlobalOffset, other.lastKnownGlobalOffset);
		std::swap(itemsConsumedFromCurrent, other.itemsConsumedFromCurrent);
		std::swap(currentProducer, other.currentProducer);
		std::swap(desiredProducer, other.desiredProducer);
	}

	// Disable copying and assignment
	ConsumerToken(ConsumerToken const&) = delete;
	ConsumerToken& operator=(ConsumerToken const&) = delete;

private:
	template<typename T, typename Traits> friend class ConcurrentQueue;

private: // but shared with ConcurrentQueue
	std::uint32_t initialOffset;
	std::uint32_t lastKnownGlobalOffset;
	std::uint32_t itemsConsumedFromCurrent;
	details::ConcurrentQueueProducerTypelessBase* currentProducer;
	details::ConcurrentQueueProducerTypelessBase* desiredProducer;
};

template<typename T, typename Traits = ConcurrentQueueDefaultTraits>
class ConcurrentQueue
{
public:
    struct ExplicitProducer;

	typedef moodycamel::ProducerToken producer_token_t;
	typedef moodycamel::ConsumerToken consumer_token_t;

	typedef typename Traits::index_t index_t;
	typedef typename Traits::size_t size_t;

	static const size_t BLOCK_SIZE = static_cast<size_t>(Traits::BLOCK_SIZE);
	static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = static_cast<size_t>(Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD);
	static const size_t EXPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::EXPLICIT_INITIAL_INDEX_SIZE);
	static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = static_cast<std::uint32_t>(Traits::EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4307)		// + integral constant overflow (that's what the ternary expression is for!)
#pragma warning(disable: 4309)		// static_cast: Truncation of constant value
#endif
	static const size_t MAX_SUBQUEUE_SIZE = (details::const_numeric_max<size_t>::value - static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) < BLOCK_SIZE) ? details::const_numeric_max<size_t>::value : ((static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	static_assert(!std::numeric_limits<size_t>::is_signed && std::is_integral<size_t>::value, "Traits::size_t must be an unsigned integral type");
	static_assert(!std::numeric_limits<index_t>::is_signed && std::is_integral<index_t>::value, "Traits::index_t must be an unsigned integral type");
	static_assert(sizeof(index_t) >= sizeof(size_t), "Traits::index_t must be at least as wide as Traits::size_t");
	static_assert((BLOCK_SIZE > 1) && !(BLOCK_SIZE & (BLOCK_SIZE - 1)), "Traits::BLOCK_SIZE must be a power of 2 (and at least 2)");
	static_assert((EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD > 1) && !(EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD & (EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD - 1)), "Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD must be a power of 2 (and greater than 1)");
	static_assert((EXPLICIT_INITIAL_INDEX_SIZE > 1) && !(EXPLICIT_INITIAL_INDEX_SIZE & (EXPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::EXPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");

public:
	// Creates a queue with at least `capacity` element slots; note that the
	// actual number of elements that can be inserted without additional memory
	// allocation depends on the number of producers and the block size (e.g. if
	// the block size is equal to `capacity`, only a single block will be allocated
	// up-front, which means only a single producer will be able to enqueue elements
	// without an extra allocation -- blocks aren't shared between producers).
	// This method is not thread safe -- it is up to the user to ensure that the
	// queue is fully constructed before it starts being used by other threads (this
	// includes making the memory effects of construction visible, possibly with a
	// memory barrier).
	explicit ConcurrentQueue(size_t capacity = 6 * BLOCK_SIZE)
		: producerListTail(nullptr),
		producerCount(0),
		initialBlockPoolIndex(0),
		nextExplicitConsumerId(0),
		globalExplicitConsumerOffset(0)
	{
		populate_initial_block_list(capacity / BLOCK_SIZE + ((capacity & (BLOCK_SIZE - 1)) == 0 ? 0 : 1));
	}

	// Computes the correct amount of pre-allocated blocks for you based
	// on the minimum number of elements you want available at any given
	// time, and the maximum concurrent number of each type of producer.
	ConcurrentQueue(size_t minCapacity, size_t maxExplicitProducers)
		: producerListTail(nullptr),
		producerCount(0),
		initialBlockPoolIndex(0),
		nextExplicitConsumerId(0),
		globalExplicitConsumerOffset(0)
	{
		size_t blocks = (((minCapacity + BLOCK_SIZE - 1) / BLOCK_SIZE) - 1) * (maxExplicitProducers + 1) + 2 * (maxExplicitProducers);
		populate_initial_block_list(blocks);
	}

	// Note: The queue should not be accessed concurrently while it's
	// being deleted. It's up to the user to synchronize this.
	// This method is not thread safe.
	~ConcurrentQueue()
	{
		// Destroy producers
		auto ptr = producerListTail.load(std::memory_order_relaxed);
		while (ptr != nullptr) {
			auto next = ptr->next_prod();
			if (ptr->token != nullptr) {
				ptr->token->producer = nullptr;
			}
			destroy(ptr);
			ptr = next;
		}

		// Destroy global free list
		auto block = freeList.head_unsafe();
		while (block != nullptr) {
			auto next = block->freeListNext.load(std::memory_order_relaxed);
			if (block->dynamicallyAllocated) {
				destroy(block);
			}
			block = next;
		}

		// Destroy initial free list
		destroy_array(initialBlockPool, initialBlockPoolSize);
	}

	// Disable copying and copy assignment
	ConcurrentQueue(ConcurrentQueue const&) = delete;
    ConcurrentQueue(ConcurrentQueue&& other) = delete;
	ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;
    ConcurrentQueue& operator=(ConcurrentQueue&& other) = delete;

public:
    tracy_force_inline T* enqueue_begin(producer_token_t const& token, index_t& currentTailIndex)
    {
        return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::enqueue_begin(currentTailIndex);
    }

	template<class NotifyThread, class ProcessData>
    size_t try_dequeue_bulk_single(consumer_token_t& token, NotifyThread notifyThread, ProcessData processData )
    {
        if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
            if (!update_current_producer_after_rotation(token)) {
                return 0;
            }
        }

        size_t count = static_cast<ProducerBase*>(token.currentProducer)->dequeue_bulk(notifyThread, processData);
        token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(count);

        auto tail = producerListTail.load(std::memory_order_acquire);
        auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
        if (ptr == nullptr) {
            ptr = tail;
        }
        if( count == 0 )
        {
            while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
                auto dequeued = ptr->dequeue_bulk(notifyThread, processData);
                if (dequeued != 0) {
                    token.currentProducer = ptr;
                    token.itemsConsumedFromCurrent = static_cast<std::uint32_t>(dequeued);
                    return dequeued;
                }
                ptr = ptr->next_prod();
                if (ptr == nullptr) {
                    ptr = tail;
                }
            }
            return 0;
        }
        else
        {
            token.currentProducer = ptr;
            token.itemsConsumedFromCurrent = 0;
            return count;
        }
    }

	// Returns an estimate of the total number of elements currently in the queue. This
	// estimate is only accurate if the queue has completely stabilized before it is called
	// (i.e. all enqueue and dequeue operations have completed and their memory effects are
	// visible on the calling thread, and no further operations start while this method is
	// being called).
	// Thread-safe.
	size_t size_approx() const
	{
		size_t size = 0;
		for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
			size += ptr->size_approx();
		}
		return size;
	}

	// Returns true if the underlying atomic variables used by
	// the queue are lock-free (they should be on most platforms).
	// Thread-safe.
	static bool is_lock_free()
	{
		return
			details::static_is_lock_free<bool>::value == 2 &&
			details::static_is_lock_free<size_t>::value == 2 &&
			details::static_is_lock_free<std::uint32_t>::value == 2 &&
			details::static_is_lock_free<index_t>::value == 2 &&
			details::static_is_lock_free<void*>::value == 2;
	}

private:
	friend struct ProducerToken;
	friend struct ConsumerToken;
	friend struct ExplicitProducer;

	///////////////////////////////
	// Queue methods
	///////////////////////////////

	inline bool update_current_producer_after_rotation(consumer_token_t& token)
	{
		// Ah, there's been a rotation, figure out where we should be!
		auto tail = producerListTail.load(std::memory_order_acquire);
		if (token.desiredProducer == nullptr && tail == nullptr) {
			return false;
		}
		auto prodCount = producerCount.load(std::memory_order_relaxed);
		auto globalOffset = globalExplicitConsumerOffset.load(std::memory_order_relaxed);
		if (details::cqUnlikely(token.desiredProducer == nullptr)) {
			// Aha, first time we're dequeueing anything.
			// Figure out our local position
			// Note: offset is from start, not end, but we're traversing from end -- subtract from count first
			std::uint32_t offset = prodCount - 1 - (token.initialOffset % prodCount);
			token.desiredProducer = tail;
			for (std::uint32_t i = 0; i != offset; ++i) {
				token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
				if (token.desiredProducer == nullptr) {
					token.desiredProducer = tail;
				}
			}
		}

		std::uint32_t delta = globalOffset - token.lastKnownGlobalOffset;
		if (delta >= prodCount) {
			delta = delta % prodCount;
		}
		for (std::uint32_t i = 0; i != delta; ++i) {
			token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
			if (token.desiredProducer == nullptr) {
				token.desiredProducer = tail;
			}
		}

		token.lastKnownGlobalOffset = globalOffset;
		token.currentProducer = token.desiredProducer;
		token.itemsConsumedFromCurrent = 0;
		return true;
	}

	///////////////////////////
	// Free list
	///////////////////////////

	template <typename N>
	struct FreeListNode
	{
		FreeListNode() : freeListRefs(0), freeListNext(nullptr) { }

		std::atomic<std::uint32_t> freeListRefs;
		std::atomic<N*> freeListNext;
	};

	// A simple CAS-based lock-free free list. Not the fastest thing in the world under heavy contention, but
	// simple and correct (assuming nodes are never freed until after the free list is destroyed), and fairly
	// speedy under low contention.
	template<typename N>		// N must inherit FreeListNode or have the same fields (and initialization of them)
	struct FreeList
	{
		FreeList() : freeListHead(nullptr) { }
		FreeList(FreeList&& other) : freeListHead(other.freeListHead.load(std::memory_order_relaxed)) { other.freeListHead.store(nullptr, std::memory_order_relaxed); }
		void swap(FreeList& other) { details::swap_relaxed(freeListHead, other.freeListHead); }

		FreeList(FreeList const&) = delete;
		FreeList& operator=(FreeList const&) = delete;

		inline void add(N* node)
		{
			// We know that the should-be-on-freelist bit is 0 at this point, so it's safe to
			// set it using a fetch_add
			if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST, std::memory_order_acq_rel) == 0) {
				// Oh look! We were the last ones referencing this node, and we know
				// we want to add it to the free list, so let's do it!
		 		add_knowing_refcount_is_zero(node);
			}
		}

		inline N* try_get()
		{
			auto head = freeListHead.load(std::memory_order_acquire);
			while (head != nullptr) {
				auto prevHead = head;
				auto refs = head->freeListRefs.load(std::memory_order_relaxed);
				if ((refs & REFS_MASK) == 0 || !head->freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
					head = freeListHead.load(std::memory_order_acquire);
					continue;
				}

				// Good, reference count has been incremented (it wasn't at zero), which means we can read the
				// next and not worry about it changing between now and the time we do the CAS
				auto next = head->freeListNext.load(std::memory_order_relaxed);
				if (freeListHead.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
					// Yay, got the node. This means it was on the list, which means shouldBeOnFreeList must be false no
					// matter the refcount (because nobody else knows it's been taken off yet, it can't have been put back on).
					assert((head->freeListRefs.load(std::memory_order_relaxed) & SHOULD_BE_ON_FREELIST) == 0);

					// Decrease refcount twice, once for our ref, and once for the list's ref
					head->freeListRefs.fetch_sub(2, std::memory_order_release);
					return head;
				}

				// OK, the head must have changed on us, but we still need to decrease the refcount we increased.
				// Note that we don't need to release any memory effects, but we do need to ensure that the reference
				// count decrement happens-after the CAS on the head.
				refs = prevHead->freeListRefs.fetch_sub(1, std::memory_order_acq_rel);
				if (refs == SHOULD_BE_ON_FREELIST + 1) {
					add_knowing_refcount_is_zero(prevHead);
				}
			}

			return nullptr;
		}

		// Useful for traversing the list when there's no contention (e.g. to destroy remaining nodes)
		N* head_unsafe() const { return freeListHead.load(std::memory_order_relaxed); }

	private:
		inline void add_knowing_refcount_is_zero(N* node)
		{
			// Since the refcount is zero, and nobody can increase it once it's zero (except us, and we run
			// only one copy of this method per node at a time, i.e. the single thread case), then we know
			// we can safely change the next pointer of the node; however, once the refcount is back above
			// zero, then other threads could increase it (happens under heavy contention, when the refcount
			// goes to zero in between a load and a refcount increment of a node in try_get, then back up to
			// something non-zero, then the refcount increment is done by the other thread) -- so, if the CAS
			// to add the node to the actual list fails, decrease the refcount and leave the add operation to
			// the next thread who puts the refcount back at zero (which could be us, hence the loop).
			auto head = freeListHead.load(std::memory_order_relaxed);
			while (true) {
				node->freeListNext.store(head, std::memory_order_relaxed);
				node->freeListRefs.store(1, std::memory_order_release);
				if (!freeListHead.compare_exchange_strong(head, node, std::memory_order_release, std::memory_order_relaxed)) {
					// Hmm, the add failed, but we can only try again when the refcount goes back to zero
					if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST - 1, std::memory_order_release) == 1) {
						continue;
					}
				}
				return;
			}
		}

	private:
		// Implemented like a stack, but where node order doesn't matter (nodes are inserted out of order under contention)
		std::atomic<N*> freeListHead;

	static const std::uint32_t REFS_MASK = 0x7FFFFFFF;
	static const std::uint32_t SHOULD_BE_ON_FREELIST = 0x80000000;
	};

	///////////////////////////
	// Block
	///////////////////////////

	struct Block
	{
		Block()
			: next(nullptr), elementsCompletelyDequeued(0), freeListRefs(0), freeListNext(nullptr), shouldBeOnFreeList(false), dynamicallyAllocated(true)
		{
		}

		inline bool is_empty() const
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Check flags
				for (size_t i = 0; i < BLOCK_SIZE; ++i) {
					if (!emptyFlags[i].load(std::memory_order_relaxed)) {
						return false;
					}
				}

				// Aha, empty; make sure we have all other memory effects that happened before the empty flags were set
				std::atomic_thread_fence(std::memory_order_acquire);
				return true;
			}
			else {
				// Check counter
				if (elementsCompletelyDequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
					std::atomic_thread_fence(std::memory_order_acquire);
					return true;
				}
				assert(elementsCompletelyDequeued.load(std::memory_order_relaxed) <= BLOCK_SIZE);
				return false;
			}
		}

		// Returns true if the block is now empty (does not apply in explicit context)
		inline bool set_empty(index_t i)
		{
			if (BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
				// Set flag
				assert(!emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].load(std::memory_order_relaxed));
				emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].store(true, std::memory_order_release);
				return false;
			}
			else {
				// Increment counter
				auto prevVal = elementsCompletelyDequeued.fetch_add(1, std::memory_order_release);
				assert(prevVal < BLOCK_SIZE);
				return prevVal == BLOCK_SIZE - 1;
			}
		}

		// Sets multiple contiguous item statuses to 'empty' (assumes no wrapping and count > 0).
		// Returns true if the block is now empty (does not apply in explicit context).
		inline bool set_many_empty(index_t i, size_t count)
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Set flags
				std::atomic_thread_fence(std::memory_order_release);
				i = BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1)) - count + 1;
				for (size_t j = 0; j != count; ++j) {
					assert(!emptyFlags[i + j].load(std::memory_order_relaxed));
					emptyFlags[i + j].store(true, std::memory_order_relaxed);
				}
				return false;
			}
			else {
				// Increment counter
				auto prevVal = elementsCompletelyDequeued.fetch_add(count, std::memory_order_release);
				assert(prevVal + count <= BLOCK_SIZE);
				return prevVal + count == BLOCK_SIZE;
			}
		}

		inline void set_all_empty()
		{
			if (BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
				// Set all flags
				for (size_t i = 0; i != BLOCK_SIZE; ++i) {
					emptyFlags[i].store(true, std::memory_order_relaxed);
				}
			}
			else {
				// Reset counter
				elementsCompletelyDequeued.store(BLOCK_SIZE, std::memory_order_relaxed);
			}
		}

		inline void reset_empty()
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Reset flags
				for (size_t i = 0; i != BLOCK_SIZE; ++i) {
					emptyFlags[i].store(false, std::memory_order_relaxed);
				}
			}
			else {
				// Reset counter
				elementsCompletelyDequeued.store(0, std::memory_order_relaxed);
			}
		}

		inline T* operator[](index_t idx) noexcept { return static_cast<T*>(static_cast<void*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }
		inline T const* operator[](index_t idx) const noexcept { return static_cast<T const*>(static_cast<void const*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }

	private:
		// IMPORTANT: This must be the first member in Block, so that if T depends on the alignment of
		// addresses returned by malloc, that alignment will be preserved. Apparently clang actually
		// generates code that uses this assumption for AVX instructions in some cases. Ideally, we
		// should also align Block to the alignment of T in case it's higher than malloc's 16-byte
		// alignment, but this is hard to do in a cross-platform way. Assert for this case:
		static_assert(std::alignment_of<T>::value <= std::alignment_of<details::max_align_t>::value, "The queue does not support super-aligned types at this time");
		// Additionally, we need the alignment of Block itself to be a multiple of max_align_t since
		// otherwise the appropriate padding will not be added at the end of Block in order to make
		// arrays of Blocks all be properly aligned (not just the first one). We use a union to force
		// this.
		union {
			char elements[sizeof(T) * BLOCK_SIZE];
			details::max_align_t dummy;
		};
	public:
		Block* next;
		std::atomic<size_t> elementsCompletelyDequeued;
		std::atomic<bool> emptyFlags[BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD ? BLOCK_SIZE : 1];
	public:
		std::atomic<std::uint32_t> freeListRefs;
		std::atomic<Block*> freeListNext;
		std::atomic<bool> shouldBeOnFreeList;
		bool dynamicallyAllocated;		// Perhaps a better name for this would be 'isNotPartOfInitialBlockPool'
	};
	static_assert(std::alignment_of<Block>::value >= std::alignment_of<details::max_align_t>::value, "Internal error: Blocks must be at least as aligned as the type they are wrapping");

	///////////////////////////
	// Producer base
	///////////////////////////

	struct ProducerBase : public details::ConcurrentQueueProducerTypelessBase
	{
		ProducerBase(ConcurrentQueue* parent_) :
			tailIndex(0),
			headIndex(0),
			dequeueOptimisticCount(0),
			dequeueOvercommit(0),
			tailBlock(nullptr),
			parent(parent_)
		{
		}

		virtual ~ProducerBase() { };

		template<class NotifyThread, class ProcessData>
		inline size_t dequeue_bulk(NotifyThread notifyThread, ProcessData processData)
		{
			return static_cast<ExplicitProducer*>(this)->dequeue_bulk(notifyThread, processData);
		}

		inline ProducerBase* next_prod() const { return static_cast<ProducerBase*>(next); }

		inline size_t size_approx() const
		{
			auto tail = tailIndex.load(std::memory_order_relaxed);
			auto head = headIndex.load(std::memory_order_relaxed);
			return details::circular_less_than(head, tail) ? static_cast<size_t>(tail - head) : 0;
		}

		inline index_t getTail() const { return tailIndex.load(std::memory_order_relaxed); }
	protected:
		std::atomic<index_t> tailIndex;		// Where to enqueue to next
		std::atomic<index_t> headIndex;		// Where to dequeue from next

		std::atomic<index_t> dequeueOptimisticCount;
		std::atomic<index_t> dequeueOvercommit;

		Block* tailBlock;

	public:
		ConcurrentQueue* parent;
	};

    public:
	///////////////////////////
	// Explicit queue
	///////////////////////////
	struct ExplicitProducer : public ProducerBase
	{
		explicit ExplicitProducer(ConcurrentQueue* _parent) :
			ProducerBase(_parent),
			blockIndex(nullptr),
			pr_blockIndexSlotsUsed(0),
			pr_blockIndexSize(EXPLICIT_INITIAL_INDEX_SIZE >> 1),
			pr_blockIndexFront(0),
			pr_blockIndexEntries(nullptr),
			pr_blockIndexRaw(nullptr)
		{
			size_t poolBasedIndexSize = details::ceil_to_pow_2(_parent->initialBlockPoolSize) >> 1;
			if (poolBasedIndexSize > pr_blockIndexSize) {
				pr_blockIndexSize = poolBasedIndexSize;
			}

			new_block_index(0);		// This creates an index with double the number of current entries, i.e. EXPLICIT_INITIAL_INDEX_SIZE
		}

		~ExplicitProducer()
		{
			// Destruct any elements not yet dequeued.
			// Since we're in the destructor, we can assume all elements
			// are either completely dequeued or completely not (no halfways).
			if (this->tailBlock != nullptr) {		// Note this means there must be a block index too
				// First find the block that's partially dequeued, if any
				Block* halfDequeuedBlock = nullptr;
				if ((this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) != 0) {
					// The head's not on a block boundary, meaning a block somewhere is partially dequeued
					// (or the head block is the tail block and was fully dequeued, but the head/tail are still not on a boundary)
					size_t i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & (pr_blockIndexSize - 1);
					while (details::circular_less_than<index_t>(pr_blockIndexEntries[i].base + BLOCK_SIZE, this->headIndex.load(std::memory_order_relaxed))) {
						i = (i + 1) & (pr_blockIndexSize - 1);
					}
					assert(details::circular_less_than<index_t>(pr_blockIndexEntries[i].base, this->headIndex.load(std::memory_order_relaxed)));
					halfDequeuedBlock = pr_blockIndexEntries[i].block;
				}

				// Start at the head block (note the first line in the loop gives us the head from the tail on the first iteration)
				auto block = this->tailBlock;
				do {
					block = block->next;
					if (block->ConcurrentQueue::Block::is_empty()) {
						continue;
					}

					size_t i = 0;	// Offset into block
					if (block == halfDequeuedBlock) {
						i = static_cast<size_t>(this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
					}

					// Walk through all the items in the block; if this is the tail block, we need to stop when we reach the tail index
					auto lastValidIndex = (this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 ? BLOCK_SIZE : static_cast<size_t>(this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
					while (i != BLOCK_SIZE && (block != this->tailBlock || i != lastValidIndex)) {
						(*block)[i++]->~T();
					}
				} while (block != this->tailBlock);
			}

			// Destroy all blocks that we own
			if (this->tailBlock != nullptr) {
				auto block = this->tailBlock;
				do {
					auto nextBlock = block->next;
					if (block->dynamicallyAllocated) {
						destroy(block);
					}
					else {
						this->parent->add_block_to_free_list(block);
					}
					block = nextBlock;
				} while (block != this->tailBlock);
			}

			// Destroy the block indices
			auto header = static_cast<BlockIndexHeader*>(pr_blockIndexRaw);
			while (header != nullptr) {
				auto prev = static_cast<BlockIndexHeader*>(header->prev);
				header->~BlockIndexHeader();
				(Traits::free)(header);
				header = prev;
			}
		}

        inline void enqueue_begin_alloc(index_t currentTailIndex)
        {
            // We reached the end of a block, start a new one
            if (this->tailBlock != nullptr && this->tailBlock->next->ConcurrentQueue::Block::is_empty()) {
                // We can re-use the block ahead of us, it's empty!
                this->tailBlock = this->tailBlock->next;
                this->tailBlock->ConcurrentQueue::Block::reset_empty();

                // We'll put the block on the block index (guaranteed to be room since we're conceptually removing the
                // last block from it first -- except instead of removing then adding, we can just overwrite).
                // Note that there must be a valid block index here, since even if allocation failed in the ctor,
                // it would have been re-attempted when adding the first block to the queue; since there is such
                // a block, a block index must have been successfully allocated.
            }
            else {
                // We're going to need a new block; check that the block index has room
                if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize) {
                    // Hmm, the circular block index is already full -- we'll need
                    // to allocate a new index. Note pr_blockIndexRaw can only be nullptr if
                    // the initial allocation failed in the constructor.
                    new_block_index(pr_blockIndexSlotsUsed);
                }

                // Insert a new block in the circular linked list
                auto newBlock = this->parent->ConcurrentQueue::requisition_block();
                newBlock->ConcurrentQueue::Block::reset_empty();
                if (this->tailBlock == nullptr) {
                    newBlock->next = newBlock;
                }
                else {
                    newBlock->next = this->tailBlock->next;
                    this->tailBlock->next = newBlock;
                }
                this->tailBlock = newBlock;
                ++pr_blockIndexSlotsUsed;
            }

            // Add block to block index
            auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
            entry.base = currentTailIndex;
            entry.block = this->tailBlock;
            blockIndex.load(std::memory_order_relaxed)->front.store(pr_blockIndexFront, std::memory_order_release);
            pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
        }

        tracy_force_inline T* enqueue_begin(index_t& currentTailIndex)
        {
            currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
            if (details::cqUnlikely((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0)) {
                this->enqueue_begin_alloc(currentTailIndex);
            }
            return (*this->tailBlock)[currentTailIndex];
        }

        tracy_force_inline std::atomic<index_t>& get_tail_index()
        {
            return this->tailIndex;
        }

		template<class NotifyThread, class ProcessData>
		size_t dequeue_bulk(NotifyThread notifyThread, ProcessData processData)
		{
			auto tail = this->tailIndex.load(std::memory_order_relaxed);
			auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
			auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
			if (details::circular_less_than<size_t>(0, desiredCount)) {
				desiredCount = desiredCount < 8192 ? desiredCount : 8192;
				std::atomic_thread_fence(std::memory_order_acquire);

				auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);
				assert(overcommit <= myDequeueCount);

				tail = this->tailIndex.load(std::memory_order_acquire);
				auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
				if (details::circular_less_than<size_t>(0, actualCount)) {
					actualCount = desiredCount < actualCount ? desiredCount : actualCount;
					if (actualCount < desiredCount) {
						this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
					}

					// Get the first index. Note that since there's guaranteed to be at least actualCount elements, this
					// will never exceed tail.
					auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);

					// Determine which block the first element is in
					auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
					auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

					auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
					auto firstBlockBaseIndex = firstIndex & ~static_cast<index_t>(BLOCK_SIZE - 1);
					auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(firstBlockBaseIndex - headBase) / BLOCK_SIZE);
					auto indexIndex = (localBlockIndexHead + offset) & (localBlockIndex->size - 1);

					notifyThread( this->threadId );

					// Iterate the blocks and dequeue
					auto index = firstIndex;
					do {
						auto firstIndexInBlock = index;
						auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
						endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
						auto block = localBlockIndex->entries[indexIndex].block;

						const auto sz = endIndex - index;
						processData( (*block)[index], sz );
						index += sz;

						block->ConcurrentQueue::Block::set_many_empty(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
						indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);
					} while (index != firstIndex + actualCount);

					return actualCount;
				}
				else {
					// Wasn't anything to dequeue after all; make the effective dequeue count eventually consistent
					this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
				}
			}

			return 0;
		}

	private:
		struct BlockIndexEntry
		{
			index_t base;
			Block* block;
		};

		struct BlockIndexHeader
		{
			size_t size;
			std::atomic<size_t> front;		// Current slot (not next, like pr_blockIndexFront)
			BlockIndexEntry* entries;
			void* prev;
		};

		bool new_block_index(size_t numberOfFilledSlotsToExpose)
		{
			auto prevBlockSizeMask = pr_blockIndexSize - 1;

			// Create the new block
			pr_blockIndexSize <<= 1;
			auto newRawPtr = static_cast<char*>((Traits::malloc)(sizeof(BlockIndexHeader) + std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * pr_blockIndexSize));
			if (newRawPtr == nullptr) {
				pr_blockIndexSize >>= 1;		// Reset to allow graceful retry
				return false;
			}

			auto newBlockIndexEntries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(newRawPtr + sizeof(BlockIndexHeader)));

			// Copy in all the old indices, if any
			size_t j = 0;
			if (pr_blockIndexSlotsUsed != 0) {
				auto i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & prevBlockSizeMask;
				do {
					newBlockIndexEntries[j++] = pr_blockIndexEntries[i];
					i = (i + 1) & prevBlockSizeMask;
				} while (i != pr_blockIndexFront);
			}

			// Update everything
			auto header = new (newRawPtr) BlockIndexHeader;
			header->size = pr_blockIndexSize;
			header->front.store(numberOfFilledSlotsToExpose - 1, std::memory_order_relaxed);
			header->entries = newBlockIndexEntries;
			header->prev = pr_blockIndexRaw;		// we link the new block to the old one so we can free it later

			pr_blockIndexFront = j;
			pr_blockIndexEntries = newBlockIndexEntries;
			pr_blockIndexRaw = newRawPtr;
			blockIndex.store(header, std::memory_order_release);

			return true;
		}

	private:
		std::atomic<BlockIndexHeader*> blockIndex;

		// To be used by producer only -- consumer must use the ones in referenced by blockIndex
		size_t pr_blockIndexSlotsUsed;
		size_t pr_blockIndexSize;
		size_t pr_blockIndexFront;		// Next slot (not current)
		BlockIndexEntry* pr_blockIndexEntries;
		void* pr_blockIndexRaw;
	};

    ExplicitProducer* get_explicit_producer(producer_token_t const& token)
    {
        return static_cast<ExplicitProducer*>(token.producer);
    }

    private:

	//////////////////////////////////
	// Block pool manipulation
	//////////////////////////////////

	void populate_initial_block_list(size_t blockCount)
	{
		initialBlockPoolSize = blockCount;
		if (initialBlockPoolSize == 0) {
			initialBlockPool = nullptr;
			return;
		}

		initialBlockPool = create_array<Block>(blockCount);
		if (initialBlockPool == nullptr) {
			initialBlockPoolSize = 0;
		}
		for (size_t i = 0; i < initialBlockPoolSize; ++i) {
			initialBlockPool[i].dynamicallyAllocated = false;
		}
	}

	inline Block* try_get_block_from_initial_pool()
	{
		if (initialBlockPoolIndex.load(std::memory_order_relaxed) >= initialBlockPoolSize) {
			return nullptr;
		}

		auto index = initialBlockPoolIndex.fetch_add(1, std::memory_order_relaxed);

		return index < initialBlockPoolSize ? (initialBlockPool + index) : nullptr;
	}

	inline void add_block_to_free_list(Block* block)
	{
		freeList.add(block);
	}

	inline void add_blocks_to_free_list(Block* block)
	{
		while (block != nullptr) {
			auto next = block->next;
			add_block_to_free_list(block);
			block = next;
		}
	}

	inline Block* try_get_block_from_free_list()
	{
		return freeList.try_get();
	}

	// Gets a free block from one of the memory pools, or allocates a new one (if applicable)
	Block* requisition_block()
	{
		auto block = try_get_block_from_initial_pool();
		if (block != nullptr) {
			return block;
		}

		block = try_get_block_from_free_list();
		if (block != nullptr) {
			return block;
		}

		return create<Block>();
	}

	//////////////////////////////////
	// Producer list manipulation
	//////////////////////////////////

	ProducerBase* recycle_or_create_producer()
	{
		bool recycled;
		return recycle_or_create_producer(recycled);
	}

    ProducerBase* recycle_or_create_producer(bool& recycled)
    {
        // Try to re-use one first
        for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
            if (ptr->inactive.load(std::memory_order_relaxed)) {
                if( ptr->size_approx() == 0 )
                {
                    bool expected = true;
                    if (ptr->inactive.compare_exchange_strong(expected, /* desired */ false, std::memory_order_acquire, std::memory_order_relaxed)) {
                        // We caught one! It's been marked as activated, the caller can have it
                        recycled = true;
                        return ptr;
                    }
                }
            }
        }

        recycled = false;
        return add_producer(static_cast<ProducerBase*>(create<ExplicitProducer>(this)));
    }

	ProducerBase* add_producer(ProducerBase* producer)
	{
		// Handle failed memory allocation
		if (producer == nullptr) {
			return nullptr;
		}

		producerCount.fetch_add(1, std::memory_order_relaxed);

		// Add it to the lock-free list
		auto prevTail = producerListTail.load(std::memory_order_relaxed);
		do {
			producer->next = prevTail;
		} while (!producerListTail.compare_exchange_weak(prevTail, producer, std::memory_order_release, std::memory_order_relaxed));

		return producer;
	}

	void reown_producers()
	{
		// After another instance is moved-into/swapped-with this one, all the
		// producers we stole still think their parents are the other queue.
		// So fix them up!
		for (auto ptr = producerListTail.load(std::memory_order_relaxed); ptr != nullptr; ptr = ptr->next_prod()) {
			ptr->parent = this;
		}
	}

	//////////////////////////////////
	// Utility functions
	//////////////////////////////////

	template<typename U>
	static inline U* create_array(size_t count)
	{
		assert(count > 0);
		return static_cast<U*>((Traits::malloc)(sizeof(U) * count));
	}

	template<typename U>
	static inline void destroy_array(U* p, size_t count)
	{
		((void)count);
		if (p != nullptr) {
			assert(count > 0);
			(Traits::free)(p);
		}
	}

	template<typename U>
	static inline U* create()
	{
		auto p = (Traits::malloc)(sizeof(U));
		return new (p) U;
	}

	template<typename U, typename A1>
	static inline U* create(A1&& a1)
	{
		auto p = (Traits::malloc)(sizeof(U));
		return new (p) U(std::forward<A1>(a1));
	}

	template<typename U>
	static inline void destroy(U* p)
	{
		if (p != nullptr) {
			p->~U();
		}
		(Traits::free)(p);
	}

private:
	std::atomic<ProducerBase*> producerListTail;
	std::atomic<std::uint32_t> producerCount;

	std::atomic<size_t> initialBlockPoolIndex;
	Block* initialBlockPool;
	size_t initialBlockPoolSize;

	FreeList<Block> freeList;

	std::atomic<std::uint32_t> nextExplicitConsumerId;
	std::atomic<std::uint32_t> globalExplicitConsumerOffset;
};

template<typename T, typename Traits>
ProducerToken::ProducerToken(ConcurrentQueue<T, Traits>& queue)
	: producer(queue.recycle_or_create_producer())
{
	if (producer != nullptr) {
		producer->token = this;
        producer->threadId = detail::GetThreadHandleImpl();
	}
}

template<typename T, typename Traits>
ConsumerToken::ConsumerToken(ConcurrentQueue<T, Traits>& queue)
	: itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
{
	initialOffset = queue.nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
	lastKnownGlobalOffset = static_cast<std::uint32_t>(-1);
}

template<typename T, typename Traits>
inline void swap(ConcurrentQueue<T, Traits>& a, ConcurrentQueue<T, Traits>& b) noexcept
{
	a.swap(b);
}

inline void swap(ProducerToken& a, ProducerToken& b) noexcept
{
	a.swap(b);
}

inline void swap(ConsumerToken& a, ConsumerToken& b) noexcept
{
	a.swap(b);
}

}

} /* namespace tracy */

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/*** End of inlined file: tracy_concurrentqueue.h ***/


/*** Start of inlined file: TracyCallstack.hpp ***/
#ifndef __TRACYCALLSTACK_HPP__
#define __TRACYCALLSTACK_HPP__


/*** Start of inlined file: TracyCallstack.h ***/
#ifndef __TRACYCALLSTACK_H__
#define __TRACYCALLSTACK_H__

#if !defined _WIN32 && !defined __CYGWIN__
#  include <sys/param.h>
#endif

#if defined _WIN32 || defined __CYGWIN__
#  define TRACY_HAS_CALLSTACK 1
#elif defined __ANDROID__
#  if !defined __arm__ || __ANDROID_API__ >= 21
#    define TRACY_HAS_CALLSTACK 2
#  else
#    define TRACY_HAS_CALLSTACK 5
#  endif
#elif defined __linux
#  if defined _GNU_SOURCE && defined __GLIBC__
#    define TRACY_HAS_CALLSTACK 3
#  else
#    define TRACY_HAS_CALLSTACK 2
#  endif
#elif defined __APPLE__
#  define TRACY_HAS_CALLSTACK 4
#elif defined BSD
#  define TRACY_HAS_CALLSTACK 6
#endif

#endif

/*** End of inlined file: TracyCallstack.h ***/

#if TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 5
#  include <unwind.h>
#elif TRACY_HAS_CALLSTACK >= 3
#  include <execinfo.h>
#endif

#ifdef TRACY_HAS_CALLSTACK

#include <assert.h>
#include <stdint.h>

namespace tracy
{

struct CallstackSymbolData
{
    const char* file;
    uint32_t line;
    bool needFree;
    uint64_t symAddr;
};

struct CallstackEntry
{
    const char* name;
    const char* file;
    uint32_t line;
    uint32_t symLen;
    uint64_t symAddr;
};

struct CallstackEntryData
{
    const CallstackEntry* data;
    uint8_t size;
    const char* imageName;
};

CallstackSymbolData DecodeSymbolAddress( uint64_t ptr );
CallstackSymbolData DecodeCodeAddress( uint64_t ptr );
const char* DecodeCallstackPtrFast( uint64_t ptr );
CallstackEntryData DecodeCallstackPtr( uint64_t ptr );
void InitCallstack();

#if TRACY_HAS_CALLSTACK == 1

TRACY_API uintptr_t* CallTrace( int depth );

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 && depth < 63 );
    return CallTrace( depth );
}

#elif TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 5

struct BacktraceState
{
    void** current;
    void** end;
};

static _Unwind_Reason_Code tracy_unwind_callback( struct _Unwind_Context* ctx, void* arg )
{
    auto state = (BacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP( ctx );
    if( pc )
    {
        if( state->current == state->end ) return _URC_END_OF_STACK;
        *state->current++ = (void*)pc;
    }
    return _URC_NO_REASON;
}

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 && depth < 63 );

    auto trace = (uintptr_t*)tracy_malloc( ( 1 + depth ) * sizeof( uintptr_t ) );
    BacktraceState state = { (void**)(trace+1), (void**)(trace+1+depth) };
    _Unwind_Backtrace( tracy_unwind_callback, &state );

    *trace = (uintptr_t*)state.current - trace + 1;

    return trace;
}

#elif TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 );

    auto trace = (uintptr_t*)tracy_malloc( ( 1 + (size_t)depth ) * sizeof( uintptr_t ) );
    const auto num = (size_t)backtrace( (void**)(trace+1), depth );
    *trace = num;

    return trace;
}

#endif

}

#endif

#endif

/*** End of inlined file: TracyCallstack.hpp ***/


/*** Start of inlined file: TracySysTime.hpp ***/
#ifndef __TRACYSYSTIME_HPP__
#define __TRACYSYSTIME_HPP__

#if defined _WIN32 || defined __CYGWIN__ || defined __linux__ || defined __APPLE__
#  define TRACY_HAS_SYSTIME
#else
#  include <sys/param.h>
#endif

#ifdef BSD
#  define TRACY_HAS_SYSTIME
#endif

#ifdef TRACY_HAS_SYSTIME

#include <stdint.h>

namespace tracy
{

class SysTime
{
public:
    SysTime();
    float Get();

    void ReadTimes();

private:
    uint64_t idle, used;
};

}
#endif

#endif

/*** End of inlined file: TracySysTime.hpp ***/


/*** Start of inlined file: TracyFastVector.hpp ***/
#ifndef __TRACYFASTVECTOR_HPP__
#define __TRACYFASTVECTOR_HPP__

#include <assert.h>
#include <stddef.h>

namespace tracy
{

template<typename T>
class FastVector
{
public:
    using iterator = T*;
    using const_iterator = const T*;

    FastVector( size_t capacity )
        : m_ptr( (T*)tracy_malloc( sizeof( T ) * capacity ) )
        , m_write( m_ptr )
        , m_end( m_ptr + capacity )
    {
        assert( capacity != 0 );
    }

    FastVector( const FastVector& ) = delete;
    FastVector( FastVector&& ) = delete;

    ~FastVector()
    {
        tracy_free( m_ptr );
    }

    FastVector& operator=( const FastVector& ) = delete;
    FastVector& operator=( FastVector&& ) = delete;

    bool empty() const { return m_ptr == m_write; }
    size_t size() const { return m_write - m_ptr; }

    T* data() { return m_ptr; }
    const T* data() const { return m_ptr; };

    T* begin() { return m_ptr; }
    const T* begin() const { return m_ptr; }
    T* end() { return m_write; }
    const T* end() const { return m_write; }

    T& front() { assert( !empty() ); return m_ptr[0]; }
    const T& front() const { assert( !empty() ); return m_ptr[0]; }

    T& back() { assert( !empty() ); return m_write[-1]; }
    const T& back() const { assert( !empty() ); return m_write[-1]; }

    T& operator[]( size_t idx ) { return m_ptr[idx]; }
    const T& operator[]( size_t idx ) const { return m_ptr[idx]; }

    T* push_next()
    {
        if( m_write == m_end ) AllocMore();
        return m_write++;
    }

    T* prepare_next()
    {
        if( m_write == m_end ) AllocMore();
        return m_write;
    }

    void commit_next()
    {
        m_write++;
    }

    void clear()
    {
        m_write = m_ptr;
    }

    void swap( FastVector& vec )
    {
        const auto ptr1 = m_ptr;
        const auto ptr2 = vec.m_ptr;
        const auto write1 = m_write;
        const auto write2 = vec.m_write;
        const auto end1 = m_end;
        const auto end2 = vec.m_end;

        m_ptr = ptr2;
        vec.m_ptr = ptr1;
        m_write = write2;
        vec.m_write = write1;
        m_end = end2;
        vec.m_end = end1;
    }

private:
    tracy_no_inline void AllocMore()
    {
        const auto cap = size_t( m_end - m_ptr ) * 2;
        const auto size = size_t( m_write - m_ptr );
        T* ptr = (T*)tracy_malloc( sizeof( T ) * cap );
        memcpy( ptr, m_ptr, size * sizeof( T ) );
        tracy_free_fast( m_ptr );
        m_ptr = ptr;
        m_write = m_ptr + size;
        m_end = m_ptr + cap;
    }

    T* m_ptr;
    T* m_write;
    T* m_end;
};

}

#endif

/*** End of inlined file: TracyFastVector.hpp ***/


/*** Start of inlined file: TracyQueue.hpp ***/
#ifndef __TRACYQUEUE_HPP__
#define __TRACYQUEUE_HPP__

#include <stdint.h>

namespace tracy
{

enum class QueueType : uint8_t
{
    ZoneText,
    ZoneName,
    Message,
    MessageColor,
    MessageCallstack,
    MessageColorCallstack,
    MessageAppInfo,
    ZoneBeginAllocSrcLoc,
    ZoneBeginAllocSrcLocCallstack,
    CallstackSerial,
    Callstack,
    CallstackAlloc,
    CallstackSample,
    FrameImage,
    ZoneBegin,
    ZoneBeginCallstack,
    ZoneEnd,
    LockWait,
    LockObtain,
    LockRelease,
    LockSharedWait,
    LockSharedObtain,
    LockSharedRelease,
    LockName,
    MemAlloc,
    MemAllocNamed,
    MemFree,
    MemFreeNamed,
    MemAllocCallstack,
    MemAllocCallstackNamed,
    MemFreeCallstack,
    MemFreeCallstackNamed,
    GpuZoneBegin,
    GpuZoneBeginCallstack,
    GpuZoneBeginAllocSrcLoc,
    GpuZoneBeginAllocSrcLocCallstack,
    GpuZoneEnd,
    GpuZoneBeginSerial,
    GpuZoneBeginCallstackSerial,
    GpuZoneBeginAllocSrcLocSerial,
    GpuZoneBeginAllocSrcLocCallstackSerial,
    GpuZoneEndSerial,
    PlotData,
    ContextSwitch,
    ThreadWakeup,
    GpuTime,
    GpuContextName,
    Terminate,
    KeepAlive,
    ThreadContext,
    GpuCalibration,
    Crash,
    CrashReport,
    ZoneValidation,
    ZoneColor,
    ZoneValue,
    FrameMarkMsg,
    FrameMarkMsgStart,
    FrameMarkMsgEnd,
    SourceLocation,
    LockAnnounce,
    LockTerminate,
    LockMark,
    MessageLiteral,
    MessageLiteralColor,
    MessageLiteralCallstack,
    MessageLiteralColorCallstack,
    GpuNewContext,
    CallstackFrameSize,
    CallstackFrame,
    SymbolInformation,
    CodeInformation,
    SysTimeReport,
    TidToPid,
    HwSampleCpuCycle,
    HwSampleInstructionRetired,
    HwSampleCacheReference,
    HwSampleCacheMiss,
    HwSampleBranchRetired,
    HwSampleBranchMiss,
    PlotConfig,
    ParamSetup,
    AckServerQueryNoop,
    AckSourceCodeNotAvailable,
    CpuTopology,
    SingleStringData,
    SecondStringData,
    MemNamePayload,
    StringData,
    ThreadName,
    PlotName,
    SourceLocationPayload,
    CallstackPayload,
    CallstackAllocPayload,
    FrameName,
    FrameImageData,
    ExternalName,
    ExternalThreadName,
    SymbolCode,
    SourceCode,
    NUM_TYPES
};

#pragma pack( 1 )

struct QueueThreadContext
{
    uint64_t thread;
};

struct QueueZoneBeginLean
{
    int64_t time;
};

struct QueueZoneBegin : public QueueZoneBeginLean
{
    uint64_t srcloc;    // ptr
};

struct QueueZoneEnd
{
    int64_t time;
};

struct QueueZoneValidation
{
    uint32_t id;
};

struct QueueZoneColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueZoneValue
{
    uint64_t value;
};

struct QueueStringTransfer
{
    uint64_t ptr;
};

struct QueueFrameMark
{
    int64_t time;
    uint64_t name;      // ptr
};

struct QueueFrameImage
{
    uint32_t frame;
    uint16_t w;
    uint16_t h;
    uint8_t flip;
};

struct QueueFrameImageFat : public QueueFrameImage
{
    uint64_t image;     // ptr
};

struct QueueSourceLocation
{
    uint64_t name;
    uint64_t function;  // ptr
    uint64_t file;      // ptr
    uint32_t line;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueZoneTextFat
{
    uint64_t text;      // ptr
    uint16_t size;
};

enum class LockType : uint8_t
{
    Lockable,
    SharedLockable
};

struct QueueLockAnnounce
{
    uint32_t id;
    int64_t time;
    uint64_t lckloc;    // ptr
    LockType type;
};

struct QueueLockTerminate
{
    uint32_t id;
    int64_t time;
};

struct QueueLockWait
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockObtain
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockRelease
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockMark
{
    uint64_t thread;
    uint32_t id;
    uint64_t srcloc;    // ptr
};

struct QueueLockName
{
    uint32_t id;
};

struct QueueLockNameFat : public QueueLockName
{
    uint64_t name;      // ptr
    uint16_t size;
};

enum class PlotDataType : uint8_t
{
    Float,
    Double,
    Int
};

struct QueuePlotData
{
    uint64_t name;      // ptr
    int64_t time;
    PlotDataType type;
    union
    {
        double d;
        float f;
        int64_t i;
    } data;
};

struct QueueMessage
{
    int64_t time;
};

struct QueueMessageColor : public QueueMessage
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueMessageLiteral : public QueueMessage
{
    uint64_t text;      // ptr
};

struct QueueMessageColorLiteral : public QueueMessageColor
{
    uint64_t text;      // ptr
};

struct QueueMessageFat : public QueueMessage
{
    uint64_t text;      // ptr
    uint16_t size;
};

struct QueueMessageColorFat : public QueueMessageColor
{
    uint64_t text;      // ptr
    uint16_t size;
};

// Don't change order, only add new entries at the end, this is also used on trace dumps!
enum class GpuContextType : uint8_t
{
    Invalid,
    OpenGl,
    Vulkan,
    OpenCL,
    Direct3D12,
    Direct3D11
};

enum GpuContextFlags : uint8_t
{
    GpuContextCalibration   = 1 << 0
};

struct QueueGpuNewContext
{
    int64_t cpuTime;
    int64_t gpuTime;
    uint64_t thread;
    float period;
    uint8_t context;
    GpuContextFlags flags;
    GpuContextType type;
};

struct QueueGpuZoneBeginLean
{
    int64_t cpuTime;
    uint64_t thread;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuZoneBegin : public QueueGpuZoneBeginLean
{
    uint64_t srcloc;
};

struct QueueGpuZoneEnd
{
    int64_t cpuTime;
    uint64_t thread;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuTime
{
    int64_t gpuTime;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuCalibration
{
    int64_t gpuTime;
    int64_t cpuTime;
    int64_t cpuDelta;
    uint8_t context;
};

struct QueueGpuContextName
{
    uint8_t context;
};

struct QueueGpuContextNameFat : public QueueGpuContextName
{
    uint64_t ptr;
    uint16_t size;
};

struct QueueMemNamePayload
{
    uint64_t name;
};

struct QueueMemAlloc
{
    int64_t time;
    uint64_t thread;
    uint64_t ptr;
    char size[6];
};

struct QueueMemFree
{
    int64_t time;
    uint64_t thread;
    uint64_t ptr;
};

struct QueueCallstackFat
{
    uint64_t ptr;
};

struct QueueCallstackAllocFat
{
    uint64_t ptr;
    uint64_t nativePtr;
};

struct QueueCallstackSample
{
    int64_t time;
    uint64_t thread;
};

struct QueueCallstackSampleFat : public QueueCallstackSample
{
    uint64_t ptr;
};

struct QueueCallstackFrameSize
{
    uint64_t ptr;
    uint8_t size;
};

struct QueueCallstackFrame
{
    uint32_t line;
    uint64_t symAddr;
    uint32_t symLen;
};

struct QueueSymbolInformation
{
    uint32_t line;
    uint64_t symAddr;
};

struct QueueCodeInformation
{
    uint64_t ptr;
    uint32_t line;
    uint64_t symAddr;
};

struct QueueCrashReport
{
    int64_t time;
    uint64_t text;      // ptr
};

struct QueueSysTime
{
    int64_t time;
    float sysTime;
};

struct QueueContextSwitch
{
    int64_t time;
    uint64_t oldThread;
    uint64_t newThread;
    uint8_t cpu;
    uint8_t reason;
    uint8_t state;
};

struct QueueThreadWakeup
{
    int64_t time;
    uint64_t thread;
};

struct QueueTidToPid
{
    uint64_t tid;
    uint64_t pid;
};

struct QueueHwSample
{
    uint64_t ip;
    int64_t time;
};

enum class PlotFormatType : uint8_t
{
    Number,
    Memory,
    Percentage
};

struct QueuePlotConfig
{
    uint64_t name;      // ptr
    uint8_t type;
};

struct QueueParamSetup
{
    uint32_t idx;
    uint64_t name;      // ptr
    uint8_t isBool;
    int32_t val;
};

struct QueueCpuTopology
{
    uint32_t package;
    uint32_t core;
    uint32_t thread;
};

struct QueueHeader
{
    union
    {
        QueueType type;
        uint8_t idx;
    };
};

struct QueueItem
{
    QueueHeader hdr;
    union
    {
        QueueThreadContext threadCtx;
        QueueZoneBegin zoneBegin;
        QueueZoneBeginLean zoneBeginLean;
        QueueZoneEnd zoneEnd;
        QueueZoneValidation zoneValidation;
        QueueZoneColor zoneColor;
        QueueZoneValue zoneValue;
        QueueStringTransfer stringTransfer;
        QueueFrameMark frameMark;
        QueueFrameImage frameImage;
        QueueFrameImageFat frameImageFat;
        QueueSourceLocation srcloc;
        QueueZoneTextFat zoneTextFat;
        QueueLockAnnounce lockAnnounce;
        QueueLockTerminate lockTerminate;
        QueueLockWait lockWait;
        QueueLockObtain lockObtain;
        QueueLockRelease lockRelease;
        QueueLockMark lockMark;
        QueueLockName lockName;
        QueueLockNameFat lockNameFat;
        QueuePlotData plotData;
        QueueMessage message;
        QueueMessageColor messageColor;
        QueueMessageLiteral messageLiteral;
        QueueMessageColorLiteral messageColorLiteral;
        QueueMessageFat messageFat;
        QueueMessageColorFat messageColorFat;
        QueueGpuNewContext gpuNewContext;
        QueueGpuZoneBegin gpuZoneBegin;
        QueueGpuZoneBeginLean gpuZoneBeginLean;
        QueueGpuZoneEnd gpuZoneEnd;
        QueueGpuTime gpuTime;
        QueueGpuCalibration gpuCalibration;
        QueueGpuContextName gpuContextName;
        QueueGpuContextNameFat gpuContextNameFat;
        QueueMemAlloc memAlloc;
        QueueMemFree memFree;
        QueueMemNamePayload memName;
        QueueCallstackFat callstackFat;
        QueueCallstackAllocFat callstackAllocFat;
        QueueCallstackSample callstackSample;
        QueueCallstackSampleFat callstackSampleFat;
        QueueCallstackFrameSize callstackFrameSize;
        QueueCallstackFrame callstackFrame;
        QueueSymbolInformation symbolInformation;
        QueueCodeInformation codeInformation;
        QueueCrashReport crashReport;
        QueueSysTime sysTime;
        QueueContextSwitch contextSwitch;
        QueueThreadWakeup threadWakeup;
        QueueTidToPid tidToPid;
        QueueHwSample hwSample;
        QueuePlotConfig plotConfig;
        QueueParamSetup paramSetup;
        QueueCpuTopology cpuTopology;
    };
};
#pragma pack()

enum { QueueItemSize = sizeof( QueueItem ) };

static constexpr size_t QueueDataSize[] = {
    sizeof( QueueHeader ),                                  // zone text
    sizeof( QueueHeader ),                                  // zone name
    sizeof( QueueHeader ) + sizeof( QueueMessage ),
    sizeof( QueueHeader ) + sizeof( QueueMessageColor ),
    sizeof( QueueHeader ) + sizeof( QueueMessage ),         // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessageColor ),    // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessage ),         // app info
    sizeof( QueueHeader ) + sizeof( QueueZoneBeginLean ),   // allocated source location
    sizeof( QueueHeader ) + sizeof( QueueZoneBeginLean ),   // allocated source location, callstack
    sizeof( QueueHeader ),                                  // callstack memory
    sizeof( QueueHeader ),                                  // callstack
    sizeof( QueueHeader ),                                  // callstack alloc
    sizeof( QueueHeader ) + sizeof( QueueCallstackSample ),
    sizeof( QueueHeader ) + sizeof( QueueFrameImage ),
    sizeof( QueueHeader ) + sizeof( QueueZoneBegin ),
    sizeof( QueueHeader ) + sizeof( QueueZoneBegin ),       // callstack
    sizeof( QueueHeader ) + sizeof( QueueZoneEnd ),
    sizeof( QueueHeader ) + sizeof( QueueLockWait ),
    sizeof( QueueHeader ) + sizeof( QueueLockObtain ),
    sizeof( QueueHeader ) + sizeof( QueueLockRelease ),
    sizeof( QueueHeader ) + sizeof( QueueLockWait ),        // shared
    sizeof( QueueHeader ) + sizeof( QueueLockObtain ),      // shared
    sizeof( QueueHeader ) + sizeof( QueueLockRelease ),     // shared
    sizeof( QueueHeader ) + sizeof( QueueLockName ),
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // named
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // named
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // callstack
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // callstack, named
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // callstack
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // callstack, named
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// allocated source location
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// allocated source location, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneEnd ),
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // serial
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // serial, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// serial, allocated source location
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// serial, allocated source location, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneEnd ),      // serial
    sizeof( QueueHeader ) + sizeof( QueuePlotData ),
    sizeof( QueueHeader ) + sizeof( QueueContextSwitch ),
    sizeof( QueueHeader ) + sizeof( QueueThreadWakeup ),
    sizeof( QueueHeader ) + sizeof( QueueGpuTime ),
    sizeof( QueueHeader ) + sizeof( QueueGpuContextName ),
    // above items must be first
    sizeof( QueueHeader ),                                  // terminate
    sizeof( QueueHeader ),                                  // keep alive
    sizeof( QueueHeader ) + sizeof( QueueThreadContext ),
    sizeof( QueueHeader ) + sizeof( QueueGpuCalibration ),
    sizeof( QueueHeader ),                                  // crash
    sizeof( QueueHeader ) + sizeof( QueueCrashReport ),
    sizeof( QueueHeader ) + sizeof( QueueZoneValidation ),
    sizeof( QueueHeader ) + sizeof( QueueZoneColor ),
    sizeof( QueueHeader ) + sizeof( QueueZoneValue ),
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // continuous frames
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // start
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // end
    sizeof( QueueHeader ) + sizeof( QueueSourceLocation ),
    sizeof( QueueHeader ) + sizeof( QueueLockAnnounce ),
    sizeof( QueueHeader ) + sizeof( QueueLockTerminate ),
    sizeof( QueueHeader ) + sizeof( QueueLockMark ),
    sizeof( QueueHeader ) + sizeof( QueueMessageLiteral ),
    sizeof( QueueHeader ) + sizeof( QueueMessageColorLiteral ),
    sizeof( QueueHeader ) + sizeof( QueueMessageLiteral ),  // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessageColorLiteral ), // callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuNewContext ),
    sizeof( QueueHeader ) + sizeof( QueueCallstackFrameSize ),
    sizeof( QueueHeader ) + sizeof( QueueCallstackFrame ),
    sizeof( QueueHeader ) + sizeof( QueueSymbolInformation ),
    sizeof( QueueHeader ) + sizeof( QueueCodeInformation ),
    sizeof( QueueHeader ) + sizeof( QueueSysTime ),
    sizeof( QueueHeader ) + sizeof( QueueTidToPid ),
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cpu cycle
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // instruction retired
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cache reference
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cache miss
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // branch retired
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // branch miss
    sizeof( QueueHeader ) + sizeof( QueuePlotConfig ),
    sizeof( QueueHeader ) + sizeof( QueueParamSetup ),
    sizeof( QueueHeader ),                                  // server query acknowledgement
    sizeof( QueueHeader ),                                  // source code not available
    sizeof( QueueHeader ) + sizeof( QueueCpuTopology ),
    sizeof( QueueHeader ),                                  // single string data
    sizeof( QueueHeader ),                                  // second string data
    sizeof( QueueHeader ) + sizeof( QueueMemNamePayload ),
    // keep all QueueStringTransfer below
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // string data
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // thread name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // plot name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // allocated source location payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // callstack payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // callstack alloc payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // frame name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // frame image data
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // external name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // external thread name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // symbol code
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // source code
};

static_assert( QueueItemSize == 32, "Queue item size not 32 bytes" );
static_assert( sizeof( QueueDataSize ) / sizeof( size_t ) == (uint8_t)QueueType::NUM_TYPES, "QueueDataSize mismatch" );
static_assert( sizeof( void* ) <= sizeof( uint64_t ), "Pointer size > 8 bytes" );
static_assert( sizeof( void* ) == sizeof( uintptr_t ), "Pointer size != uintptr_t" );

}

#endif

/*** End of inlined file: TracyQueue.hpp ***/


/*** Start of inlined file: TracyMutex.hpp ***/
#ifndef __TRACYMUTEX_HPP__
#define __TRACYMUTEX_HPP__

#if defined _MSC_VER

#  include <shared_mutex>

namespace tracy
{
using TracyMutex = std::shared_mutex;
}

#else

#include <mutex>

namespace tracy
{
using TracyMutex = std::mutex;
}

#endif

#endif

/*** End of inlined file: TracyMutex.hpp ***/


/*** Start of inlined file: TracyProtocol.hpp ***/
#ifndef __TRACYPROTOCOL_HPP__
#define __TRACYPROTOCOL_HPP__

#include <limits>
#include <stdint.h>

namespace tracy
{

constexpr unsigned Lz4CompressBound( unsigned isize ) { return isize + ( isize / 255 ) + 16; }

enum : uint32_t { ProtocolVersion = 48 };
enum : uint16_t { BroadcastVersion = 2 };

using lz4sz_t = uint32_t;

enum { TargetFrameSize = 256 * 1024 };
enum { LZ4Size = Lz4CompressBound( TargetFrameSize ) };
static_assert( LZ4Size <= std::numeric_limits<lz4sz_t>::max(), "LZ4Size greater than lz4sz_t" );
static_assert( TargetFrameSize * 2 >= 64 * 1024, "Not enough space for LZ4 stream buffer" );

enum { HandshakeShibbolethSize = 8 };
static const char HandshakeShibboleth[HandshakeShibbolethSize] = { 'T', 'r', 'a', 'c', 'y', 'P', 'r', 'f' };

enum HandshakeStatus : uint8_t
{
    HandshakePending,
    HandshakeWelcome,
    HandshakeProtocolMismatch,
    HandshakeNotAvailable,
    HandshakeDropped
};

enum { WelcomeMessageProgramNameSize = 64 };
enum { WelcomeMessageHostInfoSize = 1024 };

#pragma pack( 1 )

// Must increase left query space after handling!
enum ServerQuery : uint8_t
{
    ServerQueryTerminate,
    ServerQueryString,
    ServerQueryThreadString,
    ServerQuerySourceLocation,
    ServerQueryPlotName,
    ServerQueryCallstackFrame,
    ServerQueryFrameName,
    ServerQueryDisconnect,
    ServerQueryExternalName,
    ServerQueryParameter,
    ServerQuerySymbol,
    ServerQuerySymbolCode,
    ServerQueryCodeLocation,
    ServerQuerySourceCode,
    ServerQueryDataTransfer,
    ServerQueryDataTransferPart
};

struct ServerQueryPacket
{
    ServerQuery type;
    uint64_t ptr;
    uint32_t extra;
};

enum { ServerQueryPacketSize = sizeof( ServerQueryPacket ) };

enum CpuArchitecture : uint8_t
{
    CpuArchUnknown,
    CpuArchX86,
    CpuArchX64,
    CpuArchArm32,
    CpuArchArm64
};

struct WelcomeFlag
{
    enum _t : uint8_t
    {
        OnDemand        = 1 << 0,
        IsApple         = 1 << 1,
        CodeTransfer    = 1 << 2,
        CombineSamples  = 1 << 3,
    };
};

struct WelcomeMessage
{
    double timerMul;
    int64_t initBegin;
    int64_t initEnd;
    uint64_t delay;
    uint64_t resolution;
    uint64_t epoch;
    uint64_t exectime;
    uint64_t pid;
    int64_t samplingPeriod;
    uint8_t flags;
    uint8_t cpuArch;
    char cpuManufacturer[12];
    uint32_t cpuId;
    char programName[WelcomeMessageProgramNameSize];
    char hostInfo[WelcomeMessageHostInfoSize];
};

enum { WelcomeMessageSize = sizeof( WelcomeMessage ) };

struct OnDemandPayloadMessage
{
    uint64_t frames;
    uint64_t currentTime;
};

enum { OnDemandPayloadMessageSize = sizeof( OnDemandPayloadMessage ) };

struct BroadcastMessage
{
    uint16_t broadcastVersion;
    uint16_t listenPort;
    uint32_t protocolVersion;
    int32_t activeTime;        // in seconds
    char programName[WelcomeMessageProgramNameSize];
};

enum { BroadcastMessageSize = sizeof( BroadcastMessage ) };

#pragma pack()

}

#endif

/*** End of inlined file: TracyProtocol.hpp ***/

#if defined _WIN32 || defined __CYGWIN__
#  include <intrin.h>
#endif
#ifdef __APPLE__
#  include <TargetConditionals.h>
#  include <mach/mach_time.h>
#endif

#if !defined TRACY_TIMER_FALLBACK && ( defined _WIN32 || defined __CYGWIN__ || ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 ) || ( defined TARGET_OS_IOS && TARGET_OS_IOS == 1 ) )
#  define TRACY_HW_TIMER
#endif

#if !defined TRACY_HW_TIMER
#  include <chrono>
#endif

#ifndef TracyConcat
#  define TracyConcat(x,y) TracyConcatIndirect(x,y)
#endif
#ifndef TracyConcatIndirect
#  define TracyConcatIndirect(x,y) x##y
#endif

namespace tracy
{
#if defined(TRACY_DELAYED_INIT) && defined(TRACY_MANUAL_LIFETIME)
TRACY_API void StartupProfiler();
TRACY_API void ShutdownProfiler();
#endif

class GpuCtx;
class Profiler;
class Socket;
class UdpBroadcast;

struct GpuCtxWrapper
{
    GpuCtx* ptr;
};

TRACY_API moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* GetToken();
TRACY_API Profiler& GetProfiler();
TRACY_API std::atomic<uint32_t>& GetLockCounter();
TRACY_API std::atomic<uint8_t>& GetGpuCtxCounter();
TRACY_API GpuCtxWrapper& GetGpuCtx();
TRACY_API uint64_t GetThreadHandle();
TRACY_API bool ProfilerAvailable();
TRACY_API int64_t GetFrequencyQpc();

struct SourceLocationData
{
    const char* name;
    const char* function;
    const char* file;
    uint32_t line;
    uint32_t color;
};

#ifdef TRACY_ON_DEMAND
struct LuaZoneState
{
    uint32_t counter;
    bool active;
};
#endif

#define TracyLfqPrepare( _type ) \
    moodycamel::ConcurrentQueueDefaultTraits::index_t __magic; \
    auto __token = GetToken(); \
    auto& __tail = __token->get_tail_index(); \
    auto item = __token->enqueue_begin( __magic ); \
    MemWrite( &item->hdr.type, _type );

#define TracyLfqCommit \
    __tail.store( __magic + 1, std::memory_order_release );

#define TracyLfqPrepareC( _type ) \
    tracy::moodycamel::ConcurrentQueueDefaultTraits::index_t __magic; \
    auto __token = tracy::GetToken(); \
    auto& __tail = __token->get_tail_index(); \
    auto item = __token->enqueue_begin( __magic ); \
    tracy::MemWrite( &item->hdr.type, _type );

#define TracyLfqCommitC \
    __tail.store( __magic + 1, std::memory_order_release );

typedef void(*ParameterCallback)( uint32_t idx, int32_t val );

class Profiler
{
    struct FrameImageQueueItem
    {
        void* image;
        uint32_t frame;
        uint16_t w;
        uint16_t h;
        uint8_t offset;
        bool flip;
    };

public:
    Profiler();
    ~Profiler();

    void SpawnWorkerThreads();

    static tracy_force_inline int64_t GetTime()
    {
#ifdef TRACY_HW_TIMER
#  if defined TARGET_OS_IOS && TARGET_OS_IOS == 1
        return mach_absolute_time();
#  elif defined _WIN32 || defined __CYGWIN__
#    ifdef TRACY_TIMER_QPC
        return GetTimeQpc();
#    else
        return int64_t( __rdtsc() );
#    endif
#  elif defined __i386 || defined _M_IX86
        uint32_t eax, edx;
        asm volatile ( "rdtsc" : "=a" (eax), "=d" (edx) );
        return ( uint64_t( edx ) << 32 ) + uint64_t( eax );
#  elif defined __x86_64__ || defined _M_X64
        uint64_t rax, rdx;
        asm volatile ( "rdtsc" : "=a" (rax), "=d" (rdx) );
        return (int64_t)(( rdx << 32 ) + rax);
#  else
#    error "TRACY_HW_TIMER detection logic needs fixing"
#  endif
#else
#  if defined __linux__ && defined CLOCK_MONOTONIC_RAW
        struct timespec ts;
        clock_gettime( CLOCK_MONOTONIC_RAW, &ts );
        return int64_t( ts.tv_sec ) * 1000000000ll + int64_t( ts.tv_nsec );
#  else
        return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
#  endif
#endif
    }

    tracy_force_inline uint32_t GetNextZoneId()
    {
        return m_zoneId.fetch_add( 1, std::memory_order_relaxed );
    }

    static tracy_force_inline QueueItem* QueueSerial()
    {
        auto& p = GetProfiler();
        p.m_serialLock.lock();
        return p.m_serialQueue.prepare_next();
    }

    static tracy_force_inline QueueItem* QueueSerialCallstack( void* ptr )
    {
        auto& p = GetProfiler();
        p.m_serialLock.lock();
        p.SendCallstackSerial( ptr );
        return p.m_serialQueue.prepare_next();
    }

    static tracy_force_inline void QueueSerialFinish()
    {
        auto& p = GetProfiler();
        p.m_serialQueue.commit_next();
        p.m_serialLock.unlock();
    }

    static tracy_force_inline void SendFrameMark( const char* name )
    {
        if( !name ) GetProfiler().m_frameCount.fetch_add( 1, std::memory_order_relaxed );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::FrameMarkMsg );
        MemWrite( &item->frameMark.time, GetTime() );
        MemWrite( &item->frameMark.name, uint64_t( name ) );
        TracyLfqCommit;
    }

    static tracy_force_inline void SendFrameMark( const char* name, QueueType type )
    {
        assert( type == QueueType::FrameMarkMsgStart || type == QueueType::FrameMarkMsgEnd );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        auto item = QueueSerial();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->frameMark.time, GetTime() );
        MemWrite( &item->frameMark.name, uint64_t( name ) );
        QueueSerialFinish();
    }

    static tracy_force_inline void SendFrameImage( const void* image, uint16_t w, uint16_t h, uint8_t offset, bool flip )
    {
#ifndef TRACY_NO_FRAME_IMAGE
        auto& profiler = GetProfiler();
        assert( profiler.m_frameCount.load( std::memory_order_relaxed ) < std::numeric_limits<uint32_t>::max() );
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto sz = size_t( w ) * size_t( h ) * 4;
        auto ptr = (char*)tracy_malloc( sz );
        memcpy( ptr, image, sz );

        profiler.m_fiLock.lock();
        auto fi = profiler.m_fiQueue.prepare_next();
        fi->image = ptr;
        fi->frame = uint32_t( profiler.m_frameCount.load( std::memory_order_relaxed ) - offset );
        fi->w = w;
        fi->h = h;
        fi->flip = flip;
        profiler.m_fiQueue.commit_next();
        profiler.m_fiLock.unlock();
#endif
    }

    static tracy_force_inline void PlotData( const char* name, int64_t val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Int );
        MemWrite( &item->plotData.data.i, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void PlotData( const char* name, float val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Float );
        MemWrite( &item->plotData.data.f, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void PlotData( const char* name, double val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Double );
        MemWrite( &item->plotData.data.d, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void ConfigurePlot( const char* name, PlotFormatType type )
    {
        TracyLfqPrepare( QueueType::PlotConfig );
        MemWrite( &item->plotConfig.name, (uint64_t)name );
        MemWrite( &item->plotConfig.type, (uint8_t)type );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    static tracy_force_inline void Message( const char* txt, size_t size, int callstack )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::Message : QueueType::MessageCallstack );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        MemWrite( &item->messageFat.time, GetTime() );
        MemWrite( &item->messageFat.text, (uint64_t)ptr );
        MemWrite( &item->messageFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    static tracy_force_inline void Message( const char* txt, int callstack )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageLiteral : QueueType::MessageLiteralCallstack );
        MemWrite( &item->messageLiteral.time, GetTime() );
        MemWrite( &item->messageLiteral.text, (uint64_t)txt );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageColor( const char* txt, size_t size, uint32_t color, int callstack )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageColor : QueueType::MessageColorCallstack );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        MemWrite( &item->messageColorFat.time, GetTime() );
        MemWrite( &item->messageColorFat.text, (uint64_t)ptr );
        MemWrite( &item->messageColorFat.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->messageColorFat.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->messageColorFat.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        MemWrite( &item->messageColorFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageColor( const char* txt, uint32_t color, int callstack )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageLiteralColor : QueueType::MessageLiteralColorCallstack );
        MemWrite( &item->messageColorLiteral.time, GetTime() );
        MemWrite( &item->messageColorLiteral.text, (uint64_t)txt );
        MemWrite( &item->messageColorLiteral.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->messageColorLiteral.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->messageColorLiteral.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageAppInfo( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::MessageAppInfo );
        MemWrite( &item->messageFat.time, GetTime() );
        MemWrite( &item->messageFat.text, (uint64_t)ptr );
        MemWrite( &item->messageFat.size, (uint16_t)size );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    static tracy_force_inline void MemAlloc( const void* ptr, size_t size, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemAlloc( QueueType::MemAlloc, thread, ptr, size );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemFree( const void* ptr, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemFree( QueueType::MemFree, thread, ptr );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemAllocCallstack( const void* ptr, size_t size, int depth, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemAlloc( QueueType::MemAllocCallstack, thread, ptr, size );
        profiler.m_serialLock.unlock();
#else
        MemAlloc( ptr, size, secure );
#endif
    }

    static tracy_force_inline void MemFreeCallstack( const void* ptr, int depth, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemFree( QueueType::MemFreeCallstack, thread, ptr );
        profiler.m_serialLock.unlock();
#else
        MemFree( ptr, secure );
#endif
    }

    static tracy_force_inline void MemAllocNamed( const void* ptr, size_t size, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemName( name );
        SendMemAlloc( QueueType::MemAllocNamed, thread, ptr, size );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemFreeNamed( const void* ptr, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemName( name );
        SendMemFree( QueueType::MemFreeNamed, thread, ptr );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemAllocCallstackNamed( const void* ptr, size_t size, int depth, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemName( name );
        SendMemAlloc( QueueType::MemAllocCallstackNamed, thread, ptr, size );
        profiler.m_serialLock.unlock();
#else
        MemAlloc( ptr, size, secure );
#endif
    }

    static tracy_force_inline void MemFreeCallstackNamed( const void* ptr, int depth, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemName( name );
        SendMemFree( QueueType::MemFreeCallstackNamed, thread, ptr );
        profiler.m_serialLock.unlock();
#else
        MemFree( ptr, secure );
#endif
    }

    static tracy_force_inline void SendCallstack( int depth )
    {
#ifdef TRACY_HAS_CALLSTACK
        auto ptr = Callstack( depth );
        TracyLfqPrepare( QueueType::Callstack );
        MemWrite( &item->callstackFat.ptr, (uint64_t)ptr );
        TracyLfqCommit;
#endif
    }

    static tracy_force_inline void ParameterRegister( ParameterCallback cb ) { GetProfiler().m_paramCallback = cb; }
    static tracy_force_inline void ParameterSetup( uint32_t idx, const char* name, bool isBool, int32_t val )
    {
        TracyLfqPrepare( QueueType::ParamSetup );
        tracy::MemWrite( &item->paramSetup.idx, idx );
        tracy::MemWrite( &item->paramSetup.name, (uint64_t)name );
        tracy::MemWrite( &item->paramSetup.isBool, (uint8_t)isBool );
        tracy::MemWrite( &item->paramSetup.val, val );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    void SendCallstack( int depth, const char* skipBefore );
    static void CutCallstack( void* callstack, const char* skipBefore );

    static bool ShouldExit();

    tracy_force_inline bool IsConnected() const
    {
        return m_isConnected.load( std::memory_order_acquire );
    }

#ifdef TRACY_ON_DEMAND
    tracy_force_inline uint64_t ConnectionId() const
    {
        return m_connectionId.load( std::memory_order_acquire );
    }

    tracy_force_inline void DeferItem( const QueueItem& item )
    {
        m_deferredLock.lock();
        auto dst = m_deferredQueue.push_next();
        memcpy( dst, &item, sizeof( item ) );
        m_deferredLock.unlock();
    }
#endif

    void RequestShutdown() { m_shutdown.store( true, std::memory_order_relaxed ); m_shutdownManual.store( true, std::memory_order_relaxed ); }
    bool HasShutdownFinished() const { return m_shutdownFinished.load( std::memory_order_relaxed ); }

    void SendString( uint64_t str, const char* ptr, QueueType type ) { SendString( str, ptr, strlen( ptr ), type ); }
    void SendString( uint64_t str, const char* ptr, size_t len, QueueType type );
    void SendSingleString( const char* ptr ) { SendSingleString( ptr, strlen( ptr ) ); }
    void SendSingleString( const char* ptr, size_t len );
    void SendSecondString( const char* ptr ) { SendSecondString( ptr, strlen( ptr ) ); }
    void SendSecondString( const char* ptr, size_t len );

    // Allocated source location data layout:
    //  2b  payload size
    //  4b  color
    //  4b  source line
    //  fsz function name
    //  1b  null terminator
    //  ssz source file name
    //  1b  null terminator
    //  nsz zone name (optional)

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, const char* function )
    {
        return AllocSourceLocation( line, source, function, nullptr, 0 );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, const char* function, const char* name, size_t nameSz )
    {
        return AllocSourceLocation( line, source, strlen(source), function, strlen(function), name, nameSz );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz )
    {
        return AllocSourceLocation( line, source, sourceSz, function, functionSz, nullptr, 0 );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz )
    {
        const auto sz32 = uint32_t( 2 + 4 + 4 + functionSz + 1 + sourceSz + 1 + nameSz );
        assert( sz32 <= std::numeric_limits<uint16_t>::max() );
        const auto sz = uint16_t( sz32 );
        auto ptr = (char*)tracy_malloc( sz );
        memcpy( ptr, &sz, 2 );
        memset( ptr + 2, 0, 4 );
        memcpy( ptr + 6, &line, 4 );
        memcpy( ptr + 10, function, functionSz );
        ptr[10 + functionSz] = '\0';
        memcpy( ptr + 10 + functionSz + 1, source, sourceSz );
        ptr[10 + functionSz + 1 + sourceSz] = '\0';
        if( nameSz != 0 )
        {
            memcpy( ptr + 10 + functionSz + 1 + sourceSz + 1, name, nameSz );
        }
        return uint64_t( ptr );
    }

private:
    enum class DequeueStatus { DataDequeued, ConnectionLost, QueueEmpty };

    static void LaunchWorker( void* ptr ) { ((Profiler*)ptr)->Worker(); }
    void Worker();

#ifndef TRACY_NO_FRAME_IMAGE
    static void LaunchCompressWorker( void* ptr ) { ((Profiler*)ptr)->CompressWorker(); }
    void CompressWorker();
#endif

    void ClearQueues( tracy::moodycamel::ConsumerToken& token );
    void ClearSerial();
    DequeueStatus Dequeue( tracy::moodycamel::ConsumerToken& token );
    DequeueStatus DequeueContextSwitches( tracy::moodycamel::ConsumerToken& token, int64_t& timeStop );
    DequeueStatus DequeueSerial();
    bool CommitData();

    tracy_force_inline bool AppendData( const void* data, size_t len )
    {
        const auto ret = NeedDataSize( len );
        AppendDataUnsafe( data, len );
        return ret;
    }

    tracy_force_inline bool NeedDataSize( size_t len )
    {
        assert( len <= TargetFrameSize );
        bool ret = true;
        if( m_bufferOffset - m_bufferStart + (int)len > TargetFrameSize )
        {
            ret = CommitData();
        }
        return ret;
    }

    tracy_force_inline void AppendDataUnsafe( const void* data, size_t len )
    {
        memcpy( m_buffer + m_bufferOffset, data, len );
        m_bufferOffset += int( len );
    }

    bool SendData( const char* data, size_t len );
    void SendLongString( uint64_t ptr, const char* str, size_t len, QueueType type );
    void SendSourceLocation( uint64_t ptr );
    void SendSourceLocationPayload( uint64_t ptr );
    void SendCallstackPayload( uint64_t ptr );
    void SendCallstackPayload64( uint64_t ptr );
    void SendCallstackAlloc( uint64_t ptr );
    void SendCallstackFrame( uint64_t ptr );
    void SendCodeLocation( uint64_t ptr );

    bool HandleServerQuery();
    void HandleDisconnect();
    void HandleParameter( uint64_t payload );
    void HandleSymbolQuery( uint64_t symbol );
    void HandleSymbolCodeQuery( uint64_t symbol, uint32_t size );
    void HandleSourceCodeQuery();

    void AckServerQuery();
    void AckSourceCodeNotAvailable();

    void CalibrateTimer();
    void CalibrateDelay();
    void ReportTopology();

    static tracy_force_inline void SendCallstackSerial( void* ptr )
    {
#ifdef TRACY_HAS_CALLSTACK
        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, QueueType::CallstackSerial );
        MemWrite( &item->callstackFat.ptr, (uint64_t)ptr );
        GetProfiler().m_serialQueue.commit_next();
#endif
    }

    static tracy_force_inline void SendMemAlloc( QueueType type, const uint64_t thread, const void* ptr, size_t size )
    {
        assert( type == QueueType::MemAlloc || type == QueueType::MemAllocCallstack || type == QueueType::MemAllocNamed || type == QueueType::MemAllocCallstackNamed );

        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->memAlloc.time, GetTime() );
        MemWrite( &item->memAlloc.thread, thread );
        MemWrite( &item->memAlloc.ptr, (uint64_t)ptr );
        if( compile_time_condition<sizeof( size ) == 4>::value )
        {
            memcpy( &item->memAlloc.size, &size, 4 );
            memset( &item->memAlloc.size + 4, 0, 2 );
        }
        else
        {
            assert( sizeof( size ) == 8 );
            memcpy( &item->memAlloc.size, &size, 4 );
            memcpy( ((char*)&item->memAlloc.size)+4, ((char*)&size)+4, 2 );
        }
        GetProfiler().m_serialQueue.commit_next();
    }

    static tracy_force_inline void SendMemFree( QueueType type, const uint64_t thread, const void* ptr )
    {
        assert( type == QueueType::MemFree || type == QueueType::MemFreeCallstack || type == QueueType::MemFreeNamed || type == QueueType::MemFreeCallstackNamed );

        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->memFree.time, GetTime() );
        MemWrite( &item->memFree.thread, thread );
        MemWrite( &item->memFree.ptr, (uint64_t)ptr );
        GetProfiler().m_serialQueue.commit_next();
    }

    static tracy_force_inline void SendMemName( const char* name )
    {
        assert( name );
        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, QueueType::MemNamePayload );
        MemWrite( &item->memName.name, (uint64_t)name );
        GetProfiler().m_serialQueue.commit_next();
    }

#if ( defined _WIN32 || defined __CYGWIN__ ) && defined TRACY_TIMER_QPC
    static int64_t GetTimeQpc();
#endif

    double m_timerMul;
    uint64_t m_resolution;
    uint64_t m_delay;
    std::atomic<int64_t> m_timeBegin;
    uint64_t m_mainThread;
    uint64_t m_epoch, m_exectime;
    std::atomic<bool> m_shutdown;
    std::atomic<bool> m_shutdownManual;
    std::atomic<bool> m_shutdownFinished;
    Socket* m_sock;
    UdpBroadcast* m_broadcast;
    bool m_noExit;
    uint32_t m_userPort;
    std::atomic<uint32_t> m_zoneId;
    int64_t m_samplingPeriod;

    uint64_t m_threadCtx;
    int64_t m_refTimeThread;
    int64_t m_refTimeSerial;
    int64_t m_refTimeCtx;
    int64_t m_refTimeGpu;

    void* m_stream;     // LZ4_stream_t*
    char* m_buffer;
    int m_bufferOffset;
    int m_bufferStart;

    char* m_lz4Buf;

    FastVector<QueueItem> m_serialQueue, m_serialDequeue;
    TracyMutex m_serialLock;

#ifndef TRACY_NO_FRAME_IMAGE
    FastVector<FrameImageQueueItem> m_fiQueue, m_fiDequeue;
    TracyMutex m_fiLock;
#endif

    std::atomic<uint64_t> m_frameCount;
    std::atomic<bool> m_isConnected;
#ifdef TRACY_ON_DEMAND
    std::atomic<uint64_t> m_connectionId;

    TracyMutex m_deferredLock;
    FastVector<QueueItem> m_deferredQueue;
#endif

#ifdef TRACY_HAS_SYSTIME
    void ProcessSysTime();

    SysTime m_sysTime;
    uint64_t m_sysTimeLast = 0;
#else
    void ProcessSysTime() {}
#endif

    ParameterCallback m_paramCallback;

    char* m_queryData;
    char* m_queryDataPtr;

#if defined _WIN32 || defined __CYGWIN__
    void* m_exceptionHandler;
#endif
#ifdef __linux__
    struct {
        struct sigaction pwr, ill, fpe, segv, pipe, bus, abrt;
    } m_prevSignal;
#endif
    bool m_crashHandlerInstalled;
};

}

#endif

/*** End of inlined file: TracyProfiler.hpp ***/

namespace tracy
{

class LockableCtx
{
public:
    tracy_force_inline LockableCtx( const SourceLocationData* srcloc )
        : m_id( GetLockCounter().fetch_add( 1, std::memory_order_relaxed ) )
#ifdef TRACY_ON_DEMAND
        , m_lockCount( 0 )
        , m_active( false )
#endif
    {
        assert( m_id != std::numeric_limits<uint32_t>::max() );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockAnnounce );
        MemWrite( &item->lockAnnounce.id, m_id );
        MemWrite( &item->lockAnnounce.time, Profiler::GetTime() );
        MemWrite( &item->lockAnnounce.lckloc, (uint64_t)srcloc );
        MemWrite( &item->lockAnnounce.type, LockType::Lockable );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

    LockableCtx( const LockableCtx& ) = delete;
    LockableCtx& operator=( const LockableCtx& ) = delete;

    tracy_force_inline ~LockableCtx()
    {
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockTerminate );
        MemWrite( &item->lockTerminate.id, m_id );
        MemWrite( &item->lockTerminate.time, Profiler::GetTime() );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline bool BeforeLock()
    {
#ifdef TRACY_ON_DEMAND
        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return false;
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockWait );
        MemWrite( &item->lockWait.thread, GetThreadHandle() );
        MemWrite( &item->lockWait.id, m_id );
        MemWrite( &item->lockWait.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
        return true;
    }

    tracy_force_inline void AfterLock()
    {
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockObtain );
        MemWrite( &item->lockObtain.thread, GetThreadHandle() );
        MemWrite( &item->lockObtain.id, m_id );
        MemWrite( &item->lockObtain.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterUnlock()
    {
#ifdef TRACY_ON_DEMAND
        m_lockCount.fetch_sub( 1, std::memory_order_relaxed );
        if( !m_active.load( std::memory_order_relaxed ) ) return;
        if( !GetProfiler().IsConnected() )
        {
            m_active.store( false, std::memory_order_relaxed );
            return;
        }
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockRelease );
        MemWrite( &item->lockRelease.thread, GetThreadHandle() );
        MemWrite( &item->lockRelease.id, m_id );
        MemWrite( &item->lockRelease.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterTryLock( bool acquired )
    {
#ifdef TRACY_ON_DEMAND
        if( !acquired ) return;

        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return;
#endif

        if( acquired )
        {
            auto item = Profiler::QueueSerial();
            MemWrite( &item->hdr.type, QueueType::LockObtain );
            MemWrite( &item->lockObtain.thread, GetThreadHandle() );
            MemWrite( &item->lockObtain.id, m_id );
            MemWrite( &item->lockObtain.time, Profiler::GetTime() );
            Profiler::QueueSerialFinish();
        }
    }

    tracy_force_inline void Mark( const SourceLocationData* srcloc )
    {
#ifdef TRACY_ON_DEMAND
        const auto active = m_active.load( std::memory_order_relaxed );
        if( !active ) return;
        const auto connected = GetProfiler().IsConnected();
        if( !connected )
        {
            if( active ) m_active.store( false, std::memory_order_relaxed );
            return;
        }
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockMark );
        MemWrite( &item->lockMark.thread, GetThreadHandle() );
        MemWrite( &item->lockMark.id, m_id );
        MemWrite( &item->lockMark.srcloc, (uint64_t)srcloc );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void CustomName( const char* name, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, name, size );
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockName );
        MemWrite( &item->lockNameFat.id, m_id );
        MemWrite( &item->lockNameFat.name, (uint64_t)ptr );
        MemWrite( &item->lockNameFat.size, (uint16_t)size );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

private:
    uint32_t m_id;

#ifdef TRACY_ON_DEMAND
    std::atomic<uint32_t> m_lockCount;
    std::atomic<bool> m_active;
#endif
};

template<class T>
class Lockable
{
public:
    tracy_force_inline Lockable( const SourceLocationData* srcloc )
        : m_ctx( srcloc )
    {
    }

    Lockable( const Lockable& ) = delete;
    Lockable& operator=( const Lockable& ) = delete;

    tracy_force_inline void lock()
    {
        const auto runAfter = m_ctx.BeforeLock();
        m_lockable.lock();
        if( runAfter ) m_ctx.AfterLock();
    }

    tracy_force_inline void unlock()
    {
        m_lockable.unlock();
        m_ctx.AfterUnlock();
    }

    tracy_force_inline bool try_lock()
    {
        const auto acquired = m_lockable.try_lock();
        m_ctx.AfterTryLock( acquired );
        return acquired;
    }

    tracy_force_inline void Mark( const SourceLocationData* srcloc )
    {
        m_ctx.Mark( srcloc );
    }

    tracy_force_inline void CustomName( const char* name, size_t size )
    {
        m_ctx.CustomName( name, size );
    }

private:
    T m_lockable;
    LockableCtx m_ctx;
};

class SharedLockableCtx
{
public:
    tracy_force_inline SharedLockableCtx( const SourceLocationData* srcloc )
        : m_id( GetLockCounter().fetch_add( 1, std::memory_order_relaxed ) )
#ifdef TRACY_ON_DEMAND
        , m_lockCount( 0 )
        , m_active( false )
#endif
    {
        assert( m_id != std::numeric_limits<uint32_t>::max() );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockAnnounce );
        MemWrite( &item->lockAnnounce.id, m_id );
        MemWrite( &item->lockAnnounce.time, Profiler::GetTime() );
        MemWrite( &item->lockAnnounce.lckloc, (uint64_t)srcloc );
        MemWrite( &item->lockAnnounce.type, LockType::SharedLockable );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

    SharedLockableCtx( const SharedLockableCtx& ) = delete;
    SharedLockableCtx& operator=( const SharedLockableCtx& ) = delete;

    tracy_force_inline ~SharedLockableCtx()
    {
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockTerminate );
        MemWrite( &item->lockTerminate.id, m_id );
        MemWrite( &item->lockTerminate.time, Profiler::GetTime() );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline bool BeforeLock()
    {
#ifdef TRACY_ON_DEMAND
        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return false;
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockWait );
        MemWrite( &item->lockWait.thread, GetThreadHandle() );
        MemWrite( &item->lockWait.id, m_id );
        MemWrite( &item->lockWait.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
        return true;
    }

    tracy_force_inline void AfterLock()
    {
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockObtain );
        MemWrite( &item->lockObtain.thread, GetThreadHandle() );
        MemWrite( &item->lockObtain.id, m_id );
        MemWrite( &item->lockObtain.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterUnlock()
    {
#ifdef TRACY_ON_DEMAND
        m_lockCount.fetch_sub( 1, std::memory_order_relaxed );
        if( !m_active.load( std::memory_order_relaxed ) ) return;
        if( !GetProfiler().IsConnected() )
        {
            m_active.store( false, std::memory_order_relaxed );
            return;
        }
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockRelease );
        MemWrite( &item->lockRelease.thread, GetThreadHandle() );
        MemWrite( &item->lockRelease.id, m_id );
        MemWrite( &item->lockRelease.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterTryLock( bool acquired )
    {
#ifdef TRACY_ON_DEMAND
        if( !acquired ) return;

        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return;
#endif

        if( acquired )
        {
            auto item = Profiler::QueueSerial();
            MemWrite( &item->hdr.type, QueueType::LockObtain );
            MemWrite( &item->lockObtain.thread, GetThreadHandle() );
            MemWrite( &item->lockObtain.id, m_id );
            MemWrite( &item->lockObtain.time, Profiler::GetTime() );
            Profiler::QueueSerialFinish();
        }
    }

    tracy_force_inline bool BeforeLockShared()
    {
#ifdef TRACY_ON_DEMAND
        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return false;
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockSharedWait );
        MemWrite( &item->lockWait.thread, GetThreadHandle() );
        MemWrite( &item->lockWait.id, m_id );
        MemWrite( &item->lockWait.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
        return true;
    }

    tracy_force_inline void AfterLockShared()
    {
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockSharedObtain );
        MemWrite( &item->lockObtain.thread, GetThreadHandle() );
        MemWrite( &item->lockObtain.id, m_id );
        MemWrite( &item->lockObtain.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterUnlockShared()
    {
#ifdef TRACY_ON_DEMAND
        m_lockCount.fetch_sub( 1, std::memory_order_relaxed );
        if( !m_active.load( std::memory_order_relaxed ) ) return;
        if( !GetProfiler().IsConnected() )
        {
            m_active.store( false, std::memory_order_relaxed );
            return;
        }
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockSharedRelease );
        MemWrite( &item->lockRelease.thread, GetThreadHandle() );
        MemWrite( &item->lockRelease.id, m_id );
        MemWrite( &item->lockRelease.time, Profiler::GetTime() );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void AfterTryLockShared( bool acquired )
    {
#ifdef TRACY_ON_DEMAND
        if( !acquired ) return;

        bool queue = false;
        const auto locks = m_lockCount.fetch_add( 1, std::memory_order_relaxed );
        const auto active = m_active.load( std::memory_order_relaxed );
        if( locks == 0 || active )
        {
            const bool connected = GetProfiler().IsConnected();
            if( active != connected ) m_active.store( connected, std::memory_order_relaxed );
            if( connected ) queue = true;
        }
        if( !queue ) return;
#endif

        if( acquired )
        {
            auto item = Profiler::QueueSerial();
            MemWrite( &item->hdr.type, QueueType::LockSharedObtain );
            MemWrite( &item->lockObtain.thread, GetThreadHandle() );
            MemWrite( &item->lockObtain.id, m_id );
            MemWrite( &item->lockObtain.time, Profiler::GetTime() );
            Profiler::QueueSerialFinish();
        }
    }

    tracy_force_inline void Mark( const SourceLocationData* srcloc )
    {
#ifdef TRACY_ON_DEMAND
        const auto active = m_active.load( std::memory_order_relaxed );
        if( !active ) return;
        const auto connected = GetProfiler().IsConnected();
        if( !connected )
        {
            if( active ) m_active.store( false, std::memory_order_relaxed );
            return;
        }
#endif

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockMark );
        MemWrite( &item->lockMark.thread, GetThreadHandle() );
        MemWrite( &item->lockMark.id, m_id );
        MemWrite( &item->lockMark.srcloc, (uint64_t)srcloc );
        Profiler::QueueSerialFinish();
    }

    tracy_force_inline void CustomName( const char* name, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, name, size );
        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::LockName );
        MemWrite( &item->lockNameFat.id, m_id );
        MemWrite( &item->lockNameFat.name, (uint64_t)ptr );
        MemWrite( &item->lockNameFat.size, (uint16_t)size );
#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif
        Profiler::QueueSerialFinish();
    }

private:
    uint32_t m_id;

#ifdef TRACY_ON_DEMAND
    std::atomic<uint32_t> m_lockCount;
    std::atomic<bool> m_active;
#endif
};

template<class T>
class SharedLockable
{
public:
    tracy_force_inline SharedLockable( const SourceLocationData* srcloc )
        : m_ctx( srcloc )
    {
    }

    SharedLockable( const SharedLockable& ) = delete;
    SharedLockable& operator=( const SharedLockable& ) = delete;

    tracy_force_inline void lock()
    {
        const auto runAfter = m_ctx.BeforeLock();
        m_lockable.lock();
        if( runAfter ) m_ctx.AfterLock();
    }

    tracy_force_inline void unlock()
    {
        m_lockable.unlock();
        m_ctx.AfterUnlock();
    }

    tracy_force_inline bool try_lock()
    {
        const auto acquired = m_lockable.try_lock();
        m_ctx.AfterTryLock( acquired );
        return acquired;
    }

    tracy_force_inline void lock_shared()
    {
        const auto runAfter = m_ctx.BeforeLockShared();
        m_lockable.lock_shared();
        if( runAfter ) m_ctx.AfterLockShared();
    }

    tracy_force_inline void unlock_shared()
    {
        m_lockable.unlock_shared();
        m_ctx.AfterUnlockShared();
    }

    tracy_force_inline bool try_lock_shared()
    {
        const auto acquired = m_lockable.try_lock_shared();
        m_ctx.AfterTryLockShared( acquired );
        return acquired;
    }

    tracy_force_inline void Mark( const SourceLocationData* srcloc )
    {
        m_ctx.Mark( srcloc );
    }

    tracy_force_inline void CustomName( const char* name, size_t size )
    {
        m_ctx.CustomName( name, size );
    }

private:
    T m_lockable;
    SharedLockableCtx m_ctx;
};

}

#endif

/*** End of inlined file: TracyLock.hpp ***/


/*** Start of inlined file: TracyScoped.hpp ***/
#ifndef __TRACYSCOPED_HPP__
#define __TRACYSCOPED_HPP__

#include <limits>
#include <stdint.h>
#include <string.h>

namespace tracy
{

class ScopedZone
{
public:
    ScopedZone( const ScopedZone& ) = delete;
    ScopedZone( ScopedZone&& ) = delete;
    ScopedZone& operator=( const ScopedZone& ) = delete;
    ScopedZone& operator=( ScopedZone&& ) = delete;

    tracy_force_inline ScopedZone( const SourceLocationData* srcloc, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        TracyLfqPrepare( QueueType::ZoneBegin );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( const SourceLocationData* srcloc, int depth, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        GetProfiler().SendCallstack( depth );

        TracyLfqPrepare( QueueType::ZoneBeginCallstack );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        TracyLfqPrepare( QueueType::ZoneBeginAllocSrcLoc );
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, int depth, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        GetProfiler().SendCallstack( depth );

        TracyLfqPrepare( QueueType::ZoneBeginAllocSrcLocCallstack );
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ~ScopedZone()
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneEnd );
        MemWrite( &item->zoneEnd.time, Profiler::GetTime() );
        TracyLfqCommit;
    }

    tracy_force_inline void Text( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::ZoneText );
        MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    tracy_force_inline void Name( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::ZoneName );
        MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    tracy_force_inline void Color( uint32_t color )
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneColor );
        MemWrite( &item->zoneColor.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->zoneColor.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->zoneColor.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        TracyLfqCommit;
    }

    tracy_force_inline void Value( uint64_t value )
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneValue );
        MemWrite( &item->zoneValue.value, value );
        TracyLfqCommit;
    }

    tracy_force_inline bool IsActive() const { return m_active; }

private:
    const bool m_active;

#ifdef TRACY_ON_DEMAND
    uint64_t m_connectionId;
#endif
};

}

#endif

/*** End of inlined file: TracyScoped.hpp ***/

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define ZoneNamed( varname, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define ZoneNamedN( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define ZoneNamedC( varname, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define ZoneNamedNC( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );

#  define ZoneTransient( varname, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, TRACY_CALLSTACK, active );
#  define ZoneTransientN( varname, name, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), TRACY_CALLSTACK, active );
#else
#  define ZoneNamed( varname, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define ZoneNamedN( varname, name, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define ZoneNamedC( varname, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define ZoneNamedNC( varname, name, color, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), active );

#  define ZoneTransient( varname, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, active );
#  define ZoneTransientN( varname, name, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), active );
#endif

#define ZoneScoped ZoneNamed( ___tracy_scoped_zone, true )
#define ZoneScopedN( name ) ZoneNamedN( ___tracy_scoped_zone, name, true )
#define ZoneScopedC( color ) ZoneNamedC( ___tracy_scoped_zone, color, true )
#define ZoneScopedNC( name, color ) ZoneNamedNC( ___tracy_scoped_zone, name, color, true )

#define ZoneText( txt, size ) ___tracy_scoped_zone.Text( txt, size );
#define ZoneTextV( varname, txt, size ) varname.Text( txt, size );
#define ZoneName( txt, size ) ___tracy_scoped_zone.Name( txt, size );
#define ZoneNameV( varname, txt, size ) varname.Name( txt, size );
#define ZoneColor( color ) ___tracy_scoped_zone.Color( color );
#define ZoneColorV( varname, color ) varname.Color( color );
#define ZoneValue( value ) ___tracy_scoped_zone.Value( value );
#define ZoneValueV( varname, value ) varname.Value( value );
#define ZoneIsActive ___tracy_scoped_zone.IsActive()
#define ZoneIsActiveV( varname ) varname.IsActive()

#define FrameMark tracy::Profiler::SendFrameMark( nullptr );
#define FrameMarkNamed( name ) tracy::Profiler::SendFrameMark( name );
#define FrameMarkStart( name ) tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgStart );
#define FrameMarkEnd( name ) tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgEnd );

#define FrameImage( image, width, height, offset, flip ) tracy::Profiler::SendFrameImage( image, width, height, offset, flip );

#define TracyLockable( type, varname ) tracy::Lockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, #type " " #varname, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TracyLockableN( type, varname, desc ) tracy::Lockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, desc, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TracySharedLockable( type, varname ) tracy::SharedLockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, #type " " #varname, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define TracySharedLockableN( type, varname, desc ) tracy::SharedLockable<type> varname { [] () -> const tracy::SourceLocationData* { static constexpr tracy::SourceLocationData srcloc { nullptr, desc, __FILE__, __LINE__, 0 }; return &srcloc; }() };
#define LockableBase( type ) tracy::Lockable<type>
#define SharedLockableBase( type ) tracy::SharedLockable<type>
#define LockMark( varname ) static constexpr tracy::SourceLocationData __tracy_lock_location_##varname { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; varname.Mark( &__tracy_lock_location_##varname );
#define LockableName( varname, txt, size ) varname.CustomName( txt, size );

#define TracyPlot( name, val ) tracy::Profiler::PlotData( name, val );
#define TracyPlotConfig( name, type ) tracy::Profiler::ConfigurePlot( name, type );

#define TracyAppInfo( txt, size ) tracy::Profiler::MessageAppInfo( txt, size );

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TracyMessage( txt, size ) tracy::Profiler::Message( txt, size, TRACY_CALLSTACK );
#  define TracyMessageL( txt ) tracy::Profiler::Message( txt, TRACY_CALLSTACK );
#  define TracyMessageC( txt, size, color ) tracy::Profiler::MessageColor( txt, size, color, TRACY_CALLSTACK );
#  define TracyMessageLC( txt, color ) tracy::Profiler::MessageColor( txt, color, TRACY_CALLSTACK );

#  define TracyAlloc( ptr, size ) tracy::Profiler::MemAllocCallstack( ptr, size, TRACY_CALLSTACK, false );
#  define TracyFree( ptr ) tracy::Profiler::MemFreeCallstack( ptr, TRACY_CALLSTACK, false );
#  define TracySecureAlloc( ptr, size ) tracy::Profiler::MemAllocCallstack( ptr, size, TRACY_CALLSTACK, true );
#  define TracySecureFree( ptr ) tracy::Profiler::MemFreeCallstack( ptr, TRACY_CALLSTACK, true );

#  define TracyAllocN( ptr, size, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, TRACY_CALLSTACK, false, name );
#  define TracyFreeN( ptr, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, TRACY_CALLSTACK, false, name );
#  define TracySecureAllocN( ptr, size, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, TRACY_CALLSTACK, true, name );
#  define TracySecureFreeN( ptr, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, TRACY_CALLSTACK, true, name );
#else
#  define TracyMessage( txt, size ) tracy::Profiler::Message( txt, size, 0 );
#  define TracyMessageL( txt ) tracy::Profiler::Message( txt, 0 );
#  define TracyMessageC( txt, size, color ) tracy::Profiler::MessageColor( txt, size, color, 0 );
#  define TracyMessageLC( txt, color ) tracy::Profiler::MessageColor( txt, color, 0 );

#  define TracyAlloc( ptr, size ) tracy::Profiler::MemAlloc( ptr, size, false );
#  define TracyFree( ptr ) tracy::Profiler::MemFree( ptr, false );
#  define TracySecureAlloc( ptr, size ) tracy::Profiler::MemAlloc( ptr, size, true );
#  define TracySecureFree( ptr ) tracy::Profiler::MemFree( ptr, true );

#  define TracyAllocN( ptr, size, name ) tracy::Profiler::MemAllocNamed( ptr, size, false, name );
#  define TracyFreeN( ptr, name ) tracy::Profiler::MemFreeNamed( ptr, false, name );
#  define TracySecureAllocN( ptr, size, name ) tracy::Profiler::MemAllocNamed( ptr, size, true, name );
#  define TracySecureFreeN( ptr, name ) tracy::Profiler::MemFreeNamed( ptr, true, name );
#endif

#ifdef TRACY_HAS_CALLSTACK
#  define ZoneNamedS( varname, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZoneNamedNS( varname, name, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZoneNamedCS( varname, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define ZoneNamedNCS( varname, name, color, depth, active ) static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,__LINE__) { name, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, color }; tracy::ScopedZone varname( &TracyConcat(__tracy_source_location,__LINE__), depth, active );

#  define ZoneTransientS( varname, depth, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), nullptr, 0, depth, active );
#  define ZoneTransientNS( varname, name, depth, active ) tracy::ScopedZone varname( __LINE__, __FILE__, strlen( __FILE__ ), __FUNCTION__, strlen( __FUNCTION__ ), name, strlen( name ), depth, active );

#  define ZoneScopedS( depth ) ZoneNamedS( ___tracy_scoped_zone, depth, true )
#  define ZoneScopedNS( name, depth ) ZoneNamedNS( ___tracy_scoped_zone, name, depth, true )
#  define ZoneScopedCS( color, depth ) ZoneNamedCS( ___tracy_scoped_zone, color, depth, true )
#  define ZoneScopedNCS( name, color, depth ) ZoneNamedNCS( ___tracy_scoped_zone, name, color, depth, true )

#  define TracyAllocS( ptr, size, depth ) tracy::Profiler::MemAllocCallstack( ptr, size, depth, false );
#  define TracyFreeS( ptr, depth ) tracy::Profiler::MemFreeCallstack( ptr, depth, false );
#  define TracySecureAllocS( ptr, size, depth ) tracy::Profiler::MemAllocCallstack( ptr, size, depth, true );
#  define TracySecureFreeS( ptr, depth ) tracy::Profiler::MemFreeCallstack( ptr, depth, true );

#  define TracyAllocNS( ptr, size, depth, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, depth, false, name );
#  define TracyFreeNS( ptr, depth, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, depth, false, name );
#  define TracySecureAllocNS( ptr, size, depth, name ) tracy::Profiler::MemAllocCallstackNamed( ptr, size, depth, true, name );
#  define TracySecureFreeNS( ptr, depth, name ) tracy::Profiler::MemFreeCallstackNamed( ptr, depth, true, name );

#  define TracyMessageS( txt, size, depth ) tracy::Profiler::Message( txt, size, depth );
#  define TracyMessageLS( txt, depth ) tracy::Profiler::Message( txt, depth );
#  define TracyMessageCS( txt, size, color, depth ) tracy::Profiler::MessageColor( txt, size, color, depth );
#  define TracyMessageLCS( txt, color, depth ) tracy::Profiler::MessageColor( txt, color, depth );
#else
#  define ZoneNamedS( varname, depth, active ) ZoneNamed( varname, active )
#  define ZoneNamedNS( varname, name, depth, active ) ZoneNamedN( varname, name, active )
#  define ZoneNamedCS( varname, color, depth, active ) ZoneNamedC( varname, color, active )
#  define ZoneNamedNCS( varname, name, color, depth, active ) ZoneNamedNC( varname, name, color, active )

#  define ZoneTransientS( varname, depth, active ) ZoneTransient( varname, active )
#  define ZoneTransientNS( varname, name, depth, active ) ZoneTransientN( varname, name, active )

#  define ZoneScopedS( depth ) ZoneScoped
#  define ZoneScopedNS( name, depth ) ZoneScopedN( name )
#  define ZoneScopedCS( color, depth ) ZoneScopedC( color )
#  define ZoneScopedNCS( name, color, depth ) ZoneScopedNC( name, color )

#  define TracyAllocS( ptr, size, depth ) TracyAlloc( ptr, size )
#  define TracyFreeS( ptr, depth ) TracyFree( ptr )
#  define TracySecureAllocS( ptr, size, depth ) TracySecureAlloc( ptr, size )
#  define TracySecureFreeS( ptr, depth ) TracySecureFree( ptr )

#  define TracyAllocNS( ptr, size, depth, name ) TracyAlloc( ptr, size, name )
#  define TracyFreeNS( ptr, depth, name ) TracyFree( ptr, name )
#  define TracySecureAllocNS( ptr, size, depth, name ) TracySecureAlloc( ptr, size, name )
#  define TracySecureFreeNS( ptr, depth, name ) TracySecureFree( ptr, name )

#  define TracyMessageS( txt, size, depth ) TracyMessage( txt, size )
#  define TracyMessageLS( txt, depth ) TracyMessageL( txt )
#  define TracyMessageCS( txt, size, color, depth ) TracyMessageC( txt, size, color )
#  define TracyMessageLCS( txt, color, depth ) TracyMessageLC( txt, color )
#endif

#define TracyParameterRegister( cb ) tracy::Profiler::ParameterRegister( cb );
#define TracyParameterSetup( idx, name, isBool, val ) tracy::Profiler::ParameterSetup( idx, name, isBool, val );
#define TracyIsConnected tracy::GetProfiler().IsConnected()

#endif

#endif

