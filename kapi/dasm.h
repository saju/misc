/**************************************************************************************

KAPI : A simple java disassembler for Win32 x86 (MS VC++ 6)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


dasm.h - heart & soul of kapi

saju.pillai@gmail.com
*************************************************************************************/


#ifndef DASM_H
#define DASM_H

#include <stdlib.h> /** for malloc() **/
#include <stdio.h>  

/** For x86 M$VC++, following mappings exist between native and java primitives **/

typedef __int64   u8;    /** 8 Long (lucky) **/
typedef long int  u4;    /** 4 Integer **/
typedef short int u2;    /** 2 Character (int type) **/
typedef char      u1;    /** 1 int type **/

typedef float     jf;    /** 4 Float (IEEE 754) **/
typedef double    jd;    /** 8 Double  **/

/************ Java Types ***************/

#define CONSTANT_Integer  0x01
#define CONSTANT_Double   0x02
#define CONSTANT_Float    0x04
#define CONSTANT_Long     0x08
#define CONSTANT_String   0x10


/************ Java constant pool types - BEGIN ******************/

#define JC_Class 	         7
#define JC_Fieldref 	         9
#define JC_Methodref 	        10
#define JC_InterfaceMethodref 	11
#define JC_String 	         8
#define JC_Integer 	         3
#define JC_Float 	         4
#define JC_Long 	         5
#define JC_Double 	         6
#define JC_NameAndType 	        12
#define JC_Utf8 	         1


typedef struct _jC_Generic_1 jC_Class;
typedef struct _jC_Generic_1 jC_String;
struct _jC_Generic_1 {
  u2 idx;
};

typedef struct _jC_ref jC_Fieldref;
typedef struct _jC_ref jC_Methodref;
typedef struct _jC_ref jC_InterfaceMethodref;
struct _jC_ref {
  u2 class_idx;
  u2 name_type_idx;
};

typedef struct _jC_Integer jC_Integer;
struct _jC_Integer {
  u4 val;
};

typedef struct _jC_Float jC_Float;
struct _jC_Float {
  u1 sp;
  jf val;
};

typedef struct _jC_Long jC_Long;
struct _jC_Long {
  u8 val;
};

typedef struct _jC_Double jC_Double;
struct _jC_Double {
  u1 sp;
  jd val;
};

typedef struct _jC_NameAndType jC_NameAndType;
struct _jC_NameAndType {
  u2 name_idx;
  u2 descriptor_idx;
};

typedef struct _jC_Utf8 jC_Utf8;
struct _jC_Utf8 {
  u2 length;
  u1 *bytes;
};


typedef struct _cp_info cp_info;
struct _cp_info {
  u1 tag;
  void *j_const_t;
};


/********* Macro's for easy retrieval of cp info ***********/

#define JC_GEN_1_IDX(A, I) ((struct _jC_Generic_1 *)((A)->constant_pool[(I)].j_const_t))->idx
#define JC_CLASS_IDX(A, I) JC_GEN_1_IDX(A, I)
#define JC_STRING_IDX(A, I) JC_GEN_1_IDX(A, I)

#define JC_REF_CLASS_IDX(A,I) ((struct _jC_ref *)((A)->constant_pool[(I)].j_const_t))->class_idx
#define JC_REF_NAME_TYPE_IDX(A,I) ((struct _jC_ref *)((A)->constant_pool[(I)].j_const_t))->name_type_idx
#define JC_FIELD_CLASS_IDX(A,I) JC_REF_CLASS_IDX(A, I)
#define JC_METHOD_CLASS_IDX(A,I) JC_REF_CLASS_IDX(A, I)
#define JC_INTERFACE_CLASS_IDX(A,I) JC_REF_CLASS_IDX(A, I)
#define JC_FIELD_NAME_TYPE_IDX(A,I) JC_REF_NAME_TYPE_IDX(A, I)
#define JC_METHOD_NAME_TYPE_IDX(A,I) JC_REF_NAME_TYPE_IDX(A, I)
#define JC_INTERFACE_NAME_TYPE_IDX(A,I) JC_REF_NAME_TYPE_IDX(A, I)

