/*****************************************************************************
 * This is the f28377_color_organ project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "display.h"
#include "led_driver.h"
#include "utils.h"
#include "font.h"


//Define the overall array of LED panels. The current design is for each panel
//to be a separate string of LEDs, driven by a separate Launchpad GPIO. For this
//reason, it is important that these definitions match with the global variables
//col[][] and panel_off[][].
#define NUM_PANEL    NUM_STRINGS

#ifdef  LARGE_ARRAY
#define PANEL_ROW    8
#define PANEL_COL    36
#define TBT_STEP     3
#define MAX_ROW     (PANEL_ROW * 2)
#define MAX_COL     (PANEL_COL * 2)

#else
#define PANEL_ROW    10
#define PANEL_COL    12
#define TBT_STEP     2
#define MAX_ROW      PANEL_ROW
#define MAX_COL     (PANEL_COL * NUM_PANEL)
#endif

#define TBT_ROW          (MAX_ROW / TBT_STEP)
#define TBT_COL          (MAX_COL / TBT_STEP)

#define TBT_CHAN_BLOCKS  (TBT_ROW * TBT_COL / COLOR_CHANNELS / 2)
#define OLD_STYLE_BULBS

#ifdef FLOODS
#define HYST_SIZE    3
#endif

//Global variables
Led       led[MAX_LEDS];       //the array of color information used by the LED driver
uint32_t  work_buff[MAX_LEDS]; //temporary work buffer (can be used by all displays)
uint32_t  disp_frames;
uint32_t  display_frames;      //used only in the display driver
uint32_t  display_mode;
uint16_t  display;
uint16_t  gap_display;
uint16_t  gap_color;
uint16_t  gap_timer;

Led       color_tab_left[COLOR_CHANNELS];
Led       color_tab_right[COLOR_CHANNELS];

#ifdef FLOODS
Led       fled[FLOOD_LEDS];
uint16_t  top_chan_l[MAX_CHAN];  //Color channels for the frame sorted by power
uint16_t  top_chan_l_prev[MAX_CHAN];
uint16_t  top_chan_r[MAX_CHAN];
uint16_t  top_chan_r_prev[MAX_CHAN];
uint16_t  top_chan_l_lvl[MAX_CHAN];
uint16_t  top_chan_r_lvl[MAX_CHAN];
uint16_t  hyst_l[COLOR_CHANNELS][HYST_SIZE];
uint16_t  hyst_r[COLOR_CHANNELS][HYST_SIZE];
uint16_t  hyst_sum_l[COLOR_CHANNELS];
uint16_t  hyst_sum_r[COLOR_CHANNELS];
uint16_t  chan_count_l;
uint16_t  chan_count_r;
uint16_t  hyst_idx;
uint16_t  wash_row;
uint16_t  wash_col;
uint16_t  wash_clr_idx;
#endif


//Temporary storage for the two-by-two function
uint16_t tbt_map[TBT_ROW][TBT_COL];

extern uint16_t pause;
extern volatile uint16_t frame_sync;
extern volatile uint16_t end_of_gap;
extern volatile uint32_t gap_frames;
extern volatile uint32_t frame_cnt;


// The following tables are built to convert a 2D y:x position into an index
// in the 1D arrays of LEDs.

#pragma DATA_SECTION(col, "ramConsts")

#ifdef LARGE_ARRAY
//Define a "wide" 2x2 panel arrangement:
const uint16_t col[MAX_ROW][MAX_COL] = // y rows by x cols
 //panel 0                                                                                                                                                                             panel 2
{{  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,   576,  577,  578,  579,  580,  581,  582,  583,  584,  585,  586,  587,  588,  589,  590,  591,  592,  593,  594,  595,  596,  597,  598,  599,  600,  601,  602,  603,  604,  605,  606,  607,  608,  609,  610,  611},
 { 71,  70,  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,  37,  36,   647,  646,  645,  644,  643,  642,  641,  640,  639,  638,  637,  636,  635,  634,  633,  632,  631,  630,  629,  628,  627,  626,  625,  624,  623,  622,  621,  620,  619,  618,  617,  616,  615,  614,  613,  612},
 { 72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107,   648,  649,  650,  651,  652,  653,  654,  655,  656,  657,  658,  659,  660,  661,  662,  663,  664,  665,  666,  667,  668,  669,  670,  671,  672,  673,  674,  675,  676,  677,  678,  679,  680,  681,  682,  683},
 {143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108,   719,  718,  717,  716,  715,  714,  713,  712,  711,  710,  709,  708,  707,  706,  705,  704,  703,  702,  701,  700,  699,  698,  697,  696,  695,  694,  693,  692,  691,  690,  689,  688,  687,  686,  685,  684},
 {144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,   720,  721,  722,  723,  724,  725,  726,  727,  728,  729,  730,  731,  732,  733,  734,  735,  736,  737,  738,  739,  740,  741,  742,  743,  744,  745,  746,  747,  748,  749,  750,  751,  752,  753,  754,  755},
 {215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180,   791,  790,  789,  788,  787,  786,  785,  784,  783,  782,  781,  780,  779,  778,  777,  776,  775,  774,  773,  772,  771,  770,  769,  768,  767,  766,  765,  764,  763,  762,  761,  760,  759,  758,  757,  756},
 {216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,   792,  793,  794,  795,  796,  797,  798,  799,  800,  801,  802,  803,  804,  805,  806,  807,  808,  809,  810,  811,  812,  813,  814,  815,  816,  817,  818,  819,  820,  821,  822,  823,  824,  825,  826,  827},
 {287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264, 263, 262, 261, 260, 259, 258, 257, 256, 255, 254, 253, 252,   863,  862,  861,  860,  859,  858,  857,  856,  855,  854,  853,  852,  851,  850,  849,  848,  847,  846,  845,  844,  843,  842,  841,  840,  839,  838,  837,  836,  835,  834,  833,  832,  831,  830,  829,  828},

 //panel 1                                                                                                                                                                             panel 3
 {288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323,   864,  865,  866,  867,  868,  869,  870,  871,  872,  873,  874,  875,  876,  877,  878,  879,  880,  881,  882,  883,  884,  885,  886,  887,  888,  889,  890,  891,  892,  893,  894,  895,  896,  897,  898,  899},
 {359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330, 329, 328, 327, 326, 325, 324,   935,  934,  933,  932,  931,  930,  929,  928,  927,  926,  925,  924,  923,  922,  921,  920,  919,  918,  917,  916,  915,  914,  913,  912,  911,  910,  909,  908,  907,  906,  905,  904,  903,  902,  901,  900},
 {360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395,   936,  937,  938,  939,  940,  941,  942,  943,  944,  945,  946,  947,  948,  949,  950,  951,  952,  953,  954,  955,  956,  957,  958,  959,  960,  961,  962,  963,  964,  965,  966,  967,  968,  969,  970,  971},
 {431, 430, 429, 428, 427, 426, 425, 424, 423, 422, 421, 420, 419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 402, 401, 400, 399, 398, 397, 396,  1007, 1006, 1005, 1004, 1003, 1002, 1001, 1000,  999,  998,  997,  996,  995,  994,  993,  992,  991,  990,  989,  988,  987,  986,  985,  984,  983,  982,  981,  980,  979,  978,  977,  976,  975,  974,  973,  972},
 {432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467,  1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043},
 {503, 502, 501, 500, 499, 498, 497, 496, 495, 494, 493, 492, 491, 490, 489, 488, 487, 486, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 474, 473, 472, 471, 470, 469, 468,  1079, 1078, 1077, 1076, 1075, 1074, 1073, 1072, 1071, 1070, 1069, 1068, 1067, 1066, 1065, 1064, 1063, 1062, 1061, 1060, 1059, 1058, 1057, 1056, 1055, 1054, 1053, 1052, 1051, 1050, 1049, 1048, 1047, 1046, 1045, 1044},
 {504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539,  1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115},
 {575, 574, 573, 572, 571, 570, 569, 568, 567, 566, 565, 564, 563, 562, 561, 560, 559, 558, 557, 556, 555, 554, 553, 552, 551, 550, 549, 548, 547, 546, 545, 544, 543, 542, 541, 540,  1151, 1150, 1149, 1148, 1147, 1146, 1145, 1144, 1143, 1142, 1141, 1140, 1139, 1138, 1137, 1136, 1135, 1134, 1133, 1132, 1131, 1130, 1129, 1128, 1127, 1126, 1125, 1124, 1123, 1122, 1121, 1120, 1119, 1118, 1117, 1116}};
#else
//Define a "wide" 1x4 panel arrangement:
const uint16_t col[MAX_ROW][MAX_COL] = // y rows by x cols
 //panel 0                                                     panel 1                                                      panel 2                                                      panel 3
{{ 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120,  251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240,  371, 370, 369, 368, 367, 366, 365, 364, 363, 362, 361, 360},
 { 12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,  372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383},
 { 35,  34,  33,  32,  31,  30,  29,  28,  27,  26,  25,  24,  155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,  275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,  395, 394, 393, 392, 391, 390, 389, 388, 387, 386, 385, 384},
 { 36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,  276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287,  396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407},
 { 59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,  179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168,  299, 298, 297, 296, 295, 294, 293, 292, 291, 290, 289, 288,  419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408},
 { 60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311,  420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431},
 { 83,  82,  81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192,  323, 322, 321, 320, 319, 318, 317, 316, 315, 314, 313, 312,  443, 442, 441, 440, 439, 438, 437, 436, 435, 434, 433, 432},
 { 84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,  324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335,  444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455},
 {107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96,  227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216,  347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336,  467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 457, 456},
 {108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,  228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359,  468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479}};
#endif

#ifdef FLOODS

#define FLOOD_ROW      16
#define FLOOD_COL      16

#pragma DATA_SECTION(col, "ramConsts")

//Define an octagonal arrangement:
const uint16_t fcol[FLOOD_ROW][FLOOD_COL] = // y rows by x cols
{{ 15, 16, 47, 48, 79, 80, 111, 112, 143, 144, 175, 176, 207, 208, 239, 240},
 { 14, 17, 46, 49, 78, 81, 110, 113, 142, 145, 174, 177, 206, 209, 238, 241},
 { 13, 18, 45, 50, 77, 82, 109, 114, 141, 146, 173, 178, 205, 210, 237, 242},
 { 12, 19, 44, 51, 76, 83, 108, 115, 140, 147, 172, 179, 204, 211, 236, 243},
 { 11, 20, 43, 52, 75, 84, 107, 116, 139, 148, 171, 180, 203, 212, 235, 244},
 { 10, 21, 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 202, 213, 234, 245},
 {  9, 22, 41, 54, 73, 86, 105, 118, 137, 150, 169, 182, 201, 214, 233, 246},
 {  8, 23, 40, 55, 72, 87, 104, 119, 136, 151, 168, 183, 200, 215, 232, 247},
 {  7, 24, 39, 56, 71, 88, 103, 120, 135, 152, 167, 184, 199, 216, 231, 248},
 {  6, 25, 38, 57, 70, 89, 102, 121, 134, 153, 166, 185, 198, 217, 230, 249},
 {  5, 26, 37, 58, 69, 90, 101, 122, 133, 154, 165, 186, 197, 218, 229, 250},
 {  4, 27, 36, 59, 68, 91, 100, 123, 132, 155, 164, 187, 196, 219, 228, 251},
 {  3, 28, 35, 60, 67, 92,  99, 124, 131, 156, 163, 188, 195, 220, 227, 252},
 {  2, 29, 34, 61, 66, 93,  98, 125, 130, 157, 162, 189, 194, 221, 226, 253},
 {  1, 30, 33, 62, 65, 94,  97, 126, 129, 158, 161, 190, 193, 222, 225, 254},
 {  0, 31, 32, 63, 64, 95,  96, 127, 128, 159, 160, 191, 192, 223, 224, 255}};
#endif


//This determines the panel's position in the logical [row, col] array.
#ifdef LARGE_ARRAY
const uint16_t panel_off[NUM_PANEL][2] =
{{0,         0},          //panel 0:  y=0, x=0
 {PANEL_ROW, 0},          //panel 1:  y=8, x=0
 {0,         PANEL_COL},  //panel 2:  y=0, x=36
 {PANEL_ROW, PANEL_COL}}; //panel 3:  y=8, x=36
#else
const uint16_t panel_off[NUM_PANEL][2] =
{{0, 0},                  //panel 0:  y=0, x=0
 {0, PANEL_COL},          //panel 1:  y=0, x=12
 {0, PANEL_COL*2},        //panel 2:  y=0, x=24
 {0, PANEL_COL*3}};       //panel 3:  y=0, x=36
#endif


#pragma DATA_SECTION(rainbow, "ramConsts")
#define MAX_RGB_VAL           0xf0
#define MID_RGB_VAL           0x78
#define MIN_RGB_VAL           0x28

//The function get_rgb() can be used to break these colors into component values.
const uint32_t rainbow[12] = {0xf00000,  //red
                              0xc82800,  //orange
                              0xf0f000,  //yellow
                              0x60e000,  //lime
                              0x00f000,  //green
                              0x00e033,  //l green
                              0x00e090,  //cyan
                              0x0070b0,  //marine
                              0x0000f0,  //blue
                              0x7700e0,  //l blue
                              0x680088,  //purple
                              0x800072}; //fushia


/*  Nine channel, one octave per channel color organ
    Frequency ranges: (44Khz sample rate, 86Hz per bin)

    Octave 1:      0.. 85Hz    bin: 0         color: Red
    Octave 2:     86..171Hz    bin: 1         color: Fushia
    Octave 3:    172..343Hz    bin: 2..3      color: Purple
    Octave 4:    344..687Hz    bin: 4..7      color: Blue
    Octave 5:    688..1375Hz   bin: 8..15     color: Aqua
    Octave 6:   1376..2751Hz   bin: 16..31    color: Cyan
    Octave 7:   2752..5503Hz   bin: 32..63    color: Green
    Octave 8:   5504..11007Hz  bin: 64..127   color: Lime
    Octave 9+: 11008..20000Hz  bin: 128..232  color: Yellow

    Or, three channel color organ frequency ranges:

    Low:          0..257Hz    bin: 0..3      color: Red
    Mid:        258..3439Hz   bin: 4..39     color: Blue
    High:      3440..20000Hz  bin: 40..232   color: Green
*/
#ifdef NINE_OCTAVE_BIN
const uint16_t chan_bin[COLOR_CHANNELS+1] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 233};

