

#ifndef SURFACE_CMD_LINE_H_
#define SURFACE_CMD_LINE_H_

#include "Arduino.h"
#include "JandsCardBus.h"


static String command_line = "";
static String pwd = "Channel"; // current directory / object
static String preset = "1.";
static bool expect_store_destination = false;
static int last_button = 0;
static int exec_page = 1;
static int preset_number = 1;

/* 
  MA2 preset numbers 
  ==================
  1 = dimmer
  2 = position
  3 = gobo
  4 = color
  5 = beam
  6 = focus
  7 = control
  8 = shapers
  9 = video
*/


// parse and execute the command.
void SurfaceCmdLine_Process(String &cmd)
{

  cmd = cmd.trim();

  // if a 'solo' command, indicated we are to change our default object prompt
  if (cmd == "Group")
  {
    pwd = "Group";
  }
  else if (cmd == "Fixture")
  {
    pwd = "Fixture";
  }
  else if (cmd == "Channel")
  {
    pwd = "Channel";
  }
  else if (cmd == "Group")
  {
    pwd = "Group";
  }

  Serial.println(cmd); // send to host

  cmd = ""; // clear the command line
}

// removes the last word in the command line
void remove_last_keyword(String &cmd)
{
  cmd = cmd.trim();
  int lio = cmd.lastIndexOf(" ");
  cmd = cmd.remove(lio + 1);
  // was  "Fixture 2 Thru 12"
  // now  "Fixture 2 Thru "
}

// compare last commane line entry to keyword, return true if same;
bool compare_last_keyword(const String cmd, const String keyword)
{
  int lio = cmd.trim().lastIndexOf(" ");
  if (lio < 0)
    lio = 0;
  String tmp1 = cmd.substring(lio).trim();

  // Serial.printf("Last='%s' Key='%s'\n", tmp1.c_str(), keyword.c_str());
  if (tmp1 == keyword)
  {
    // Serial.println("FOUND KEYWORD!");
    return true;
  }
  return false;
}

// return true if more than one space in a string
bool more_than_one_space(String &cmd)
{
  int a = cmd.indexOf(" ");
  if (a)
  {
    int b = cmd.indexOf(" ", a);
    if (b)
    {
      return true;
    }
  }
  return false;
}

