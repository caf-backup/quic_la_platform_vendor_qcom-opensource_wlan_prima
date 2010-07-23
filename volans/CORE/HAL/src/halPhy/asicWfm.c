/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicWfm.cc: Encapsulates functionality to setup, start and stop test waveforms
   Author:  Mark Nelson
   Date:    2/15/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include <ani_assert.h>
#include <sys_api.h>

#ifndef VERIFY_HALPHY_SIMV_MODEL
const tWaveformSample pWave[1024] = {

    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , 199},
    {945 , 391},
    {850 , 568},
    {723 , 723},
    {568 , 850},
    {391 , 945},
    {199 , 1003},
    {0 , 1023},
    {-199 , 1003},
    {-391 , 945},
    {-568 , 850},
    {-723 , 723},
    {-850 , 568},
    {-945 , 391},
    {-1003 , 199},
    {-1023 , 0},
    {-1003 , -199},
    {-945 , -391},
    {-850 , -568},
    {-723 , -723},
    {-568 , -850},
    {-391 , -945},
    {-199 , -1003},
    {0 , -1023},
    {199 , -1003},
    {391 , -945},
    {568 , -850},
    {723 , -723},
    {850 , -568},
    {945 , -391},
    {1003 , -199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {1023 , 0},
    {1003 , -199},
    {945 , -391},
    {850 , -568},
    {723 , -723},
    {568 , -850},
    {391 , -945},
    {199 , -1003},
    {0 , -1023},
    {-199 , -1003},
    {-391 , -945},
    {-568 , -850},
    {-723 , -723},
    {-850 , -568},
    {-945 , -391},
    {-1003 , -199},
    {-1023 , 0},
    {-1003 , 199},
    {-945 , 391},
    {-850 , 568},
    {-723 , 723},
    {-568 , 850},
    {-391 , 945},
    {-199 , 1003},
    {0 , 1023},
    {199 , 1003},
    {391 , 945},
    {568 , 850},
    {723 , 723},
    {850 , 568},
    {945 , 391},
    {1003 , 199},
    {0 , 0},
    {125 , -141},
    {185 , -238},
    {186 , -271},
    {143 , -239},
    {80 , -154},
    {19 , -44},
    {-21 , 58},
    {-36 , 120},
    {-27 , 115},
    {-5 , 31},
    {13 , -125},
    {16 , -332},
    {-6 , -553},
    {-55 , -746},
    {-118 , -874},
    {-180 , -906},
    {-219 , -832},
    {-218 , -660},
    {-166 , -415},
    {-63 , -134},
    {76 , 138},
    {231 , 364},
    {372 , 515},
    {474 , 578},
    {516 , 556},
    {492 , 468},
    {408 , 343},
    {286 , 212},
    {154 , 100},
    {47 , 26},
    {-9 , -4},
    {0 , 0},
    {76 , 26},
    {201 , 55},
    {347 , 73},
    {475 , 70},
    {548 , 47},
    {539 , 13},
    {436 , -16},
    {245 , -24},
    {-11 , 1},
    {-293 , 66},
    {-559 , 162},
    {-767 , 274},
    {-886 , 379},
    {-900 , 453},
    {-815 , 475},
    {-651 , 435},
    {-441 , 335},
    {-223 , 192},
    {-32 , 31},
    {105 , -116},
    {178 , -223},
    {190 , -269},
    {154 , -250},
    {93 , -174},
    {30 , -66},
    {-15 , 40},
    {-35 , 113},
    {-30 , 122},
    {-10 , 54},
    {10 , -88},
    {17 , -288},
    {0 , -510},
    {-43 , -712},
    {-105 , -855},
    {-169 , -908},
    {-214 , -855},
    {-222 , -701},
    {-180 , -468},
    {-87 , -191},
    {46 , 86},
    {200 , 324},
    {346 , 492},
    {458 , 572},
    {513 , 566},
    {502 , 490},
    {429 , 370},
    {312 , 237},
    {180 , 120},
    {65 , 38},
    {-3 , -1},
    {-7 , -3},
    {56 , 20},
    {173 , 50},
    {318 , 71},
    {452 , 72},
    {539 , 53},
    {548 , 20},
    {464 , -11},
    {289 , -24},
    {43 , -6},
    {-237 , 50},
    {-510 , 141},
    {-732 , 251},
    {-870 , 360},
    {-906 , 442},
    {-839 , 475},
    {-689 , 448},
    {-485 , 359},
    {-265 , 223},
    {-67 , 63},
    {82 , -89},
    {169 , -206},
    {192 , -266},
    {164 , -259},
    {106 , -193},
    {42 , -89},
    {-8 , 20},
    {-34 , 102},
    {-33 , 126},
    {-14 , 74},
    {7 , -54},
    {18 , -245},
    {5 , -465},
    {-33 , -675},
    {-92 , -833},
    {-157 , -906},
    {-207 , -874},
    {-224 , -740},
    {-193 , -519},
    {-110 , -248},
    {17 , 32},
    {169 , 282},
    {319 , 465},
    {440 , 563},
    {507 , 574},
    {510 , 510},
    {448 , 396},
    {337 , 263},
    {206 , 141},
    {85 , 51},
    {5 , 2},
    {-12 , -5},
    {38 , 14},
    {147 , 44},
    {289 , 68},
    {428 , 74},
    {527 , 58},
    {554 , 27},
    {489 , -6},
    {331 , -24},
    {96 , -13},
    {-180 , 35},
    {-458 , 120},
    {-693 , 229},
    {-850 , 340},
    {-907 , 429},
    {-860 , 473},
    {-724 , 458},
    {-528 , 381},
    {-309 , 253},
    {-103 , 96},
    {57 , -60},
    {157 , -187},
    {192 , -259},
    {172 , -265},
    {119 , -210},
    {54 , -111},
    {0 , 0},
    {-31 , 90},
    {-35 , 127},
    {-19 , 91},
    {3 , -23},
    {17 , -203},
    {10 , -421},
    {-23 , -636},
    {-79 , -807},
    {-144 , -899},
    {-199 , -889},
    {-224 , -774},
    {-203 , -569},
    {-130 , -304},
    {-11 , -22},
    {137 , 236},
    {290 , 435},
    {419 , 551},
    {499 , 578},
    {514 , 527},
    {465 , 421},
    {362 , 290},
    {232 , 164},
    {107 , 66},
    {17 , 9},
    {-14 , -6},
    {23 , 8},
    {121 , 38},
    {259 , 65},
    {402 , 74},
    {512 , 63},
    {555 , 34},
    {510 , 0},
    {369 , -22},
    {148 , -18},
    {-123 , 22},
    {-404 , 101},
    {-652 , 206},
    {-827 , 319},
    {-904 , 414},
    {-878 , 469},
    {-757 , 466},
    {-570 , 402},
    {-352 , 282},
    {-141 , 128},
    {30 , -30},
    {142 , -165},
    {190 , -250},
    {180 , -269},
    {131 , -225},
    {67 , -133},
    {9 , -21},
    {-27 , 75},
    {-36 , 125},
    {-23 , 105},
    {0 , 5},
    {16 , -163},
    {13 , -376},
    {-14 , -595},
    {-67 , -778},
    {-131 , -888},
    {-190 , -900},
    {-222 , -805},
    {-212 , -616},
    {-149 , -360},
    {-38 , -78},
    {106 , 188},
    {261 , 401},
    {397 , 535},
    {488 , 580},
    {517 , 543},
    {479 , 445},
    {386 , 317},
    {259 , 187},
    {130 , 82},
    {31 , 17},
    {-13 , -6},
    {10 , 4},
    {97 , 32},
    {230 , 60},
    {375 , 74},
    {495 , 67},
    {553 , 40},
    {526 , 6},
    {404 , -19},
    {197 , -21},
    {-66 , 11},
    {-349 , 83},
    {-607 , 184},
    {-799 , 297},
    {-897 , 397},
    {-891 , 462},
    {-788 , 472},
    {-611 , 419},
    {-397 , 309},
    {-181 , 160},
    {1022 , 0},
    {961 , 343},
    {786 , 645},
    {519 , 867},
    {195 , 983},
    {-145 , 980},
    {-461 , 862},
    {-712 , 646},
    {-872 , 361},
    {-922 , 45},
    {-862 , -261},
    {-704 , -522},
    {-472 , -706},
    {-199 , -796},
    {77 , -786},
    {323 , -684},
    {511 , -511},
    {620 , -293},
    {645 , -63},
    {590 , 147},
    {472 , 315},
    {312 , 422},
    {139 , 461},
    {-21 , 436},
    {-149 , 361},
    {-231 , 255},
    {-261 , 139},
    {-245 , 36},
    {-195 , -38},
    {-128 , -77},
    {-63 , -77},
    {-16 , -47},
    {0 , 0},
    {-16 , 47},
    {-63 , 77},
    {-128 , 77},
    {-195 , 38},
    {-245 , -36},
    {-261 , -139},
    {-231 , -255},
    {-149 , -361},
    {-21 , -436},
    {139 , -461},
    {312 , -422},
    {472 , -315},
    {590 , -147},
    {645 , 63},
    {620 , 293},
    {511 , 511},
    {323 , 684},
    {77 , 786},
    {-199 , 796},
    {-472 , 706},
    {-704 , 522},
    {-862 , 261},
    {-922 , -45},
    {-872 , -361},
    {-712 , -646},
    {-461 , -862},
    {-145 , -980},
    {195 , -983},
    {519 , -867},
    {786 , -645},
    {961 , -343},
    {1022 , 0},
    {961 , 343},
    {786 , 645},
    {519 , 867},
    {195 , 983},
    {-145 , 980},
    {-461 , 862},
    {-712 , 646},
    {-872 , 361},
    {-922 , 45},
    {-862 , -261},
    {-704 , -522},
    {-472 , -706},
    {-199 , -796},
    {77 , -786},
    {323 , -684},
    {511 , -511},
    {620 , -293},
    {645 , -63},
    {590 , 147},
    {472 , 315},
    {312 , 422},
    {139 , 461},
    {-21 , 436},
    {-149 , 361},
    {-231 , 255},
    {-261 , 139},
    {-245 , 36},
    {-195 , -38},
    {-128 , -77},
    {-63 , -77},
    {-16 , -47},
    {0 , 0},
    {-16 , 47},
    {-63 , 77},
    {-128 , 77},
    {-195 , 38},
    {-245 , -36},
    {-261 , -139},
    {-231 , -255},
    {-149 , -361},
    {-21 , -436},
    {139 , -461},
    {312 , -422},
    {472 , -315},
    {590 , -147},
    {645 , 63},
    {620 , 293},
    {511 , 511},
    {323 , 684},
    {77 , 786},
    {-199 , 796},
    {-472 , 706},
    {-704 , 522},
    {-862 , 261},
    {-922 , -45},
    {-872 , -361},
    {-712 , -646},
    {-461 , -862},
    {-145 , -980},
    {195 , -983},
    {519 , -867},
    {786 , -645},
    {961 , -343},
    {1022 , 0},
    {961 , 343},
    {786 , 645},
    {519 , 867},
    {195 , 983},
    {-145 , 980},
    {-461 , 862},
    {-712 , 646},
    {-872 , 361},
    {-922 , 45},
    {-862 , -261},
    {-704 , -522},
    {-472 , -706},
    {-199 , -796},
    {77 , -786},
    {323 , -684},
    {511 , -511},
    {620 , -293},
    {645 , -63},
    {590 , 147},
    {472 , 315},
    {312 , 422},
    {139 , 461},
    {-21 , 436},
    {-149 , 361},
    {-231 , 255},
    {-261 , 139},
    {-245 , 36},
    {-195 , -38},
    {-128 , -77},
    {-63 , -77},
    {-16 , -47},
    {0 , 0},
    {-16 , 47},
    {-63 , 77},
    {-128 , 77},
    {-195 , 38},
    {-245 , -36},
    {-261 , -139},
    {-231 , -255},
    {-149 , -361},
    {-21 , -436},
    {139 , -461},
    {312 , -422},
    {472 , -315},
    {590 , -147},
    {645 , 63},
    {620 , 293},
    {511 , 511},
    {323 , 684},
    {77 , 786},
    {-199 , 796},
    {-472 , 706},
    {-704 , 522},
    {-862 , 261},
    {-922 , -45},
    {-872 , -361},
    {-712 , -646},
    {-461 , -862},
    {-145 , -980},
    {195 , -983},
    {519 , -867},
    {786 , -645},
    {961 , -343},
    {1022 , 0},
    {961 , 343},
    {786 , 645},
    {519 , 867},
    {195 , 983},
    {-145 , 980},
    {-461 , 862},
    {-712 , 646},
    {-872 , 361},
    {-922 , 45},
    {-862 , -261},
    {-704 , -522},
    {-472 , -706},
    {-199 , -796},
    {77 , -786},
    {323 , -684},
    {511 , -511},
    {620 , -293},
    {645 , -63},
    {590 , 147},
    {472 , 315},
    {312 , 422},
    {139 , 461},
    {-21 , 436},
    {-149 , 361},
    {-231 , 255},
    {-261 , 139},
    {-245 , 36},
    {-195 , -38},
    {-128 , -77},
    {-63 , -77},
    {-16 , -47},
    {0 , 0},
    {-16 , 47},
    {-63 , 77},
    {-128 , 77},
    {-195 , 38},
    {-245 , -36},
    {-261 , -139},
    {-231 , -255},
    {-149 , -361},
    {-21 , -436},
    {139 , -461},
    {312 , -422},
    {472 , -315},
    {590 , -147},
    {645 , 63},
    {620 , 293},
    {511 , 511},
    {323 , 684},
    {77 , 786},
    {-199 , 796},
    {-472 , 706},
    {-704 , 522},
    {-862 , 261},
    {-922 , -45},
    {-872 , -361},
    {-712 , -646},
    {-461 , -862},
    {-145 , -980},
    {195 , -983},
    {519 , -867},
    {786 , -645},
    {961 , -343},
};
#endif


