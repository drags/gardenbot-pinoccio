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
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);


void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);

  // DHT Temp/Humidity sensor
  dht.begin();
  addBitlashFunction("dht.print", (bitlash_function)tempPrint);
  addBitlashFunction("dht.report", (bitlash_function)dhtReport);

  Serial.print("My ARDUINO is ");
  Serial.println(ARDUINO);
  // TSL2561 Light sensor
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  } else {
    Serial.println("Apparently found TSL sensor");
  }
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  //tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  addBitlashFunction("tsl.print", (bitlash_function)lightPrint);

  // Moisture
  //Scout.
  addBitlashFunction("moisture.report", (bitlash_function)moistureReport);

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
  
static StringBuffer DHTReportHQ(void) {
  float dht_data[4];
  StringBuffer report(100);
  
  getTemp(&dht_data[0]);
  report.appendSprintf("[%d,[%d,%d,%d],[%d,%d,%d]",
        keyMap("dht"), "humidity", "temperature_c", "temperature_f",
        dht_data[0], dht_data[1], dht_data[2]
  );
  
  return Scout.handler.report(report);
}

numvar dhtReport(void) {
  speol(DHTReportHQ());
  return true;
}

void tempPrint() {
  float dht_data[4];
  getTemp(&dht_data[0]);
  
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


numvar lightPrint(void) {
  /* Get a new sensor event */
  sensors_event_t event;
  tsl.getEvent(&event);

  Serial.print("Got event from sensor: ");
  Serial.println(event.sensor_id);
  Serial.print("Got event at time: ");
  Serial.println(event.timestamp);

  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    Serial.print(event.light); Serial.println(" lux");
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
    Serial.print(event.light); Serial.println(" lux");
  }
}

//numvar lightGain(void) {
//  a = getarg(
numvar moistureReport(void) {
  int numSensors = getarg(1);
  StringBuffer report(100);

  report.appendSprintf("[%d,[%d,%d,%d],[%d,%d,%d]",
        "moisture",
        Scout.pinRead(Scout.getPinFromName("a0")),
        Scout.pinRead(Scout.getPinFromName("a1")),
        Scout.pinRead(Scout.getPinFromName("a2"))
  );
  //for (int i = 0; i < numSensors; i++) {
    //StringBuffer sensor_pin(2);
    //sensor_pin.appendSprintf("a%d", i);
    //Serial.print("Looking for analog pin: "); Serial.println(sensor_pin);
  //}
}