#define JC_INTEGER(A, I) ((jC_Integer *)((A)->constant_pool[(I)].j_const_t))->val

#define JC_FLOAT(A, I) ((jC_Float *)((A)->constant_pool[(I)].j_const_t))->val
#define JC_FLOAT_SP(A, I) ((jC_Float *)((A)->constant_pool[(I)].j_const_t))->sp

#define JC_LONG(A, I) (((jC_Long *)((A)->constant_pool[(I)].j_const_t))->val)

#define JC_DOUBLE(A, I) (((jC_Double *)((A)->constant_pool[(I)].j_const_t))->val)
#define JC_DOUBLE_SP(A, I) ((jC_Double *)((A)->constant_pool[(I)].j_const_t))->sp

#define JC_NAME_TYPE_NIDX(A, I) ((jC_NameAndType *)((A)->constant_pool[(I)].j_const_t))->name_idx
#define JC_NAME_TYPE_DIDX(A, I) ((jC_NameAndType *)((A)->constant_pool[(I)].j_const_t))->descriptor_idx

#define JC_UTF8_LEN(A, I) ((jC_Utf8 *)((A)->constant_pool[(I)].j_const_t))->length
#define JC_UTF8_BYTES(A, I) ((jC_Utf8 *)((A)->constant_pool[(I)].j_const_t))->bytes


    
/********* Java Access Flags  *************/

#define ACC_PUBLIC       0x0001
#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_FINAL        0x0010
#define ACC_SYNCHRONIZED 0x0020
#define ACC_VOLATILE     0x0040
#define ACC_TRANSIENT    0x0080
#define ACC_NATIVE       0x0100
#define ACC_INTERFACE    0x0200
#define ACC_ABSTRACT     0x0400
#define ACC_STRICT       0x0800
#define ACC_SUPER        0x0020 /* yes ! */

/******** Java Exception Table **********/

typedef struct _exception_table exception_table;
struct _exception_table {
  u2 start_pc;
  u2 end_pc;
  u2 handler_pc;
  u2 catch_type;
};

typedef struct _exceptions exceptions;
struct _exceptions {
  u2 num;
  u2 *except_index ;
};

/********* Java Attribute description *********/

typedef struct _attrib_info attrib_info;
struct _attrib_info {
  u2 attrib_name_idx;
  u4 attrib_length;
  u1 special;
  void *p;
};

typedef struct _attrib_code attrib_code;
struct _attrib_code {
  u2 max_stack;
  u2 max_locals;
  u4 code_length;
  u4 offset; 
  u2 except_len;
  exception_table *exception_table;
  u2 attrib_count;
  attrib_info *attribs;
};

#define ATTRIB_DISCARD    0x0001
#define ATTRIB_VALUE      0x0002
#define ATTRIB_CODE       0x0004
#define ATTRIB_EXCEPTION  0x0008

#define ATTRIB(A, I)      (A)->attribs[(I)]

/********* Java Field INFO Struct *************/

typedef struct _field_info field_info;
struct _field_info {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attrib_count;
  attrib_info *attribs;
};

#define FIELD_COUNT(A) (A)->fields_count
#define FIELD_ACC_FLAG(A, I)  (A)->fields[(I)].access_flags
#define FIELD_NAME_IDX(A, I)  (A)->fields[(I)].name_index
#define FIELD_DESC_IDX(A, I)  (A)->fields[(I)].descriptor_index
#define FIELD_ATTRIB_COUNT(A,I) (A)->fields[(I)].attrib_count
#define FIELD_ATTRIB_NIDX(A, I, J) (A)->fields[(I)].attribs[(J)].attrib_name_idx
#define FIELD_ATTRIB_LEN(A, I, J) (A)->fields[(I)].attribs[(J)].attrib_length
#define FIELD_ATTRIB(A, I, J) (A)->fields[(I)].attribs[(J)]
#define FIELD_ATTRIB_P(A, I, J) ((A)->fields[(I)].attribs[(J)].p)
#define FIELD_ATTRIB_SPCL(A, I, J) FIELD_ATTRIB(A,I,J).special

