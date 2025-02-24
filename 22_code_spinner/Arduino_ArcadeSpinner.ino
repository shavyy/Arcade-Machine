/*  
 *  Arduino USB Arcade Spinner
 *  (C) Wilfried JEANNIARD [https://github.com/willoucom]
 *  
 *  Based on project by Alexey Melnikov [https://github.com/MiSTer-devel/Retro-Controllers-USB-MiSTer/blob/master/PaddleTwoControllersUSB/PaddleTwoControllersUSB.ino]
 *  Based on project by Mikael Norrgård <mick@daemonbite.com>
 *  
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 */

///////////////// Customizable settings /////////////////////////
// For debug (check serial monitor)
 //#define DEBUG

// Spinner pulses per revolution
#define SPINNER_PPR 600

// Spinner/Mouse sensitivity
// 1 is more sensitive 
// 999 is less sensitive
#define MOUSE_SENSITIVITY 1

/////////////////////////////////////////////////////////////////

// Pins used by encoder
#define pinA 3
#define pinB 2

////////////////////////////////////////////////////////

// ID for special support in MiSTer 
// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "MiSTer-S1 Spinner";

#include <Mouse.h>
#include "Gamepad.h"

// Create Gamepad
Gamepad_ Gamepad;
GamepadReport rep;

// Default virtual spinner position
int16_t drvpos = 0;
// Default real spinner position
int16_t r_drvpos = 0;
// Default virtual mouse position
int16_t m_drvpos = 0;

// Variables for paddle_emu
#define SP_MAX ((SPINNER_PPR*4*270UL)/360)
int32_t sp_clamp = SP_MAX/2;

// For emulation
bool mouse_emu = 0;

// Interrupt pins of Rotary Encoder
void drv_proc()
{
  static int8_t prev = drvpos;
  int8_t a = digitalRead(pinA);
  int8_t b = digitalRead(pinB);

  int8_t spval = (b << 1) | (b^a);
  int8_t diff = (prev - spval)&3;

  if(diff == 1) 
  {
    r_drvpos += 1;
    if(sp_clamp < SP_MAX) sp_clamp++;
  }
  if(diff == 3) 
  {
    r_drvpos -= 1;
    if(sp_clamp > 0) sp_clamp--;
  }

  m_drvpos = r_drvpos / MOUSE_SENSITIVITY;
  prev = spval;
}

// Run at startup
void setup()
{
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  Gamepad.reset();

  // Encoder
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  // Init encoder reading
  drv_proc();
  // Attach interrupt to each pin of the encoder
  attachInterrupt(digitalPinToInterrupt(pinA), drv_proc, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), drv_proc, CHANGE);

  mouse_emu = !mouse_emu;
  Mouse.begin(); 
}

// Main loop
void loop()
{
  // Mouse Emulation
  if(mouse_emu) {
    static uint16_t m_prev = 0;
    int16_t val = ((int16_t)(m_drvpos - m_prev));
    if(val>127) val = 127; else if(val<-127) val = -127;
    m_prev += val;
    Mouse.move(val, 0);
    rep.spinner = 0;
  }

  // Only report controller state if it has changed
  if (memcmp(&Gamepad._GamepadReport, &rep, sizeof(GamepadReport)))
  {
    #ifdef DEBUG
      // Very verbose debug
      Serial.print(gp_serial); Serial.print(" ");
      Serial.print(drvpos); Serial.print(" ");
      Serial.print(mouse_emu); Serial.print(" ");
      Serial.print(rep.spinner); Serial.print(" ");
    #endif
    // Send Gamepad changes
    Gamepad._GamepadReport = rep;
    Gamepad.send();
  }
}
