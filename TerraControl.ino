 /*
 * 21.10.2011 junand
 *
 * Steuerung des Terrariums
 *
 * ==============================================
 * Pins
 * ==============================================
 * Pin A0      - Foto Sensor
 * Pin D8      - ser. LCD Tx
 * Pin D6      - DHT11
 * Pin  5      - ser. LCD Rx (nicht genutzt)
 * Pin  A5/D19 - RF-Sender
 * Pin  7      - LED unused
 * Pin 10 - CS for EternetShield (HIGH = disable)
 * Pin 11, 12, 13 - EtherShield
 * ==============================================
 *
 */

//#define DEBUG_OFFLINE

#define VERBOSE false

#define VERSION PSTR("V2.00 (14.12.2012)")

 /*
 TODO
   timestamp für schaltvorgänge an Web
   .../values gibt letzten schaltzeitpunkt zurück
   variable ein/ausschaltung winter/sommer
     sommer 12-14h winter 10h
   protkollierung von daten in mysql
     timestamp|quelle|event|wert1-5
       quelle=rechner
       event=messung (temp,hum,sensor<9, ein, aus
   set threshold
   SetHandler im Web mit Syntaxprüfung und return code in http rsponse
 */

// ==============================================

#include <Streaming.h>
#include <DHT11.h>
#include <SoftwareSerial.h>
#include <RemoteSwitch.h>
#include <SPI.h>
#include <Time.h>

#include "LcdCommands.h"

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <Ethernet.h>
//#include <EthernetDHCP.h>
//#include <Udp.h>

// ==============================================

const int DEBUG_MODUL_MAIN = B00000001;

// ==============================================
// Pin defines

// !!! analog Pin
const int PinLightSensor = 0;        // A0

// LCD
const int PinSoftwareSerialRx = 5;
const int PinSoftwareSerialTx = 8;

const int PinRf = 19; // Transmitter for RC, A5/D19

const int PinLed = 7; // Led for light state

const int PinSensor = 6; // DIHT11

// ==============================================

struct Time {
  int hh;
  int mm;
};

int cmpTime ( const struct Time* t1, const struct Time* t2 ) {
  
  if ( t1->hh < t2->hh ) return -1;
  else if ( t1->hh > t2->hh ) return +1;
  else { // ==
    if ( t1->mm < t2->mm ) return -1;
    else if ( t1->mm > t2->mm ) return +1;
  }
  
  // now both times identical 
  return 0;
   
}

// ==============================================

// TODO reset jumper; if == 0 then clear eeprom
// TODO set config data from web

int MAC_EEPROM_ADDRESS = 0x00; // 6 Byte benötigt
int CONFIG_EEPROM_ADDRESS = 0x10;

const int DISPLAY_TEMP = 1 << 0;
const int DISPLAY_DATE = 1 << 1;
const int DISPLAY_LIGHT = 1 << 2;
const int DISPLAY_SINCE = 1 << 3;
const int DISPLAY_MEM = 1 << 4;
const int DISPLAY_DEBUG = 1 << 5;

struct Config {
  byte magic1;                  // 0101 0101
  Time lightOnAt;               // every day 09:00
  Time lightOffAt;              // every day 19:00
  int lightThreshold;           // when is light on detected = 800
  int toggleDisplayPeriod;      // time to toggle display between values
  int display;                  // Flags for show values
  byte magic2;                  // 1010 1010
} config = { 0x55, { 9, 0 }, { 19, 0 }, 800, 4000, DISPLAY_TEMP | DISPLAY_DATE, 0xCC };

struct DataLog {

  time_t livingSince;                  // ms since 01.01.1970
  //unsigned long ArduinoLogTime;      // millis () at time log was written
  time_t lastSwitchLightOn;            // ms since 01.01.1970
  time_t lastSwitchLightOff;           // ms since 01.01.1970
  unsigned long lastUpdateNtp;         // ms since 01.01.1970
  
  int temp;
  int hum;
  int lightSensorValue;
  
} dataLog = { 1, 2, 3, 4, 11, 22, 33 };

// ==============================================

byte mac [] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setup () {

  Serial.begin ( 9600 );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 0, PSTR("Free RAM 1: "), getFreeMem () );
  
  // read MAC-Address
