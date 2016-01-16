//
// Controlling Power Switch
//

const int DEBUG_MODUL_SWITCH = B00000100;

int rcPin;

// ==============================================

void initRf ( int pin ) {

  pinMode ( pin, OUTPUT );
  rcPin = pin;

};

// ==============================================

struct SwitchCode {
  unsigned long onCode;
  unsigned long offCode;
};

// TODO Auslagern in .h

#define NUM_RC 3
#define NUM_DEV 4

PROGMEM SwitchCode switchCode [NUM_RC] [NUM_DEV] = {
  // Fernbedienung 1 - KÃ¼che
  // DIP 123!4!5
  { { 18218, 18222}, { 19190, 19194 }, { 19514, 19518}, { 19622, 19626 } },

  // Fernbedienung 2 - Wohnzimmer
  // DIP 1!2!3!45
  { { 171308, 171312}, { 172280, 172284 }, { 172604, 172608}, { 172712, 172716 } },

  // Fernbedienung 3 - Terrarium
  // DIP !1!2345
  { { 473114, 473118}, { 474086, 474090 }, { 474410, 474414}, { 474518, 474522 } },

  // Fernbedienung 4 - Nina
  // DIP !1234!5

};

// ==============================================

void sendCode ( unsigned long code, int period  ) {

  // Format
  // pppppppp|prrrdddd|dddddddd|dddddddd (32 bit)
  // p - Geschw. 9 bit
  // r - Wiederholungen als 2log - r=3 entspricht 8 (2^3)
  // d - Code
  
  code |= (unsigned long) period << 23;
  code |= 4L << 20;
  
  RemoteSwitch::sendTelegram ( code, rcPin );

}

// ==============================================

void switchPower ( int remoteControl, int device, int state ) {
  
//  struct SwitchCode sc = switchCode [remoteControl] [device];  // Kugellampe Wz
  unsigned long code = 0UL;
  if ( state ) {
    code = pgm_read_dword ( &switchCode [remoteControl] [device].onCode );
  }
  else {
    code = pgm_read_dword ( &switchCode [remoteControl] [device].offCode );
  }
  if ( code != 0 ) sendCode ( code, 329 );
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_SWITCH, 9, PSTR("rc="), remoteControl );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_SWITCH, 9, PSTR("dev="), device );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_SWITCH, 9, PSTR("code="), code );

}

// ==============================================

void switchPowerAll ( int state ) {
  
  for ( int rc = 0; rc < NUM_RC - 1; rc++ ) { // ohne Terrarium!!!
    
    for ( int dev = 0; dev < NUM_DEV; dev++ ) {
      
      switchPower ( rc, dev, state );
      
    }
    
  }
}

// ==============================================