//This array maps into the rainbow[] array to assign colors to each color organ channel.
const uint16_t chan_clr[COLOR_CHANNELS] = {0, 11, 10, 8, 7, 6, 4, 3, 2};
#endif

/*  Ten channel compressed frequency range color organ
    Frequency ranges: (44Khz sample rate, 86Hz per bin)

    Channel 1:      0.. 85Hz    pitch: c1..e2  16  bin: 0         color: Red
    Channel 2:     86..171Hz    pitch: f2..e3  12  bin: 1         color: Orange
    Channel 3:    172..429Hz    pitch: f3..g#4 16  bin: 2..4      color: Purple
    Channel 4:    430..1117Hz   pitch: a4..c#6 17  bin: 5..12     color: Blue
    Channel 5:   1118..1633Hz   pitch: d6..g#6  7  bin: 13..18    color: Aqua
    Channel 6:   1634..2321Hz   pitch: a6..c#7  5  bin: 19..26    color: Cyan
    Channel 7:   2322..3009Hz   pitch: d7..f#7  5  bin: 27..34    color: L Green
    Channel 8:   3010..6019Hz   pitch: g7..f#8 12  bin: 35..69    color: Green
    Channel 9:   6020..9029Hz   pitch: g8..c#9  7  bin: 70..104   color: Lime
    Channel 10:  9030..16000Hz  pitch: d9..c10 11  bin: 105..185  color: Yellow
*/

#pragma DATA_SECTION(chan_bin, "ramConsts")
const uint16_t chan_bin[COLOR_CHANNELS+1] = {0, 1, 2, 5, 13, 19, 27, 35, 70, 105, 185};

//This array maps into the rainbow[] array to assign colors to each color organ channel.
#pragma DATA_SECTION(chan_clr, "ramConsts")
const uint16_t chan_clr[COLOR_CHANNELS] = {0, 1, 11, 8, 7, 6, 5, 4, 3, 2};


#ifdef FLOODS

void reset_hyst(void)
{
  memset(hyst_l, 0, sizeof(hyst_l));
  memset(hyst_r, 0, sizeof(hyst_r));
  memset(hyst_sum_l, 0, sizeof(hyst_sum_l));
  memset(hyst_sum_r, 0, sizeof(hyst_sum_r));
  memset(top_chan_l_lvl, 0, sizeof(top_chan_l_lvl));
  memset(top_chan_r_lvl, 0, sizeof(top_chan_r_lvl));
  hyst_idx = 0;
}
#endif


void display_init(void)
{
  memset(led, 0, sizeof(led));
  gap_frames = 9999;
  disp_frames = 99999;
  display     = 9999;
  display_frames = 99999;
  display_mode = 4;
  gap_display = 9999;
  gap_color   = 9999;
  gap_timer   = 9999;

#ifdef FLOODS
  memset(fled, 0, sizeof(fled));
  reset_hyst();
  wash_row = 9999;
  wash_col = 0;
#endif
}


//This function blocks for the specified number of frame syncs to
//expire.
void wait_for_sync(uint32_t num_syncs)
{
  uint16_t idx;

  pause = 0;

  for (idx = 0; idx < num_syncs; idx ++)
  {
    while (frame_sync == 0)
    {  }

    led_driver();

    pause = 1;
  }
}


#pragma CODE_SECTION(get_rgb, "ramCode")

//This function returns the R G B components of a 32-bit color value.
void get_rgb(uint32_t color, uint16_t *r, uint16_t *g, uint16_t *b)
{
  *r = (color >> 16) & 0xff;
  *g = (color >>  8) & 0xff;
  *b =  color        & 0xff;
}


#pragma CODE_SECTION(get_rgb_scaled, "ramCode")

//This function returns the R G B components of a scaled 32-bit color value.
void get_rgb_scaled(uint32_t color, uint16_t scale, uint16_t *r, uint16_t *g, uint16_t *b)
{
  float temp = (float)scale / (float)MAX_RGB_VAL;

  get_rgb(color, r, g, b);

  *r = (uint16_t) ((float)*r * temp);
  *g = (uint16_t) ((float)*g * temp);
  *b = (uint16_t) ((float)*b * temp);
}


//This function creates a table of 10 colors, per channel. It can be called
//just once per frame since the colors don't change within a frame.
#pragma CODE_SECTION(build_color_table, "ramCode")

void build_color_table(uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  scale;
  uint16_t  r, g, b;
  uint32_t  color;

  //Create a table of colors for this frame
  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    scale = left_val[idx];
    color = rainbow[chan_clr[idx]];
    get_rgb(color, &r, &g, &b);
    color_tab_left[idx].r = (r * scale) >> 8;
    color_tab_left[idx].g = (g * scale) >> 8;
    color_tab_left[idx].b = (b * scale) >> 8;

    scale = right_val[idx];
    color = rainbow[chan_clr[idx]];
    get_rgb(color, &r, &g, &b);
    color_tab_right[idx].r = (r * scale) >> 8;
    color_tab_right[idx].g = (g * scale) >> 8;
    color_tab_right[idx].b = (b * scale) >> 8;
  }
}


#pragma CODE_SECTION(clear_display, "ramCode")

void clear_display(void)
{
  memset(led, 0, sizeof(led));
  wait_for_sync(1);
  pause = 0;
}


#pragma CODE_SECTION(do_gap_display, "ramCode")

//This function displays things while the audio is silent.  It must be allowed to save its
//state and exit each frame so that the gap detect can run....
void do_gap_display(void)
{
  uint16_t idx;

  #ifndef SLAVE
  if (gap_frames % (FRAMES_PER_SEC * 12) == 0)
  {
    idx = rnd(9999);
    gap_display = idx % 7;
    gap_color   = rainbow[idx % 12];
    gap_timer   = 0;
  }

  if (gap_timer == 0)
  {
    switch (gap_display)
    {
      case 0:
        show_text(1, 0, gap_color, 0, "Hello?");
        break;
      case 1:
        show_text(1, 0, gap_color, 0, "Let's Boogie!");
        break;
      case 2:
        show_text(1, 0, gap_color, 0, "Get Down!");
        break;
      case 3:
        show_text(1, 0, gap_color, 0, "Crank It Up!");
        break;
      case 4:
        show_text(1, 0, gap_color, 0, "WOO HOO!");
        break;
      case 5:
        show_text(1, 0, gap_color, 0, "Let's Do It!");
        break;
      case 6:
        show_text(1, 0, gap_color, 0, "You There?");
        break;
    }
  }

  if (gap_timer == 60)
    clear_display();
  gap_timer ++;
  #endif

  #ifdef FLOODS
  // Left-Right color fades
  wash();

  reset_hyst();
  #endif
}


#pragma CODE_SECTION(copy_pixel, "ramCode")
void copy_pixel(uint16_t dest, uint16_t src)
{
  led[dest].r = led[src].r;
  led[dest].g = led[src].g;
  led[dest].b = led[src].b;
}


//This function creates a downward flowing pixel pattern.
#pragma CODE_SECTION(pixel_up, "ramCode")

void pixel_up(void)
{
  uint32_t  tmp;
  uint16_t  l1, l2;
  uint16_t  r, g, b;
  int16_t   rw, cl;

  while (1)
  {
    //Move all rows down by 1.
    for (rw = 0; rw < MAX_ROW-1; rw ++)
    {
      for (cl = 0; cl < MAX_COL; cl ++)
      {
        l1 = col[rw+1][cl];
        if ((cl+1) < MAX_COL)
          l2 = col[rw][cl+1];
        else
          l2 = col[rw][(cl+1)-MAX_COL];

        copy_pixel(l2, l1);
      }
    }

    //Generate a new row on the top
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      l1 = col[MAX_ROW-1][cl];

      tmp = rnd(22);
      if (tmp < 12)
      {
        if (rnd(100) > 35)
          tmp = rainbow[tmp & 0xc]; //only R, G or B
        else
          tmp = rainbow[tmp];
      }
      else
        tmp = 0; //off

      get_rgb(tmp, &r, &g, &b);
      led[l1].r = r;
      led[l1].g = g;
      led[l1].b = b;
    }

    wait_for_sync(2);
  }
}


#ifdef FLOODS

#pragma CODE_SECTION(wash_pix, "ramCode")

void wash_pix(uint16_t flood, uint16_t clr_idx, uint16_t column)
{
  uint32_t  tmp;
  uint16_t  l1;
  uint16_t  r, g, b;
  uint16_t  cl = column;

  tmp = rainbow[clr_idx];
  get_rgb(tmp, &r, &g, &b);

  l1 = fcol[wash_row][cl] + (FLOOD_STRING_MEM_LEN * flood);
  fled[l1].r = r;
  fled[l1].g = g;
  fled[l1].b = b;

  cl = FLOOD_COL - (1 + column);

  l1 = fcol[wash_row][cl] + (FLOOD_STRING_MEM_LEN * flood);
  fled[l1].r = r;
  fled[l1].g = g;
  fled[l1].b = b;
}


//This function creates a downward flowing color wash.
#pragma CODE_SECTION(wash, "ramCode")

void wash(void)
{
  uint16_t tmp_idx;

  //Test for the starting condition
  if (wash_row > FLOOD_ROW)
  {
    wash_row = FLOOD_ROW - 1;
    wash_col = 0;
    wash_clr_idx = rnd(12);
  }

  wash_pix(0, wash_clr_idx, wash_col); //flood 0

  tmp_idx = wash_clr_idx + 2;
  if (tmp_idx >= 12) tmp_idx -= 12;

  wash_pix(1, tmp_idx, wash_col); //Flood 2


  wash_col ++;
  if (wash_col == (FLOOD_COL / 2)) //finished the row
  {
    wash_col = 0;
    if (wash_row == 0)
      wash_row = 9999;
    else
      wash_row --;
  }

  wait_for_sync(1);
}
#endif


