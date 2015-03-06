//   Derived from code developed by Chad Phillips, see:  http://windmeadow.com/node/42
//
//   For info on wiring up the chuck to the Arduino, download Bionic Arduino Class 4 from TodBot blog:
//   http://todbot.com/blog/2007/11/24/bionic-arduino-class-notes-3-4/
//
//   Further derived from XenonJohn in this tutorial: http://www.instructables.com/id/Wii-Nunchuck-as-general-purpose-controller-via-Ard/
//   http://www.instructables.com/files/orig/FQM/OY8B/G8LWOSNX/FQMOY8BG8LWOSNX.tmp


// Inputs:
// 5V output on Arduino to Nunchuck (works better than using 3.3V line)
// GND
// SDA (data) to analog pin 4
// SCK (clock)to analog pin 5
// the middle upper and middle lower contacts of nunchuck plug are not used

// Outputs:
// Outputs +5V to these pins when recieving Analog input from nunchuck:
//   Digital Pin 2: Motor 1 Forwards
//   Digital Pin 3: Motor 1 Backwards
//   Digital Pin 4: Motor 2 Forwards
//   Digital Pin 5: Motor 2 Backwards
//   Digital Pin 6: Motor 3 Forwards   }
//   Digital Pin 7: Motor 3 Backwards  } These motors run together
//   Digital Pin 8: Motor 4 Forwards   } for Surfacing/Diving
//   Digital Pin 9: Motor 4 Backwards  }

#include <Wire.h>
#include <string.h>

#undef int
#include <stdio.h>

uint8_t outbuf[6];		// array to store arduino output
int cnt = 0;
int ledPin = 13;

#define mot1F 2   //defines pin 2 as output for joystick : fwd/bwd, turn
#define mot1B 3   //defines pin 3 as output for joystick : fwd/bwd, turn
#define mot2F 4   //defines pin 4 as output for joystick : fwd/bwd, turn
#define mot2B 5   //defines pin 5 as output for joystick : fwd/bwd, turn
#define mot3F 6   //defines pin 6 as output for joystick : up/down, tilt
#define mot3B 7   //defines pin 7 as output for joystick : up/down, tilt
#define mot4F 8   //defines pin 8 as output for joystick : up/down, tilt
#define mot4B 9   //defines pin 9 as output for joystick : up/down, tilt

void setup ()
{
  //Outputs serial data to serial monitor
  Serial.begin (19200);  //Runs at 19200 baud rate
  Serial.print ("Finished setup\n");

  pinMode(mot1F, OUTPUT); //tell arduino it is an output
  pinMode(mot2F, OUTPUT); //tell arduino it is an output
  pinMode(mot3F, OUTPUT); //tell arduino it is an output
  pinMode(mot4F, OUTPUT); //tell arduino it is an output
  pinMode(mot1B, OUTPUT); //tell arduino it is an output
  pinMode(mot2B, OUTPUT); //tell arduino it is an output
  pinMode(mot3B, OUTPUT); //tell arduino it is an output
  pinMode(mot4B, OUTPUT); //tell arduino it is an output


  Wire.begin ();    // join i2c bus with address 0x52
  nunchuck_init (); // send the initilization handshake
}

void nunchuck_init ()
{
  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.write (0x40);		// sends memory address
  Wire.write (0x00);		// sends sent a zero.  
  Wire.endTransmission ();	// stop transmitting
}

void send_zero ()
{
  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.write (0x00);		// sends one byte
  Wire.endTransmission ();	// stop transmitting
}

void loop ()
{
  Wire.requestFrom (0x52, 6);	// request data from nunchuck
  while (Wire.available ())
  {
    outbuf[cnt] = nunchuk_decode_byte (Wire.read());	// receive byte as an integer
    digitalWrite (ledPin, HIGH);	// sets the LED on
    cnt++;
  }

  // If we recieved the 6 bytes, then go print them
  if (cnt >= 5)
  {
    print ();
  }

  cnt = 0;
  send_zero (); // send the request for next bytes
  delay (100);
}