#ifdef ANI_MANF_DIAG
eHalStatus asicTxFirSetChainBypass(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_BOOLEAN chainBypassEnable)
{
    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_MASK, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_OFFSET, (tANI_U32)chainBypassEnable);
            return (eHAL_STATUS_SUCCESS);
        default:
            //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
            assert(0);
            return (eHAL_STATUS_FAILURE);;
    }
}

eHalStatus asicTxFirSetPaOverride(tpAniSirGlobal pMac, tANI_BOOLEAN overrideEnable, ePhyTxChains chainPaEnables)
{
    if (overrideEnable == eANI_BOOLEAN_TRUE)
    {
        switch (chainPaEnables)
        {
            //Modify the pa_override and pa_override_value fields together
            case PHY_TX_CHAIN_0:
                rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG,
                                 (QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK | QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK),
                                 QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET,
                                 3
                                );
                return (eHAL_STATUS_SUCCESS);
            case PHY_NO_TX_CHAINS:
                rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG,
                                 (QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK | QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK),
                                 QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET,
                                 1
                                );
                return (eHAL_STATUS_SUCCESS);
            default:
                //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
                assert(0);
            return (eHAL_STATUS_FAILURE);;
        }
    }
    else
    {
        rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK, QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET, 0);
        return (eHAL_STATUS_SUCCESS);
    }
}
#endif

