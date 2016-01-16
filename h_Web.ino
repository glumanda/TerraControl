//
// WebServer
//

const int DEBUG_MODUL_WEBBASE = B00010000;

// ==============================================

//byte mac [] = { 0x90, 0xa2, 0xda, 0x00, 0x6c, 0x21 }; // Arduino Uno Ethernet-Shield

IPAddress ip;
IPAddress gatewayip;
IPAddress dnsip;

EthernetServer server ( 80 );

// ==============================================

inline byte* getMac () { return (byte*) mac; }
//inline IPAddress getIp () { return ip; }
//inline IPAddress getGateway () { return gatewayip; }

// ==============================================

void initWeb () {

  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("mac="), toMacString ( mac ) );
  
#ifdef DEBUG_OFFLINE
  return;
#endif
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("trying dhcp ...") );
  
  Ethernet.begin ( mac );
  
  ip = Ethernet.localIP ();
  gatewayip = Ethernet.gatewayIP ();
  dnsip = Ethernet.dnsServerIP ();
    
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("A DHCP lease has been obtained:") );
  dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  ip="), toIpString ( (uint8_t*) &ip ) );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  gw="), toIpString ( (uint8_t*) &gatewayip ) );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  dns="), toIpString ( (uint8_t*) &dnsip ) );
  
  server.begin ();

}

// ==============================================

#define NONE 0
#define GET 1
#define POST 2

EthernetClient client = NULL;

boolean isHttpRequest () {
  
  return client && client.connected () && client.available ();
  
}

void handleWebRequest () {
  
#ifdef DEBUG_OFFLINE
  return;
#endif

  client = server.available ();

  // 2) read http header
  int method = NONE;
  char requestMethod [20];
  char page [20];
  char getparms [40] = "x";
  int i = 0;
  
  const byte PARSER_STATE_ERROR = -1;
  const byte PARSER_STATE_METHOD = 1;
  const byte PARSER_STATE_PAGE = 2;
  const byte PARSER_STATE_GETPARM = 3;
  const byte PARSER_STATE_HEADER = 4;
  const byte PARSER_STATE_END = 5;
  
  byte parserState = PARSER_STATE_METHOD;
  
  char lastChar = ' ';
  int charCount = 0;
  
  // TODO in Funktion readHttpHeaders auslagern
  while ( isHttpRequest () && parserState != PARSER_STATE_END ) {
    
    uint8_t c = client.read ();
    if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 5, c );
    
    if ( parserState == PARSER_STATE_METHOD ) {
      if ( c == ' ' ) {
        requestMethod [i] = '\0';
        i = 0; // Redundanz
        parserState = PARSER_STATE_PAGE;
        if ( strcmp_P ( requestMethod,  PSTR("GET") ) == 0 )       method = GET;
        else if ( strcmp_P ( requestMethod,  PSTR("POST") ) == 0 ) method = POST;
      }
      else {
        requestMethod [i++] = c;
      }
    }
    else if ( parserState == PARSER_STATE_PAGE ) {
      if ( c == '/' ) { // geht nur fÃ¼r "einfache" seiten in requests (also nict mehrerer '/')
        i = 0;
        page [i++] = c;
      }
      else if ( c == ' ' || c == '?' ) {
        page [i] = '\0';
        i = 0; // Redundanz
        parserState = (c == '?') ? PARSER_STATE_GETPARM : PARSER_STATE_HEADER;
      }
      else {
        page [i++] = c;
      }
    }
    else if ( parserState == PARSER_STATE_GETPARM ) {
      if ( c == ' ' ) { // Ende Parameter
        getparms [i] = '\0';
        i = 0; // Redundanz
        parserState = PARSER_STATE_HEADER;
      }
      else {
        getparms [i++] = c;
      }
    }
    else if ( parserState == PARSER_STATE_HEADER ) {
      // seek to end of http-header 
      if ( lastChar == '\n' && c == '\r' || lastChar == '\r' && c == '\n') { // TODO Standard klÃ¤ren  nur \n\r bisher gesehen
        if ( charCount == 0 ) {
          parserState = PARSER_STATE_END; // empty and last line found
        }
        else {
          charCount = 0; // new line begins
        }
      }
      else {
        if ( c != '\n' && c != '\r' ) charCount++;
        lastChar = c;
      }
    }
    else {
      parserState = PARSER_STATE_ERROR;
      dbgln ( DEBUG_MODUL_WEBBASE, 0, PSTR("Parser Error") );
    }
  }
  
  if ( method != NONE ) {
    
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("request="), method );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("page="), page );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("getparms="), getparms );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("Free RAM: "), getFreeMem () );
      
    // call page handler
    if ( strcasecmp_P (  page, PSTR("/blinkled") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/blinkled") );
      sendHeaderOk ();
      blinkLedHandler ();
    }
    else if ( strcasecmp_P ( page, PSTR("/switchpower") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/switchpower") );
      sendHeaderOk ();
      if ( method == GET ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  => GET") );
        switchPowerHandler ( getparms );
      }
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("switch handler processed") );
    }
    else if ( strcasecmp_P ( page, PSTR("/set") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/set") );
      sendHeaderOk ();
      if ( method == GET ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  => GET") );
        setHandler ( getparms );
      }
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("set handler processed") );
    }
    else if ( strcasecmp_P ( page, PSTR("/values") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/values") );
      sendHeaderOk ();
      getValuesHandler (); // after http header!!!
    }
    else if ( strcasecmp_P ( page, PSTR("/display") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/display") );
      sendHeaderOk ();
      if ( method == GET ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("  => GET") );
        displayHandler ( getparms );
      }
    }
    else if ( strcasecmp_P ( page, PSTR("/reboot") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/reboot") );
      sendHeaderOk ();
      rebootHandler (); // after http header!!!
    }
    else if ( strcasecmp_P ( page, PSTR("/") ) == 0 ) {
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("/index.html") );
      sendHeaderOk ();
      indexPageHandler ();
    }
    else {
      sendHeaderNotFound ();
    }
    
    client.stop ();
    client = NULL;
    delay ( 100 ); // time for browser
    
  }
      
}