//  byte result [] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  for ( int i = 0; i < 6; i++ ) {
    mac [i] = EEPROM.read ( MAC_EEPROM_ADDRESS + i );
  }
  dbgln ( DEBUG_MODUL_MAIN, 0, PSTR("MAC address="), toMacString ( mac ) );
  
  // read config
  struct Config eepromCfg;
  int len = EEPROM_readAnything ( CONFIG_EEPROM_ADDRESS, eepromCfg );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("eeprom read: len="), len );
  if ( eepromCfg.magic1 == config.magic1 && eepromCfg.magic2 == config.magic2 ) {
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("found magic entries") );
    config = eepromCfg;
  }
  else {
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("nothing in eeprom") );
    len = EEPROM_writeAnything ( CONFIG_EEPROM_ADDRESS, config );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("eeprom WRITE: len="), len );
  }

  initLcd ( PinSoftwareSerialRx, PinSoftwareSerialTx );

  initLed ( PinLed );
  
  initWeb ();
  initNtp ();
  
  dataLog.livingSince = now ();  
  if ( VERBOSE ) dbg ( DEBUG_MODUL_MAIN, 0, PSTR("date="), toDateString ( &dataLog.livingSince ) );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 0, PSTR(" time="), toTimeString ( &dataLog.livingSince ) );
  
  initRf ( PinRf );
  
  initLightSensor ( PinLightSensor );
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("Free RAM 2: "), getFreeMem () );
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 0, PSTR("setup ready ...") );
  
}

// ==============================================

int displayMode = 0;
unsigned long displayPeriodBegin = 0;
unsigned long lastSwitch = 0;
const unsigned long switchDelay = 5000L;

void loop () {
  
  readSensor ( PinSensor, &dataLog.temp, &dataLog.hum );
  
  time_t time = now ();
  
  if ( ( displayPeriodBegin + config.toggleDisplayPeriod ) < millis () ) {
    displayPeriodBegin = millis ();
    displayMode = ++displayMode % 6;  // alle Modi, Steuerung übber config.display
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 10, PSTR("display mode="), displayMode );
  }
  
  switch ( displayMode ) {
    
    case 0:  // temp + hum
      if ( config.display & DISPLAY_TEMP ) {
        lcdPrintMeasure ( 0,  PSTR("Temperatur:  "), dataLog.temp,  PSTR("C") );  // TODO Zeichen ° fehlt noch 
        lcdPrintMeasure ( 1,  PSTR("Luftfeuchte: "), dataLog.hum,  PSTR("%") );
      }
      break;
      
    case 1:  // date + time
      if ( config.display & DISPLAY_DATE ) {
        lcdPrintDate ( 0, &time );
        lcdPrintTime ( 1, &time );
      }
      break;
      
    case 2:  // Light state
      if ( config.display & DISPLAY_LIGHT ) {
        lcdPrintLightState ( 0, isLightOn (), dataLog.lightSensorValue );
        lcdPrintOnOff ( 1, &config.lightOnAt, &config.lightOffAt );
      }
      break;

    case 3:  // living since
      if ( config.display & DISPLAY_SINCE ) {
        lcdPrintDate ( 0, PSTR(".. "), &dataLog.livingSince );
        lcdPrintTime ( 1, PSTR("..  "), &dataLog.livingSince );
      }
      break;
      
    case 4:  // mem state
      if ( VERBOSE ) {
        if ( config.display & DISPLAY_MEM ) {
          lcdPrintMeasure ( 0, PSTR("free mem:  "), getFreeMem (),  PSTR("    ") );
          lcdPrintText ( 1, PSTR("                ") );
        }
      }
      break;
      
    case 5:  // debug
      if ( VERBOSE ) {
        if ( config.display & DISPLAY_DEBUG ) {
          lcdPrintText ( 0, VERSION );
          lcdPrintText ( 1, PSTR("                ") );
        }
      }
      break;
      
    default:
      //dbgln ( DEBUG_MODUL_MAIN, 0, PSTR("mode falsch") );
      displayMode = -1; //0;
      break;
      
  }
  
  displayMode++;
  
  handleWebRequest ();
  
  // decide light on/off
  Time now = { hour ( time ), minute ( time )};
  
  // true -> on, false -> off
  boolean lightState = cmpTime ( &config.lightOnAt, &now ) == -1 && cmpTime ( &now, &config.lightOffAt ) == -1;

  if ( lightState != isLightOn () ) {

    // wait for n sec
    if ( ( lastSwitch + switchDelay ) < millis () ) {
      lastSwitch = millis ();
    
      if ( VERBOSE ) {
        dbg ( DEBUG_MODUL_MAIN, 5, PSTR("switch light ") );
        if ( lightState ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("ON") );
        else              dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("OFF") );
      }
      setLedState ( lightState );
      // TODO switch Parameter auslagern in EEPROM
      if ( mac [5] == 0xED ) { // Ninas Terrarium
        switchPower ( 2, 2, lightState ); // Licht an
      }
      else {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("... for Chamäleon") );
        switchPower ( 2, 0, lightState ); // Licht an/aus
        //switchPower ( 2, 1, lightState ); // Brunnen an/aus
      }
      
      if ( lightState ) {
        dataLog.lastSwitchLightOn = time;
      }
      else {
        dataLog.lastSwitchLightOff = time;
      }
      
    }

  }
  
}
 
// ==============================================
    
