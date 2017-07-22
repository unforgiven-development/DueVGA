// Hello World Color using VGA library by stimmer

// include VGA library
#include <VGA.h>

void setup() {
  // start 320x240 mode
  VGA.begin(320,240,VGA_COLOR);
}

void loop() {
  // text color
  VGA.setInk(random(256));

  // text background color
  VGA.setPaper(random(256));

  // print message
  VGA.print(" Hello Arduino ");

  VGA.waitSync();
}
