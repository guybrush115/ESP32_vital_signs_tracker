Vital signs tracker: uses ESP32 with sensors to send biometric data to a server via MQTT and display time series trends in node-red dashboard in realtime.

Components:

1. ESP32 dec module
   1. sparkfun pulse oximeter sensor
   2. DHT11 humidity -temperature sensor
2. MQTT broker
3. node-red server
   1. node-red-contrib-queued-sqlite-fix module
   2. node-red-dashboard
   3. node-red-node-ui-table
4. sqlite3 database file : database/vital_signs.db 
5. vital signs json format: database/vital_signs_example.json
