//
// Ntp
//

const int DEBUG_MODUL_NTP = B00001000;

//static byte SNTP_server_IP[]    = { 192, 168, 2, 8 }; // ntpd@glutexo

//byte SNTP_server_IP[]    = { 192, 43, 244, 18};   // time.nist.gov
byte SNTP_server_IP[]    = { 130,149,17,21};      // ntps1-0.cs.tu-berlin.de xxx
//byte SNTP_server_IP[]    = { 192,53,103,108};     // ptbtime1.ptb.de
//byte SNTP_server_IP[]    = { 64, 90, 182, 55 };   // nist1-ny.ustiming.org
//byte SNTP_server_IP[]    = { 66, 27, 60, 10 };    // ntp2d.mcc.ac.uk
//byte SNTP_server_IP[]    = { 130, 88, 200, 4 };   // ntp2c.mcc.ac.uk
//byte SNTP_server_IP[]    = { 31, 193, 9, 10 };    // clock02.mnuk01.burstnet.eu 
//byte SNTP_server_IP[]    = { 82, 68, 133, 225 };  // ntp0.borg-collective.org.uk

const unsigned long SEVENTY_YEARS = 2208988800UL; // offset between ntp and unix time
const unsigned long OFFSET_CEST = 7200L;          // offset in sec for MESZ
const unsigned long OFFSET_CET = 3600L;           // offset in sec for MEZ

const unsigned long SYNC_INTERVAL = SECS_PER_HOUR;
//const unsigned long SYNC_INTERVAL = 15UL;

// ==============================================

EthernetUDP udp;

void initNtp () {

#ifndef DEBUG_OFFLINE
  udp.begin ( 8888 ); // Zeitserver
#endif

  if ( VERBOSE) dbg ( DEBUG_MODUL_NTP, 5, PSTR("waiting for sync ") );
  setSyncInterval ( SYNC_INTERVAL );
  setSyncProvider ( getNtpTime );
//  while ( timeStatus () == timeNotSet ) {
//  while ( timeStatus () != timeSet ) {
//    dbg ( DEBUG_MODUL_NTP, 5, PSTR(".") );  // wait until the time is set by the sync provider
//  }
//  dbgln ( DEBUG_MODUL_NTP, 5, PSTR("") );
  
}

// ==============================================

// input t = CT (greenwich time)
boolean isCEST ( unsigned long t ) {
  
  boolean isCest = false;

  int m = month ( t );
  switch ( m ) {

    // CET
    case 1:       
    case 2:       
    case 11:       
    case 12:       
      isCest = false; 
      break;
      
    // CEST
    case 4:
    case 5:
    case 6: 
    case 7: 
    case 8: 
    case 9:   
      isCest = true; 
      break;
      
    // CEST -> CES / CES -> CEST
    case 10:
    case 3:
      isCest = (m==10); // Anfang Oktober ist Sommerzeit / Anfang MÃ¤rz ist Winterzeit
      if ( day ( t ) > 24 ) { // wir sind in der letzten Woche des Monats inkl. letztem Sonntag
        if ( weekday ( t ) + 31 - day ( t ) < 8 ) { // wir sind am So oder danach; 8 ist der kleinste Wert
          isCest = !isCest; // fast schon Winterzeit / Sommerzeit
          if ( weekday ( t ) == 1 && hour ( t ) < 1 ) { // es ist Sonntag und noch nicht 2 Uhr Ortszeit!
              isCest = !isCest; // aber doch noch ein bischen Sommerzeit / Winterzeit
          }
        }
      }
      break;
    
    default:
      dbgln ( DEBUG_MODUL_NTP, 0, PSTR("GROSSER Fehler in isCest () m="), m );
    
  }
  
  return isCest;
  
}

// ==============================================
/*-------- NTP code ----------*/

unsigned long getNtpTime () {
  
  if ( VERBOSE) dbgln ( DEBUG_MODUL_NTP, 5, PSTR("getNtpTime") );

#ifdef DEBUG_OFFLINE
  return 1L;
#endif
  
  sendNTPpacket ( SNTP_server_IP );

  delay ( 1000 );
  
  unsigned long time = recvNtpTime ();
  if ( time != 0L ) {
    time -= SEVENTY_YEARS;
//    time += (isCEST ( time ) ? OFFSET_CEST : OFFSET_CET); // erst hier ist time initial gesetzt
    if ( isCEST ( time ) ) {
      time += OFFSET_CEST;
    }
    else {
      time += OFFSET_CET;
    }
    
    dataLog.lastUpdateNtp = time;
  }
      
  return time;

}

const int NTP_PACKET_SIZE = 48;                   // NTP time stamp is in first 48 bytes of message
byte packetBuffer [NTP_PACKET_SIZE];              // buffer to hold incoming(outgoing packets

void sendNTPpacket ( byte *address ) {

  memset ( packetBuffer, 0, NTP_PACKET_SIZE );
  
  // Init for NTP Request
  packetBuffer [0] = B11100011; // LI, Version, Mode 0xE3
  packetBuffer [1] = 0; // Stratum
  packetBuffer [2] = 6; // max intervall between messages in sec
  packetBuffer [3] = 0xEC; // clock precision
  // bytes 4 - 11 are for root delay ad dispersion and were set to 0 by memset
  packetBuffer [12] = 49; // four byte reference id
  packetBuffer [13] = 0x4E;
  packetBuffer [14] = 49;
  packetBuffer [15] = 52;
  
  // send the packet requesting a timestamp
  // port 123
  udp.beginPacket ( address, 123 );
  udp.write ( packetBuffer, NTP_PACKET_SIZE );
  udp.endPacket ();
  
}

unsigned long recvNtpTime () {
    
  if ( udp.parsePacket () ) {
    
    udp.read ( packetBuffer, NTP_PACKET_SIZE );
    
    // the time starts at byte 40, convert four bytes into long
    unsigned long hi = word ( packetBuffer [40], packetBuffer [41] );
    unsigned long lo = word ( packetBuffer [42], packetBuffer [43] );
    
    // this is NTP time (seconds since Jan 1 1900
    unsigned long secsSince1900 = hi << 16 | lo;
    
    return secsSince1900;
    
  }

  return 0L; // return 0 if unable to get the time

}

// ==============================================