// ==============================================

void blinkLedHandler () {
  
  // Reverse the state of the LED.
  setLedState ( !getLedState () );
  
  sendText ( PSTR("Arduino lives ... ") );
  sendNumber ( millis () );
  sendText ( PSTR("new led state is ... ") );
  sendText ( ( getLedState () ? PSTR("ON") : PSTR("OFF") ) );

}

// ==============================================

void indexPageHandler () {

      sendText ( PSTR("Arduino lives since ... ") );
      sendText ( (const char*) toDateString ( &dataLog.livingSince ) );
      sendText ( PSTR(" ") );
      sendText ( (const char*) toTimeString ( &dataLog.livingSince, true ) );
      sendText ( PSTR(" for ") );
      sendNumber ( millis () );
      sendText ( PSTR("ms\r\n") );
      sendText ( PSTR("\r\n") );
      sendText ( PSTR("current time ... ") );
      time_t time = now ();
      sendText ( (const char*) toDateString ( &time ) );
      sendText ( PSTR(" ") );
      sendText ( (const char*) toTimeString ( &time, true ) ); sendText ( PSTR("\r\n") ); 
      
      sendText ( PSTR("\r\n") );
      sendText ( PSTR("last ntp-call ... ") );
      sendText ( (const char*) toDateString ( &dataLog.lastUpdateNtp ) );
      sendText ( PSTR(" ") );
      sendText ( (const char*) toTimeString ( &dataLog.lastUpdateNtp, true ) );
      sendText ( PSTR(" with ip=") ); sendText ( (const char*) toIpString ( SNTP_server_IP ) ); sendText ( PSTR("\r\n") ); 
      sendText ( PSTR("\r\n") );
      sendText ( PSTR("version=") ); sendText ( VERSION ); sendText ( PSTR("\r\n") ); 
      sendText ( PSTR("\r\n") );
      sendText ( PSTR("mac=") ); sendText ( (const char*) toMacString ( getMac () ) ); sendText ( PSTR("\r\n") ); 
//      sendText ( PSTR("ip=") ); sendText ( (const char*) toIpString ( getIp () ) ); sendText ( PSTR("\r\n") ); 
      sendText ( PSTR("ip=") ); sendIPAddress ( ip ); sendText ( PSTR("\r\n") ); 
//      sendText ( PSTR("gw=") ); sendText ( (const char*) toIpString ( getGateway () ) ); sendText ( PSTR("\r\n") ); 
      sendText ( PSTR("gw=") ); sendIPAddress ( gatewayip ); sendText ( PSTR("\r\n") ); 

}