/*
//load the raw memory for the time being. loading the I and Q samples seem to be a problem.
eHalStatus asicSetupTestWaveform(tpAniSirGlobal pMac, const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80)
{
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 1);
    {
        tANI_U32 i = 0;
        for(i = 0; i < sizeof(wfmNewMem)/4; i++)
        {
            halWriteRegister(pMac, (QWLAN_PHYDBG_DBGMEM_MREG + (4*i)), wfmNewMem[i]);
        }
    }
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 0);

    return eHAL_STATUS_SUCCESS;
}
*/
eHalStatus asicSetupTestWaveform(tpAniSirGlobal pMac, const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    pMac->hphy.wfm_clk80 = clk80;                   //records what clock sample rate 1 = 80, 0 = 20 MHz


    hv_printLog("%s: Entering\n", __FUNCTION__);
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 1);

    //SET_PHY_REG(pMac, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK);

    {
        tANI_U16 sample = 0;
        tANI_U32 Samples[MAX_TEST_WAVEFORM_SAMPLES * 2];

        //lower 16 bits goes into even numbered U32 words, and the higher 16 bits goes into the odd numbered U32 words
        for (sample = 0; (sample < numSamples); sample++)
        {
            Samples[sample * 2] = ((tANI_U32)pWave[sample].I & 0x7FF) | (((tANI_U32)pWave[sample].Q & 0x1F) << 11);  //11 bits for I, 5 LSBs of Q
            Samples[(sample * 2) + 1] = (((tANI_U32)pWave[sample].Q & 0x7FF) >> 5);
            Samples[(sample * 2) + 1] = SignExtend(Samples[(sample * 2) + 1], 6) & 0xffff;

            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + (sample * 8), Samples[sample * 2]);
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + ((sample * 8) + 4), Samples[(sample * 2) + 1]);
            //phyLog(LOGE, "I=%d        Q=%d\n", pWave[sample].I, pWave[sample].Q);
        }

        //SET_PHY_MEMORY(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG, Samples, numSamples * 2);
    }
    hv_printLog("%s: Exiting\n", __FUNCTION__);

    return (retVal);
}
#ifdef ANI_MANF_DIAG
#ifndef VERIFY_HALPHY_SIMV_MODEL

