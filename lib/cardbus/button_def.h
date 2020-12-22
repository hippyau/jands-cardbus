#pragma once


/*
Surface Button Definitions


Event 408

0-11  = preset card 1
12-24 = preset card 2
25-31 = assign card 1
32-51 = palette card 

52-91 = master card...


91-167 = program card


*/
                  // Event4 + Echelon 1K
#define TOTAL_BUTTONS   (168 + 144)


// Event 4xx
#if defined (CONFIG_EVENT_408)

#define BUTTONS_START  (0)
#define BUTTONS_END    (BUTTONS_START + 168)


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

#endif 


#if defined (CONFIG_ECHELON_1K)

#define BUTTONS_START  (92)
#define BUTTONS_END    (BUTTONS_START + 144)

// Echelon 1K Program Card

#define BTN_TRYCUE      92 // S1 led 
#define BTN_EFFECT      93 // S2 led 1 green
#define BTN_HIGHLIGHT   94 // S3 led
#define BTN_MONITOR     95 // S4 led 0 red
#define BTN_BLIND       96 // S5 led
#define BTN_BEGIN       97 // S6
#define BTN_CLEARRESTORE 98 // S7 led
#define BTN_PIG_RIGHT    99 // S8

#define BTN_COPY        100 // S9
#define BTN_ACTIVE      101 // S10 led 3
#define BTN_MOVE        102 // S11
#define BTN_LOAD        103 // S12
#define BTN_DELETE      104 // S13
#define BTN_UPDATE      105 // S14 led 2 0xFD 0x04
#define BTN_UNDO        106 // S15
#define BTN_RECORD      107 // S16 

#define BTN_BLANK1      108 // S17
#define BTN_MACRO       109 // S18
#define BTN_TIME        110 // S19
#define BTN_PAGE        111 // S20
#define BTN_ARROW_ARROW_LEFT 112 // S21
#define BTN_CUE         113 // S22
#define BTN_SETUP       114 // S23
#define BTN_BLANK2      115 // S24
#define BTN_GROUP       116 // S25 led
#define BTN_BACKSPACE   117 // S26
#define BTN_POSITION    118 // S27 led
#define BTN_BACKSLASH   119 // S28
#define BTN_COLOUR      120 // S29 led
#define BTN_MINUS       121 // S30
#define BTN_BEAM        122 // S31 led
#define BTN_PLUS        123 // S32
#define BTN_7           124 // S33
#define BTN_4           125 // S34
#define BTN_8           126 // S35
#define BTN_5           127 // S36
#define BTN_9           128 // S37
#define BTN_6           129 // S38
#define BTN_THRU        130 // S39
#define BTN_FULL        131 // S40
#define BTN_1           132 // S41
#define BTN_0           133 // S42
#define BTN_2           134 // S43
#define BTN_DOT         135 // S44
#define BTN_3           136 // S45
#define BTN_ENTER       137 // S46  S77

#define BTN_L_BLANK     139 // S48

#define BTN_LEFT_ARROW  140 // S49
#define BTN_LEFT_UP     141 // S50
#define BTN_DOWN_ARROW  142 // S51
#define BTN_RIGHT_ARROW 143 // S52

#define BTN_END         144 // S53
#define BTN_UP_UP_ARROW 145 // S54
#define BTN_DN_DN_ARROW 146 // S55
#define BTN_SET         147 // S56

#define BTN_L_RELEASE  148 // S57
#define BTN_L_GO_FWD   149 // S58
#define BTN_L_GO_BACK  150 // S59
#define BTN_L_GOTO     151 // S60

#define BTN_NEXT       152 // S61
#define BTN_L_PIG      153 // S62

#define BTN_MENUS      153 // S63 led

#define BTN_LIST       155 // S64
#define BTN_TOPSOFT1   156 // S65
#define BTN_TOPSOFT2   157 // S66
#define BTN_TOPSOFT3   158 // S67
#define BTN_TOPSOFT4   159 // S68
#define BTN_TOPSOFT5   160 // S69

#define BTN_BOTSOFT1   161 // S70
#define BTN_BOTSOFT2   162 // S71
#define BTN_BOTSOFT3   163 // S72
// S73 ?
#define BTN_ARROW_ARROW_RIGHT   165 // S74
#define BTN_BOTSOFT4   166 // S75
#define BTN_BOTSOFT5   167 // S76

#endif 