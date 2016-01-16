//
// debug feature
//
const int DEBUG_LEVEL_MIN = 0;
const int DEBUG_LEVEL_MID = 3;
const int DEBUG_LEVEL_DETAIL = 7;
const int DEBUG_LEVEL_FULL = 9;

const int MAX_DEBUG_LEVEL = 0;

 
const int DEBUG_MODULE_MASK = 0xFF;
//const int DEBUG_MODULE_MASK = 0xFF & ~DEBUG_MODUL_DHT11;
//const int DEBUG_MODULE_MASK = DEBUG_MODUL_SWITCH | DEBUG_MODUL_MAIN;
//const int DEBUG_MODULE_MASK = DEBUG_MODUL_MAIN | DEBUG_MODUL_WEBBASE | DEBUG_MODUL_WEBDETAIL;
//const int DEBUG_MODULE_MASK = DEBUG_MODUL_MAIN | DEBUG_MODUL_WEBBASE;

void _dbg ( prog_char* msg ) {

  while ( char c = pgm_read_byte ( msg++ ) ) Serial << c;

}

boolean _isDbg ( int module, int level ) {
  
  return module & DEBUG_MODULE_MASK && level <= MAX_DEBUG_LEVEL;
  
}

void dbg ( int module, int level, char c ) {
  
  Serial << c;
  
}

void dbg ( int module, int level, prog_char* msg ) {
  
  if ( !_isDbg ( module, level ) ) return;
  
  _dbg ( msg );
  
}

void dbgln ( int module, int level, prog_char* msg ) {
  
  if ( !_isDbg ( module, level ) ) return;

  _dbg ( msg);
  Serial << endl;
  
}

void dbg (  int module, int level, prog_char* msg, long val ) {
  
  if ( !_isDbg ( module, level ) ) return;

  _dbg ( msg);
  Serial << val;

}

void dbgln (  int module, int level, prog_char* msg, long val ) {
  
  if ( !_isDbg ( module, level ) ) return;

  dbg ( module, level, msg, val );
  Serial << endl;
  
}

void dbg (  int module, int level, prog_char* msg, char* val ) {
  
  if ( !_isDbg ( module, level ) ) return;

  _dbg ( msg);
  Serial << val;

}

void dbgln (  int module, int level, prog_char* msg, char* val ) {
  
  if ( !_isDbg ( module, level ) ) return;

  dbg ( module, level, msg, val );
  Serial << endl;

}