tANI_BOOLEAN playing_wfm = eANI_BOOLEAN_FALSE;
static tANI_U32 bkup_dac_cntl;
static tANI_U32 bkup_fir_mode;

eHalStatus asicStartTestWaveform(tpAniSirGlobal pMac, eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    /* Stop previous waveform generation */
    if(playing_wfm)
        asicStopTestWaveform(pMac);

    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET,
                        1);
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET,
                        1);

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 1);

    /* Disable clock-gating */
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET,
                            1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET,
                            1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET,
                            1);

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG, &bkup_fir_mode);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG, QWLAN_TXCTL_FIR_MODE_DIS_11MBPS_MASK |
                                            QWLAN_TXCTL_FIR_MODE_DIS_5MBPS_MASK |
                                            (QWLAN_TXCTL_FIR_MODE_ANT_EN_EZERO << QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET) |
                                            QWLAN_TXCTL_FIR_MODE_SELECT_FIRMODE_MASK);

    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_MASK,
                                QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_OFFSET, 1);

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, &bkup_dac_cntl);
    /* Stop overriding DAC chains */
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG,
                            QWLAN_TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK |
                            QWLAN_TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK |
                            QWLAN_TXCTL_DAC_CONTROL_TXEN0_OVERRIDE_VAL_MASK);

    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK,
                    QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK,
                    QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_OFFSET, 1);

    //asicTxFirSetPaOverride(eANI_BOOLEAN_TRUE, PHY_TX_CHAIN_0);

    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET,
                            0);

    // set conf to to use debug SRAM and put in continous mode unconditionally.
    // FIXME: The configuration needs to be changed to play the waveform from phyDBG rather than SRAM
    if (playback == WAVE_CONTINUOUS)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,
                       QWLAN_PHYDBG_CFGMODE_CONT1_MASK | QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK);
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK);
    }

    // set up start and stop addresses
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START_ADDR1_REG, startIndex);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR1_REG, endIndex);

    //FIXME: Needs to have elegant way of selection. No means right now
    if (1/*useDbgMem*/)
    {
        rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG,
                                QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK,
                                QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_OFFSET,
                                1);
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_TXF80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK |
                        QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK);
    }
    else
    {
        //This else condition is never executed !!!
        rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG,
                                QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK,
                                QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_OFFSET,
                                0);
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_DAC80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXAIF_DBGSEL_MASK);
    }

    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG2_REG, QWLAN_PHYDBG_PLYBCK_CFG2_ADDR_INC_MASK,
                                QWLAN_PHYDBG_PLYBCK_CFG2_ADDR_INC_OFFSET, 0);

    //Gen start pluse to start waveform generation to TXFIR
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START1_REG, QWLAN_PHYDBG_START1_START_MASK);

    playing_wfm = eANI_BOOLEAN_TRUE;
    sirBusyWait(200000);

    return (retVal);
}