#pragma CODE_SECTION(line_segments, "ramCode")

#define MIN_LINE_FRAMES                (FRAMES_PER_SEC * 5)
#define LINE_MOVE_FRAMES                10
#define LINE_END                       ((MAX_ROW/2/2) * MAX_COL/2)

//This function creates a display where short moving horizontal line segments
//are randomly assigned to color channels. It is symmetric horizontally and
//vertically, with movement from the center outward.
void line_segments(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2;
  uint16_t  next, len;
  uint16_t  cv;
  uint16_t  rw, cl;
  uint16_t *buff;
  int16_t   chan_cnt[COLOR_CHANNELS];
  Led      *cv_clr1;
  Led      *cv_clr2;

  if (peak_flag & (disp_frames > MIN_LINE_FRAMES))
  {
    disp_frames = 0;

    for (idx = 0; idx < COLOR_CHANNELS; idx ++)
      chan_cnt[idx] = (LINE_END / COLOR_CHANNELS) + 2;

    //Create a new random arrangement of line segments in the work buffer.
    //The arrangement is stored in a "run length encoded" (RLE) fashion.
    buff = (uint16_t *)&work_buff;
    idx  = 0;

    while (idx <= LINE_END)
    {
      idx2 = rnd(99999);
      cv  = idx2 % COLOR_CHANNELS;   //random channel
      len = (idx2 % 4) + 3;          //random line length

      if (chan_cnt[cv] <= 0) //this channel has been "used up". Find another one
      {
        for (idx2 = 0; idx2 < COLOR_CHANNELS; idx2 ++)
        {
          cv ++;
          if (cv >= COLOR_CHANNELS) cv = 0;
          if (chan_cnt[cv] > 0) break;
        }
      }

     *buff++ = cv;   //store the channel in word 0
     *buff++ = len;  //store the segment length in word 1

      chan_cnt[cv] -= len;
      idx += len;
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  idx  = 0;
  cv   = *buff++;
  len  = *buff++;
  next = len;

  for (rw = 0; rw < MAX_ROW/2; rw += 2)
  {
    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      if (idx == next)
      {
        cv  = *buff++;
        len = *buff++;
        next += len;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rw+1][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-1-rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-2-rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rw+1][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-1-rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-2-rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
    }
  }
}

#define LINE_MIN_SEG                  3
#define LINE_MAX_SEG                  6
#define LINE_ROW_SEG                  ((MAX_COL/2) / LINE_MIN_SEG)

//This function creates a display where short moving horizontal line segments
//are randomly assigned to color channels. It is symmetric horizontally and
//vertically, with movement from the center outward.
void line_segment2(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx, rot;
  uint16_t  idx2, idx3;
  uint16_t  next, len;
  uint16_t  cv;
  uint16_t  rw, cl;
  uint16_t *buff;
  int16_t   chan_cnt[COLOR_CHANNELS];
  Led      *cv_clr1;
  Led      *cv_clr2;

  //Compute the lower-left quadrant.  Others are mirror image.
  //Create a new random arrangement of line segments in the work buffer.
  buff = (uint16_t *)&work_buff;

  if (peak_flag & (disp_frames > MIN_LINE_FRAMES))
  {
    disp_frames = 0;

    for (idx = 0; idx < COLOR_CHANNELS; idx ++)
      chan_cnt[idx] = ((MAX_ROW/4*MAX_COL/2) / COLOR_CHANNELS);

    for (rw = 0; rw < MAX_ROW/2; rw += 2)
    {
      idx  = 0;
      idx3 = 2;
      if ((rw % 4) == 0)
        buff[0] = rnd(6) + 10;  //store the speed factor, 10 to 15
      else
        buff[0] = rnd(5) + 3;   //store the speed factor, 3 to 7
      buff[1] = MAX_COL/2-1; //current rotation

      while (idx < MAX_COL/2)
      {
        idx2= rnd(99999);
        cv  = idx2 % COLOR_CHANNELS;
        len = (idx2 % (LINE_MAX_SEG-LINE_MIN_SEG)) + LINE_MIN_SEG;  //line length

        if ((len + idx) > MAX_COL/2)
          len = MAX_COL/2 - idx - 1;

        //don't let len equal 1
        if (len < 2)
          len = 0xff; //abort the end of the column

        if (chan_cnt[cv] <= 0) //this channel has been "used up". Find another one
        {
          for (idx2 = 0; idx2 < COLOR_CHANNELS; idx2 ++)
          {
            cv ++;
            if (cv >= COLOR_CHANNELS) cv = 0;
            if (chan_cnt[cv] > 0) break;
          }
        }

        buff[idx3++] = cv;   //store the channel in word 0
        buff[idx3++] = len;  //store the segment length in word 1

        chan_cnt[cv] -= len;
        idx += len + 1;
      }

      buff[idx3++] = 0xff;   //mark the end
      buff[idx3++] = 0xff;

      buff += (LINE_ROW_SEG + 1) * 2; //position for the next column
    }
  }

  memset(led, 0, sizeof(led));
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (rw = 0; rw < MAX_ROW/2; rw += 2)
  {
    idx2 = buff[0]; //get the row's speed
    rot  = buff[1];
    if ((disp_frames % idx2) == 0) //update the rotation and save
    {
      rot --;
      if (rot < 0)
        rot = MAX_COL/2-1;
      buff[1] = rot;
    }

    idx  = 0;
    idx3 = 4;
    cv   = buff[2];
    len  = buff[3];
    next = len;

    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      if (idx == next)
      {
        cv  = buff[idx3++];
        len = buff[idx3++];
        next += len;
        //leave a blank col
        cl ++;
        rot --;
        if (rot < 0)
          rot = MAX_COL/2-1;

        if ((len == 0xff) || (len < 2)) break;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rw+1][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[MAX_ROW-1-rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-2-rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rw+1][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx2 = col[MAX_ROW-1-rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-2-rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
      rot --; //update for the next iteration
      if (rot < 0)
        rot = MAX_COL/2-1;
    }

    buff += (LINE_ROW_SEG + 1) * 2; //position for the next column
  }
}


#pragma CODE_SECTION(arrows, "ramCode")

#define MIN_ARROW_FRAMES                (FRAMES_PER_SEC * 3)
#define ARROW_WIDTH                     3
uint16_t display_flip;

//This function creates a display where the channels are drawn as nested
//horizontal arrows.  They are randomly flipped by swapping left-right sides.
void arrows(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx3, wid;
  uint16_t  cv;
  uint16_t  rw, cl;
  uint16_t  start_cv, start_wid;
  uint16_t *row1;

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_ARROW_FRAMES))
  {
    disp_frames = 0;
    display_flip = rnd(10000) & 1;

    start_cv  = COLOR_CHANNELS - 1;
    start_wid = ARROW_WIDTH - 1;

    for (rw = 0; rw < MAX_ROW/2; rw ++)
    {
      //Compute the upper-left quadrant
      row1 = (uint16_t *)&work_buff[rw * (MAX_COL/2)];
      cv   = start_cv;
      wid  = start_wid;

      for (cl = 0; cl < MAX_COL/2; cl ++)
      {
        if (display_flip) // Left/Right panel invert the pattern
        {
          row1[(MAX_COL/2 - 1) - cl] = cv;
        }
        else
        {
          row1[cl] = cv;
        }

        wid --;
        if (wid == 0)
        {
          wid = ARROW_WIDTH;
          if (cv > 0)
            cv --;
          else
            cv = COLOR_CHANNELS - 1;
        }
      }

      start_wid ++;
      if (start_wid > ARROW_WIDTH)
      {
        start_wid = 1;
        start_cv  ++;

        if (start_cv == COLOR_CHANNELS)
          start_cv = 0;
      }
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;

  for (rw = 0; rw < MAX_ROW/2; rw ++)
  {
    idx  = rw + (MAX_ROW/2);
    row1 = (uint16_t *)&work_buff[rw * (MAX_COL/2)];

    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      cv = row1[cl];

      //Write the left side pixels:
      idx3 = col[idx][cl];
      led[idx3].r = color_tab_left[cv].r;
      led[idx3].g = color_tab_left[cv].g;
      led[idx3].b = color_tab_left[cv].b;
      idx3 = col[(MAX_ROW/2) - (rw+1)][cl];
      led[idx3].r = color_tab_left[cv].r;
      led[idx3].g = color_tab_left[cv].g;
      led[idx3].b = color_tab_left[cv].b;

      //Write the right side pixels:
      idx3 = col[idx][(MAX_COL-1) - cl];
      led[idx3].r = color_tab_right[cv].r;
      led[idx3].g = color_tab_right[cv].g;
      led[idx3].b = color_tab_right[cv].b;
      idx3 = col[(MAX_ROW/2) - (rw+1)][(MAX_COL-1) - cl];
      led[idx3].r = color_tab_right[cv].r;
      led[idx3].g = color_tab_right[cv].g;
      led[idx3].b = color_tab_right[cv].b;
    }
  }
}


//Draws a single arrow.  xpos defines the leftmost x position.
void draw_arrow(uint16_t chan, uint16_t lr, uint16_t xpos, uint16_t dir)
{
  int16_t  x, rw;
  uint16_t idx, idx2;
  uint16_t r, g, b;
  uint32_t temp;

  if (lr == 0) //left
  {
    r = color_tab_left[chan].r;
    g = color_tab_left[chan].g;
    b = color_tab_left[chan].b;
  }
  else
  {
    r = color_tab_right[chan].r;
    g = color_tab_right[chan].g;
    b = color_tab_right[chan].b;
  }

  temp = r + g + b;
    
  if (temp > 0)
  {
    if (dir == 1) //moving left
    {
      for (rw = 0; rw < MAX_ROW/2; rw ++)
      {
        x = (int16_t)xpos + 6 - rw;
        if (rw == (MAX_ROW/2-1))
          x ++;

        for (idx = 0; idx < 2; idx ++)
        {
          x += idx;

          if ((x >= 0) && (x < MAX_COL))
          {
            idx2 = col[rw][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
            idx2 = col[MAX_ROW - (rw+1)][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
          }
        }
      }
    }
    else //moving right
    {
      for (rw = 0; rw < MAX_ROW/2; rw ++)
      {
        x = (int16_t)xpos + rw - 6;
        if (rw == (MAX_ROW/2-1))
          x --;

        for (idx = 0; idx < 2; idx ++)
        {
          x += idx;

          if ((x >= 0) && (x < MAX_COL))
          {
            idx2 = col[rw][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
            idx2 = col[MAX_ROW - (rw+1)][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
          }
        }
      }
    }
  }
}


//This function draws randomly moving ripples, 1 per channel per side.
void ripple(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx;
  uint16_t *buff;

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_ARROW_FRAMES))
  {
    disp_frames = 0;

    //Compute the left side.  Right side is a mirror image.
    //Create a new random arrangement of line segments in the work buffer.
    buff = (uint16_t *)&work_buff;

    for (idx = 0; idx < COLOR_CHANNELS; idx ++)
    {
      //Create left channel arrow
      buff[0] = rnd(5) + 1;        //move factor
      buff[1] = rnd(4) + 1;        //pause factor
      buff[2] = rnd(MAX_COL/2);    //current X pos
      buff[3] = 0;                 //current count
      buff[4] = 1;                 //current state: 1=moving, 0=paused
      if (buff[2] < (MAX_COL/4))
        buff[5] = 2;               //dir: 2=moving right
      else
        buff[5] = 1;               //dir: 1=moving left

      //Create right channel arrow
      buff[6] = rnd(5) + 1;        //move factor
      buff[7] = rnd(4) + 1;        //pause factor
      buff[8] = MAX_COL/2 + rnd(MAX_COL/2);  //current X pos
      buff[9] = 0;                 //current count
      buff[10] = 1;                //current state: 1=moving, 0=paused
      if (buff[8] < (MAX_COL*3/4))
        buff[11] = 2;              //dir: 2=moving right
      else
        buff[11] = 1;              //dir: 1=moving left

      buff += 6 * 2; //position for the next channel
    }
  }

  memset(led, 0, sizeof(led));
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    //Move left channel arrow
    if (buff[4] == 1) //if moving
    {
      if (buff[5] == 1) //moving left
      {
        buff[2] --;
        if (buff[2] < 1)
          buff[5] = 2;
      }
      else //moving right
      {
        buff[2] ++;
        if (buff[2] > (MAX_COL-3))
          buff[5] = 1;
      }
  
      buff[3] ++;
      if (buff[3] == buff[0]) //reached move count
      {
        buff[4] = 0;
        buff[3] = 0;
      }
    }
    else //paused
    {
      buff[3] ++;
      if (buff[3] == buff[1]) //reached pause count
      {
        buff[4] = 1;
        buff[3] = 0;
      }
    }

    draw_arrow(idx, 0, buff[2], buff[5]);

    //Move right channel arrow
    if (buff[10] == 1) //if moving
    {
      if (buff[11] == 1) //moving left
      {
        buff[8] --;
        if (buff[8] < 3)
          buff[11] = 2;
      }
      else //moving right
      {
        buff[8] ++;
        if (buff[8] > (MAX_COL-3))
          buff[11] = 1;
      }
  
      buff[9] ++;
      if (buff[9] == buff[6]) //reached move count
      {
        buff[10] = 0;
        buff[9]  = 0;
      }
    }
    else //paused
    {
      buff[9] ++;
      if (buff[9] == buff[7]) //reached pause count
      {
        buff[10] = 1;
        buff[9]  = 0;
      }
    }

    draw_arrow(idx, 1, buff[8], buff[11]);

    buff += 6 * 2; //position for the next channel
  }
}


#define MIN_CURVE_FRAMES              (FRAMES_PER_SEC * 3)
#define MAX_RANGE                     (MAX_ROW - CURVE_WIDTH)
#define MAX_X                          2.0
#define X_INC                         (MAX_X * 2.0 / (float)MAX_COL)
#define X_INC2                        (MAX_X / (float)MAX_COL)
#define MIN_A                          0.4
#define MAX_A                          0.8
#define NUM_A                          25
#define A_INC                         ((MAX_A - MIN_A) / NUM_A)
#define MIN_B                          4.5
#define MAX_B                          6.5
#define NUM_B                          25
#define B_INC                         ((MAX_B - MIN_B) / NUM_B)
#define MIN_C                          5.0
#define MAX_C                          16.0
#define NUM_C                          22
#define C_INC                         ((MAX_C - MIN_C) / NUM_C)

#if 0
uint16_t curve_flip;

#pragma DATA_SECTION(curve_rows, "ramConsts")
const uint16_t curve_rows[COLOR_CHANNELS] = {2, 2, 1, 2, 2, 1, 2, 2, 1, 1};
#define CURVE_WIDTH                    4 /* width of channels 0+1 */

#pragma CODE_SECTION(curves, "ramCode")

//This function creates a display where the channels are drawn as a polynomial
//curve:
//
//  y = a * x^5 - b * x^3 + c * x
//
//    0.4 <= a <= 0.8
//    4.5 <= b <= 6.5
//    5.0 <= c <= 16.0
//   -2.0 <= x <= 2.0
//
//The curve is then scaled and shifted to fit into rows 0 to (max - wave_width).
void curves(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  cv;
  uint16_t  rw, cl;
  int16_t   pos, idx3;
  int16_t  *buff;
  float     a, b, c;
  float     x, x3, x5;
  float     max, min;
  float     range, scale;
  float     shift, temp;
  float    *curv;
  Led      *cv_clr;

  if (peak_flag & (disp_frames > MIN_CURVE_FRAMES))
  {
    disp_frames = 0;
    curve_flip  = rnd(100) & 0x01;
    buff = (int16_t *)&work_buff[0];
    curv = (float *)&work_buff[MAX_COL*2];
    pos = rnd(MAX_ROW - CURVE_WIDTH - 1); //get starting point on left side

    x = -MAX_X;
    a = MIN_A + ((float)rnd(NUM_A) * A_INC);
    b = MIN_B + ((float)rnd(NUM_B) * B_INC);
    c = MIN_C + ((float)rnd(NUM_C) * C_INC);
    min = 99999.9;
    max =-99999.9;

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      //Compute the curve's path left to right. This will be the bottom row
      //of channel 0. All others are relative to it, wrapping top to bottom.

      x3 = x * x;
      x5 = x * x3 * x3;
      x3*= x;

      temp = (a * x5) - (b * x3) + (c * x);
      curv[cl] = temp;

      if (temp > max) max = temp;
      if (temp < min) min = temp;

      x += X_INC;
    }

    range = max - min;
    scale = (float)MAX_RANGE / range;
    shift = min * scale * -1.0;

    //Scale and shift the curve into the display rows
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      buff[cl] = (int16_t)((curv[cl] * scale) + shift + 0.50);
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (int16_t *)&work_buff[0];

  for (cl = 0; cl < MAX_COL; cl ++)
  {
    cv  = 0;     //start each column with channel 0
    idx = 0;

    if (curve_flip)
      pos = (MAX_ROW-1)-buff[cl]; //bottom row for channels 0
    else
      pos = buff[cl];             //bottom row for channels 0

    if (cl < (MAX_COL/2))
      cv_clr = &color_tab_left[cv];
    else
      cv_clr = &color_tab_right[cv];

    for (rw = 0; rw < MAX_ROW; rw ++)
    {
      idx3 = col[pos][cl];
      led[idx3].r = cv_clr->r;
      led[idx3].g = cv_clr->g;
      led[idx3].b = cv_clr->b;

      idx ++;
      if (idx == curve_rows[cv])
      {
        idx = 0;
        cv ++;
        if (cv == COLOR_CHANNELS)
          cv = 0;

        if (cl < (MAX_COL/2))
          cv_clr = &color_tab_left[cv];
        else
          cv_clr = &color_tab_right[cv];
      }

      if (curve_flip)
      {
        pos --;
        if (pos < 0)
          pos = MAX_ROW-1;
      }
      else
      {
        pos ++;
        if (pos >= MAX_ROW)
          pos = 0;
      }
    }
  }
}
#endif


//table to map color channels to curves, two channels per curve
#pragma DATA_SECTION(curve_chan, "ramConsts")
const uint16_t curve_chan[5 * 2] = {2, 3, 9, 8,
                                    7, 6, 4, 5,
                                    0, 1};

#pragma CODE_SECTION(curve2, "ramCode")

//This function creates a display where the channels are drawn as polynomial
//curves:
//
//  y = a * x^5 - b * x^3 + c * x
//
//    0.4 <= a <= 0.8
//    4.5 <= b <= 6.5
//    5.0 <= c <= 16.0
//   -2.0 <= x <= 2.0
//
//The 10 channels are combined into 5, and 5 curves are calculated. The curves
//are then scaled and shifted to fit into rows 0 to (max - wave_width).
void curve2(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  cv1;
  uint16_t  clr_scale1;
  uint16_t  cl;
  int16_t   pos1;
  int16_t   idx3;
  uint16_t  r1, g1, b1;
  float     a, b, c;
  float     x, x3, x5;
  float     max, min;
  float     range, scale;
  float     shift, temp;
  float    *buff;
  float    *tcurv, *curv;

  buff = (float *)&work_buff[0];        //storage for display row values

  if (peak_flag & (disp_frames > MIN_CURVE_FRAMES))
  {
    disp_frames = 0;

    //Recompute the curves static data
    for (idx = 0; idx < 5; idx ++)
    {
      buff[idx  ]  = rnd(100) & 0x01; //curve_flip
      buff[idx+5]  = MIN_A + ((float)rnd(NUM_A) * A_INC); // a
      buff[idx+10] = MIN_B + ((float)rnd(NUM_B) * B_INC); // b
      buff[idx+15] = MIN_C + ((float)rnd(NUM_C) * C_INC); // c
      buff[idx+20] = 1.0; //current inc/dec of C term
      buff[idx+25] = (float)((idx/2)+1); //speed
      idx3 = rnd(100) & 0x03;

      switch(idx3)
      {
         case 0:
           buff[idx+30] = -MAX_X; //starting x
           buff[idx+35] = X_INC;  //x increment
           break;
         case 1:
           buff[idx+30] = 0;
           buff[idx+35] = X_INC2;
           break;
         case 2:
           buff[idx+30] = MAX_X; //starting x
           buff[idx+35] = -X_INC;  //x increment
           break;
         case 3:
           buff[idx+30] = -MAX_X; //starting x
           buff[idx+35] = X_INC2;  //x increment
           break;
      }
    }
  }

  //Compute/update the curves data
  for (idx = 0; idx < 5; idx ++)
  {
    tcurv = (float *)&work_buff[100];  //temp storage
    curv  = (float *)&work_buff[200+(idx*100)];  //curve data

    a = buff[idx+5];
    b = buff[idx+10];
    c = buff[idx+15];
    idx3 = (int16_t)buff[idx+25];

//don't update the curve when zero...?
    if ((disp_frames % idx3) == 0)
    {
      c += ((float)C_INC * buff[idx+20]);
      if (c > MAX_C)
      {
        c = MAX_C;
        buff[idx+20] = -1.0;
      }
      else
      if (c < MIN_C)
      {
        c = MIN_C;
        buff[idx+20] = 1.0;
      }
      buff[idx+15] = c;
    }
    min = 99999.9;
    max =-99999.9;
    x   = buff[idx+30];

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      x3 = x * x;
      x5 = x * x3 * x3;
      x3*= x;

      temp = (a * x5) - (b * x3) + (c * x);
      tcurv[cl] = temp;

      if (temp > max) max = temp;
      if (temp < min) min = temp;
      x += buff[idx+35];
    }

    range = max - min;
    scale = (float)(MAX_ROW-2) / range;
    shift = min * scale * -1.0;

    //Scale and shift the curve into the display rows
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      curv[cl] = (int16_t)((tcurv[cl] * scale) + shift + 0.50);
    }
  }


  memset(led, 0, sizeof(led));
  build_color_table(left_val, right_val);

  disp_frames ++;

  for (idx = 0; idx < 5; idx ++)
  {
    curv = (float *)&work_buff[200+(idx*100)];  //curve data

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      if (cl == 0) //get left side channel brightness
      {
        idx3 = idx * 2;
        cv1  = curve_chan[idx3];
        clr_scale1 = left_val[cv1];
        if (left_val[curve_chan[idx3 + 1]] > clr_scale1)
          clr_scale1 = left_val[curve_chan[idx3 + 1]];

        get_rgb(rainbow[chan_clr[cv1]], &r1, &g1, &b1);
        r1 = (r1 * clr_scale1) >> 8;
        g1 = (g1 * clr_scale1) >> 8;
        b1 = (b1 * clr_scale1) >> 8;
      }
      else
      if (cl == (MAX_COL/2)) //get right side channel brightness
      {
        idx3 = idx * 2;
        cv1  = curve_chan[idx3];
        clr_scale1 = right_val[cv1];
        if (right_val[curve_chan[idx3 + 1]] > clr_scale1)
          clr_scale1 = right_val[curve_chan[idx3 + 1]];

        get_rgb(rainbow[chan_clr[cv1]], &r1, &g1, &b1);
        r1 = (r1 * clr_scale1) >> 8;
        g1 = (g1 * clr_scale1) >> 8;
        b1 = (b1 * clr_scale1) >> 8;
      }

      if (buff[idx]) //curve flip
        pos1 = (MAX_ROW-2)-curv[cl]; //bottom row for curve 0..2
      else
        pos1 = curv[cl];             //bottom row for curve 0..2

      if (clr_scale1 > 0)
      {
        idx3 = col[pos1][cl];
        led[idx3].r = r1;
        led[idx3].g = g1;
        led[idx3].b = b1;
        idx3 = col[pos1+1][cl];
        led[idx3].r = r1;
        led[idx3].g = g1;
        led[idx3].b = b1;
      }
    }
  }
}


#if 0
#pragma CODE_SECTION(chevron, "ramCode")

#define MIN_CHEVRON_FRAMES             (FRAMES_PER_SEC * 2)
uint16_t chevron_flip[4];
#define CHEV_PAT                       4
#pragma DATA_SECTION(chevron_db, "ramConsts")
const char chevron_db[CHEV_PAT] = {0x0, 0xf, 0x3, 0xc};

//This function creates a display with 4 vertical chevrons. They will independently
//flip upside down, while maintaining a symmetric arrangement.
void chevron(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx3;
  int16_t   cv;
  uint16_t  rw, cl, qd;
  int16_t   row, sx, ex;
  uint16_t  r, g, b;

  if (peak_flag & (disp_frames > MIN_CHEVRON_FRAMES))
  {
    disp_frames = 0;
    idx3 = chevron_db[rnd(CHEV_PAT)];

    for (idx = 0; idx < 4; idx ++)
      chevron_flip[idx] = (idx3 >> idx) & 1;
  }

  disp_frames ++;

  build_color_table(left_val, right_val);

  for (qd = 0; qd < 4; qd ++)
  {
    sx = qd * (MAX_COL/4); //get starting x column index

    for (cl = 0; cl < MAX_COL/8; cl ++)
    {
      ex  = sx + (MAX_COL/4 - 1 - cl); //symmetric ending x column

      if (chevron_flip[qd])
      {
        row = MAX_ROW - 1 - cl;
      }
      else
      {
        row = cl;
      }

      idx = 0;
      cv  = 0;

      if (qd < 2)
      {
        r = color_tab_left[cv].r;
        g = color_tab_left[cv].g;
        b = color_tab_left[cv].b;
      }
      else
      {
        r = color_tab_right[cv].r;
        g = color_tab_right[cv].g;
        b = color_tab_right[cv].b;
      }

      for (rw = 0; rw < MAX_ROW; rw ++)
      {
        //Write the left side pixels:
        idx3 = col[row][sx+cl];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;
        idx3 = col[row][ex];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;

        idx ++;
        if (idx == curve_rows[cv])
        {
          idx = 0;
          cv ++;
          if (cv == COLOR_CHANNELS)
            cv = 0;

          if (qd < 2)
          {
            r = color_tab_left[cv].r;
            g = color_tab_left[cv].g;
            b = color_tab_left[cv].b;
          }
          else
          {
            r = color_tab_right[cv].r;
            g = color_tab_right[cv].g;
            b = color_tab_right[cv].b;
          }
        }

        if (chevron_flip[qd])
        {
          row --;
          if (row < 0)
            row = MAX_ROW-1;
        }
        else
        {
          row ++;
          if (row == MAX_ROW)
            row = 0;
        }
      }
    }
  }
}
#endif


#define SYM_NOUN                       31
#pragma DATA_SECTION(sym_noun, "ramConsts")
const char sym_noun[SYM_NOUN][4] =
{"BABY", "BIRD", "BONE", "BUM ",
 "BOY ", "CAT ", "COW ",
 "DAY ", "DOG ", "DRUM", "EYE ",
 "FIRE", "FOOD", "FOOT", "GIRL", "HAND", "HEAD",
 "LOVE", "MEAT", "MOON",
 "PIG ", "LIP ", "MAN ", "STAR",
 "TOY ", "YOU ", "TEAM", "THEM",
 "WHO ", "WOOD", "WORM"};

#define SYM_VERB                       34
#pragma DATA_SECTION(sym_verb, "ramConsts")
const char sym_verb[SYM_VERB][4] =
{"BANG", "BEAT", "BLOW", "BOOM", "CHEW", "CHUG",
 "EAT ", "GOT ", "HEY ", "JUMP", "LIKE", "LOVE",
 "MAKE", "MOO ", "NOT ", "PEE ", "POO ", "POP ",
 "POW ", "RUN ", "PLUG", "SING",
 "SAY ", "SEE ", "SING", "SLAM", "STOP",
 "TRY ", "WAKE", "WHAT", "YES ",
 "ZAP ", "ZING", "ZOOM"};

#define SYM_ADJ                        30
#pragma DATA_SECTION(sym_adj, "ramConsts")
const char sym_adj[SYM_ADJ][4] =
{"BAD ", "BIG ", "DAMN", "DRY ", "EASY", "FAT ",
 "COLD", "COOL", "DEEP", "FAST", "HOT ", "MAD ",
 "ODD ", "OLD ", "WET ", "WOW ",
 "FUZZ", "GOOD", "HARD", "LONG",
 "LOUD", "SING", "SLIM", "SOFT", "SLOW",
 "THIN", "TORN", "UGLY", "WARM", "WILD"};

uint16_t get_verb(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_VERB);

  if (sym_verb[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_verb[idx][0];
  str[1] = sym_verb[idx][1];
  str[2] = sym_verb[idx][2];
  str[3] = sym_verb[idx][3];

  return(siz);
}

uint16_t get_noun(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_NOUN);

  if (sym_noun[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_noun[idx][0];
  str[1] = sym_noun[idx][1];
  str[2] = sym_noun[idx][2];
  str[3] = sym_noun[idx][3];

  return(siz);
}

uint16_t get_adj(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_ADJ);

  if (sym_adj[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_adj[idx][0];
  str[1] = sym_adj[idx][1];
  str[2] = sym_adj[idx][2];
  str[3] = sym_adj[idx][3];

  return(siz);
}


#pragma CODE_SECTION(vertical, "ramCode")

#define MIN_VERTICAL_FRAMES           (FRAMES_PER_SEC * 3)
#define VERT_LINE_END                 (MAX_ROW * (MAX_COL/2/2))
#define VERT_MIN_SEG                  3
#define VERT_MAX_SEG                  8
#define VERT_COL_SEG                  (MAX_ROW / VERT_MIN_SEG)

uint16_t sym_do_text;
uint16_t sym_size[2];
uint16_t sym_text_row[8];
char     sym_text[2][4];


//This function creates a display that is L-R symmetric, creating small moving
//vertical pieces. Randomly it is overlaid with pseudo-word text.
void vertical(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx, rot;
  uint16_t  idx2, idx3;
  uint16_t  next, len;
  uint16_t  cv, scale;
  uint16_t  rw, cl, xpos;
  uint16_t  r, g, b;
  uint32_t  r1, g1, b1;
  uint32_t  color, temp;
  uint16_t *buff;
  int16_t   chan_cnt[COLOR_CHANNELS];
  Led      *cv_clr1;
  Led      *cv_clr2;

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_VERTICAL_FRAMES))
  {
    disp_frames = 0;
    sym_do_text = rnd(100);
    if (sym_do_text < 13)
      sym_do_text = 1;
    else
      sym_do_text = 0;

    if (sym_do_text)
    {
      //don't allow channels 0, 1 in the background (they are used for text)
      for (idx = 0; idx < 2; idx ++)
        chan_cnt[idx] = -2;
      for (idx = 2; idx < COLOR_CHANNELS; idx ++)
        chan_cnt[idx] = (VERT_LINE_END / (COLOR_CHANNELS-2));

      for (idx = 0; idx < 8; idx ++)
      {
        sym_text_row[idx] = rnd(MAX_ROW-FONT_HEIGHT-3) + 3;

        if (idx > 0)
        {
          if (sym_text_row[idx] > (sym_text_row[idx-1]+2))
            sym_text_row[idx] = sym_text_row[idx-1]+2;
          if (sym_text_row[idx] < (sym_text_row[idx-1]-2))
            sym_text_row[idx] = sym_text_row[idx-1]-2;
        }
      }

      for (idx = 0; idx < 2; idx ++)
      {
        idx2 = rnd(3);
        switch (idx2)
        {
          case 0: //Verb + Noun
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_verb(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_noun(&sym_text[idx][0]);
            }
            break;
          case 1: //Adj + Noun
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_adj(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_noun(&sym_text[idx][0]);
            }
            break;
          case 2: //Verb + Adj
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_verb(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_adj(&sym_text[idx][0]);
            }
            break;
        }
      }
    }
    else //no text, use all channels
    {
      for (idx = 0; idx < COLOR_CHANNELS; idx ++)
        chan_cnt[idx] = (VERT_LINE_END / COLOR_CHANNELS);
    }

    //Compute the left side.  Right side is a mirror image.
    //Create a new random arrangement of line segments in the work buffer.
    buff = (uint16_t *)&work_buff;

    for (cl = 0; cl < MAX_COL/2; cl += 2)
    {
      idx  = 0;
      idx3 = 2;
      if ((cl % 4) == 0)
        buff[0] = rnd(6) + 10;   //store the speed factor, 10 to 15
      else
        buff[0] = rnd(5) + 3;   //store the speed factor, 3 to 7
      buff[1] = MAX_ROW/2; //current rotation

      while (idx < MAX_ROW)
      {
        idx2 = rnd(99999);
        if (sym_do_text)
          cv = idx2 % (COLOR_CHANNELS-2) + 2;
        else
          cv = idx2 % COLOR_CHANNELS;
        len = (idx2 % (VERT_MAX_SEG-VERT_MIN_SEG)) + VERT_MIN_SEG;    //line length

        if ((len + idx) > MAX_ROW)
          len = MAX_ROW - idx - 1;

        //don't let len equal 1
        if (len < 2)
          len = 0xff; //abort the end of the column

        if (chan_cnt[cv] <= 0) //this channel has been "used up". Find another one
        {
          for (idx2 = 0; idx2 < COLOR_CHANNELS; idx2 ++)
          {
            cv ++;
            if (cv >= COLOR_CHANNELS) cv = 0;
            if (chan_cnt[cv] > 0) break;
          }
        }

        buff[idx3++] = cv;   //store the channel in word 0
        buff[idx3++] = len;  //store the segment length in word 1

        chan_cnt[cv] -= len;
        idx += len + 1;
      }

      buff[idx3++] = 0xff;   //mark the end
      buff[idx3++] = 0xff;

      buff += (VERT_MAX_SEG + 1) * 2; //position for the next column
    }
  }

  memset(led, 0, sizeof(led));
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (cl = 0; cl < MAX_COL/2; cl += 2)
  {
    idx2 = buff[0]; //get the column's speed
    rot = buff[1];
    if ((disp_frames % idx2) == 0) //update the rotation and save
    {
      rot --;
      if (rot < 0)
        rot = MAX_ROW-1;
      buff[1] = rot;
    }

    idx  = 0;
    idx3 = 4;
    cv   = buff[2];
    len  = buff[3];
    next = len;

    for (rw = 0; rw < MAX_ROW; rw ++)
    {
      if (idx == next)
      {
        cv  = buff[idx3++];
        len = buff[idx3++];
        next += len;
        //leave a blank row
        rw ++;
        rot --;
        if (rot < 0)
          rot = MAX_ROW-1;

        if ((len == 0xff) || (len < 2)) break;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rot][cl];
      led[idx2].r = cv_clr1->r;     //left side
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rot][cl+1];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rot][MAX_COL-1-cl]; //right side
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rot][MAX_COL-2-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
      rot --; //update for the next iteration
      if (rot < 0)
        rot = MAX_ROW-1;
    }

    buff += (VERT_MAX_SEG + 1) * 2; //position for the next column
  }

  if (sym_do_text)
  {
    temp = rainbow[chan_clr[0]];
    get_rgb(temp, &r, &g, &b);

    //Combine channels 0 and 1 into a single channel.
    //Don't overwrite higher channels when there's no bass.
    if ((left_val[0] > 0) && (left_val[1] > 0))
    {
      scale = (left_val[0] > left_val[1]) ? left_val[0] : left_val[1];
      r1 = (r * scale);
      g1 = (g * scale);
      b1 = (b * scale);
      color = ((r1 << 8) & 0xff0000) +
               (g1 & 0x00ff00) +
               (b1 >> 8);

      xpos = MAX_COL/2;

      for (idx = sym_size[0]-1; idx >= 0; idx --) //left side
      {
        idx2 = font_get_idx(sym_text[0][idx]);
        xpos -= (font_char_len(idx2) + 1);
        draw_char(xpos, sym_text_row[idx], idx2, color);
      }
    }

    if ((right_val[0] > 0) && (right_val[1] > 0))
    {
      scale = (right_val[0] > right_val[1]) ? right_val[0] : right_val[1];
      r1 = (r * scale);
      g1 = (g * scale);
      b1 = (b * scale);
      color = ((r1 << 8) & 0xff0000) +
               (g1 & 0x00ff00) +
               (b1 >> 8);

      xpos = MAX_COL/2 + 1;

      for (idx = 0; idx < sym_size[1]; idx ++) //right side
      {
        idx2 = font_get_idx(sym_text[1][idx]);
        draw_char(xpos, sym_text_row[idx], idx2, color);
        xpos += font_char_len(idx2) + 1;
      }
    }
  }
}


