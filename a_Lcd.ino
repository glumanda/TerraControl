//
// Lcd via software serial
//

SoftwareSerial serialLcd = SoftwareSerial ( PinSoftwareSerialRx, PinSoftwareSerialTx );

// ==============================================

void initLcd ( int pinRx, int pinTx ) {
  
  pinMode ( pinRx, INPUT );
  pinMode ( pinTx, OUTPUT );
  
  serialLcd.begin ( 9600 );
  sendCommand ( serialLcd, ClearDisplay );

}

// ==============================================

void lcdPrint_P ( prog_char* s ) {

  while ( char c = pgm_read_byte ( s++ ) ) serialLcd.print ( c );

}

void lcdPrintMeasure ( int row, prog_char* text, int val, prog_char* unit ) {
  
  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( text );
  serialLcd.print ( val );
  lcdPrint_P ( unit );

}

void lcdPrintDate ( int row, time_t* t ) {
  
  lcdPrintDate ( row, PSTR("   "), t );
  
}

void lcdPrintDate ( int row, prog_char* text, time_t* t ) {
  
  // dd.mm.yyyy = 10 Zeichen
  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( text );
  serialLcd.print ( toDateString ( t )  );    // dd.mm.yyyy
//  lcdPrint_P ( PSTR("   ") );
  lcdPrint_P ( PSTR("  ") );
  switch ( timeStatus () ) {
    case timeNotSet    : lcdPrint_P ( PSTR("-") ); break;
    case timeNeedsSync : lcdPrint_P ( PSTR("x") ); break;
    case timeSet       : lcdPrint_P ( PSTR("+") ); break;
    default            : lcdPrint_P ( PSTR(".") ); break;
  }

}

void lcdPrintTime ( int row, time_t* t ) {
  
  lcdPrintTime ( row, PSTR("    "), t );
  
}

void lcdPrintTime ( int row, prog_char* text, time_t* t ) {

  // hh:mm:ss = 8 Zeichen
  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( text );
  serialLcd.print ( toTimeString ( t, true )  );    // hh.mm.ss
  lcdPrint_P ( PSTR("   ") );
  lcdPrint_P ( ( isCEST ( *t ) ?  PSTR("s") :  PSTR("w") ) );

}

void lcdPrintLightState ( int row, boolean state, int value ) {
  
  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( PSTR("Licht: ") );
  lcdPrint_P ( ( state ?  PSTR("AN ") :  PSTR("AUS") ) );
  lcdPrint_P ( PSTR("(") );
  serialLcd.print ( value );
  lcdPrint_P ( PSTR(")") );
  
}

void lcdPrintOnOff ( const int row, const struct Time* t1, const struct Time* t2 ) {

  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( PSTR("  ") );
  serialLcd.print ( toLeadingZeroNumberString ( t1->hh ) );  // hh
  lcdPrint_P ( PSTR(":") );
  serialLcd.print ( toLeadingZeroNumberString ( t1->mm ) );  // mm
  lcdPrint_P ( PSTR("..") );
  serialLcd.print ( toLeadingZeroNumberString ( t2->hh ) );  // hh
  lcdPrint_P ( PSTR(":") );
  serialLcd.print ( toLeadingZeroNumberString ( t2->mm ) );  // mm
  lcdPrint_P ( PSTR("  ") );

}


void lcdPrintText ( int row, prog_char* txt ) {
  
  setPos ( serialLcd, row, 0 );
  lcdPrint_P ( txt );
  
}

// ==============================================

void sendCommand ( SoftwareSerial serial, byte command ) {

//  serial.print ( 0xFE, BYTE );
  serial.write ( 0xFE );
//  serial.print ( command, BYTE );
  serial.write ( command );

}

// ==============================================

void setPos ( SoftwareSerial serial, int row, int col ) {
  
  // ( 0, 0 ) ist links oben
  constrain ( col, 0, 15 );
  constrain ( row, 0, 1 );
  
  int pos = 0x80 + row * 0x40 + col;
  
  sendCommand ( serial, pos );

}

// ==============================================

void setBacklight ( SoftwareSerial serial, byte level ) {
  
  // 128 - 157
  constrain ( level, 128, 157 );
  
//  serial.print ( 0x7C, BYTE );
  serial.write ( 0x7C );
//  serial.print ( level, BYTE );
  serial.write ( level );
  
}

// ==============================================