#if 0
eHalStatus asicStartTestWaveform(tpAniSirGlobal pMac, eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //first stop any previous waveform gen operation
    asicStopTestWaveform(pMac);
    {
        // Explicitely enable clcok to PHYDBG module
        rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);

        // disable clock-gating for TXFIR and TXCTL
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 1);
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 1);

        // Experimenting with clocks to get PA Override to work
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                             QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
        //                             QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_FFT_MASK,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_FFT_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TDC_MASK,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TDC_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
        //                     QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_OFFSET, 1);
        //end experiment


        // spatial rotation will alter the waveform and should be off
        //SET_PHY_REG(pMac->hHdd, TXFIR_SPATIAL_ROTATION_REG, 0);

        asicTxFirSetPaOverride(pMac, eANI_BOOLEAN_TRUE, PHY_TX_CHAIN_0);

        // need explanation, enable clock to TXCTL (according to python script)
        // not critical, still functional without it.
        // BW 062008: Sometimes the TXCTL clock gating needs to be temporarily disabled to do certain things.  I know that’s vague, but that’s the best answer I can give right now without having to investigate why.

        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0);

        // set conf to to use debug SRAM and put in continous mode unconditionally.
        if (playback == WAVE_CONTINUOUS)
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK | QWLAN_PHYDBG_CFGMODE_CONT1_MASK);
        }
        else
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK);
        }

        // set up start and stop addresses
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START_ADDR1_REG, startIndex);
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR1_REG, endIndex);

        // configure PHY CONF Reg and Chain Bypass according to 80MHz (bypass TxFir) or 20MHz
        // routed through TxFir, to DAC.
        if (pMac->hphy.wfm_clk80 == eANI_BOOLEAN_ON)
        {
            //set chain0, 1, 2, 3 bypass bits again since it was reset by asicStopTestWaveform previously.
            //bypass all chains and let the DAC_CONTROL manages which chain actually are transmitted
            asicTxFirSetChainBypass(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON);
            //80 MHz sampling rate, samples input to TXFIR. dup_ch0 is not set in this case. Also, it is not
            //set in other parts of the code.
            // bandwidth_mode = 80MHz, dup_ch0 = 1, mif_txtest=0, taif_dbsel = 0, txpb_mode = 0x1, txfir_dbgsel = 1
            // i.e. chain 0 ==> 0x0206);
            // txpb_mode field is critical.....
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_EPLYBK_80M << QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_OFFSET) |
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_TXF80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK |
                        QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK);
        }
        else
        {
            // 20 MHz sampling rate, samples input to TXFIR
            // bandwidth_mode = 20MHz, dup_ch0 = 1, mif_txtest=0, taif_dbsel = 0, txpb_mode = 0x1, txfir_dbgsel = 1
            // i.e. chain 0 ==> 0x8206);
            // txpb_mode field is critical.....
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_EPLYBK_20M << QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_OFFSET) |
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_TXF80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK |
                        QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK);
        };

        //Gen start pluse to start waveform generation to TXFIR
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START1_REG, QWLAN_PHYDBG_START1_START_MASK);
    }

    // remaining setting done by upper layer after return of this function call.
    // (a) turn on DAC
    // SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, determined by upper function);
    // (b) config TxTCL for FIR Mode operation
    // SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, determined by upper function);

    return (retVal);
}
#endif