#pragma CODE_SECTION(two_by_two, "ramCode")

#define MIN_TBT_FRAMES                (FRAMES_PER_SEC * 4)

#ifdef SLAVE
uint32_t prev_mode;
#endif

//This function creates a display where each 2x2 pixel subarray is assigned a
//random color organ channel.  When the peak flag is set (after a minumum number
//of frames), it will shuffle the 2x2s. For the large array, 3x3s are created.
void two_by_two(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val, uint32_t mode)
{
  uint16_t idx, idx2, idx3;
  uint16_t cv;
  uint16_t rw, cl;
  int16_t  blk_cnt;
  int16_t  shift;
  int16_t  prev_shift;
  uint16_t temp[TBT_ROW];
  Led      *cv_clr;

  cv = 0;

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_TBT_FRAMES))
  {
    disp_frames = 0;

    if (mode == 1) //fixed fade
    {
      cv   = 0;
      blk_cnt = 0;
    }

    if (mode > 1) //not a fixed pattern
    {
      //Create a new random arrangement of 2x2s or 3x3s.
      //First, create a linear arrangement to guarantee equal channel distribution
      idx = rnd(100);
      if (idx < 50)
        cv = 0; //channel numbers increasing with row number
      else
        cv = COLOR_CHANNELS - 2; //channel numbers decreasing with row number

      for (cl = 0; cl < TBT_COL; cl += 2)
      {
        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          tbt_map[rw][cl]   = cv;
          tbt_map[rw][cl+1] = cv+1;

          if (idx < 50)
          {
            cv += 2;
            if (cv >= COLOR_CHANNELS)
              cv = 0;
          }
          else
          {
            cv -= 2;
            if (cv > COLOR_CHANNELS)
              cv = COLOR_CHANNELS - 2;
          }
        }
      }
    }

    prev_shift = rnd(TBT_ROW); //get an initial shift
    shift = prev_shift;

    for (cl = 0; cl < TBT_COL; cl ++)
    {
      switch (mode)
      {
        case 0: //fixed arrow
          //create a symmetric, fixed row-based "arrow" pattern
          if (cl < (TBT_COL/2))
          {
            for (rw = 0; rw < 2; rw ++) //rows 0 and 4, 1 and 3
            {
              idx3 = (COLOR_CHANNELS + cl - rw) % COLOR_CHANNELS;
              tbt_map[rw][cl] = idx3;
              tbt_map[rw][TBT_COL - 1 - cl] = idx3;
              tbt_map[4-rw][cl] = idx3;
              tbt_map[4-rw][TBT_COL - 1 - cl] = idx3;
            }

            idx3 = (COLOR_CHANNELS + cl - 2) % COLOR_CHANNELS; //row 2
            tbt_map[2][cl] = idx3;
            tbt_map[2][TBT_COL - 1 - cl] = idx3;
          }
          break;

        case 1: //fixed fade
          if (cl >= (TBT_COL/2))
          {
            //Set row zero, because it is always inside the array
            tbt_map[0][cl] = cv;
            tbt_map[0][TBT_COL/2 - 1 - (cl-(TBT_COL/2))] = cv;

            blk_cnt ++;
            if (blk_cnt == TBT_CHAN_BLOCKS)
            {
              cv ++;
              blk_cnt = 0;
            }

            idx3 = cl - 1;
            rw   = 1;

            //move up and left 1 block
            while ((rw < TBT_ROW) && (idx3 >= TBT_COL/2)) //stay inside the array
            {
              tbt_map[rw][idx3] = cv;
              tbt_map[rw][TBT_COL/2 - 1 - (idx3-(TBT_COL/2))] = cv;

              blk_cnt ++;
              if (blk_cnt == TBT_CHAN_BLOCKS)
              {
                cv ++;
                blk_cnt = 0;
              }

              rw   ++;
              idx3 --;
            }

            if (cl == (TBT_COL-1)) //special case for the outside corners
            {
              for (rw = 1; rw < TBT_ROW; rw ++)
              {
                idx2 = rw;
                idx3 = cl;

                while ((idx2 < TBT_ROW) && (idx3 >= TBT_COL/2)) //stay inside the array
                {
                  tbt_map[idx2][idx3] = cv;
                  tbt_map[idx2][TBT_COL/2 - 1 - (idx3-(TBT_COL/2))] = cv;

                  blk_cnt ++;
                  if (blk_cnt == TBT_CHAN_BLOCKS)
                  {
                    cv ++;
                    blk_cnt = 0;
                  }

                  idx2 ++;
                  idx3 --;
                }
              }
            }
          }
          break;

        case 2: //random blocks
          //do random rotate by column
          while (shift == prev_shift)
            shift = rnd(TBT_ROW);

          prev_shift = shift;
          break;

        case 3: //random wave
          //move -1, 0 or +1 rows from the previous row to make a "wave" shape
          if ((cl & 1) == 0)
          {
            if (prev_shift == 2) // in the middle; pick up or down randomly
            {
              if (rnd(100) < 50)
                shift = 1;
              else
                shift = 3;
            }
            else //move towards the other end
            {
              if (shift < 2)
                shift ++;
              else
                shift --;
            }
          }

          prev_shift = shift;
          break;
      }

      if (mode > 1)
      {
        for (rw = 0; rw < TBT_ROW; rw ++) //save the column
          temp[rw] = tbt_map[rw][cl];

        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          idx3 = rw + shift;
          if (idx3 >= TBT_ROW) idx3 -= TBT_ROW;

          tbt_map[idx3][cl] = temp[rw];
        }
      }
    }

    //L/R panel invert the fixed patterns
    if ((mode < 2) && (rnd(100) > 50))
    {
      for (cl = 0; cl < TBT_COL/2; cl ++)
      {
        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          idx3 = tbt_map[rw][cl];
          tbt_map[rw][cl] = tbt_map[rw][cl+TBT_COL/2];
          tbt_map[rw][cl+TBT_COL/2] = idx3;
        }
      }
    }
  }

  disp_frames ++;
  build_color_table(left_val, right_val);

  #ifdef LARGE_ARRAY
  for (rw = 1; rw < MAX_ROW; rw += TBT_STEP)
  #else
  for (rw = 0; rw < MAX_ROW; rw += TBT_STEP)
  #endif
  {
    for (cl = 0; cl < MAX_COL; cl += TBT_STEP)
    {
      idx = tbt_map[rw/TBT_STEP][cl/TBT_STEP];

      //Scale the brightness to the current channel intensity
      if (cl < panel_off[2][1])
        cv_clr = &color_tab_left[idx];
      else
        cv_clr = &color_tab_right[idx];

      for (idx2 = 0; idx2 < TBT_STEP; idx2 ++)
      {
        for (idx = 0; idx < TBT_STEP; idx ++)
        {
          idx3 = col[rw+idx][cl+idx2];
          led[idx3].r = cv_clr->r;
          led[idx3].g = cv_clr->g;
          led[idx3].b = cv_clr->b;
        }
      }

      #ifdef LARGE_ARRAY
      if (rw == 1)
      {
        for (idx2 = 0; idx2 < TBT_STEP; idx2 ++)
        {
          idx3 = col[0][cl+idx2];
          led[idx3].r = cv_clr->r >> 2;
          led[idx3].g = cv_clr->g >> 2;
          led[idx3].b = cv_clr->b >> 2;
        }
      }
      #endif
    }
  }
}