// Process key input into the surface command line...
void SurfaceCmdLine(uint8_t key)
{

  // if store was last solo command, we are now expecting a destination button on a preset, assign or palette card
  if (expect_store_destination)
  {
    if ((key >= 0) & (key < 32))
    {
      command_line += "Exec ";
      command_line += String(exec_page);
      command_line += ".";
      command_line += key + 1;
    }
    else if (key < 52)
    { // palette buttons 32-52
      command_line += "Preset ";
      command_line += String(preset_number);
      command_line += ".";
      command_line += String(key - 32);
    }
    SurfaceCmdLine_Process(command_line);
    expect_store_destination = false;
  }
  else

      // if shift held and a flash button pressed, select that executor....
      if (SHIFT_HELD)
  {
    if ((key >= 0) & (key < 32))
    {
      command_line = "Select Exec ";
      command_line += String(exec_page);
      command_line += ".";
      command_line += String(key + 1);
      SurfaceCmdLine_Process(command_line);
    }
  }

  if (key == BTN_0)
  {
    command_line += "0";
  }
  else if (key == BTN_1)
  {
    if (SHIFT_HELD)
    {
      if (command_line.length() == 0)
      {
        command_line += "Copy ";
      }
    }
    else
      command_line += "1";
  }
  else

      if (key == BTN_2)
  {
    if (SHIFT_HELD)
    {
      if (command_line.length() == 0)
      {
        command_line += "Delete ";
      }
    }
    else
      command_line += "2";
  }
  else if (key == BTN_3)
  { // 3 / Stack
    if (SHIFT_HELD)
    {
      command_line += "Sequence ";
    }
    else
    {
      command_line += "3";
    }
  }
  else if (key == BTN_4)
  {
    if (SHIFT_HELD)
    {
      command_line = "Page ";
    }
    else
    {
      command_line += "4";
    }
  }
  else if (key == BTN_5)
  {
    if (SHIFT_HELD)
    {
      if (command_line.length() == 0)
      {
        command_line += "Assign ";
      }
    }
    else
      command_line += "5";
  }
  else if (key == BTN_6)
  {
    if (SHIFT_HELD)
    {
      if (command_line.length() == 0)
      {
        command_line += "Backup";
        SurfaceCmdLine_Process(command_line);
      }
    }
    else
      command_line += "6";
  }
  else if (key == BTN_7)
  {
    command_line += "7";
  }
  else if (key == BTN_8)
  {
    if (SHIFT_HELD)
    {
      if (command_line.length() == 0)
      {
        command_line += "Edit ";
      }
    }
    else
      command_line += "8";
  }
  else if (key == BTN_9)
  {
    command_line += "9";
  }
  else

      if (key == BTN_PLUS)
  {
    if (command_line.length() == 0)
    {
      command_line = "DefGoForward";
      SurfaceCmdLine_Process(command_line);
    }
    else
      command_line += "+";
  }
  else if (key == BTN_MINUS)
  {
    if (command_line.length() == 0)
    {
      command_line = "DefGoBack";
      SurfaceCmdLine_Process(command_line);
    }
    else
      command_line += "-";
  }
  else

      if (key == BTN_RIGHT)
  {
    if (command_line.length() == 0)
    {
      command_line = "Next";
      SurfaceCmdLine_Process(command_line);
    }
    else
    {
      command_line += "."; // right key is decimal
    }
  }
  else

      if (key == BTN_LEFT)
  {
    if (command_line.length() == 0)
    {
      command_line = "Prev";
      SurfaceCmdLine_Process(command_line);
    }
    else
    {
      remove_last_keyword(command_line);
    }
  }
  else

      if (key == BTN_FADER)
  { // multi-stacked button  -  both Fader and Exec
    if (last_button == key)
    {
      remove_last_keyword(command_line);
      command_line += "Fader ";
      key = 255; // reset key, so that last key will not be the same as this key, forcing a switch to Exec on next press
    }
    else
      command_line += "Exec ";
  }
  else

      // example of stacking more than one keyword on a button
      if (key == BTN_POSITION)
  {
    if (command_line.length() == 0)
    {
      command_line = "Position";
      command_line += " ";
    }
    else
    {
      if (compare_last_keyword(command_line, "Position"))
      {
        remove_last_keyword(command_line);
        command_line += "Focus";
        command_line += " ";
      }
      else if (compare_last_keyword(command_line, "Focus"))
      {
        remove_last_keyword(command_line);
        command_line += "Prism";
        command_line += " ";
      }
      else if (compare_last_keyword(command_line, "Prism"))
      {
        remove_last_keyword(command_line);
        command_line += "Position";
        command_line += " ";
      }
      else
        command_line += "Position";
      command_line += " ";
    }
  }

  else if (key == BTN_GROUP)
  {
    command_line += "Group ";
  }
  else if (key == BTN_FIXTURE)
  {
    command_line += "Fixture ";
  }
  else if (key == BTN_SCROLLER)
  {
    command_line += "Channel ";
  }
  else if (key == BTN_HALT)
  {
    if (command_line.length() == 0)
    {
      command_line = "Thru ";
    }
    else
    {
      command_line += " Thru ";
    }
  }
  else if (key == BTN_EXIT)
  {
    command_line = "";
  }

  else if (key == BTN_RELEASE)
  { // Release / @
    if (command_line.length() == 0)
    {
      command_line = "At ";
    }
    else
    {
      if (last_button == BTN_RELEASE)
      {
        // second press of @ is to default at "normal" (100) full
        key = 255; // clear last button
        command_line += "100";
        SurfaceCmdLine_Process(command_line);
      }
      else if (more_than_one_space(command_line))
      {
        command_line += " At ";
      }
      else
      {
        SurfaceCmdLine_Process(command_line);
        command_line = "At ";
      }
    }
  }

  else if (key == BTN_CLEAR)
  {
    if (command_line.length() == 0)
    {
      command_line = "Clear";
      SurfaceCmdLine_Process(command_line); // send clear command
    }
    else
    {
      command_line = ""; // erase line
    }
  }

  else if (key == BTN_RECORD)
  {
    if (!SHIFT_HELD)
    {

      if (command_line.length() == 0)
      {
        command_line += "Store ";
        expect_store_destination = true;
      }
      else
      {
        // enter - please
        SurfaceCmdLine_Process(command_line);
      }
    }

    else
    { // shift_help

      if (command_line.length() != 0)
      {
        command_line = "Update";
        SurfaceCmdLine_Process(command_line);
      }
    }
  }

  last_button = key;

  // update display
  Surface->assign.lcd.setCursor(0, 0);
  Surface->assign.lcd.print("                                        ");
  Surface->assign.lcd.setCursor(0, 0);
  Surface->assign.lcd.printf("[%s]> ", pwd.c_str());
  Surface->assign.lcd.print(command_line);
}


#endif