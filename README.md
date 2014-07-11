# Gardenbot

[Gardenbot](http://gardenbot.org/) built using a [Pinocc.io](http://pinocc.io) scout, [DHT22](https://www.adafruit.com/products/385) temperature/humidity sensor, [TSL2561](http://www.adafruit.com/products/439) light sensor, and [DFRobot SEN0114](http://www.dfrobot.com/index.php?route=product/product&product_id=599#.UhflOL-N_H0) soil moisture sensors.

Early alpha stage. Pushing data to graphite. Soil moisture sensors are pegged at 100%, most likely due to poor circuit design :) (See including Fritzing sketch)

## api-to-graphite.py

Currently:
	- doesn't enforce sane filtering (ie: doesn't complain about missing account id or troop id filter when given a scout id)