#if 0
eHalStatus asicStopTestWaveform(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    rdModWrAsicField(pMac, QWLAN_MIF_MIF_MEM_CFG_REG,
                     QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                     QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                     0    //set back to normal for host access
                    );

    /* Disable playback */
    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_MASK,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET,
                            0); //TX playback disabled
    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_OFFSET,
                            1); //When high, txfir will select phydbg data
    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                            QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK,
                            QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_OFFSET,
                            1); //When high, txfir will select phydbg data

    /* Halt running playback */
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 1);

    /* Disable clock-gating */
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET,
                            1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET,
                            1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET,
                            1);

    /* Stop overriding DAC chains */
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG,
                            QWLAN_TXCTL_DAC_CONTROL_CH3STDBY_OVERRIDE_VAL_MASK |
                            QWLAN_TXCTL_DAC_CONTROL_CH2STDBY_OVERRIDE_VAL_MASK |
                            QWLAN_TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_MASK |
                            QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_DPD_BYPASS_MASK |
                    QWLAN_TXFIR_CFG_LOLEAKAGE_BYPASS_MASK | QWLAN_TXFIR_CFG_IQIMB_BYPASS_MASK);

    asicTxFirSetPaOverride(pMac, eANI_BOOLEAN_FALSE, PHY_TX_CHAIN_0);

    rdModWrAsicField(pMac, QWLAN_TXCTL_FIR_MODE_REG, QWLAN_TXCTL_FIR_MODE_ANT_EN_MASK,
                            QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET, QWLAN_TXCTL_FIR_MODE_ANT_EN_EZERO);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0);

    return (retVal);
}
#endif