// ==============================================

void switchPowerHandler ( char* parms ) {

  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("/switchpower parms="), parms );
  
  boolean found = false;
  int i = 0;
  char* token = parms; // zeigt auf Namen des ersten Parameters
  
  int rc = -2;
  int dev = 1;
  int state = 1;
  int* var;
  
  if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 9, PSTR("content=") );
  
  while ( !found ) {

    char c = parms [i++];
    
    if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 9, c );
  
    // Variablen lesen, bisher keine SyntaxprÃ¼fung
    if ( c == '\n' || c == '\r' ) { // ???
      // ignore
    }
    else if ( c == '&' || c == '\0' ) {
      if ( c == '\0' ) found = true;
      parms [i-1] = '\0';
      *var = atoi ( token );
      token = parms + i; // zeigt nun auf den Namen des Parameters
    }
    else if ( c == '=' ) {
      parms [i-1] = '\0';
      if ( strcmp_P ( token, PSTR("rc") ) == 0 )         var = &rc;
      else if ( strcmp_P ( token, PSTR("dev") ) == 0 )   var = &dev;
      else if ( strcmp_P ( token, PSTR("state") ) == 0 ) var = &state;
      token = parms + i; // zeigt nun auf den Wert des Parameters
    }
    else {
      // do nothing
    }
    
  }
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("") );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("rc="), rc );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("dev="), dev );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("state="), state );
  
  if ( rc == 0 ) {
    switchPowerAll ( state );
  }
  else if ( rc > 0 ) {
    switchPower ( rc-1, dev-1, state );
  }
  
}

// ==============================================

void setHandler ( char* parms ) {

  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("/set parms="), parms );
  
  boolean eol = false;
  int i = 0;
  char* token = parms; // zeigt auf Namen des ersten Parameters
  int value;
  
  Time on = config.lightOnAt;
  Time off = config.lightOffAt;
  int threshold = 9999;
  
  Time* time;
  
  int* var;
  
  if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 9, PSTR("content=") );
  
  while ( !eol ) {

    char c = parms [i++];
    
    if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 9, c );
  
    // Variablen lesen, bisher keine SyntaxprÃ¼fung
    if ( c == '\n' || c == '\r' ) { // ???
      // ignore
    }
    else if ( c == '&' || c == '\0' ) {
      
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("& or 0 found: pos="), i );
      if ( c == '\0' ) eol = true;
      
      parms [i-1] = '\0';
      int len = 0; // wird eigebtlich nicht gebraucht
      if ( strcmp_P ( token, PSTR("on") ) == 0 ) {
        len = scanTimeValue ( parms + value, &on );
      }
      else if ( strcmp_P ( token, PSTR("off") ) == 0 ) {
        len = scanTimeValue ( parms + value, &off );
      }
      else if ( strcmp_P ( token, PSTR("thr") ) == 0 ) {
        len = scanIntValue ( parms + value, &threshold );
      }
/*      else if ( strcmp_P ( token, PSTR("ntp") ) == 0 ) {
        len = scanIpAddressValue ( parms + value, &threshold );
      } */
      //i += len;

      token = parms + i; // zeigt nun auf die nÃ¤chste Var.
      
    }
    else if ( c == '=' ) {
      
      parms [i-1] = '\0'; // VarNamen "beenden"
      value = i;
      
    }
    else {
      // do nothing
    }
    
  }
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("") ); // wg. content=
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("on.hh="), on.hh );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("on.mm="), on.mm );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("off.hh="), off.hh );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("off.mm="), off.mm );
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("thr="), threshold );
//  dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("ntp="), ip );
  
  boolean write = false;
  
  // config belegen, falls konsistent
  if ( on.hh >= 0 && on.hh < 24 && on.mm >= 0 && on.mm < 60 &&
       off.hh >= 0 && off.hh < 24 && off.mm >= 0 && off.mm < 60 &&
       cmpTime ( &on, &off ) == -1 ) {
         
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("on < off") );
    write = cmpTime ( &on, &config.lightOnAt ) != 0 || cmpTime ( &off, &config.lightOffAt ) != 0;
    config.lightOnAt = on;
    config.lightOffAt = off;
  }
  else {
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("on off falsch") );
  }
  
  if ( 0 <= threshold && threshold < 1024 ) {
    write = write || config.lightThreshold != threshold;
    config.lightThreshold = threshold;
  }
  else {
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("thr falsch") );
  } 
  
  if ( write ) {
    // in eeprom schreiben
    int len = -99;
    len = EEPROM_writeAnything ( CONFIG_EEPROM_ADDRESS, config );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("eeprom WRITE: len="), len );
    
  }
  
}

