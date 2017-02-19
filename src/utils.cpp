/**
 ******************************************************************************
 * @Filename:	utils.cpp
 * @Project: 	loraRC
 * @Author: 	Jose Barros
 * @Copyright (C) 2017 Jose Barros
 * @Email:  	josemanuelbarros@gmail.com
 *****************************************************************************/
/*
 * @License:
 * This file is part of loraRC.
 * loraRC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loraRC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with loraRC.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "utils.h"

Utils *Utils::mutils = NULL;
String Utils::temp = "";
FILE Utils::serial_stdout = FILE();
Utils::Utils() {
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;
}

void Utils::printfInit() {
  if(mutils == NULL) {
    mutils = new Utils();
  }
}

int Utils::serial_putchar(char c, FILE* f) {
  if (c == '\n') serial_putchar('\r', f);
  return Serial.write(c) == 1 ? 0 : 1;
}

unsigned long Utils::getUsSince(unsigned long value) {
  unsigned long timenow = micros();
  if(timenow < value) {
    return 0xFFFF - value + timenow;
  }
  else return value - timenow;
}

unsigned long Utils::getmsSince(unsigned long value) {
  unsigned long timenow = millis();
  if(timenow < value) {
    return 0xFFFF - value + timenow;
  }
  else return value - timenow;
}

void Utils::printDelayed(String str) {
  Utils::temp += str;
}

void Utils::handlePrintDelayed() {
  if(Utils::temp == "")
    return;
  Serial.println(temp);
  temp = "";
}
#if (MEMORYPROFILE)
#include <memoryfree.h>

extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
// RAMEND and SP seem to be available without declaration here

int16_t ramSize=0;   // total amount of ram available for partitioning
int16_t dataSize=0;  // partition size for .data section
int16_t bssSize=0;   // partition size for .bss section
int16_t heapSize=0;  // partition size for current snapshot of the heap section
int16_t stackSize=0; // partition size for current snapshot of the stack section
int16_t freeMem1=0;  // available ram calculation #1
int16_t freeMem2=0;  // available ram calculation #2

//* This function places the current value of the heap and stack pointers in the
// * variables. You can call it from any place in your code and save the data for
// * outputting or displaying later. This allows you to check at different parts of
// * your program flow.
// * The stack pointer starts at the top of RAM and grows downwards. The heap pointer
// * starts just above the static variables etc. and grows upwards. SP should always
// * be larger than HP or you'll be in big trouble! The smaller the gap, the more
// * careful you need to be. Julian Gall 6-Feb-2009.
// *
uint8_t *heapptr, *stackptr;
uint16_t diff=0;
void Utils::check_mem() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}

void Utils::memrep()                     // run over and over again
{
  Serial.print("\n\n--------------------------------------------");
  Serial.print("\n\nmemory report [");
  Serial.print( freeMemory() );
  Serial.print("] (bytes) must be > 0 for no heap/stack collision");

  Serial.print("\n\nSP should be larger than HP!");

  check_mem();

  Serial.print("\nheapptr=[0x"); Serial.print( (int) heapptr, HEX); Serial.print("] (growing upward, "); Serial.print( (int) heapptr, DEC); Serial.print(" decimal)");

  Serial.print("\nstackptr=[0x"); Serial.print( (int) stackptr, HEX); Serial.print("] (growing downward, "); Serial.print( (int) stackptr, DEC); Serial.print(" decimal)");

  Serial.print("\ndifference should be positive: diff=stackptr-heapptr, diff=[0x");
  diff=stackptr-heapptr;
  Serial.print( (int) diff, HEX); Serial.print("] (which is ["); Serial.print( (int) diff, DEC); Serial.print("] (bytes decimal)");
#if 0
  // ---------------- Print memory profile -----------------
  Serial.print("\n\n__data_start=[0x"); Serial.print( (int) &__data_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__data_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__data_end=[0x"); Serial.print((int) &__data_end, HEX ); Serial.print("] which is ["); Serial.print( (int) &__data_end, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__bss_start=[0x"); Serial.print((int) & __bss_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__bss_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__bss_end=[0x"); Serial.print( (int) &__bss_end, HEX ); Serial.print("] which is ["); Serial.print( (int) &__bss_end, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__heap_start=[0x"); Serial.print( (int) &__heap_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__heap_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__malloc_heap_start=[0x"); Serial.print( (int) __malloc_heap_start, HEX ); Serial.print("] which is ["); Serial.print( (int) __malloc_heap_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__malloc_margin=[0x"); Serial.print( (int) &__malloc_margin, HEX ); Serial.print("] which is ["); Serial.print( (int) &__malloc_margin, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__brkval=[0x"); Serial.print( (int) __brkval, HEX ); Serial.print("] which is ["); Serial.print( (int) __brkval, DEC); Serial.print("] bytes decimal");

  Serial.print("\nSP=[0x"); Serial.print( (int) SP, HEX ); Serial.print("] which is ["); Serial.print( (int) SP, DEC); Serial.print("] bytes decimal");

  Serial.print("\nRAMEND=[0x"); Serial.print( (int) RAMEND, HEX ); Serial.print("] which is ["); Serial.print( (int) RAMEND, DEC); Serial.print("] bytes decimal");

  // summaries:
  ramSize   = (int) RAMEND       - (int) &__data_start;
  dataSize  = (int) &__data_end  - (int) &__data_start;
  bssSize   = (int) &__bss_end   - (int) &__bss_start;
  heapSize  = (int) __brkval     - (int) &__heap_start;
  stackSize = (int) RAMEND       - (int) SP;
  freeMem1  = (int) SP           - (int) __brkval;
  freeMem2  = ramSize - stackSize - heapSize - bssSize - dataSize;
  Serial.print("\n--- section size summaries ---");
  Serial.print("\nram   size=["); Serial.print( ramSize, DEC ); Serial.print("] bytes decimal");
  Serial.print("\n.data size=["); Serial.print( dataSize, DEC ); Serial.print("] bytes decimal");
  Serial.print("\n.bss  size=["); Serial.print( bssSize, DEC ); Serial.print("] bytes decimal");
  Serial.print("\nheap  size=["); Serial.print( heapSize, DEC ); Serial.print("] bytes decimal");
  Serial.print("\nstack size=["); Serial.print( stackSize, DEC ); Serial.print("] bytes decimal");
  Serial.print("\nfree size1=["); Serial.print( freeMem1, DEC ); Serial.print("] bytes decimal");
  Serial.print("\nfree size2=["); Serial.print( freeMem2, DEC ); Serial.print("] bytes decimal");

  Serial.println();
#endif
}
#endif
