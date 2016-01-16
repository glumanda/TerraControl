// ==============================================
// Ultitlity Functions

static char buf [20];

//------------------------------------------------

char* toDateString ( time_t* t ) {
  
  toNumberString ( buf + 0, day ( *t ),true, 10, 2 ); // dd
  buf [2] = '.';
  toNumberString ( buf + 3, month ( *t ),true, 10, 2 ); // mm
  buf [5] = '.';
  toNumberString ( buf + 6, year ( *t ),true, 10, 4 ); // yyyy
  buf [10] = '\0';
  
  return buf;
  
}

inline char* toTimeString ( time_t* t ) {
  
  return toTimeString ( t, false );
  
}

char* toTimeString ( time_t* t, boolean withSeconds ) {
  
  toNumberString ( buf + 0, hour ( *t ),true, 10, 2 ); // dd 
  buf [2] = ':';
  toNumberString ( buf + 3, minute ( *t ),true, 10, 2 ); // mm
  buf [5] = '\0';
  if ( withSeconds ) {
    buf [5] = ':';
    toNumberString ( buf + 6, second ( *t ),true, 10, 2 ); // ss
    buf [8] = '\0';
  }
  
  return buf;
  
}

// ==============================================

char* toMacString ( byte* array ) {
  
  return toNetString ( array, 6, ':', 16 );
  
}

char* toIpString ( byte* array ) {
  
  return toNetString ( array, 4, '.', 10 );
  
}

char* toNetString ( byte* array, int len, char separator, int base ) {
 
  for ( int i = 0; i < sizeof ( buf ); i++ ) buf [i] = '\0';
  
  int i = 0;
  int j = 0;
  while ( i < len ){
    itoa ( (int) array [i++], buf + j, base );
    while ( buf [j] ) { j++; } // search end of str:
    buf [j++] = separator;
  }

  buf [--j] = '\0';
  
  return buf;

}

char* toLeadingZeroNumberString ( int n ) {

  return toNumberString ( buf, n, true, 10, 2 );

}

char* toNumberString ( int n ) {
  
  return toNumberString ( buf, n, false, 10, 0 );

}

char* toNumberString ( char* buf, int n, boolean leadingZero, int base, int len ) {

  for ( int i = 0; i < sizeof ( buf ); i++ ) buf [i] = '\0';
  
  itoa ( n, buf, base );
  int slen = strlen ( buf );
  
  if ( leadingZero && len > slen ) {
    int leadingZeroLen = len - slen;
    for ( int i = 0; i < leadingZeroLen; i++ ) buf [len-1 - i] = buf [slen-1 - i];   // wegrÃ¤umen
    for ( int i = 0; i < leadingZeroLen; i++ ) buf [i] = '0';                        // auffÃ¼llen
  }
  
  return buf;

}

// ==============================================