int scanIntValue ( char* p, int* result ) {

  int i = 0;
  boolean done = false;
  
  int val = 0;
  
  while ( !done ) {
    
    char c = p [i++];
//    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("val="), val );      
    
    if ( c ==  '&' || c ==  '\0' ) {
      done = true;
    }
    else if ( isdigit ( c ) ) {
      val *= 10;
      val += c - '0';
    }
    
  }

  *result = val;
  
  return i;

}

int scanTimeValue ( char* p, Time* result ) {
  
  int i = 0;
  boolean done = false;
  
  int hh = 0;
  int mm = 0;
  int* val = &hh;
  
  while ( !done ) {
    
    char c = p [i++];
//    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("val="), val );      
    
    if ( c ==  '&' || c ==  '\0' ) {
      done = true;
    }
    else if ( isdigit ( c ) ) {
      *val *= 10;
      *val += c - '0';
    }
    else if ( c == ':' ) {
      val = &mm;
    }
    
  }

  result->hh = hh;
  result->mm = mm;
  
  return i;

}

// ==============================================
// read Temp/hum

void getValuesHandler () {
  
  // temp=nn|hum=nn|on=hh:mm|off=hh:mm|light=on/off|thr=nn|tz=me(s)z|date=dd.mm.yyyy|time=hh:mm:ss
  
  sendText (  PSTR("temp=") ); sendNumber ( dataLog.temp );
  sendText (  PSTR("|hum=") ); sendNumber ( dataLog.hum );
  
  sendText (  PSTR("|on=") );
  sendLeadingZeroNumber ( config.lightOnAt.hh );
  sendText (  PSTR(":") );
  sendLeadingZeroNumber ( config.lightOnAt.mm );

  sendText (  PSTR("|off=") );
  sendLeadingZeroNumber ( config.lightOffAt.hh );
  sendText (  PSTR(":") );
  sendLeadingZeroNumber ( config.lightOffAt.mm );

  sendText (  PSTR("|light=") );
  sendText ( (prog_char*) ( isLightOn () ?  PSTR("on") :  PSTR("off") ) );
  
  sendText (  PSTR("|sensor=") );
  sendNumber ( dataLog.lightSensorValue );
  
  sendText (  PSTR("|thr=") );
  sendNumber ( config.lightThreshold );

  time_t t = now ();
  sendText (  PSTR("|tz=") );
  sendText ( (prog_char*) (isCEST ( t ) ? PSTR("MESZ") : PSTR("MEZ") ) );

  sendText (  PSTR("|date=") );
  sendText ( (const char*) toDateString ( &t ) );

  sendText (  PSTR("|time=") );
  sendText ( (const char*) toTimeString ( &t, true ) );
  
  sendText (  PSTR("|since=") );
  sendText ( (const char*) toDateString ( &dataLog.livingSince ) );
  sendText (  PSTR(" ") );
  sendText ( (const char*) toTimeString ( &dataLog.livingSince, true ) );
  
  sendText (  PSTR("|version=") );
  sendText ( VERSION );
  
  sendText ( PSTR("\r\n") );
  
}

// ==============================================

void rebootHandler () {
  
  client.stop (); // sonst wird die co0nnection nicht geschlossen
  delay ( 100 ); // time for browser
  
  softwareReset ();
  
}

// Restarts program from beginning but does not reset the peripherals and registers
void softwareReset () {
  
  asm volatile ("  jmp 0");
  
}   

// ==============================================

