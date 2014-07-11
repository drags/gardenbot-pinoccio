/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2014, Pinoccio Inc. All rights reserved.                   *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the MIT License as described in license.txt.         *
\**************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
extern "C" {
  #include <key/key.h>
}
#include "version.h"

void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here
  keyMap("dht", 0);
  keyMap("humidity", 0);
  keyMap("temp_c", 0);
  keyMap("temp_f", 0);
  keyMap("heat_index", 0);
  keyMap("moisture", 0);
  keyMap("apples", 0);
  keyMap("bananas", 0);
  keyMap("cucumbers", 0);
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}