#if 0
#pragma CODE_SECTION(color_bars, "ramCode")

//This function creates a simple color organ display with one row of LEDs
//given to each channel.  Channel 0 (lowest frequencies) will go to Row 0.
void color_bars(uint16_t side, uint16_t *chan_val)
{
  uint32_t color;
  uint16_t idx, idx2, idx3;
  uint16_t r, g, b, cv;
  uint16_t side_offset;

  if (side == LEFT)
    side_offset = 0;
  else
    side_offset = PANEL_COL * 2;


  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    if (idx < MAX_ROW)
    {
      //Get the base color
      color = rainbow[chan_clr[idx]];
      get_rgb(color, &r, &g, &b);

      //Scale the brightness to the current channel intensity
      cv = chan_val[idx];
      r = (r * cv) >> 8;
      g = (g * cv) >> 8;
      b = (b * cv) >> 8;

      //Paint the row with it
      for (idx2 = 0; idx2 < PANEL_COL*2; idx2 ++)
      {
        idx3 = col[idx][idx2+side_offset];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;
      }
    }
  }
}
#endif


typedef struct
{
  uint16_t  state:2;   //0=not moving, 1=up, 2=down
  uint16_t  rate:3;    //x*5 frames. valid 1 fast ... 7 slow
  uint16_t  count:3;
  uint16_t  frames:10;
  uint16_t  x:7;
  uint16_t  y:7;
} lavadata;


