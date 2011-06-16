/********************************************************************************

KAPI : A simple java disassembler for Win32 x86 (MS VC++ 6)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


emit.c - Pretty printing code. Has no regard for performance and leaks worse than a sieve

saju.pillai@gmail.com
***************************************************************************************/


#include <dasm.h>
#include <asm.h>
#include <stdarg.h>

class *t;

static u2 rec = 0;
static u1 in_line = 0;
static u4 code_start;

void emit_special(char *fmt,...) 
{
  char buff[4096];
  va_list list;
  u2 i; 

  va_start(list, fmt);
  vsprintf(buff, fmt, list);
  va_end(list);

  printf("%s", buff);
  fflush(NULL);
}


void emit(char *fmt,...) 
{
  char buff[4096];
  va_list list;
  u2 i; 

  va_start(list, fmt);
  vsprintf(buff, fmt, list);
  va_end(list);

  if (!in_line) {
      in_line = 1;
      for (i = 0; i < 3 * rec; i++)
	putc(' ', stdout);
    }
  printf("%s", buff);
  fflush(NULL);
}

void emit_space()
{
  emit(" ");
}

void emit_line()
{
  in_line = 0;
  putc('\n', stdout);
}

void emit_block_start()
{
  emit("{");
  emit_line();
  rec++;
}

void emit_block_end()
{
  emit_line();
  rec--;
  emit("}");
}

void emit_slash_dot(u1 *p, u4 l) 
{
  u2 i;

  for (i = 0; i < l; i++) {
    if (*(p+i) == '/')
      emit(".");
    else
      emit("%c", *(p+i));
  }
  emit_space();
}

void emit_ascii_idx(u2 idx)
{
  u1 *q, *p=_get_asciz(t, idx);
  u4 l = strlen(p);
  u2 i;

  q = malloc(l + 1);
  strcpy(q, p);

  emit_slash_dot(q, l);

  free(q);
}

void emit_interfaces()
{
  u2 i, j = INTERFACE_COUNT(t);
  
  if (IS_INTERFACE(ACCESS_FLAGS(t)) || !j) 
    return;

  emit("implements ");

  for (i = 0; i < j; i++) {
    emit_ascii_idx(t->interfaces[i]);
    if ((j - i) > 1) 
      putc(',', stdout);
  }
}

void emit_public(u2 f)
{
  if (IS_PUBLIC(f))
    emit("public ");
}

void emit_private(u2 f)
{
  if (IS_PRIVATE(f))
    emit("private ");
}

void emit_final(u2 f)
{
  if (IS_FINAL(f))
    emit("final ");
}

void emit_static(u2 f)
{
  if (IS_STATIC(f))
    emit("static ");
}

void emit_volatile(u2 f)
{
  if (IS_VOLATILE(f))
    emit("volatile ");
}

void emit_transient(u2 f)
{
  if (IS_TRANSIENT(f))
    emit("transient ");
}

void emit_abstract(u2 f)
{
  if (IS_ABSTRACT(f))
    emit("abstract ");
}

void emit_protected(u2 f)
{
  if (IS_PROTECTED(f))
    emit("protected ");
}

void emit_strict(u2 f)
{
  if (IS_STRICT(f))
    emit("strict ");
}

void emit_native(u2 f)
{
  if (IS_NATIVE(f))
    emit("native ");
}

void emit_synchronized(u2 f)
{
  if (IS_SYNCHRONIZED(f))
    emit("synchronized ");
}

void emit_type(u2 f)
{
  if (IS_INTERFACE(f))
    emit("interface ");
  else
    emit("class ");
}

void emit_self_name()
{
  emit_ascii_idx(THIS_CLASS(t));
}

void emit_super()
{
  emit("extends ");
  emit_ascii_idx(SUPER_CLASS(t));
}

void emit_class_flag()
{
  u2 f = ACCESS_FLAGS(t);

  emit_public(f); emit_private(f); emit_final(f); emit_abstract(f);
  emit_type(f);
}