void displayHandler ( char* parms ) {

  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("/display parms="), parms );
  
  boolean eol = false;
  int i = 0;
  char* token = parms; // zeigt auf Namen des ersten Parameters
  int value;
  
  int display = config.display;
  
  int* var;
  
  if ( VERBOSE ) dbg ( DEBUG_MODUL_WEBBASE, 9, PSTR("content=") );
  
  while ( !eol ) {

    char c = parms [i++];
    
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("c="), c );
  
    // Variablen lesen, bisher keine SyntaxprÃ¼fung
    if ( c == '\n' || c == '\r' ) { // ???
      // ignore
    }
    else if ( c == '&' || c == '\0' ) {
      
      if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("& or \0 found: pos="), i );
      if ( c == '\0' ) eol = true;
      
      int len = 0; // wird nicht benÃ¶tigt
      boolean flag = false;
      // temp, date, light, since, mem, debug
      if ( strcmp_P ( token, PSTR("temp") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display temp") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_TEMP );
      }
      else if ( strcmp_P ( token, PSTR("date") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display date") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_DATE );
      }
      else if ( strcmp_P ( token, PSTR("light") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display light") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_LIGHT );
      }
      else if ( strcmp_P ( token, PSTR("since") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display since") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_SINCE );
      }
      else if ( strcmp_P ( token, PSTR("mem") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display mem") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_MEM );
      }
      else if ( strcmp_P ( token, PSTR("debug") ) == 0 ) {
        if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("display debug") );
        len = scanFlagValue ( parms + value, &flag );
        setFlag ( &display, flag, DISPLAY_DEBUG );
      }
      //i += len;

      token = parms + i; // zeigt nun auf die nÃ¤chste Var.
      
    }
    else if ( c == '=' ) {

      parms [i-1] = '\0'; // VarNamen "beenden"
      value = i;

    }
    else {
      // do nothing
    }
    
  }
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("") ); // wg. content=
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("dispaly="), display );
  
  boolean write = false;
  
  if ( config.display != display ) {
    
    write = true;
    config.display = display;
    
  }
  
  if ( write ) {
    // in eeprom schreiben
    int len = -99;
    len = EEPROM_writeAnything ( CONFIG_EEPROM_ADDRESS, config );
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_MAIN, 5, PSTR("eeprom WRITE: len="), len );
    
  }

}

void setFlag ( int* display, boolean flag, int mask ) {
  
  if ( flag ) {
    *display |= mask;
  }
  else {
    *display &= ~mask;
  }
  
}

int scanFlagValue ( char* p, boolean* result ) {

  int i = 0;
  boolean done = false;
  
  boolean flag = false;
  
  //char old_c = '\0';
  
  while ( !done ) {
    
    char c = p [i++];
    if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("cc="), c );      
    
    if ( c ==  '&' || c ==  '\0' ) {
      done = true;
    //  old_c = p [i-1];
      p [i-1] = '\0'; // "beenden"

    }
  }
  
  if ( strcmp_P ( p, PSTR("on") ) == 0 ) {
    flag = true;
  }
  else if ( strcmp_P ( p, PSTR("off") ) == 0 ) {
    flag = false;
  }

  *result = flag;
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("flag="), flag );      
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 9, PSTR("len="), i );      
  
//  if ( old_c != '\0' ) p [i-1] = old_c;
  
  return i;

}

// ==============================================

void sendHeaderOk () {
  
  sendText ( PSTR("HTTP/1.1 200 OK\r\n") );
  sendText ( PSTR("Content-Type: text/plain\r\n") );
  sendText ( PSTR("\r\n") );

}

void sendHeaderNotFound () {
  
  if ( VERBOSE ) dbgln ( DEBUG_MODUL_WEBBASE, 5, PSTR("error 404") );
  
  sendText ( PSTR("HTTP/1.1 404 Not Found\r\n") );
  sendText ( PSTR("\r\n") );

}

// ==============================================

void sendText ( const char* s ) {
  
  client << s;
  
}

void sendText ( prog_char* s ) {
  
  int len = 0;
  while ( char c = pgm_read_byte ( s++ ) ) client << c;
  
}

void sendNumber ( unsigned long n ) {
  
  client << _DEC ( n );
  
}

void sendLeadingZeroNumber ( unsigned long n ) {
  
  if ( n < 10 ) client <<  "0";
  client << _DEC ( n );
  
}

void sendIPAddress ( IPAddress ip ) {

  for ( int i = 0; i < 3; i++ ) {
    sendNumber ( ip [i] );
    sendText ( PSTR(".") );
  }
  sendNumber ( ip [3] );

}

// ==============================================
  