//GLOBAL HERE FOR TESTING ONLY
//  uint16_t  x, xlim;
//  uint16_t  y, ylim;
#pragma CODE_SECTION(draw_blob, "ramCode")
void draw_blob(uint16_t channel, uint16_t blob, uint16_t lr, lavadata *lptr)
{
  uint16_t  idx;
  uint16_t  r, g, b;
  uint16_t  rw, cl;
  uint16_t  x, xlim;
  uint16_t  y, ylim;

  if (lr == 0) //left
  {
    r = color_tab_left[channel].r;
    g = color_tab_left[channel].g;
    b = color_tab_left[channel].b;
  }
  else //right
  {
    r = color_tab_right[channel].r;
    g = color_tab_right[channel].g;
    b = color_tab_right[channel].b;
  }

  //Don't draw a black blob (it won't move either)
  if ((r == 0) && (g == 0) && (b == 0))
    return;

  if (blob == 0)
  {
    if (lptr->count == 2)
    {
      xlim = 7;
      ylim = 4;
    }
    else
    {
      xlim = 6;
      ylim = 3;
    }
  }
  else
  {
    if (lptr->count == 2)
    {
      xlim = 4;
      ylim = 7;
    }
    else
    {
      xlim = 3;
      ylim = 6;
    }
  }

  //Draw the blob
  for (rw = 0; rw < ylim; rw ++)
  {
    for (cl = 0; cl < xlim; cl ++)
    {
      x = lptr->x + cl;
      y = lptr->y + rw;

      if ((x < MAX_COL) && (y < MAX_ROW))  //clip
      {
        if (((rw == 0) && (cl == 0)) ||    //remove corners
            ((rw == 0) && (cl == (xlim-1))) ||
            ((rw == (ylim-1)) && (cl == 0)) ||
            ((rw == (ylim-1)) && (cl == (xlim-1))))
        {
        }
        else
        {
          //Write the left side pixels:
          idx = col[y][x];
          led[idx].r = r;
          led[idx].g = g;
          led[idx].b = b;
        }
      }
    }
  }

  //Update the blob position and state
  lptr->frames ++;

  switch (lptr->state)
  {
    case 0:  //not moving
      if (lptr->frames == (lptr->rate * 14))
      {
        lptr->frames = 0;

        if (lptr->y < MAX_ROW/2)
        {
          lptr->state = 1;
        }
        else
        {
          lptr->state = 2;
        }
      }
      break;
    case 1:  //up
      if (lptr->frames == (lptr->rate * 7))
      {
        lptr->frames = 0;

        if (lptr->y < MAX_ROW-2)
        {
          lptr->y ++;
        }
        else
        {
          lptr->state = 0;
        }
      }
      break;
    case 2:  //down
      if (lptr->frames == (lptr->rate * 7))
      {
        lptr->frames = 0;

        if (lptr->y > 0)
        {
          lptr->y --;
        }
        else
        {
          lptr->state = 0;
        }
      }
      break;
  }
}


