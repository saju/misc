/**************************************************************************************

KAPI : A simple java disassembler for Win32 x86 (32bit MS VC++)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


README

saju.pillai@gmail.com
*************************************************************************************/

Kapi : Corruption of the word "coffee". Used throughout South India to describe all
       types & sorts of coffee.

Kapi was initially meant to be a tool for a more ambitious project. Kapi outgrew it's
original purpose. This is just a fun programming project & was never intended to be
anything else !!

kapi will disassemble a class file and try to generate a friendlier o/p than javap.
kapi simply put is the "good twin" of javap.

kapi must be compiled with 32 bit M$VC++ for x86 or similar compiler that provides same primitves as M$ (including __int64)



Bugs:
kapi has no regard for performance or memory. 
kapi doesn't talk in UTF8. (assumes everybody speaks english :) )
kapi doesn't recognize InnerClasses explicitly


Compile:
cl /I. kapi.c emit.c misc.c


Files making up kapi...
kapi.c
emit.c
misc.c
asm.h
dasm.h





