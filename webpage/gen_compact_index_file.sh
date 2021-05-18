#!/bin/bash


find ../data/web/ -name "*.gz" -exec rm {} \;
cp -a http/* ../data/web/
cp src/ip_arduino.js ../data/web/js/chart.js
cat dist/chart.js >> ../data/web/js/chart.js

find local/ -name "*.gz" -exec rm {} \;
cp -a http/* local/
cp src/ip.js local/js/chart.js
cat dist/chart.js >> local/js/chart.js

find ../data/web -type f -exec gzip {} \;