#pragma CODE_SECTION(lavalamp, "ramCode")

#define MIN_LAVALAMP_FRAMES             (FRAMES_PER_SEC * 5)
#define LAVABLOBS                       2

#pragma DATA_SECTION(lava_order, "ramConsts")
const uint16_t lava_order[COLOR_CHANNELS] = {6, 8, 4, 1, 9, 5, 2, 7, 3, 0};

//This function creates a display that mimics a lavalamp.
void lavalamp(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2, idx3;
  uint16_t  count;
  lavadata *lptr = (lavadata *)&work_buff[MAX_LEDS>>1];

  if (peak_flag & (disp_frames > MIN_LAVALAMP_FRAMES))
  {
    disp_frames = 0;

    for (idx = 0; idx < COLOR_CHANNELS; idx ++)
    {
      for (idx2 = 0; idx2 < 2; idx2 ++) //left/right
      {
        count = rnd(100);

        if (count < 25)
        {
          lptr->count = 4; //small
          count = 4;
        }
        else
        {
          lptr->count = 2; //large
          count = 2;
        }

        for (idx3 = 0; idx3 < count; idx3 ++)
        {
          lptr->rate   = rnd(6)+1;
          lptr->frames = 0;
          lptr->count  = count;
          lptr->x = rnd(MAX_COL/2 - 4) + (idx2 * (MAX_COL/2));
          lptr->y = rnd(MAX_ROW - 3);

          if (lptr->y > (MAX_ROW/2))
            lptr->state = 2;
          else
            lptr->state = 1;

          lptr ++;
        }
      }
    }

    lptr = (lavadata *)&work_buff[MAX_LEDS>>1];
  }

  disp_frames ++;

  //clear the display each frame
  memset(led, 0, sizeof(led));
  build_color_table(left_val, right_val);

  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    for (idx2 = 0; idx2 < 2; idx2 ++) //left/right
    {
      count = lptr->count;

      for (idx3 = 0; idx3 < count; idx3 ++)
      {
        draw_blob(lava_order[idx], idx3, idx2, lptr);

        lptr ++;
      }
    }
  }
}


#ifdef FLOODS
#pragma CODE_SECTION(floods, "ramCode")

//This function is the main display routine for the flood lights. It takes
//histagrammed and averaged channel data from the prep routine and displays
//the top two channels.
void floods(uint16_t *left_val, uint16_t *right_val)
{
  uint32_t color1, color2;
  uint32_t color3, color4;
  uint16_t idx;
  uint16_t lidx1, lidx2;
  uint16_t ridx1, ridx2;
  uint16_t rw, cl;
  uint16_t r1, g1, b1, cv1;
  uint16_t r2, g2, b2, cv2;

//Future updates: ?
//if largest is > 4x any other, then use for both halves.
//update every frame?

  //To reduce flicker, see if the top 2 simply swapped sides
  if ((top_chan_l[0] == top_chan_l_prev[1]) ||
      (top_chan_l[1] == top_chan_l_prev[0]))
  {
    idx = top_chan_l[0];
    top_chan_l[0] = top_chan_l_prev[1];
    top_chan_l[1] = idx;
  }
  if ((top_chan_r[0] == top_chan_r_prev[1]) ||
      (top_chan_r[1] == top_chan_r_prev[0]))
  {
    idx = top_chan_r[0];
    top_chan_r[0] = top_chan_r_prev[1];
    top_chan_r[1] = idx;
  }

  switch (chan_count_l)
  {
    case 0: //no channels this frame - keep the previous one(s)
      lidx1  = top_chan_l_prev[0];
      lidx2  = top_chan_l_prev[1];
      color1 = rainbow[chan_clr[lidx1]];
      color2 = rainbow[chan_clr[lidx2]];
      idx = top_chan_l_lvl[0];
      top_chan_l_lvl[0] = (idx >> 1) + (idx >> 2);
      idx = top_chan_l_lvl[1];
      top_chan_l_lvl[1] = (idx >> 1) + (idx >> 2);
      break;

    case 1: //just one channel this frame
      lidx1  = top_chan_l[0];
      lidx2  = top_chan_l[1];
      color1 = rainbow[chan_clr[lidx1]];
      color2 = color1;

      for (idx = 0; idx < MAX_CHAN; idx++)
        top_chan_l_prev[idx] = top_chan_l[idx];
      break;

    case 2: //two or more channels this frame
      lidx1  = top_chan_l[0];
      lidx2  = top_chan_l[1];
      color1 = rainbow[chan_clr[lidx1]];
      color2 = rainbow[chan_clr[lidx2]];

      for (idx = 0; idx < MAX_CHAN; idx++)
        top_chan_l_prev[idx] = top_chan_l[idx];
      break;
  }

  switch (chan_count_r)
  {
    case 0: //no channels this frame - keep the previous one(s)
      ridx1  = top_chan_r_prev[0];
      ridx2  = top_chan_r_prev[1];
      color3 = rainbow[chan_clr[ridx1]];
      color4 = rainbow[chan_clr[ridx2]];
      idx = top_chan_r_lvl[0];
      top_chan_r_lvl[0] = (idx >> 1) + (idx >> 2);
      idx = top_chan_r_lvl[1];
      top_chan_r_lvl[1] = (idx >> 1) + (idx >> 2);
      break;

    case 1: //just one channel this frame
      ridx1  = top_chan_r[0];
      ridx2  = top_chan_r[1];
      color3 = rainbow[chan_clr[ridx1]];
      color4 = color3;

      for (idx = 0; idx < MAX_CHAN; idx++)
        top_chan_r_prev[idx] = top_chan_r[idx];
      break;

    case 2: //two or more channels this frame
      ridx1  = top_chan_r[0];
      ridx2  = top_chan_r[1];
      color3 = rainbow[chan_clr[ridx1]];
      color4 = rainbow[chan_clr[ridx2]];

      for (idx = 0; idx < MAX_CHAN; idx++)
        top_chan_r_prev[idx] = top_chan_r[idx];
      break;
  }

  for (cl = 0; cl < FLOOD_COL; cl ++)
  {
    if (cl == 0)
    {
      //Scale the brightness to the current channel intensity
      //Get the base color for this side
      get_rgb(color1, &r1, &g1, &b1);
      get_rgb(color3, &r2, &g2, &b2);

      cv1 = left_val[lidx1];
      if (cv1 < MIN_RGB_VAL)
        cv1 = MIN_RGB_VAL;

      cv2 = right_val[ridx1];
      if (cv2 < MIN_RGB_VAL)
        cv2 = MIN_RGB_VAL;

      //If this side of the panel drops > 25% keep it at 25%
      if (cv1 < top_chan_l_lvl[0])
        cv1 = top_chan_l_lvl[0];

      if (cv2 < top_chan_r_lvl[0])
        cv2 = top_chan_r_lvl[0];

      top_chan_l_lvl[0] = (cv1 >> 1) + (cv1 >> 2);
      top_chan_r_lvl[0] = (cv2 >> 1) + (cv2 >> 2);

      r1 = (r1 * cv1) >> 8;
      g1 = (g1 * cv1) >> 8;
      b1 = (b1 * cv1) >> 8;

      r2 = (r2 * cv2) >> 8;
      g2 = (g2 * cv2) >> 8;
      b2 = (b2 * cv2) >> 8;
    }
    else if (cl == (FLOOD_COL / 2))
    {
      //Scale the brightness to the current channel intensity
      //Get the base color for this side
      get_rgb(color2, &r1, &g1, &b1);
      get_rgb(color4, &r2, &g2, &b2);

      cv1 = left_val[lidx2];
      if (cv1 < MIN_RGB_VAL)
        cv1 = MIN_RGB_VAL;

      cv2 = right_val[ridx2];
      if (cv2 < MIN_RGB_VAL)
        cv2 = MIN_RGB_VAL;

      //If this side of the panel drops > 25% keep it at 25%
      if (cv1 < top_chan_l_lvl[1])
        cv1 = top_chan_l_lvl[1];

      if (cv2 < top_chan_r_lvl[1])
        cv2 = top_chan_r_lvl[1];

      top_chan_l_lvl[1] = (cv1 >> 1) + (cv1 >> 2);
      top_chan_r_lvl[1] = (cv2 >> 1) + (cv2 >> 2);

      r1 = (r1 * cv1) >> 8;
      g1 = (g1 * cv1) >> 8;
      b1 = (b1 * cv1) >> 8;

      r2 = (r2 * cv2) >> 8;
      g2 = (g2 * cv2) >> 8;
      b2 = (b2 * cv2) >> 8;
    }

    for (rw = 0; rw < FLOOD_ROW; rw ++)
    {
      idx = fcol[rw][cl];
      fled[idx].r = r1; //left channel, flood 1
      fled[idx].g = g1;
      fled[idx].b = b1;

      idx += FLOOD_STRING_MEM_LEN;
      fled[idx].r = r2; //right channel, flood 1
      fled[idx].g = g2;
      fled[idx].b = b2;
    }
  }
}
#endif


#define HIGH_BIAS     0x20
#define HIGH_PASS     0x10
#pragma CODE_SECTION(color_organ_prep, "ramCode")

//This function prepares the FFT output bins for the display routines.
#ifndef FLOODS
void color_organ_prep(float *fft_bin, uint16_t *chan_val)
#else
void color_organ_prep(uint16_t  side,
                         float *fft_bin,
                      uint16_t *chan_val,
                      uint16_t *top_chan,
                      uint16_t *hyst_sum)