#define STOP_ITER_LIMIT 10000

eHalStatus asicStopTestWaveform(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_MASK,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET,
                            0); //TX playback disabled
    rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXAIF_DBGSEL_MASK,
                            QWLAN_PHYDBG_PLYBCK_CFG_TXAIF_DBGSEL_OFFSET,
                            0); //TX playback disabled

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 1);

    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET,
                            1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET,
                            1);
    //self._csr.txctl.dac_control(txen3_override_val = "rf_off",
    //                 txen2_override_val = "txen_off", txen1_override_val = "rf_off",
    //                 txen0_override_val = "txen_off", ch3stdby_override_val = "dac_off",
    //                 ch2stdby_override_val = "dac_off", ch1stdby_override_val = "dac_off",
    //                 ch0stdby_override_val = "dac_off")
    //self._csr.txctl.dac_control(txen_override_en="normal", dac_override_en="normal")
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, bkup_dac_cntl);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG, bkup_fir_mode);

#if 0
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET,
                        0);
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK,
                        QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET,
                        0);
#endif

    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_MASK,
                                QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_OFFSET, 0);
    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK,
                    QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET, 0);
    rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK,
                    QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_OFFSET, 0);

    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET,
                            0);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET,
                            0);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                            QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET,
                            0);

    //SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_STOP1_MASK);

    playing_wfm = eANI_BOOLEAN_FALSE;

    return retVal;
}
#endif
#endif
