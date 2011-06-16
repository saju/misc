/**************************************************************************************

KAPI : A simple java disassembler for Win32 x86 (MS VC++ 6)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


misc.c - Type conversions, file reading & other misc functions.

saju.pillai@gmail.com
*************************************************************************************/

#include <dasm.h>
#include <math.h>

/*** swap endianess of a 2 byte quantity ***/

u2 xchg_2(u2 n)
{
  _asm
    {
      mov ax, n
      xchg ah, al
    }	
}

/************************************
swap endianess for a 4 byte quantity

M$VC++ compiler was generating junk for the regular htonl() code.
bswap is available for 486 and above and takes only 1 to 3 cycles for swaps
*************************************/
u4 xchg_4(u4 l)
{
  _asm
    {
      mov eax, l
      bswap eax
    }
}


/********************************************************************
Convert Java Long to Native Long 

((u8)a << 32) + b is *way* slow on my 32bit x86 M$VC++ box. Lack of a 64 bit register
forces the compiler to generate asm that looks like

load MSB into eax
cdq (make it (u8))
load 32 into ecx
call __allshl (left shift all)
save result in mem (returned in ecx & esi)
load LSB into eax
cdq
add (4 bytes)
adc (next 4 bytes)
save result.

Since the MSB and LSB are available to us on the stack one after the other, we can 
just ask the compiler to treat them as a single __int64 unit and return it - *simple*
and verydi verydi fast.
*********************************************************************/

u8 make_long(u4 a, u4 b)
{
  return *(u8 *)&a; // a must be LSB - we are little endian remember.
}


/*********************************************************************
Convert Java Float to Native Float

Java Float's are encoded in the IEEE 754 floating point single format.
The passed in u4 is set appropriately to 
0 -> No error
1 -> Positive_Infinity
2 -> Negative_Infinity
3 -> NaN
**********************************************************************/
jf make_float(u4 *a)
{
  if (*a == 0x7F800000) {
    *a = 1;
    return 0.0f;
  }
  else if (*a == 0xFF800000) {
    *a = 2;
    return 0.0f;
  }
  else if (((*a >= 0x7f800001) && (*a <= 0x7fffffff)) || 
	   ((*a >= 0xff800001) && (*a <= 0xffffffff))) {
      *a = 3;
      return 0.0f;
    }
  else {
    u4 s = ((*a >> 31) == 0) ? 1 : -1; // sign bit
    u4 e = ((*a >> 23) & 0xff); // exponent
    u4 m = (e == 0) ? (*a & 0x7fffff) << 1 : (*a & 0x7fffff) | 0x800000; // mantissa
    *a = 0;
    return s * m * (float)pow((float)2.0f, e - 150);
  }
}


/*************************************************************************
Convert Java Double to Native Double

Java Double's are encoded in IEEE 754 floating point double format
The first passed in u4 is modified appropriately to 
0 -> No error
1 -> Positive_Infinity
2 -> Negative_Infinity
3 -> NaN
**********************************************************************/
jd make_double(u4 *i, u4 *j)
{
  u8 a = make_long(*j , *i);

  if (a ==0x7ff0000000000000L ) {
    *i = 1;
    return 0.0;
  }
  else if (a == 0xfff0000000000000L) {
    *i = 2;
    return 0.0;
  }
  else if (((a >= 0x7ff0000000000001i64) && (a <= 0x7fffffffffffffffi64)) || 
	   ((a >= 0xfff0000000000001i64) && (a <= 0xffffffffffffffffi64))) {
      *i = 3;
      return 0.0;
    }
  else {
    u4 s = ((a >> 63) == 0) ? 1 : -1; // sign bit
    u4 e = (u4)((a >> 52) & 0x7ffL);
    u8 m = (e == 0) ? (a & 0xfffffffffffffi64) << 1 : 
      (a & 0xfffffffffffffi64) | 0x10000000000000i64; // mantissa
    *i = 0;
    return s * m * pow(2.0, e - 1075);
  }
}

int field_type(u1 *desc)
{
  u2 l = strlen(desc);
  u1 *p = desc + l -1;

  if (*p == 'B'|| *p == 'C' || *p =='Z' || *p == 'S' || *p == 'I')
    return CONSTANT_Integer;
  else if (*p == 'D')
    return CONSTANT_Double;
  else if (*p == 'F')
    return CONSTANT_Float;
  else if (*p == 'J')
    return CONSTANT_Long;
  else
    return CONSTANT_String;
}

void emit_desc(u1 *desc)
{
  u2 l = strlen(desc);
  u1 *p = desc + l -1;

  if (*p == 'B')
    printf("byte");
  else if (*p == 'C')
    printf("char");
  else if (*p == 'D')
    printf("double");
  else if (*p == 'F')
    printf("float");
  else if (*p == 'J')
    printf("long");
  else if (*p == 'S')
    printf("short");
  else if (*p == 'Z')
    printf("boolean");
  else if (*p == 'I')
    printf("int");
  else if (*p == ';') {
    for (p = desc; *p != ';'; p++) {
      if (*p == '[' || *p == 'L')
	continue;
      else if (*p == '/')
	putchar('.');
      else
	putchar(*p); 
    }
    for (p = desc; *p != 'L'; p++);
  }

  if (p != desc) {
    for (; p != desc; p--)
      printf("[]");
  }
}

FILE *cf;

int class_open(char *file)
{
  cf = fopen(file, "rb");
  if (!cf)
    return 0;
  else 
    return 1;
}

u4 read_p_u4()
{
  u4 m;

  fread(&m, sizeof(u4), 1, cf);
  return m;
}
u4 read_u4()
{
  u4 m;

  fread(&m, sizeof(u4), 1, cf);
  return xchg_4(m);
}

u2 read_u2()
{
  u2 m = 0;
  int x;
  
  x = fread(&m, sizeof(u2), 1, cf);
  return xchg_2(m);
}

u1 read_u1()
{
  u1 m;

  fread(&m, sizeof(u1), 1, cf);
  return m;
}

unsigned char read_uc()
{
  unsigned char c;
  
  fread(&c, sizeof(c), 1, cf);
  return c;
}

u4 curr_offset()
{
  return ftell(cf);
}

void skip_bytes(u4 bytes)
{
  fseek(cf, bytes, SEEK_CUR);
}

void seek_to(u4 bytes) 
{
  fseek(cf, bytes, SEEK_SET);
}



/** 
FIXME : I am dumb.
English is 1 byte encoding in UTF8 - assume we are dealing with english speaking 
people only for now :D
**/
u1 *read_Utf8(u4 l)
{
  u1 *buff ;

  if (!l)
    return NULL;

  buff = malloc(l+1);

  fread(buff, sizeof(u1), l, cf);
  buff[l] = '\0';
  return buff;
}

/**
me too
**/
int print_Utf8(u1 *b, u2 l)
{  
  u2 i;

  return printf("%s",b);
}

u1 *concat_Utf8(u1 *a, u1 *b, u1 *c)
{
  u1 *l; 
  
  if (!a || !b || !c) 
    return NULL;

  l = malloc(strlen(a) + strlen(b) + strlen(c)+1);
  sprintf(l, "%s%s%s", a, b, c);
  return l;
}


void quit(char *msg)
{
  perror(msg);
  exit(-1);
}