#endif
{
  uint16_t idx, idx2, idx3, end;
  float    temp, max;
  float    maxlo, maxmid, maxhi;
  #ifdef FLOODS
  uint16_t imax;
  uint16_t chan_sort[COLOR_CHANNELS];
  #endif
  #ifdef OLD_STYLE_BULBS
  uint16_t prev_cv[COLOR_CHANNELS];
  #endif

  idx3  = 1;
  maxhi = 0;
  maxmid= 0;
  maxlo = 0;

  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    #ifdef OLD_STYLE_BULBS
    //Save off the last frame's value
    prev_cv[idx] = chan_val[idx];
    #endif

    max = 0;
    end = chan_bin[idx+1]; //this depends on one extra entry in the array

    for (idx2 = chan_bin[idx]; idx2 < end; idx2 ++)
    {
      //Square the data to make more difference between large and small values
      temp = (fft_bin[idx3] * fft_bin[idx3]);

      if (max < temp)
        max = temp;
      idx3 += 1;
    }

    chan_val[idx] = (uint16_t) max;

    if (idx < 3) //low freq channels
    {
      if (maxlo < max)
        maxlo = max;
    }
    else
    if (idx < 6) //mid freq channels
    {
      if (maxmid < max)
        maxmid = max;
    }
    else //high freq channels
    {
      if (maxhi < max)
        maxhi = max;
    }

    //Don't pass very small values through for bass channels.
    if ((idx <= 1) && (chan_val[idx] < HIGH_PASS))
      chan_val[idx] = 0;
  }

  //Since the highest channel registers lower with the same signal strength
  //give it a boost if it's not zero.
  if (chan_val[COLOR_CHANNELS-1] > 0)
    chan_val[COLOR_CHANNELS-1] += HIGH_BIAS;

  //Give half as much boost to the next-to-highest channel.
  if (chan_val[COLOR_CHANNELS-2] > 0)
    chan_val[COLOR_CHANNELS-2] += (HIGH_BIAS >> 1);

  //Scale the data to the maximum component RGB value.
  if (maxlo > (float)MAX_RGB_VAL)
  {
    temp = maxlo / (float)MAX_RGB_VAL;

    for (idx = 0; idx < 3; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxmid > (float)MAX_RGB_VAL)
  {
    temp = maxmid / (float)MAX_RGB_VAL;

    for (idx = 3; idx < 6; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxhi > (float)MAX_RGB_VAL)
  {
    temp = maxhi / (float)MAX_RGB_VAL;

    for (idx = 6; idx < COLOR_CHANNELS; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  #ifdef OLD_STYLE_BULBS
  //Incandescent bulbs cannot flash at 15 or 30Hz. Apply a fading factor to
  //slow the LEDs down a bit. We don't want to put people into a trance... ;^)
  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    //If the new value is less than half, set it to half instead.
    if (chan_val[idx] < (prev_cv[idx] >> 1))
      chan_val[idx] = prev_cv[idx] >> 1;
  }
  #endif

  #ifdef FLOODS
  //Update the hysteresis buffers
  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    if (side == 0) //left
    {
      //subtract the oldest entry from the sum
      hyst_sum[idx] -= hyst_l[idx][hyst_idx];

      //add the new entry to the sum and buffer
      hyst_sum[idx] += chan_val[idx];
      hyst_l[idx][hyst_idx] = chan_val[idx];
    }
    else //right
    {
      //subtract the oldest entry from the sum
      hyst_sum[idx] -= hyst_r[idx][hyst_idx];

      //add the new entry to the sum and buffer
      hyst_sum[idx] += chan_val[idx];
      hyst_r[idx][hyst_idx] = chan_val[idx];
    }
  }

  if (side == 1) //right, and now done with both sides so update the index
  {
    hyst_idx ++;
    if (hyst_idx == HYST_SIZE)
      hyst_idx = 0;

    chan_count_r = 0;
  }
  else
    chan_count_l = 0;


  //Sort the strength of the color channels using the hysteresis sums (sort the top two)

  for (idx = 0; idx < MAX_CHAN; idx++)
  {
    imax = 0; //will keep the max found
    idx3 = 0; //will keep the index

    for (idx2 = idx; idx2 < COLOR_CHANNELS; idx2++)
    {
      if (idx == 0)
        chan_sort[idx2] = hyst_sum[idx2];

      if (chan_sort[idx2] > imax)
      {
        imax = chan_sort[idx2];
        idx3 = idx2; //save the index
      }
    }

    //Now save the max's channel index and clear it from the list
    if (imax > 0)
    {
      if (side == 0)
        chan_count_l ++;
      else
        chan_count_r ++;
    }

    top_chan[idx]   = idx3;
    chan_sort[idx3] = 0; //clear the max from the list
  }
  #endif
}


#pragma CODE_SECTION(draw_char, "ramCode")

//Draw a character starting at lower-left x,y, and clip to the array.
void draw_char(int16_t x, int16_t y, int16_t idx, uint32_t color)
{
  int16_t   ix, iy, clm, tmp;
  uint16_t  bits;
  uint16_t r, g, b;
  font_t *fchr = (font_t *) &fontchar[idx];

  get_rgb(color, &r, &g, &b);
  clm = 0;

  for (ix = x; ix < x + fchr->wid; ix++)
  {
    bits = fchr->pix[clm / 2]; //grab the right 16 bits
    if (clm & 0x01)
      bits &= 0xff; //keep 2nd byte
    else
      bits >>= 8;   //keep 1st byte

    //remove extra zero bits (if any) at the bottom of each column
    bits >>= FONT_YOFF;

    for (iy = y; iy < y + FONT_HEIGHT; iy++)
    {
      if ((ix < MAX_COL) && (iy < MAX_ROW))
      {
        if (bits & 0x01)
        {
          tmp = col[iy][ix];
          led[tmp].r = r;
          led[tmp].g = g;
          led[tmp].b = b;
        }

        bits >>= 1;
      }
    }

    clm ++;
  }
}


#pragma CODE_SECTION(show_text, "ramCode")

// panel   = 0 .. 3
//dispmode = 0 = centered in one panel
//         = 1 = centered in total array
//         = 2 = scroll from right
void show_text(int16_t dispmode, int16_t panel, uint32_t color, int16_t delay, char *str)
{
  int16_t  x, y;
  uint16_t idx, idx2, idx3;
  uint16_t s_len, p_len;
  uint16_t word_idx;
  uint16_t curr_len;
  uint16_t curr_char;
  uint32_t disp_clr;
  uint16_t str_idx[FONT_MAX_CHAR];
  uint16_t char_wid[FONT_MAX_CHAR];
  uint16_t word_clr[5];

  disp_clr = color;

  s_len = font_str_len(str);          //get string length in ASCII characters
  if (s_len > FONT_MAX_CHAR)
  {
    s_len = FONT_MAX_CHAR;
    str[s_len] = 0; //insert a NULL to terminate
  }

  p_len = font_pix_len(str, str_idx); //get string length in pixels
  if (p_len > MAX_COL)
    dispmode = 2;

  if (dispmode < 2)
  {
    if (p_len > PANEL_COL) //won't fit in one panel
      dispmode = 1; //force total array mode

    if (dispmode == 0) //center in one panel
    {
      x = panel_off[panel][1] + (PANEL_COL/2) - (p_len/2);
      y = panel_off[panel][0] + (PANEL_ROW/2) - (FONT_HEIGHT/2);
    }
    else if (dispmode == 1) //center in total array
    {
      x = (MAX_COL / 2) - (p_len / 2);
      y = (MAX_ROW / 2) - (FONT_HEIGHT / 2);
    }

    if (x < 0) x = 0;
    if (color == 0xffffffff)
      disp_clr = rainbow[rnd(12)];

    for (idx = 0; idx < s_len; idx++)
    {
      if ((color == 0xffffffff) && (str_idx[idx] == FONT_SPACE_IDX))
        disp_clr = rainbow[rnd(12)];

      draw_char(x, y, str_idx[idx], disp_clr);
      x += fontchar[str_idx[idx]].wid + 1;
    }

    for (idx3 = 0; idx3 < delay; idx3 ++)
    {
      wait_for_sync(1);
      pause = 0;

      if (end_of_gap)
        return;
    }
  }
  else //scroll from right, blanking what was displayed in the prior loop
  {
    curr_len  = 0;
    curr_char = 0xffff;
    y = (MAX_ROW / 2) - (FONT_HEIGHT / 2);

    for (idx = 0; idx < 5; idx++)
      word_clr[idx] = rnd(12);

    for (idx = 0; idx < p_len; idx++)
    {
      if (curr_len < (idx + 1))
      {
        curr_char ++;
        curr_len += fontchar[str_idx[curr_char]].wid + 1;
        char_wid[curr_char] = curr_len;
      }

      for (idx2 = 0; idx2 <= curr_char; idx2++)
      {
        word_idx = 0;
        idx3 = MAX_COL - idx + char_wid[idx2];
        draw_char(idx3, y, str_idx[idx2], 0x000000); //black out the previous write

        if ((color == 0xffffffff) && (str_idx[idx] == FONT_SPACE_IDX))
          disp_clr = rainbow[word_clr[word_idx++]];
        draw_char(idx3-1, y, str_idx[idx2], disp_clr);
      }

      wait_for_sync(1);
      pause = 0;

      if (end_of_gap)
        return;
    }

    wait_for_sync(delay);
    pause = 0;
  }
}


#if 0
#pragma CODE_SECTION(test_rgb, "ramCode")
//test a string of LEDs
void test_rgb(void)
{
  uint16_t idx;

  while(1)
  {
    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0xc0;
      led[idx].g = 0;
      led[idx].b = 0;
    }
    wait_for_sync(30);


    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0xc0;
      led[idx].b = 0;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0;
      led[idx].b = 0xc0;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0x60;
      led[idx].g = 0x60;
      led[idx].b = 0;
    }
    wait_for_sync(30);


    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0x60;
      led[idx].b = 0x60;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0x60;
      led[idx].g = 0;
      led[idx].b = 0x60;
    }
    wait_for_sync(30);
  }
}
#endif


//This function performs power on (or reset) displays to verify that the LED
//panels are correctly operating prior to the color organ running.
#define INIT_DISPLAY_FRAMES               (FRAMES_PER_SEC + (FRAMES_PER_SEC >> 1))
#pragma CODE_SECTION(initial_display, "ramCode")

void initial_display(void)
{
  char str[2];

  memset(led, 0, sizeof(led));

  //Display the panel number in each panel to identify correct hookup.
  str[0] = '1'; str[1] = 0;
  show_text(0, 0, 0x0000a0, 3, str);
  str[0] = '2';
  show_text(0, 1, 0x0000a0, 3, str);
  str[0] = '3';
  show_text(0, 2, 0x0000a0, 3, str);
  str[0] = '4';
  show_text(0, 3, 0x0000a0, INIT_DISPLAY_FRAMES, str);
  clear_display();

  #ifndef SLAVE
  show_text(1, 0, 0x00f000, INIT_DISPLAY_FRAMES, "Let's Go!");
  #endif

  //Set the end of gap flag so that the first frame will be a peak.
  end_of_gap = 1;
}


#define MIN_DISPLAY_FRAMES                (FRAMES_PER_SEC * 5)
#pragma CODE_SECTION(do_display, "ramCode")

//This is the main display driver for the color organ.
void do_display(uint16_t peak_flag, uint16_t *chan_left, uint16_t *chan_right)
{
  #if defined(MASTER) || defined(SLAVE)
  uint16_t prev_display;
  #endif

  if (display == 9999) //force start-up to initialize a peak
  {
    peak_flag = 1;
  }

  //Choose a new pattern to display - on a peak boundary
  #ifndef SLAVE
  if (peak_flag & (display_frames > MIN_DISPLAY_FRAMES))
  {
    display_frames = 0;
    disp_frames = 99999;
    #ifdef MASTER
    prev_display = display;
    #endif

    display = rnd(100);
//if (display < 50)
//display = 20;
//else
//display = 95;

    switch (display)
    {
      case 0 ... 14:
        display_mode = 0; //line segments
        break;

      case 15 ... 32:
        display_mode = 1; //ripple
        break;

      case 33 ... 49:
        display_mode = 2; //curves
        break;

      case 50 ... 64:
        display_mode = 3; //vertical line segments
        break;

      case 65 ... 84:
        display_mode = 4; //lavalamp
        break;

      case 85 ... 89:
        display_mode = 5; //two_by_two (arrow)
        break;

      case 90 ... 94:
        display_mode = 6; //two_by_two (fade)
        break;

      case 95 ... 99:
        display_mode = 7; //two_by_two (random wave)
        break;
    };

    #ifdef MASTER
    if (display != prev_display)
    {
      //If running as master, set a new mode value for the slave
      set_mode(display_mode); //can only send values 0..7
    }
    #endif
  }
  #endif //not a slave

  #ifdef SLAVE
  //If running as slave, grab the master's current mode every frame
  prev_display = display_mode;
  get_mode(&display_mode);

  if (display_mode != prev_display)
  {
    peak_flag = 1;
    disp_frames = 99999;
  }
  #endif

  display_frames ++;

  switch (display_mode)
  {
    case 0:
      line_segment2(peak_flag, chan_left, chan_right);
      break;

    case 1:
      ripple(peak_flag, chan_left, chan_right);
      break;

    case 2:
      curve2(peak_flag, chan_left, chan_right);
      break;

    case 3:
      vertical(peak_flag, chan_left, chan_right);
      break;

    case 4:
      lavalamp(peak_flag, chan_left, chan_right);
      break;

    case 5:
      two_by_two(peak_flag, chan_left, chan_right, 0); //arrow
      break;

    case 6:
      two_by_two(peak_flag, chan_left, chan_right, 1); //fade
      break;

    case 7:
      two_by_two(peak_flag, chan_left, chan_right, 3); //random wave
      break;
  };

  #ifdef FLOODS
  floods(chan_left, chan_right);
  #endif
}