void emit_field_flags(u2 f)
{
  emit_public(f); emit_private(f); emit_final(f); emit_volatile(f);
  emit_static(f); emit_transient(f); emit_protected(f);
}

u1 *get_primitive(u1 *p)  
{
  // r must be malloc'd and large enough 

  if (*p == 'B')
    return "byte";
  else if (*p == 'C')
    return "char";
  else if (*p == 'D')
    return "double";
  else if (*p == 'F')
     return "float";
  else if (*p == 'J')
    return "long";
  else if (*p == 'S')
    return "short";
  else if (*p == 'Z')
    return "boolean";
  else if (*p == 'I')
    return "int";
  else if (*p == 'V')
    return "void";
  else
    return " FUCKED ";
}

u1 *get_complex(u1 *q)
{
  u1 *p = malloc(strlen(q) + 1);
  u2 i;

  strcpy(p, q);
  for (i = 0; p[i] != ';' && p[i] != '\0'; i++) {
    if (p[i] == '/')
      p[i] = '.';
  }
  p[i] = '\0';
  return p;
}


void emit_field_type(u2 i) 
{
  u1 *q = _get_asciz(t, i);
  u2 l = strlen(q);
  u1 *p = q + l -1;


  if (*p  != ';')
    emit(get_primitive(p));

  else {
    for (p = q; *p != 'L'; p++);
    emit(get_complex(p+1));
    for (p = q; *p != 'L'; p++);
    }

  if (p != q) {
    for (; p != q; p--)
      emit("[]");
  }
}

void emit_field_name(u2 i)
{
  emit_field_type(FIELD_DESC_IDX(t, i));
  emit_space();
  emit_ascii_idx(FIELD_NAME_IDX(t, i));
}

