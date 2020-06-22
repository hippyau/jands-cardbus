#pragma once

/*
Surface Button Definitions - based on a Event 408

0-11  = preset card 1
12-24 = preset card 2
25-31 = assign card 1
32-51 = palette card 

52-91 = master card...
*/

#define TOTAL_BUTTONS   92


// Master Card Buttons
#define BTN_EXIT        76
#define BTN_RECORD      78

// Numbers and their shifted functions
#define BTN_0           77
#define BTN_Setup       BTN_0

#define BTN_1           79
#define BTN_Copy        BTN_1

#define BTN_2           80
#define BTN_Clear       BTN_2

#define BTN_3           81
#define BTN_Stack       BTN_3

#define BTN_4           82
#define BTN_Page        BTN_4

#define BTN_5           83
#define BTN_Assign      BTN_5

#define BTN_6           68
#define BTN_File        BTN_6

#define BTN_7           69
#define BTN_Mem         BTN_7

#define BTN_8           70    
#define BTN_Edit        BNT_8

#define BTN_9           71
#define BTN_Chase       BTN_9

#define BTN_LEFT        88
#define BTN_RIGHT       89
#define BTN_MINUS       90
#define BTN_PLUS        91
#define BTN_RELEASE     75
#define BTN_HALT        74
#define BTN_GO          73
#define BTN_FADER       72

// above master faders
#define BTN_HOLD        85
#define BTN_FIXT        84
#define BTN_FLASH       86
#define BTN_DBO         87

// Programmer buttons
#define BTN_CLEAR       58
#define BTN_SCROLLER    57
#define BTN_FIXTURE     56 
#define BTN_POSITION    55
#define BTN_COLOUR      54
#define BTN_BEAM        53
#define BTN_GROUP       52
#define BTN_SHIFT       59  // modifier
#define BTN_SOFT1       60
#define BTN_SOFT2       61
#define BTN_SOFT3       62
#define BTN_SOFT4       63
#define BTN_SOFT5       64


