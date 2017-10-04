// ライセンス: GPL2
//
// ユニコード(ucs2) <-> 文字参照変換テーブル
//

#ifndef _SPCHAR_TBL_H
#define _SPCHAR_TBL_H

struct UCSTBL
{
    unsigned short ucs;
    char str[ 12 ];
};


static constexpr UCSTBL const ucstbl_empty[] = {
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_a[] = {
    { 38,   "amp;" },
    { 180,  "acute;" },
    { 224,  "agrave;" },
    { 225,  "aacute;" },
    { 226,  "acirc;" },
    { 227,  "atilde;" },
    { 228,  "auml;" },
    { 229,  "aring;" },
    { 230,  "aelig;" },
    { 945,  "alpha;" },
    { 8501, "alefsym;" },
    { 8736, "ang;" },
    { 8743, "and;" },
    { 8776, "asymp;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_b[] = {
    { 166,  "brvbar;" },
    { 946,  "beta;" },
    { 8222, "bdquo;" },
    { 8226, "bull;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_c[] = {
    { 162,  "cent;" },
    { 164,  "curren;" },
    { 169,  "copy;" },
    { 184,  "cedil;" },
    { 231,  "ccedil;" },
    { 710,  "circ;" },
    { 967,  "chi;" },
    { 8629, "crarr;" },
    { 8745, "cap;" },
    { 8746, "cup;" },
    { 8773, "cong;" },
    { 9827, "clubs;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_d[] = {
    { 176,  "deg;" },
    { 247,  "divide;" },
    { 948,  "delta;" },
    { 8224, "dagger;" },
    { 8595, "darr;" },
    { 8659, "dArr;" },
    { 9830, "diams;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_e[] = {
    { 232,  "egrave;" },
    { 233,  "eacute;" },
    { 234,  "ecirc;" },
    { 235,  "euml;" },
    { 240,  "eth;" },
    { 949,  "epsilon;" },
    { 951,  "eta;" },
    { 8194, "ensp;" },
    { 8195, "emsp;" },
    { 8364, "euro;" },
    { 8707, "exist;" },
    { 8709, "empty;" },
    { 8801, "equiv;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_f[] = {
    { 188,  "frac14;" },
    { 189,  "frac12;" },
    { 190,  "frac34;" },
    { 402,  "fnof;" },
    { 8260, "frasl;" },
    { 8704, "forall;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_g[] = {
    { 62,   "gt;" },
    { 947,  "gamma;" },
    { 8805, "ge;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_h[] = {
    { 8230, "hellip;" },
    { 8596, "harr;" },
    { 8660, "hArr;" },
    { 9829, "hearts;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_i[] = {
    { 161,  "iexcl;" },
    { 191,  "iquest;" },
    { 236,  "igrave;" },
    { 237,  "iacute;" },
    { 238,  "icirc;" },
    { 239,  "iuml;" },
    { 953,  "iota;" },
    { 8465, "image;" },
    { 8712, "isin;" },
    { 8734, "infin;" },
    { 8747, "int;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_k[] = {
    { 954,  "kappa;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_l[] = {
    { 60,   "lt;" },
    { 171,  "laquo;" },
    { 955,  "lambda;" },
    { 8206, "lrm;" },
    { 8216, "lsquo;" },
    { 8220, "ldquo;" },
    { 8249, "lsaquo;" },
    { 8592, "larr;" },
    { 8656, "lArr;" },
    { 8727, "lowast;" },
    { 8804, "le;" },
    { 8968, "lceil;" },
    { 8970, "lfloor;" },
    { 9001, "lang;" },
    { 9674, "loz;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_m[] = {
    { 175,  "macr;" },
    { 181,  "micro;" },
    { 183,  "middot;" },
    { 956,  "mu;" },
    { 8212, "mdash;" },
    { 8722, "minus;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_n[] = {
    { 160,  "nbsp;" },
    { 172,  "not;" },
    { 241,  "ntilde;" },
    { 957,  "nu;" },
    { 8211, "ndash;" },
    { 8711, "nabla;" },
    { 8713, "notin;" },
    { 8715, "ni;" },
    { 8800, "ne;" },
    { 8836, "nsub;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_o[] = {
    { 170,  "ordf;" },
    { 186,  "ordm;" },
    { 242,  "ograve;" },
    { 243,  "oacute;" },
    { 244,  "ocirc;" },
    { 245,  "otilde;" },
    { 246,  "ouml;" },
    { 248,  "oslash;" },
    { 339,  "oelig;" },
    { 959,  "omicron;" },
    { 969,  "omega;" },
    { 8254, "oline;" },
    { 8744, "or;" },
    { 8853, "oplus;" },
    { 8855, "otimes;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_p[] = {
    { 163,  "pound;" },
    { 177,  "plusmn;" },
    { 182,  "para;" },
    { 960,  "pi;" },
    { 966,  "phi;" },
    { 968,  "psi;" },
    { 982,  "piv;" },
    { 8240, "permil;" },
    { 8242, "prime;" },
    { 8706, "part;" },
    { 8719, "prod;" },
    { 8733, "prop;" },
    { 8869, "perp;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_q[] = {
    { 34,   "quot;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_r[] = {
    { 174,  "reg;" },
    { 187,  "raquo;" },
    { 961,  "rho;" },
    { 8207, "rlm;" },
    { 8217, "rsquo;" },
    { 8221, "rdquo;" },
    { 8250, "rsaquo;" },
    { 8476, "real;" },
    { 8594, "rarr;" },
    { 8658, "rArr;" },
    { 8730, "radic;" },
    { 8969, "rceil;" },
    { 8971, "rfloor;" },
    { 9002, "rang;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_s[] = {
    { 167,  "sect;" },
    { 173,  "shy;" },
    { 178,  "sup2;" },
    { 179,  "sup3;" },
    { 185,  "sup1;" },
    { 223,  "szlig;" },
    { 353,  "scaron;" },
    { 962,  "sigmaf;" },
    { 963,  "sigma;" },
    { 8218, "sbquo;" },
    { 8721, "sum;" },
    { 8764, "sim;" },
    { 8834, "sub;" },
    { 8835, "sup;" },
    { 8838, "sube;" },
    { 8839, "supe;" },
    { 8901, "sdot;" },
    { 9824, "spades;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_t[] = {
    { 215,  "times;" },
    { 254,  "thorn;" },
    { 732,  "tilde;" },
    { 952,  "theta;" },
    { 964,  "tau;" },
    { 977,  "thetasym;" },
    { 8201, "thinsp;" },
    { 8482, "trade;" },
    { 8756, "there4;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_u[] = {
    { 168,  "uml;" },
    { 249,  "ugrave;" },
    { 250,  "uacute;" },
    { 251,  "ucirc;" },
    { 252,  "uuml;" },
    { 965,  "upsilon;" },
    { 978,  "upsih;" },
    { 8593, "uarr;" },
    { 8657, "uArr;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_w[] = {
    { 8472, "weierp;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_x[] = {
    { 958,  "xi;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_y[] = {
    { 165,  "yen;" },
    { 253,  "yacute;" },
    { 255,  "yuml;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_z[] = {
    { 950,  "zeta;" },
    { 8203, "zwsp;" },
    { 8204, "zwnj;" },
    { 8205, "zwj;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_A[] = {
    { 192,  "Agrave;" },
    { 193,  "Aacute;" },
    { 194,  "Acirc;" },
    { 195,  "Atilde;" },
    { 196,  "Auml;" },
    { 197,  "Aring;" },
    { 198,  "AElig;" },
    { 913,  "Alpha;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_B[] = {
    { 914,  "Beta;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_C[] = {
    { 199,  "Ccedil;" },
    { 935,  "Chi;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_D[] = {
    { 916,  "Delta;" },
    { 8225, "Dagger;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_E[] = {
    { 200,  "Egrave;" },
    { 201,  "Eacute;" },
    { 202,  "Ecirc;" },
    { 203,  "Euml;" },
    { 208,  "ETH;" },
    { 917,  "Epsilon;" },
    { 919,  "Eta;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_G[] = {
    { 915,  "Gamma;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_I[] = {
    { 204,  "Igrave;" },
    { 205,  "Iacute;" },
    { 206,  "Icirc;" },
    { 207,  "Iuml;" },
    { 921,  "Iota;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_K[] = {
    { 922,  "Kappa;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_L[] = {
    { 923,  "Lambda;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_M[] = {
    { 924,  "Mu;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_N[] = {
    { 209,  "Ntilde;" },
    { 925,  "Nu;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_O[] = {
    { 210,  "Ograve;" },
    { 211,  "Oacute;" },
    { 212,  "Ocirc;" },
    { 213,  "Otilde;" },
    { 214,  "Ouml;" },
    { 216,  "Oslash;" },
    { 338,  "OElig;" },
    { 927,  "Omicron;" },
    { 937,  "Omega;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_P[] = {
    { 928,  "Pi;" },
    { 934,  "Phi;" },
    { 936,  "Psi;" },
    { 8243, "Prime;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_R[] = {
    { 929,  "Rho;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_S[] = {
    { 352,  "Scaron;" },
    { 931,  "Sigma;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_T[] = {
    { 222,  "THORN;" },
    { 920,  "Theta;" },
    { 932,  "Tau;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_U[] = {
    { 217,  "Ugrave;" },
    { 218,  "Uacute;" },
    { 219,  "Ucirc;" },
    { 220,  "Uuml;" },
    { 933,  "Upsilon;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_X[] = {
    { 926,  "Xi;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_Y[] = {
    { 221,  "Yacute;" },
    { 376,  "Yuml;" },
    { 0, "" }
};

static constexpr UCSTBL const ucstbl_Z[] = {
    { 918, "Zeta;" },
    { 0, "" }
};

static constexpr UCSTBL const * const ucstbl_small[] = {
    ucstbl_a, ucstbl_b, ucstbl_c, ucstbl_d, ucstbl_e,
    ucstbl_f, ucstbl_g, ucstbl_h, ucstbl_i, ucstbl_empty,
    ucstbl_k, ucstbl_l, ucstbl_m, ucstbl_n, ucstbl_o,
    ucstbl_p, ucstbl_q, ucstbl_r, ucstbl_s, ucstbl_t,
    ucstbl_u, ucstbl_empty, ucstbl_w, ucstbl_x, ucstbl_y,
    ucstbl_z
};

static constexpr UCSTBL const * const ucstbl_large[] = {
    ucstbl_A, ucstbl_B, ucstbl_C, ucstbl_D, ucstbl_E,
    ucstbl_empty, ucstbl_G, ucstbl_empty, ucstbl_I, ucstbl_empty,
    ucstbl_K, ucstbl_L, ucstbl_M, ucstbl_N, ucstbl_O,
    ucstbl_P, ucstbl_empty, ucstbl_R, ucstbl_S, ucstbl_T,
    ucstbl_U, ucstbl_empty, ucstbl_empty, ucstbl_X, ucstbl_Y,
    ucstbl_Z
};


enum
{
    UCS_HT      = 9,
    UCS_LF      = 10,
    UCS_FF      = 12,
    UCS_CR      = 13,
    UCS_SP      = 32,
    UCS_NBSP    = 160,
    UCS_ZWSP    = 0x200B,
    UCS_ZWNJ    = 0x200C,
    UCS_ZWJ     = 0x200D,
    UCS_LRM     = 0x200E,
    UCS_RLM     = 0x200F,
    UCS_LS      = 0x2028,
    UCS_PS      = 0x2029,
    UCS_REPLACE = 0xFFFD
};


static constexpr unsigned short const charref_tbl [] = {
    0x20AC, //	EURO SIGN (€)
    0x81,   //	U+0081 (HOP)
    0x201A, //	SINGLE LOW-9 QUOTATION MARK (‚)
    0x0192, //	LATIN SMALL LETTER F WITH HOOK (ƒ)
    0x201E, //	DOUBLE LOW-9 QUOTATION MARK („)
    0x2026, //	HORIZONTAL ELLIPSIS (…)
    0x2020, //	DAGGER (†)
    0x2021, //	DOUBLE DAGGER (‡)
    0x02C6, //	MODIFIER LETTER CIRCUMFLEX ACCENT (ˆ)
    0x2030, //	PER MILLE SIGN (‰)
    0x0160, //	LATIN CAPITAL LETTER S WITH CARON (Š)
    0x2039, //	SINGLE LEFT-POINTING ANGLE QUOTATION MARK (‹)
    0x0152, //	LATIN CAPITAL LIGATURE OE (Œ)
    0x8D,   //	U+008D (RI)
    0x017D, //	LATIN CAPITAL LETTER Z WITH CARON (Ž)
    0x8F,   //	U+008F (SS3)
    0x90,   //	U+0090 (DCS)
    0x2018, //	LEFT SINGLE QUOTATION MARK (‘)
    0x2019, //	RIGHT SINGLE QUOTATION MARK (’)
    0x201C, //	LEFT DOUBLE QUOTATION MARK (“)
    0x201D, //	RIGHT DOUBLE QUOTATION MARK (”)
    0x2022, //	BULLET (•)
    0x2013, //	EN DASH (–)
    0x2014, //	EM DASH (—)
    0x02DC, //	SMALL TILDE (˜)
    0x2122, //	TRADE MARK SIGN (™)
    0x0161, //	LATIN SMALL LETTER S WITH CARON (š)
    0x203A, //	SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (›)
    0x0153, //	LATIN SMALL LIGATURE OE (œ)
    0x9D,   //	U+009D (OSC)
    0x017E, //	LATIN SMALL LETTER Z WITH CARON (ž)
    0x0178, //	LATIN CAPITAL LETTER Y WITH DIAERESIS (Ÿ)
};

#endif