void emit_field_value(u2 i)
{
  u2 c;
  u2 j = FIELD_ATTRIB_COUNT(t, i);
  if (!j)
    return;

  for (c = 0; c < j; c++) {
    if (FIELD_ATTRIB_SPCL(t, i, c) == ATTRIB_VALUE) {

      switch (field_type(_get_asciz(t, FIELD_DESC_IDX(t, i))))
	    {
	    case CONSTANT_Integer:
	      emit("= %ld ", _get_int(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    case CONSTANT_Long:
	      emit("= %I64dl ",_get_long(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    case CONSTANT_Float:
	      emit("= %.10ff ", _get_float(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    case CONSTANT_Double:
	      emit("= %.10fd ", _get_double(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    case CONSTANT_String:
	      emit("= \"%s\" ", _get_asciz(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    default:
	      emit(" unknown_type %d . cp[%d]\n ",_get_asciz(t, FIELD_DESC_IDX(t, i)),
		     _get_asciz(t, (u2)FIELD_ATTRIB_P(t, i, c)));
	      break;
	    }
    }
  }
}


void emit_method_flags(u2 f)
{
  emit_public(f); emit_private(f); emit_protected(f); emit_static(f);
  emit_final(f); emit_synchronized(f); emit_native(f); emit_abstract(f);
  emit_strict(f);
}

u1 *frob_method_in(u1 *desc)
{
  u2 l = strlen(desc);
  u1 *p = malloc(l * 3);
  u1 *q;

  strcpy(p, "(");

  for (q = desc+1; *q != ')'; q++) {
    u1 br;
    for (br = 1; *q == '['; br++, q++);
    if (*q != 'L')
      strcat(p, get_primitive(q));
    else {
      strcat(p, get_complex(q+1));
      for (; *q != ';'; q++);
    }
    while(--br) 
      strcat(p, "[]");
    if (*(q + 1) != ')')
      strcat(p, ", ");
  }
  strcat(p, ")");
  return p;
}

u1 *frob_method_out(u1 *desc) 
{
  u2 l = strlen(desc);
  u1 *p = malloc(l * 3);
  u1 *q = desc + l - 1;

  if (*q != ';') 
    strcpy(p, get_primitive(q));
  else {
    for (; *q != ')'; q--); 
    for (; *q != 'L'; q++);

    strcpy(p, get_complex(q+1)); // leak
  }
  for (--q; *q == '['; --q)
    strcat(p, "[]");
  
  return p;
}

u1 *frob_qualified(u1 *desc)
{
  u2 l = strlen(desc), i;
  u1 *p = malloc(l * 3);
  u1 *q = p;
  

  for (i = 0, q = desc; *q != ':'; q++, i++) 
    p[i] = *q == '/' ? '.': *q;
  
  q = frob_method_in(++q);
  strcat(p, q);
  
  return p;

}

void emit_method_name(u2 i)
{
  u1 *name = _get_asciz(t, METHOD(t, i).name_index);
  u1 *desc = _get_asciz(t, METHOD(t, i).desc_index);
  u1 *out, *in;

  out = frob_method_out(desc);
  in =  frob_method_in(desc);

  if (!strcmp(name, "<init>")) {
    emit("%s ",out); emit_self_name(); emit(in);
  }
  else if (!strcmp(name, "<clinit>")) {
     emit("%s ",out); emit(name); emit(in);
     emit(" /* class initialization */");
  }
  else {
    emit("%s ", out); emit(name);  emit(in);
  }

  free(out);
  free(in);
}

void emit_method_exception(u2 c) 
{
  u2 d;
  u1 first_E = 1;

  for (d = 0; d < METHOD(t, c).attrib_count; d++) {
    if (METHOD_ATTRIB(t,c,d).special == ATTRIB_EXCEPTION) {
      u2 a;
      u1 *b;

      for (a=0; a < METHOD_EXCEPTIONS(t,c,d)->num; a++) {
	emit("%s ", first_E ? " throws":",");
	b = _get_asciz(t, METHOD_EXCEPT_IDX(t,c,d)[a]);
	emit_slash_dot(b, strlen(b));
	first_E = 0;
      }
    }
  }
}


void frob_field(u1 *desc)
{
  u2 l = strlen(desc), i = 0;
  u1 *p = desc;
  
  while(*p++ != ':')
    i++;

  while(*p == '[')
    p++;

  if (*p != 'L')
    emit("(%s",get_primitive(p));
  else
    emit("(%s",get_complex(p+1));

  p = desc + i + 1;

  while (*p == '[') {
    p++;
    emit("[]");
  }
  emit(")");
  desc[i] = '\0'; // mem is returned from concat() always - so safe to screw
  emit(" "); emit_slash_dot(desc, strlen(desc));
}

op *lookup(uc code) 
{
  u2 i;

  for (i = 0; table[i].mnem != 0xff; i++)
    if (table[i].mnem == code)
      return &table[i];
  return NULL;
}

u4 lookup_switch(op *w, u4 cat)
{
  u4 curr = curr_offset() - code_start;
  u1 c = 0, i;
  u4 d[4];
  signed long npairs, default_v, key, val;

  while ((curr + c) % 4) {
    c++;
    read_u1();
  }

  d[0] = read_uc(); d[1] = read_uc(); d[2] = read_uc(); d[3] = read_uc();
  default_v = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];

  d[0] = read_uc(); d[1] = read_uc(); d[2] = read_uc(); d[3] = read_uc();
  npairs = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];

  emit_line();
  
  emit_block_start();
  emit("default : #%ld", default_v);
  emit_line();
  emit("Jump Table -->");
  emit_line();
  
  for (curr = 0; curr < npairs; curr++) {

    d[0] = read_uc(); d[1] = read_uc(); d[2] = read_uc(); d[3] = read_uc();
    key = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
    d[0] = read_uc(); d[1] = read_uc(); d[2] = read_uc(); d[3] = read_uc();
    val = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];

    emit("%4ld : #%ld", key, val+cat);
    emit_line();
  }
  emit_block_end();
  return (2 * 4 * npairs + 4 + 4 + c);
}

u4 table_switch(op *w, u4 cat) 
{
   u4 curr = curr_offset() - code_start;
   u1 c = 0;
   u4 l[4], h[4];
   signed long high, low, default_v;

   while ((curr + c) % 4) {
     c++;
     read_u1();
   }
   
   l[0] = read_uc(); l[1] = read_uc(); l[2] = read_uc(); l[3] = read_uc();
   default_v = (l[0] << 24) | (l[1] << 16) | (l[2] << 8) | l[3];

   l[0] = read_uc(); l[1] = read_uc(); l[2] = read_uc(); l[3] = read_uc();
   low = (l[0] << 24) | (l[1] << 16) | (l[2] << 8) | l[3];

   h[0] = read_uc(); h[1] = read_uc(); h[2] = read_uc(); h[3] = read_uc();
   high = (h[0] << 24) | (h[1] << 16) | (h[2] << 8) | h[3];

   emit_line();
  
   emit_block_start();
   emit("default : #%ld", default_v);
   emit_line();
   emit("Jump Table -->");
   emit_line();

   for (curr = 0; curr < (high - low + 1) ; curr++) {
     l[0] = read_uc(); l[1] = read_uc(); l[2] = read_uc(); l[3] = read_uc();
     default_v = (l[0] << 24) | (l[1] << 16) | (l[2] << 8) | l[3];
     emit("%4ld : #%ld", curr, default_v + cat);
     emit_line();
   }
   emit_block_end();
   return ((high - low + 1)*4 + 4 + 4 + 4 + c);
}

u4 wide(op *w, u4 cat)
{
  u1 mn;

  mn = read_u1(); read_u1(); read_u1();
  if (mn == 0x84) { 
    read_u1(); read_u1(); 
    return 5;
  }
  return 3;
}

u4 newarray(op *w, u4 cat) 
{
  uc a = read_uc();

  switch(a) {
  case 4:
    emit (" // boolean array");
    break;
  case 5:
    emit (" // char array");
    break;
    case 6:
    emit (" // float array");
    break;
   case 7:
    emit (" // double array"); 
    break;
  case 8:
    emit (" // byte array");
    break;
   case 9:
    emit (" // short array");
    break;
    case 10:
    emit (" // int array");
    break;
   case 11:
     emit (" // long array");
     break;
  }
  emit("[stack[(int)1]]");
  return 1;
}

u4 asciz(op *w, u4 cat) 
{
  u2 a, b;
  u1 *c;
  
  a = read_uc();
  b = read_uc();

  a = (a << 8) | b;
  c = _get_asciz(t, a);

  if (w->mnem == 0xbd) { 
    u1 *d = c;

    for (; *d == '['; d++);
    if (*(d+1) == '\0')
      d = get_primitive(d);
    else {
      if (*d == 'L')
	d++;
      d = get_complex(d);
    }

    emit(" // a new array of %s", d);

    for (d = c; *d == '['; d++)
      emit("[]");

    return 2;
  }
  else if (w->mnem == 0xc5) {
    uc e = read_uc();
    u1 *d = c;

    for (; *d == '['; d++);
    if (*(d+1) == '\0')
      d = get_primitive(d);
    else {
      if (*d == 'L')
	d++;
      d = get_complex(d);
    }

    emit(" // a new array of %s", d);

    for (d = c; *d == '['; d++)
      emit("[]");
    
    return 3;
  }

  else if (w->mnem == 0xc0)
    emit(" // check if object is of type ");

  else if (w->mnem == 0xb4 || w->mnem == 0xb2 || w->mnem == 0xb5 || w->mnem == 0xb3) {
    emit(" // "), frob_field(c);
    return 2;
  }

  else if (w->mnem == 0xc1)
    emit(" // is stack[(ref)1] an instance of ");

  else if (w->mnem == 0xb9 || w->mnem == 0xb7 || w->mnem == 0xb8 || w->mnem == 0xb6) {

    u1 *out = frob_method_out(c);
    u1 *in = frob_qualified(c);

    emit(" // %s %s", out, in);
    if (w->mnem == 0xb9) {
      read_uc(); read_uc();
      return 4;
    }
    return 2;
  }

  else if (w->mnem == 0xbb) 
    emit(" // ");

  emit_slash_dot(c, strlen(c));
  return 2;
}

u4 gotos(op *w, u4 cat) 
{
  u4 a, b, c, d;
  signed short e;
  u4 f;

  a = read_uc();
  b = read_uc();
  
  if (w->mnem == 0xa7 || w->mnem == 0xa8) {
    e = (a << 8) | b;
    emit("  #%ld", e + cat);
    return 2;
  }

  else if (w->mnem == 0xc8 || w->mnem == 0xc9) {
    c = read_uc(); d = read_uc();
    f = (a << 24) | (b << 16) | (c << 8) | d;
    
    emit("  #%ld", f + cat);
    return 4;
  }

  return 1;
}

u4 pushy(op *w, u4 cat) 
{
  u2 a = read_uc();

  if (w->mnem == 0x13 || w->mnem == 0x14) {
    u2 b  = read_uc();
    a = (a << 8) | b;
  }

  --a;

  switch (CP_TAG(t, a)) {
  case JC_String:
    emit("  // (String) \"%s\"", _get_asciz(t, JC_STRING_IDX(t, a)));
    break;
  case JC_Integer:
    emit("  // (int) %ld", JC_INTEGER(t, a));
    break;
  case JC_Float: 
    emit(" // (float) %f", JC_FLOAT(t, a));
    break;
  case JC_Long:
    emit(" // (long) %I64d", JC_LONG(t, a));
    break;
  case JC_Double:
    emit(" //(double) %f", JC_FLOAT(t, a));
    break;
  default:
    emit("// unknown");
    break;
  }
  return 1;
}

u4 ifs(op *w, u4 cat) 
{
  u4 a, b, c, d, e;
  u2 f;
  
  a = read_uc();
  b = read_uc();

  f = a << 8 | b;

  if (w->mnem == 0xa5) 
    emit(" // if stack[(ref)1] == stack[(ref)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa6) 
    emit(" // if stack[(ref)1] != stack[(ref)2] goto #%ld", f + cat);
  else if (w->mnem == 0x9f)
    emit(" // if stack[(int)1] == stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa0) 
    emit(" // if stack[(int)1] != stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa1)
    emit(" // if stack[(int)1] < stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa2) 
    emit(" // if stack[(int)1] >= stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa3)
    emit(" // if stack[(int)1] > stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0xa4)
    emit(" // if stack[(int)1] <= stack[(int)2] goto #%ld", f + cat);
  else if (w->mnem == 0x99)
    emit(" // if stack[(int)1] == 0 goto #%ld", f + cat);
  else if (w->mnem == 0x9a) 
    emit(" // if stack[(int)1] != 0 goto #%ld", f + cat);
  else if (w->mnem == 0x9b)
    emit(" // if stack[(int)1] < 0 goto #%ld", f + cat);
  else if (w->mnem == 0x9c) 
    emit(" // if stack[(int)1] >= 0 goto #%ld", f + cat);
  else if (w->mnem == 0x9d)
    emit(" // if stack[(int)1] > 0 goto #%ld", f + cat);
  else if (w->mnem == 0x9e)
    emit(" // if stack[(int)1] <= 0 goto #%ld", f + cat);
  else if (w->mnem == 0xc6)
    emit(" // if stack[(ref)1] is null goto #%ld", f + cat);
  else if (w->mnem == 0xc7)
    emit(" // if stack[(ref)1] is not null goto #%ld", f + cat);
  return 2;
}

u4 bipush(op *w, u4 cat) 
{
  emit(" %d  // immediate assign", (signed int)read_u1());

  return 1;
}

u4 def_asm(op *w, u4 cat)
{
  u1 i;
  u4 a[8], f;
  
  for (i = 0; i < w->ops; i++)
    a[i] = read_uc();
  
  if (w->ops == 1)
    emit("  %ld", a[0]);

  return w->ops;
}

void emit_try(u2 c, u2 d, u4 i)
{
  u2 e;

  for (e = 0; e < METHOD_CODE(t,c,d)->except_len; e++) {
    if (METHOD_CODE_EXCEPT(t,c,d,e).start_pc == i) {
      emit_line();
      emit("try ");
      emit_block_start();
    }
  }
}

void emit_catch (u2 c, u2 d, u4 i) 
{
  u2 e;

  for (e = 0; e < METHOD_CODE(t,c,d)->except_len; e++) {
    if (METHOD_CODE_EXCEPT(t,c,d,e).end_pc == i) {
      u1 *a;

      if (METHOD_CODE_EXCEPT(t,c,d,e).catch_type)
	a = _get_asciz(t, METHOD_CODE_EXCEPT(t,c,d,e).catch_type);
      else
	a = "Any Exception";

      emit_block_end();
      emit(" catch ( ");
      emit_slash_dot(a, strlen(a));
      emit("#%ld )", METHOD_CODE_EXCEPT(t,c,d,e).handler_pc);
      emit_line();
    }
  }
}

void emit_opcode(u1 *asm, u2 c, u2 d, u4 i) 
{
  u2 e;

  for (e = 0; e < METHOD_CODE(t,c,d)->except_len; e++) {
    if (METHOD_CODE_EXCEPT(t,c,d,e).handler_pc == i) {
      emit_special("Exception Handler #%ld", i);
      emit_line();
      break;
    }
  }
  emit_special("#%-6ld", i);
  emit(asm);
}

void emit_jasm(u4 len, u4 off, u2 c, u2 d)
{
  u4 i, k;

  seek_to(off);

  code_start = off;

  for (i = 0; i < len; i++) {
    op *curr;
    uc code = read_u1();
    
    curr = lookup(code);
    if (!curr) 
      quit("Bad asm");
    
    emit_try(c, d, i);
    emit_opcode(curr->name, c, d, i);

    if (!curr->sp) 
      k = def_asm(curr, i);
    else 
      k = curr->sp(curr, i);
    
    emit_catch(c, d, i);
   
    i += k;
    emit_line();
  }
}
  

void emit_code(u2 c)
{
  u2 d;

  for (d = 0; d < METHOD(t,c).attrib_count; d++) {
    if (METHOD_ATTRIB(t,c,d).special == ATTRIB_CODE) {
      emit_jasm(METHOD_CODE(t,c,d)->code_length, METHOD_CODE(t,c,d)->offset, c, d);
    }
  }
}

void emit_methods()
{
  u2 c;

  for (c=0; c < METHOD_COUNT(t); c++) {
    emit_method_flags(METHOD(t,c).access_flags);
    emit_method_name(c);
    emit_method_exception(c);
    emit_line();
    emit_block_start();
    emit_code(c);
    emit_block_end();
    emit_line();
    emit_line();
  }
}


void emit_global_fields()
{
  u2 i;

  for (i = 0; i < FIELD_COUNT(t); i++) {
    emit_field_flags(FIELD_ACC_FLAG(t, i));
    emit_field_name(i);
    emit_field_value(i);
    emit(";");
    emit_line();
  }
  emit_line();
}

void emit_class_decl()
{
  emit_class_flag(); emit_self_name(); emit_super(); emit_interfaces();
}
 
int emit_asm(class *cT)
{
  t = cT;

  emit_class_decl();
  emit_line();
  emit_block_start();
  emit_line();
  emit_global_fields();
  emit_methods();
  emit_block_end();
  return 1;
}




