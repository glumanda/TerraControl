//
// calculate fre Memory
//

extern int __bss_end;
extern void* __brkval;

// ==============================================
#if VERBOSE

int getFreeMem () {
  
  int freeVal = 0;
  
  if ( (int) __brkval == 0 ) {
    freeVal = ((int) &freeVal) - ((int) &__bss_end);
  }
  else {
    freeVal = ((int) &freeVal) - ((int) &__brkval);
  }
  
  return freeVal;

}

#endif

// ==============================================