/********** Java Method INFO Struct *************/
typedef struct _method_info method_info;
struct _method_info {
  u2 access_flags;
  u2 name_index;
  u2 desc_index;
  u2 attrib_count;
  attrib_info *attribs;
};

#define METHOD_COUNT(A) (A)->meth_count
#define METHOD(A, I) (A)->methods[(I)]
#define METHOD_ATTRIB(A, I, J) METHOD(A, I).attribs[(J)]
#define METHOD_CODE(A,I,J) ((attrib_code *)(METHOD_ATTRIB(A,I,J).p))
#define METHOD_CODE_EXCEPT(A,I,J,K) METHOD_CODE(A,I,J)->exception_table[(K)]
#define METHOD_EXCEPTIONS(A,I,J) ((exceptions *)(METHOD_ATTRIB(A,I,J).p))
#define METHOD_EXCEPT_IDX(A,I,J) METHOD_EXCEPTIONS(A,I,J)->except_index

/********* Misc useful Macro's ************/
#define INTERFACE_COUNT(A)  (A)->if_count
#define MAJOR_VERSION(A)    (A)->major_version
#define MINOR_VERSION(A)    (A)->minor_version
#define ACCESS_FLAGS(A)     ((A)->access_flags)
#define CP_COUNT(A)         (A)->constant_pool_count
#define ATTRIB_COUNT        (A)->attrib_count

#define CP_TAG(A, I)        (A)->constant_pool[(I)].tag


#define IS_PUBLIC(A)        ((A & ACC_PUBLIC) ? 1 : 0)
#define IS_PRIVATE(A)       ((A & ACC_PRIVATE) ? 1 : 0)
#define IS_PROTECTED(A)     ((A & ACC_PROTECTED) ? 1 : 0)
#define IS_STATIC(A)        ((A & ACC_STATIC) ? 1 : 0)
#define IS_SYNCHRONIZED(A)  ((A & ACC_SYNCHRONIZED) ? 1 : 0)
#define IS_VOLATILE(A)      ((A & ACC_VOLATILE) ? 1 : 0)
#define IS_TRANSIENT(A)     ((A & ACC_TRANSIENT) ? 1 : 0)
#define IS_NATIVE(A)        ((A & ACC_NATIVE) ? 1 : 0)
#define IS_INTERFACE(A)     ((A & ACC_INTERFACE) ? 1 : 0)
#define IS_ABSTRACT(A)      ((A & ACC_ABSTRACT) ? 1 : 0)
#define IS_FINAL(A)         ((A & ACC_FINAL) ? 1 : 0)
#define IS_SUPER_SPECIAL(A) ((A & ACC_SUPER) ? 1 : 0)
#define IS_STRICT(A)        ((A & ACC_STRICT) ? 1 : 0)

#define THIS_CLASS(A)       (A)->this_class
#define SUPER_CLASS(A)      (A)->super_class

typedef struct _class class;
struct _class {
  u4 magic;
  u2 minor_version;
  u2 major_version;
  u2 constant_pool_count;
  cp_info *constant_pool;
  u2 access_flags;
  u2 this_class;
  u2 super_class;
  u2 if_count;
  u2 *interfaces;
  u2 fields_count;
  field_info *fields;
  u2 meth_count;
  method_info *methods;
  u2 attrib_count;
  attrib_info *attribs; // unused right now
};

unsigned char read_uc();
u1 read_u1();
u2 read_u2();
u4 read_u4();
u2 xchg_2(u2 i);
u4 xchg_4(u4 i);
u1 *read_Utf8(u4 i);
u1 *concat_Utf8(u1 *a, u1 *b, u1 *c);
void emit_desc(u1 *a);
u8 make_long(u4 a, u4 b);
jf make_float(u4 *a);
jd make_double(u4 *a, u4 *b);
u1 *_get_asciz(class *, u2);
u4 _get_int(class *, u2);
u8 _get_long(class *, u2);
jf _get_float(class *, u2);
jd _get_double(class *, u2);
u1 *_get_asciz(class *, u2);

#endif /* DASM_H */
