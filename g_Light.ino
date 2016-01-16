//
// Light Sensor
//

int sensorPin;

// ==============================================
// !!! analog pin

void initLightSensor ( int pin ) {
  
  sensorPin = pin;
  
}

// ==============================================

boolean isLightOn () {
  
  dataLog.lightSensorValue = analogRead ( sensorPin );
  return ( dataLog.lightSensorValue > config.lightThreshold ? true : false );  // resistor bridge with 4,7 k

}

// ==============================================


