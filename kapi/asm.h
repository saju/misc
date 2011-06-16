/**************************************************************************************

KAPI : A simple java disassembler for Win32 x86 (MS VC++ 6)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


asm.h - opcodes are managed here

saju.pillai@gmail.com
*************************************************************************************/

#ifndef ASM_H
#define ASM_H

typedef unsigned char uc;

typedef struct _op op;
struct _op {
  uc mnem;
  u1 ops;
  u1 *name;
  u4 (*sp)(op *, u4);
};

typedef struct _op_entry op_entry;
struct _op_entry{
  u1 type;
  u2 cp_index;
};

u2 sp;

u4 lookup_switch(op *, u4);
u4 table_switch(op *, u4);
u4 wide(op *, u4);
u4 asciz(op *, u4);
u4 bipush(op *, u4);
u4 gotos(op *, u4);
u4 ifs(op *, u4);
u4 pushy(op *, u4);
u4 newarray(op *, u4);
u4 def_asm(op *, u4);

op table[] = {
  {0x32, 0, "aaload", 0},
  {0x53, 0, "aastore", 0},
  {0x01, 0, "aconst_null", 0},
  {0x19, 1, "aload", 0},
  {0x2a, 0, "aload_0", 0},
  {0x2b, 0, "aload_1", 0},
  {0x2c, 0, "aload_2", 0},
  {0x2d, 0, "aload_3", 0},
  {0xbd, 2, "anewarray", asciz},
  {0xb0, 0, "areturn", 0},
  {0xbe, 0, "arraylength", 0},
  {0x3a, 1, "astore", 0},
  {0x4b, 0, "astore_0", 0},
  {0x4c, 0, "astore_1", 0},
  {0x4d, 0, "astore_2", 0},
  {0x4e, 0, "astore_3", 0},
  {0xbf, 0, "athrow", 0},

  {0x33, 0, "baload", 0},
  {0x54, 0, "bastore", 0},
  {0x10, 1, "bipush", bipush},

  {0x34, 0, "caload", 0},
  {0x55, 0, "castore", 0},
  {0xc0, 2, "checkcast", asciz},

  {0x90, 0, "d2f", 0},
  {0x8e, 0, "d2i", 0},
  {0x8f, 0, "d2l", 0},
  {0x63, 0, "dadd", 0},
  {0x31, 0, "daload", 0},
  {0x52, 0, "dastore", 0},
  {0x98, 0, "dcmpg", 0},
  {0x97, 0, "dcmpl", 0},
  {0xe,  0, "dconst_0", 0 },
  {0xf,  0, "dconst_1", 0},
  {0x6f, 0, "ddiv", 0},
  {0x18, 1, "dload", 0},
  {0x26, 0, "dload_0", 0},
  {0x27, 0, "dload_1", 0},
  {0x28, 0, "dload_2", 0},
  {0x29, 0, "dload_3", 0},
  {0x6b, 0, "dmul", 0},
  {0x77, 0, "dneg", 0},
  {0x73, 0, "drem", 0},
  {0xaf, 0, "dreturn", 0},
  {0x39, 1, "dstore", 0},
  {0x47, 0, "dstore_0", 0},
  {0x48, 0, "dstore_1", 0},
  {0x49, 0, "dstore_2", 0},
  {0x4a, 0, "dstore_3", 0},
  {0x67, 0, "dsub", 0},
  {0x59, 0, "dup", 0},
  {0x5a, 0, "dup_x1", 0},
  {0x5b, 0, "dup_x2", 0},
  {0x5c, 0, "dup2", 0},
  {0x5d, 0, "dup2_x1", 0},
  {0x5e, 0, "dup2_x2", 0},

  {0x8d, 0, "f2d", 0},
  {0x8b, 0, "f2i", 0},
  {0x8c, 0, "f2l", 0},
  {0x62, 0, "fadd", 0},
  {0x30, 0, "faload", 0},
  {0x51, 0, "fastore", 0},
  {0x96, 0, "fcmpg", 0},
  {0x95, 0, "fcmpgl", 0},
  {0xb,  0, "fconst_0", 0},
  {0xc,  0, "fconst_1", 0},
  {0xd,  0, "fconst_2", 0},
  {0x6e, 0, "fdiv", 0},
  {0x17, 1, "fload", 0},
  {0x22, 0, "fload_0", 0},
  {0x23, 0, "fload_1", 0},
  {0x24, 0, "fload_2", 0},
  {0x25, 0, "fload_3", 0},
  {0x6a, 0, "fmul", 0},
  {0x76, 0, "fneg", 0},
  {0x72, 0, "frem", 0},
  {0xae, 0, "freturn", 0},
  {0x38, 1, "fstore", 0},
  {0x43, 0, "fstore_0", 0},
  {0x44, 0, "fstore_1", 0},
  {0x45, 0, "fstore_2", 0},
  {0x46, 0, "fstore_3", 0},
  {0x66, 0, "fsub", 0},

  {0xb4, 2, "getfield", asciz},
  {0xb2, 2, "getstatic", asciz},
  {0xa7, 2, "goto", gotos},
  {0xc8, 4, "goto_w", gotos},

  {0x91, 0, "i2b", 0},
  {0x92, 0, "i2c", 0},
  {0x87, 0, "i2d", 0},
  {0x86, 0, "i2f", 0},
  {0x85, 0, "i2l", 0},
  {0x93, 0, "i2s", 0},
  {0x60, 0, "iadd", 0},
  {0x2e, 0, "iaload", 0},
  {0x7e, 0, "iand", 0},
  {0x4f, 0, "istore", 0},
  {0x2,  0, "iconst_ml", 0},
  {0x3,  0, "iconst_0", 0},
  {0x4,  0, "iconst_1", 0},
  {0x5,  0, "iconst_2", 0},
  {0x6,  0, "iconst_3", 0},
  {0x7,  0, "iconst_4", 0},
  {0x8,  0, "iconst_5", 0},
  {0x6c, 0, "idiv", 0},
  {0xa5, 2, "if_acmpeq", ifs},
  {0xa6, 2, "if_acmpne", ifs},
  {0x9f, 2, "if_icmpeq", ifs},
  {0xa0, 2, "if_icmpne", ifs},
  {0xa1, 2, "if_icmplt", ifs},
  {0xa2, 2, "if_icmpge", ifs},
  {0xa3, 2, "if_icmpgt", ifs},
  {0xa4, 2, "if_icmple", ifs},
  {0x99, 2, "ifeq", ifs},
  {0x9a, 2, "ifne", ifs},
  {0x9b, 2, "iflt", ifs},
  {0x9c, 2, "ifge", ifs},
  {0x9d, 2, "ifgt", ifs},
  {0x9e, 2, "ifle", ifs},
  {0xc7, 2, "ifnonnull", ifs},
  {0xc6, 2, "ifnull", ifs},
  {0x84, 2, "iinc", 0},
  {0x15, 1, "iload", 0},
  {0x1a, 0, "iload_0", 0},
  {0x1b, 0, "iload_1", 0},
  {0x1c, 0, "iload_2", 0},
  {0x1d, 0, "iload_3", 0},
  {0x68, 0, "imul", 0},
  {0x74, 0, "ineg", 0},
  {0xc1, 2, "instanceof", asciz},
  {0xb9, 4, "invokeinterface", asciz},
  {0xb7, 2, "invokespecial", asciz},
  {0xb8, 2, "invokestatic", asciz},
  {0xb6, 2, "invokevirtual", asciz},
  {0x80, 0, "ior", 0},
  {0x70, 0, "irem", 0},
  {0xac, 0, "ireturn", 0},
  {0x78, 0, "ishl", 0},
  {0x7a, 0, "ishr", 0},
  {0x36, 1, "istore", 0},
  {0x3b, 0, "istore_0", 0},
  {0x3c, 0, "istore_1", 0},
  {0x3d, 0, "istore_2", 0},
  {0x3e, 0, "istore_3", 0},
  {0x64, 0, "isub", 0},
  {0x7c, 0, "iushr", 0},
  {0x82, 0, "ixor", 0},

  {0xa8, 2, "jsr", gotos},
  {0xc9, 4, "jsr_w", gotos},
  
  {0x8a, 0, "l2d", 0},
  {0x89, 0, "l2f", 0},
  {0x88, 0, "l2i", 0},
  {0x61, 0, "ladd", 0},
  {0x2f, 0, "laload", 0},
  {0x7f, 0, "land", 0},
  {0x50, 0, "lastore", 0},
  {0x94, 0, "lcmp", 0},
  {0x9,  0, "lconst_0", 0},
  {0xa,  0, "lconst_1", 0},
  {0x12, 1, "ldc", pushy},
  {0x13, 2, "ldc_w", pushy},
  {0x14, 2, "ldc2_w", pushy},
  {0x6d, 0, "ldiv", 0},
  {0x16, 1, "lload", 0},
  {0x1e, 0, "lload_0", 0},
  {0x1f, 0, "lload_1", 0},
  {0x20, 0, "lload_2", 0},
  {0x21, 0, "lload_3", 0},
  {0x69, 0, "lmul", 0},
  {0x75, 0, "lneg", 0},
  {0xab, -1, "lookupswitch", lookup_switch},
  {0x81, 0, "lor", 0},
  {0x71, 0, "lrem", 0},
  {0xad, 0, "lreturn", 0},
  {0x79, 0, "lshl", 0},
  {0x7b, 0, "lshr", 0},
  {0x37, 1, "lstore", 0},
  {0x3f, 0, "lstore_0", 0},
  {0x40, 0, "lstore_1", 0},
  {0x41, 0, "lstore_2", 0},
  {0x42, 0, "lstore_3", 0},
  {0x65, 0, "lsub", 0},
  {0x7d, 0, "lushr", 0},
  {0x83, 0, "lxor", 0},

  {0xc2, 0, "monitorenter", 0},
  {0xc3, 0, "monitorexit", 0},
  {0xc5, 3, "multianewarray", asciz},
  
  {0xbb, 2, "new", asciz},
  {0xbc, 2, "newarray", newarray},
  {0x0,  0, "nop", 0},

  {0x57, 0, "pop", 0},
  {0x58, 0, "pop2", 0},
  {0xb5, 2, "putfield", asciz},
  {0xb3, 2, "putstatic", asciz},

  {0xa9, 1, "ret", 0},
  {0xb1, 0, "return", 0},

  {0x35, 0, "saload", 0},
  {0x56, 0, "sastore", 0},
  {0x11, 2, "sipush", 0},
  {0x5f, 0, "swap", 0},
  
  {0xaa, -1, "tableswitch", table_switch},
  
  {0xc4, -1, "wide", wide},

  {0xff, 0, NULL, 0},
};

#endif /* ASM_H */
