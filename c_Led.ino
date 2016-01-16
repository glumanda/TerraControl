//
// Controlling Led
//

//  initial state of the LED
int ledState = LOW;
int ledPin;

// ==============================================

void initLed ( int pin ) {
  
  pinMode ( pin, OUTPUT );
  setLedState ( false );
  ledPin = pin;

};

// ==============================================

void setLedState ( boolean state ) {

  ledState = state;
  digitalWrite ( ledPin, ledState );

}

// ==============================================

inline boolean getLedState () { return ledState; }

// ==============================================