// Print the input data we have recieved
// accel data is 10 bits long
// so we read 8 bits, then we have to add
// on the last 2 bits.  That is why I
// multiply them by 2 * 2
void print ()
{
  int joy_x_axis = outbuf[0];
  int joy_y_axis = outbuf[1];
  int accel_x_axis = outbuf[2] * 2 * 2; 
  int accel_y_axis = outbuf[3] * 2 * 2;
  int accel_z_axis = outbuf[4] * 2 * 2;

  int z_button = 0;
  int c_button = 0;

  // byte outbuf[5] contains bits for z and c buttons
  // it also contains the least significant bits for the accelerometer data
  // so we have to check each bit of byte outbuf[5]
  if ((outbuf[5] >> 0) & 1)
  {
    z_button = 1;
  }
  if ((outbuf[5] >> 1) & 1)
  {
    c_button = 1;
  }

  if ((outbuf[5] >> 2) & 1)
  {
    accel_x_axis += 2;
  }
  if ((outbuf[5] >> 3) & 1)
  {
    accel_x_axis += 1;
  }

  if ((outbuf[5] >> 4) & 1)
  {
    accel_y_axis += 2;
  }
  if ((outbuf[5] >> 5) & 1)
  {
    accel_y_axis += 1;
  }

  if ((outbuf[5] >> 6) & 1)
  {
    accel_z_axis += 2;
  }
  if ((outbuf[5] >> 7) & 1)
  {
    accel_z_axis += 1;
  }


  //Bind Controls to motor function
  // Nunchuck Joystick Forward = Motor 1 F + Motor 2 F
  // Nunchuck Joystick Backward = Motor 1 B + Motor 2 B
  // Nunchuck Joystick Right = Motor 1 B + Motor 2 F
  // Nunchuck Joystick Left
  // Nunchuck C Button
  // Nunchuck Z Button

  int j = joy_y_axis;
  int k = joy_x_axis;

  if(j > 180){
    digitalWrite(mot1F, HIGH);
    digitalWrite(mot2F, HIGH);
  }
  else if(j < 80){
    digitalWrite(mot1B, HIGH);
    digitalWrite(mot2B, HIGH);
  }
  else if(k >180){
    digitalWrite(mot1F, HIGH);
    digitalWrite(mot2B, LOW); 
  }
  else if(k <80){
    digitalWrite(mot1B, LOW);
    digitalWrite(mot2F, HIGH);

  } 
  else if (z_button < 1 && c_button == 1) {  //if pressing z button only, steer on left/right joystick
    digitalWrite(mot3B, HIGH);
    digitalWrite(mot4B, HIGH);
  } 
  else if (z_button == 1 && c_button < 1) {  //if c button only pressed then steer on accelerometer
    digitalWrite(mot3F, HIGH);
    digitalWrite(mot4F, HIGH);
  }
  else{
    digitalWrite(mot1F, LOW);
    digitalWrite(mot1B, LOW);
    digitalWrite(mot2F, LOW);
    digitalWrite(mot2B, LOW);
    digitalWrite(mot3F, LOW);
    digitalWrite(mot4F, LOW);
    digitalWrite(mot3B, LOW);
    digitalWrite(mot4B, LOW);
  }


  /*  Include these commands if you want to watch the data values streaming from chuck in the serial viewer window on your PC*/
  Serial.print (joy_x_axis, DEC); //values seen: 37left 134mid  234right
  Serial.print ("\t");

  Serial.print (joy_y_axis, DEC); //values seen: 218forward  124mid  23back
  Serial.print ("\t");

  Serial.print (accel_x_axis, DEC); //values: 312tiltedleft  512mid  745tiltedright
  Serial.print ("\t");

  Serial.print (accel_y_axis, DEC); //values: tippedforward754  mid576   tippedback361
  Serial.print ("\t");

  Serial.print (accel_z_axis, DEC);  // values: hovers around 530 - 743
  Serial.print ("\t");

  Serial.print (z_button, DEC); //NB: 0 when pressed, 1 when not (i.e. opposite of what you might expect)
  Serial.print ("\t");

  Serial.print (c_button, DEC); //NB: 0 when pressed, 1 when not (i.e. opposite of what you might expect)
  Serial.print ("\t");

  Serial.print ("\r\n");
}

// Encode data to format that most wiimote drivers except
// only needed if you use one of the regular wiimote drivers
char
nunchuk_decode_byte (char x)
{
  x = (x ^ 0x17) + 0x17;
  return x;
}





