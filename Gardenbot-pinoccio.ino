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
#include <util/StringBuffer.h>
#include <stdlib.h>
extern "C" {
#include <key/key.h>
}

#include "version.h"

// DHT22 Sensor library from AdaFruit: https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>
#define DHTPIN 2 // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

//TSL2561 Sensor library from AdaFruit: https://github.com/adafruit/Adafruit_TSL2561
//TSL#include <Adafruit_Sensor.h>
//TSL#include <Adafruit_TSL2561_U.h>
//TSLAdafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);


void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  keyMap("dht", 0);
  keyMap("humidity", 0);
  keyMap("temp_c", 0);
  keyMap("temp_f", 0);
  keyMap("heat_index", 0);
  keyMap("moisture", 0);
  keyMap("apples", 0);
  keyMap("bananas", 0);
  keyMap("cucumbers", 0);

  // DHT Temp/Humidity sensor
  dht.begin();
  addBitlashFunction("dht.print", (bitlash_function)tempPrint);
  addBitlashFunction("dht.report", (bitlash_function)dhtReport);

  // DFRobot Moisture sensors
  addBitlashFunction("moisture.report", (bitlash_function)moistureReport);

  // Gardenbot
  addBitlashFunction("garden.report", (bitlash_function)gardenReport);

//  Serial.print("My ARDUINO is ");
//  Serial.println(ARDUINO);
  // TSL2561 Light sensor
//TSL  if(!tsl.begin())
//TSL  {
//TSL    /* There was a problem detecting the TSL2561 ... check your connections */
//TSL    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
//TSL    while(1);
//TSL  } else {
//TSL    Serial.println("Apparently found TSL sensor");
//TSL  }
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  //tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
//TSL  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
//TSL  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

//TSL  addBitlashFunction("tsl.print", (bitlash_function)lightPrint);

  // Moisture
  //Scout.
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}

void getTemp(float *dht_data) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  dht_data[0] = h;
  dht_data[1] = t;
  dht_data[2] = f;
  dht_data[3] = hi;
}

static StringBuffer dhtReportHQ(void) {
  float dht_data[4];
  StringBuffer report(100);
  getTemp(dht_data);
  char h[8], t[8], f[8], hi[8];
  dtostrf(dht_data[0], 3, 2, h);
  dtostrf(dht_data[1], 3, 2, t);
  dtostrf(dht_data[2], 3, 2, f);
  dtostrf(dht_data[3], 3, 2, hi);

  report.appendSprintf("[%d,[%d,%d,%d,%d],[%s,%s,%s,%s]]",
        keyMap("dht", 0), keyMap("humidity", 0), keyMap("temp_c", 0), keyMap("temp_f", 0), keyMap("heat_index", 0), h, t, f, hi
    );

  return Scout.handler.report(report);
}

static numvar dhtReport(void) {
  speol(dhtReportHQ());
  return 1;
}

void tempPrint() {
  float dht_data[4];
  getTemp(dht_data);

  Serial.print("Humidity: ");
  Serial.print(dht_data[0]);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(dht_data[1]);
  Serial.print(" *C ");
  Serial.print(dht_data[2]);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(dht_data[3]);
  Serial.println(" *F");

}

numvar moistureReport(void) {
  speol(moistureReportHQ());
  return 1;
}

static StringBuffer moistureReportHQ(void) {
  StringBuffer report(100);

  report.appendSprintf("[%d,[%d,%d,%d],[%d,%d,%d]]",
        keyMap("moisture", 0),
        keyMap("apples", 0),
        keyMap("bananas", 0),
        keyMap("cucumbers", 0),
        Scout.pinRead(Scout.getPinFromName("a0")),
        Scout.pinRead(Scout.getPinFromName("a1")),
        Scout.pinRead(Scout.getPinFromName("a2"))
  );
  Serial.println(report);
  return Scout.handler.report(report);
}

numvar gardenReport(void) {
  delay(3000); // wait for Pinoccio/DHT startup
  speol(dhtReportHQ());
  delay(2000); // breathe time
  speol(moistureReportHQ());
  delay(3000); // time for reports to be pushed onto mesh
}

//TSLnumvar lightPrint(void) {
//TSL  /* Get a new sensor event */
//TSL  sensors_event_t event;
//TSL  tsl.getEvent(&event);

//TSL  Serial.print("Got event from sensor: ");
//TSL  Serial.println(event.sensor_id);
//TSL  Serial.print("Got event at time: ");
//TSL  Serial.println(event.timestamp);

  /* Display the results (light is measured in lux) */
//TSL  if (event.light)
//TSL  {
//TSL    Serial.print(event.light); Serial.println(" lux");
//TSL  }
//TSL  else
//TSL  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
//TSL    Serial.println("Sensor overload");
//TSL    Serial.print(event.light); Serial.println(" lux");
//TSL  }
//TSL}

