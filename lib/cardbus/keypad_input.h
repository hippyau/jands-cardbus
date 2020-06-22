#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "surface_buttons.h"
#include "lcd_module.h"

#include "limits.h"


class keypad_input
{
private:
    uint8_t edit_mode;  // 0 = input only, 1 = edit;    
    unsigned int * value; // the value we are editing
    unsigned int max_value;
    unsigned int orig_value; // original value to revert to on exit.
    unsigned int edval; // local copy for editing    
    bool done;
    char * name;
    uint8_t pos; // position in the string we are editing
    char display_value[16];
    uint8_t max_digits;

    lcdModule * lcd;
    uint8_t dispx, dispy; // where on the screen to draw

public:
    void updateDisplay();

    void key(uint8_t pressed); // keys fed in from keyboard
    
    void input(lcdModule * _lcd, char * strName, uint8_t x, uint8_t y);
    void edit(lcdModule * _lcd, char * strName, unsigned int * ptrValue, unsigned int _max_value, uint8_t x, uint8_t y);

    bool rollOver; // if true, value wraps to zero once max_value is reached

    bool check() { return done; }; // return true when editing is finished, currentValue() returns result.
    unsigned int currentValue() { return edval; };
};


// setup for editing a number from the keypad - enter numbers or +/- 
// press record to finish or exit to cancel.

// _lcd - pointer to the LCD display to use
// strName - char * like "Channel"
// ptrValue is a pointer to the value to edit
// _max_value is the max allowed value
// x, y - position on LCD
void keypad_input::edit(lcdModule * _lcd, char * strName, unsigned int * ptrValue, unsigned int _max_value, uint8_t x, uint8_t y)
{
     edit_mode = 1;
     value = ptrValue; 
     edval = *value;  
     orig_value = edval;
     done = false;
     max_value = _max_value;   

     if (max_value > 999) {
             max_digits = 4;
     } else if (max_value > 99) {
             max_digits = 3;
     } else if (max_value > 9) {
             max_digits = 2;
     } else max_digits = 1;

     name = strName;     
     lcd = _lcd;    
     pos = 0;

     for (uint8_t c = 0 ; c < 16 ; c++) display_value[c] = 0; 
     if (max_digits <= 3) {sprintf(display_value,"%03d",(unsigned int)value); }         
     else if (max_digits == 4)  { sprintf(display_value,"%04d",(unsigned int)value); }
     
     // display
     dispx = x;
     dispy = y;
     
     updateDisplay();
  
};

// setup for inputting a number from the keypad
// _lcd - pointer to the LCD display to use
// strName - char * like "Channel"
// x, y - position on LCD
void keypad_input::input(lcdModule * _lcd, char * strName, uint8_t x, uint8_t y)
{
     edit_mode = 0;
     value = 0; 
     edval = 0;  
     done = false;
     name = strName;     
     lcd = _lcd;    
     pos = 0;
     max_value = UINT_MAX;
     max_digits = 15;
     for (uint8_t c = 0 ; c < 16 ; c++) display_value[c] = 0; 

     // display
     dispx = x;
     dispy = y;
        
     updateDisplay();  
};



void keypad_input::updateDisplay(){
   lcd->setCursor(dispx,dispy);      
   lcd->print("                                        ");
   lcd->setCursor(dispx,dispy);      

   if (edit_mode){
    if (max_digits <= 3) lcd->printf("%s: %03d ",name,edval);
    else if (max_digits == 4) lcd->printf("%s: %04d ",name,edval);
   } else   
   lcd->printf("%s: %s",name,display_value);

}


// called when a key is pressed
void keypad_input::key(uint8_t pressed){
    uint8_t key = 255;

    switch (pressed)
    {
        case BTN_0:
                key = 0; break;
        case BTN_1:
                key = 1; break;
        case BTN_2:
                key = 2; break;
        case BTN_3:
                key = 3; break;
        case BTN_4:
                key = 4; break;
        case BTN_5:
                key = 5; break;
        case BTN_6:
                key = 6; break;
        case BTN_7:
                key = 7; break;
        case BTN_8:
                key = 8; break;
        case BTN_9:
                key = 9; break;      
        default:
            key = 255; //ignore    
            break;
    }

        if (key != 255){   
            display_value[pos] = 48 + key;
            edval = atoi(&display_value[0]);
            if (edval > max_value) edval = max_value;
            pos++;    

           if (edit_mode) {
            if (pos >= max_digits) {
                pos = 0;  // loop back to first digit
            }
           } else {
              if (pos >= 15) {                      
                done=true;
            }
           }
            updateDisplay();            
            lcd->setCursor(strlen(name)+2+pos,dispy); // put the cursor on the digit being edited            
        } else

        // inc / dec value
        if ((edit_mode) & (pressed == BTN_PLUS)) {
            edval++;
            if (edval > max_value) {
              if (rollOver) edval = 0;
               else edval = max_value;  
            }              
            pos = 0;  
            updateDisplay();          
        } else
        if ((edit_mode) & (pressed == BTN_MINUS)) {
            edval--;
            if (edval < 0){ 
                if (rollOver) edval = max_value; 
                 else edval = 0;
            }                 
            pos = 0;  
            updateDisplay();          
        } else

        // cancel editing
        if (pressed == BTN_EXIT) {
           if (edit_mode) {
            edval = orig_value;
           }                     
           done = true;                      
        } else

        // finished editing
        if (pressed == BTN_RECORD) {      
          done = true;
        }

       
}




