//
// Sensor Interface for DIHT11
//

const int DEBUG_MODUL_DHT11 = B00000010;

DHT11 dht11 = DHT11 ();

unsigned long lastMeasure = 0;

// ==============================================

void readSensor ( int pin, int* temp, int* hum ) {
  
  // funktioniert auch, wenn nach 50 Tagen millis() umschlÃ¤gt, evtl. mit timout des sensors ...
  if ( millis () < (lastMeasure + 1000 ) ) return;
  
  lastMeasure = millis ();
  
  int dht11Ret = dht11.read ( pin ); // DHT11PIN
  if ( dht11Ret != 0 ) {
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_DHT11, 9, PSTR("read DHT11: retCode="), dht11Ret );
  }
  
  switch ( dht11Ret ) {

    case 0: 
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_DHT11, 9, PSTR("  Ok") );
      *temp = dht11.temperature;
      *hum = dht11.humidity;
      break;
      
    case -1: 
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_DHT11, 5, PSTR("  Checksum error") );
      break;
      
    case -2: 
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_DHT11, 9, PSTR("Time out error") );
      break;
      
    default: 
      dbgln ( DEBUG_MODUL_DHT11, 0, PSTR("DHT11: Unknown error") );
      break;
      
  }
  
  dbgln ( DEBUG_MODUL_DHT11, 9, PSTR("temp="), *temp );
  dbgln ( DEBUG_MODUL_DHT11, 9, PSTR("humidity="), *hum );
  
  
}

// ==============================================


