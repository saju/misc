/**************************************************************************************

KAPI : A simple java disassembler for Win32 x86 (MS VC++ 6)

Usage : "kapi foo.class". Generates java asm on stdout
        "kapi foo.class 1". Dumps Constant pool and other info to stdout 

License : 

Copyright (c) 2005 Saju R Pillai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


kapi.c - driver. Reads class file into memory. Dumps constant pool to stdout

saju.pillai@gmail.com
*************************************************************************************/




#include <dasm.h>

#ifdef DEBUG
#define D_printf printf
#else
#define D_printf
#endif



u8 _get_long(class *t, u2 idx)
{
  idx--;
  if (idx >= CP_COUNT(t) - 1)
    return 0; /* wtf ? */
  if (CP_TAG(t,idx) == JC_Long) 
    return JC_LONG(t, idx);
  else 
    return 0; /** wtf ? **/
}


u4 _get_int(class *t, u2 idx)
{
  idx--;
  if (idx >= CP_COUNT(t) - 1)
    return 0; /* wtf ? */
  
  if (CP_TAG(t,idx) == JC_Integer)
    return JC_INTEGER(t, idx);
  else return 0;
}

jf _get_float(class *t, u2 idx)
{
  idx--;
  if (idx > CP_COUNT(t) -1)
    return 0.0f;
  if (CP_TAG(t,idx) == JC_Float)
    return JC_FLOAT(t, idx);
  else return 0.0f;
}

jd _get_double(class *t, u2 idx)
{
  idx--;
  if (idx > CP_COUNT(t) -1)
    return 0.0;
  if (CP_TAG(t,idx) == JC_Double)
    return JC_DOUBLE(t, idx);
  else return 0.0;
}

u1 *_get_asciz(class *t, u2 idx)
{
  static u2 recurse = 0;
  
  idx--;
  if (idx >= CP_COUNT(t) - 1) 
    return NULL;

  if (CP_TAG(t,idx) == JC_Utf8) {
    recurse = 0;
    return JC_UTF8_BYTES(t, idx);
  }
  recurse++;
  if (recurse > CP_COUNT(t) -1) {
    recurse = 0;
    return NULL;
  }
  switch (CP_TAG(t,idx)) 
    {
    case JC_String:
    case JC_Class:
      return _get_asciz(t, JC_GEN_1_IDX(t, idx));
      break;

    case JC_Fieldref:
    case JC_Methodref:
    case JC_InterfaceMethodref:
      return concat_Utf8(_get_asciz(t, JC_REF_CLASS_IDX(t, idx)), ".", 
			 _get_asciz(t, JC_REF_NAME_TYPE_IDX(t, idx)));
      break;

    case JC_NameAndType:
      return concat_Utf8(_get_asciz(t, JC_NAME_TYPE_NIDX(t, idx)), ":",
			 _get_asciz(t, JC_NAME_TYPE_DIDX(t, idx)));
      break;

    default:
       break;
    }
  return NULL;
}


u1 *get_super_name(class *t)
{
  return _get_asciz(t, SUPER_CLASS(t));
}

u1 *get_self_name(class *t)
{
  return _get_asciz(t, THIS_CLASS(t));
}

u1 *get_method(class *t, u2 idx)
{
  return _get_asciz(t, idx);
}

void dump_interfaces(class *t)
{
  u2 i, j = INTERFACE_COUNT(t);
  
  if (IS_INTERFACE(ACCESS_FLAGS(t)) || !j) 
    return;

  printf("\nInterfaces implemented :\n");
  for (i = 0; i < j; i++) 
    printf("%s\n", _get_asciz(t, t->interfaces[i]));
}

void dump_methods(class *t)
{
  u2 c;

  printf("\nMethods :%d\n", METHOD_COUNT(t));
  
  for (c = 0; c < METHOD_COUNT(t); c++) {
    u2 d;
    u2 f = METHOD(t,c).access_flags;
    if (IS_PUBLIC(f))
      printf("public ");
    if (IS_PRIVATE(f))
      printf("private ");
    if (IS_PROTECTED(f))
      printf("protected ");
    if (IS_STATIC(f))
      printf("static ");
    if (IS_FINAL(f))
      printf("final ");
    if (IS_SYNCHRONIZED(f))
      printf("synchronized ");
    if (IS_NATIVE(f))
      printf("native ");
    if (IS_ABSTRACT(f))
      printf("abstract ");
    if (IS_STRICT(f))
      printf("strict ");
    printf("%s %s", _get_asciz(t, METHOD(t,c).name_index), 
	   _get_asciz(t, METHOD(t,c).desc_index));
    printf("  :%d\n", METHOD(t,c).attrib_count);
    for (d = 0; d < METHOD(t,c).attrib_count; d++) {
      if (METHOD_ATTRIB(t,c,d).special == ATTRIB_CODE) {
	u2 e;
	printf("max_stack %ld, max_locals %ld, exception handlers %ld\n", 
	       METHOD_CODE(t,c,d)->max_stack, METHOD_CODE(t,c,d)->max_locals, 
	       METHOD_CODE(t,c,d)->except_len);
	for (e = 0; e < METHOD_CODE(t,c,d)->except_len; e++) {
	  if (METHOD_CODE_EXCEPT(t,c,d,e).catch_type == 0)
	    printf("Handled Exception: java.lang.exception\n");
	  printf("Handled Exception : %s\n", 
		 _get_asciz(t, METHOD_CODE_EXCEPT(t,c,d,e).catch_type));
	}
      }
      else if (METHOD_ATTRIB(t,c,d).special == ATTRIB_EXCEPTION) {
	u2 a;
	printf("Throws Exception\n");
	for (a=0; a < METHOD_EXCEPTIONS(t,c,d)->num; a++)
	  printf("%s\n", _get_asciz(t, METHOD_EXCEPT_IDX(t,c,d)[a]));
      }
    }
  }
}
	

void dump_fields(class *t)
{
  u2 i,j;

  printf("\nFields :%d\n", FIELD_COUNT(t));

  for (i = 0; i < FIELD_COUNT(t); i++) {

    if (IS_INTERFACE(ACCESS_FLAGS(t))) { 

      /** all fields of a interface are public static final **/

      printf("public static final ");
      emit_desc(_get_asciz(t, FIELD_DESC_IDX(t, i)));
      printf(" %s;\n", _get_asciz(t, FIELD_NAME_IDX(t, i)));
    }
    else { /** class **/

      u2 fl = FIELD_ACC_FLAG(t, i);

      if (IS_PUBLIC(fl)) 
	printf("public ");
      else if (IS_PRIVATE(fl))
	printf("private ");
      else
	printf("protected ");

      if (IS_VOLATILE(fl))
	printf("volatile ");
      if (IS_FINAL(fl))
	printf("final ");
      if (IS_TRANSIENT(fl))
	printf("transient ");
      if (IS_STATIC(fl))
	printf("static ");
      emit_desc(_get_asciz(t, FIELD_DESC_IDX(t, i)));
      printf(" %s;\n", _get_asciz(t, FIELD_NAME_IDX(t, i)));
    }
    /***
	retrieve field attributes if any
    ***/
    for (j = 0; j < FIELD_ATTRIB_COUNT(t, i); j++)
      {
	if (FIELD_ATTRIB_SPCL(t, i, j) == ATTRIB_VALUE) {
	  emit_desc(_get_asciz(t, FIELD_DESC_IDX(t, i)));
	  switch (field_type(_get_asciz(t, FIELD_DESC_IDX(t, i))))
	    {
	    case CONSTANT_Integer:
	      printf(" const %ld\n", _get_int(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    case CONSTANT_Long:
	      printf(" const %I64d\n",_get_long(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    case CONSTANT_Float:
	      printf(" const %.10f\n", _get_float(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    case CONSTANT_Double:
	      printf(" const %.10f\n", _get_double(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    case CONSTANT_String:
	      printf(" const %s\n", _get_asciz(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    default:
	      printf(" unknown_type %d . cp[%d]\n",_get_asciz(t, FIELD_DESC_IDX(t, i)),
		     _get_asciz(t, (u2)FIELD_ATTRIB_P(t, i, j)));
	      break;
	    }
	}
      }
  }
}
 
void dump_cp(class *t)
{
  u2 i;

  printf("\nConstant Pool Dump...\n");
  
  for (i = 0; i < CP_COUNT(t)-1; i++) {
    printf("const #%d", i + 1);

    switch (CP_TAG(t, i)) {
      
    case JC_Class :
      printf("= class  #%d\n", JC_CLASS_IDX(t,  i));
      break;
    case JC_String:
      printf("= String  #%d\n", JC_STRING_IDX(t, i));
      break;
    case JC_Fieldref :
      printf("= Field  #%d.#%d\n", JC_FIELD_CLASS_IDX(t, i),
	     JC_FIELD_NAME_TYPE_IDX(t, i));
      break;
    case JC_Methodref :
      printf("= Method  #%d.#%d\n", JC_METHOD_CLASS_IDX(t, i),
	     JC_METHOD_NAME_TYPE_IDX(t, i));
      break;
    case JC_InterfaceMethodref :
      printf("= Interface  #%d.#%d\n", JC_INTERFACE_CLASS_IDX(t, i),
	     JC_INTERFACE_NAME_TYPE_IDX(t, i));
      break;
    case JC_Integer:
      printf("= int %ld\n", JC_INTEGER(t, i));
      break;
    case JC_Float: 
      printf("= float %f\n",JC_FLOAT(t, i));
      break;
    case JC_Long:
      printf("= long %I64d\n", JC_LONG(t, i));
      i++;
      break;
    case JC_Double:
      printf("= double %f\n",JC_DOUBLE(t, i));
      i++;
      break;
    case JC_NameAndType:
      printf("= NameAndType #%d:#%d\n", JC_NAME_TYPE_NIDX(t, i),
	     JC_NAME_TYPE_DIDX(t, i));
      break;
    case JC_Utf8:
      printf("= Asciz %s\n", JC_UTF8_BYTES(t, i));
      break;
    default:
      printf("fucked\n");
      break;
    }
  }
}

int read_class(class *t)
{
  u2 i;

  t->magic = read_u4();
  if (t->magic != 0xCAFEBABE) 
    return 0;

  t->minor_version = read_u2();
  t->major_version = read_u2();
  t->constant_pool_count = read_u2();

  t->constant_pool = malloc(sizeof(*t->constant_pool) * t->constant_pool_count);

  for (i = 0; i < t->constant_pool_count-1; i++) {
    u1 tag = read_u1();
    switch( tag )
      {

      case JC_Class :
      case JC_String:
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(struct _jC_Generic_1));
	JC_GEN_1_IDX(t, i) = read_u2();
	break;

      case JC_Fieldref :
      case JC_Methodref :
      case JC_InterfaceMethodref :
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(struct _jC_ref));
	JC_REF_CLASS_IDX(t, i) = read_u2();
	JC_REF_NAME_TYPE_IDX(t, i) = read_u2();
	break;
	
      case JC_Integer:
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(jC_Integer));
	JC_INTEGER(t, i) = read_u4();
	break;

      case JC_Float:{
	u4 temp;
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(jC_Float));
	temp = read_u4();
	JC_FLOAT(t, i) = make_float(&temp);
	JC_FLOAT_SP(t, i) = (u1)temp;
      }
	break;

      case JC_Long: {
	u4 a, b;
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(jC_Long));
	a = read_u4(); b = read_u4();
	JC_LONG(t, i) = make_long(b, a);
	i++;
      }
	break;
      case JC_Double: {
	u4 a, b;
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(jC_Double));
	a = read_u4(); b = read_u4();
	JC_DOUBLE(t, i) = make_double(&a, &b);
	JC_DOUBLE_SP(t, i) = (u1)a;
	i++;
      }
	break;
	
      case JC_NameAndType:
	t->constant_pool[i].tag = tag;
	t->constant_pool[i].j_const_t = malloc(sizeof(jC_NameAndType));
	JC_NAME_TYPE_NIDX(t, i) = read_u2();
	JC_NAME_TYPE_DIDX(t, i) = read_u2();
	break;

      case JC_Utf8:
	{
	  u2 l = read_u2();
	  t->constant_pool[i].tag = tag;
	  t->constant_pool[i].j_const_t = malloc(sizeof(jC_Utf8));
	  JC_UTF8_LEN(t, i) = l;
	  JC_UTF8_BYTES(t, i) = read_Utf8(l);
	  break;
	}

      default :
	printf("BAD CONSTANT %d\n", tag);
	return 0;
	break;
      }
  }

  t->access_flags = read_u2();
  t->this_class = read_u2();
  t->super_class = read_u2();
  t->if_count = read_u2();
  
  if (t->if_count) {
    u2 i_f;
    t->interfaces = malloc(sizeof(u2) * t->if_count);
    for (i_f = 0; i_f < t->if_count; i_f++)
      t->interfaces[i_f] = read_u2();
  } 
  else 
    t->interfaces = NULL;

  t->fields_count = read_u2();
  if (t->fields_count) {
    u2 c;

    t->fields = malloc(sizeof(*t->fields) * t->fields_count);
    for (c = 0; c < t->fields_count; c++)
      {
	u2 d;
	t->fields[c].access_flags = read_u2();
	t->fields[c].name_index = read_u2();
	t->fields[c].descriptor_index = read_u2();
	d = read_u2();
	t->fields[c].attrib_count = d;
	if (d) 
	  {
	    u2 e;
	    t->fields[c].attribs = malloc(sizeof(attrib_info) * d);
	    for (e = 0; e < d; e++) {
	      u4 f;
	      t->fields[c].attribs[e].attrib_name_idx = read_u2();
	      f = read_u4();
	      t->fields[c].attribs[e].attrib_length = f;
	      if (f == 2)  {/** ConstantValue **/
		u2 op = read_u2();
		t->fields[c].attribs[e].p = (void *)0;
		t->fields[c].attribs[e].p = (void *)op; /** sick **/
		t->fields[c].attribs[e].special = ATTRIB_VALUE;
	      }
	      else {
		/******
		 It's ok to discard everything other than a ConstantValue for a field.
		 We shall promptly proceed to do so
		******/
		FIELD_ATTRIB_SPCL(t, c, e) = ATTRIB_DISCARD;
	      }
	    }
	  }
      }
  }

  /*****************
    Get methods
  ******************/
  t->meth_count = read_u2();
  if (t->meth_count) {
    u2 c,d;
    t->methods = malloc(sizeof(method_info) * METHOD_COUNT(t));

    for (c = 0; c < METHOD_COUNT(t); c++) {

      METHOD(t, c).access_flags = read_u2();
      METHOD(t, c).name_index = read_u2();
      METHOD(t, c).desc_index = read_u2();
      METHOD(t, c).attrib_count = read_u2();

      if (METHOD(t, c).attrib_count) 
	METHOD(t, c).attribs = malloc(sizeof(attrib_info) * METHOD(t,c).attrib_count);

      for (d = 0; d < METHOD(t, c).attrib_count; d++) {
	u2 e;

	METHOD_ATTRIB(t,c,d).attrib_name_idx = read_u2();
	METHOD_ATTRIB(t,c,d).attrib_length = read_u4();

	/*************
	   check for Code & Exception. Skip Synthetic and Deprecated
	**************/
	if (!strcmp("Code", _get_asciz(t,METHOD_ATTRIB(t,c,d).attrib_name_idx))) {

	  METHOD_ATTRIB(t,c,d).special = ATTRIB_CODE;
	  METHOD_ATTRIB(t,c,d).p = malloc(sizeof(attrib_code));
	  METHOD_CODE(t,c,d)->max_stack = read_u2();
	  METHOD_CODE(t,c,d)->max_locals = read_u2();
	  METHOD_CODE(t,c,d)->code_length = read_u4();
	  METHOD_CODE(t,c,d)->offset = curr_offset();
	  skip_bytes(METHOD_CODE(t,c,d)->code_length);
	  METHOD_CODE(t,c,d)->except_len = read_u2();

	  if (METHOD_CODE(t,c,d)->except_len) {
	    METHOD_CODE(t,c,d)->exception_table = malloc(sizeof(exception_table) * 
							METHOD_CODE(t,c,d)->except_len);
	    for (e = 0; e < METHOD_CODE(t,c,d)->except_len; e++) {
	      METHOD_CODE_EXCEPT(t,c,d,e).start_pc = read_u2();
	      METHOD_CODE_EXCEPT(t,c,d,e).end_pc = read_u2();
	      METHOD_CODE_EXCEPT(t,c,d,e).handler_pc = read_u2();
	      METHOD_CODE_EXCEPT(t,c,d,e).catch_type = read_u2();
	    }
	  }

	  METHOD_CODE(t,c,d)->attrib_count = read_u2();
	  if (METHOD_CODE(t,c,d)->attrib_count) {
	    for (e = 0; e < METHOD_CODE(t,c,d)->attrib_count; e++) {
	      /***************************
		  Only LineNumberTable & LocalVariableTable exists. They are 
		  not interesting to us.
	      *****************************/
	      read_u2();
	      skip_bytes(read_u4());
	    }
	  }
	}  /*** end of Code **/
	else if (!strcmp("Exceptions",
			 _get_asciz(t,METHOD_ATTRIB(t,c,d).attrib_name_idx))) {
	  
	  METHOD_ATTRIB(t,c,d).special = ATTRIB_EXCEPTION;
	  METHOD_ATTRIB(t,c,d).p = malloc(sizeof(exceptions));
	  METHOD_EXCEPTIONS(t,c,d)->num = read_u2();

	  if (METHOD_EXCEPTIONS(t,c,d)->num) {
	    METHOD_EXCEPT_IDX(t,c,d) = malloc(sizeof(u2) * METHOD_EXCEPTIONS(t,c,d)->num);
	    
	    for (e =0; e < METHOD_EXCEPTIONS(t,c,d)->num; e++) 
	      METHOD_EXCEPT_IDX(t,c,d)[e] = read_u2();
	  }
	} /** end of Exception **/
	else {
	  METHOD_ATTRIB(t,c,d).special = ATTRIB_DISCARD;
	  skip_bytes(METHOD_ATTRIB(t,c,d).attrib_length);
	}
      }
    }
  }

  /**********************************
   Get class wide attribs

   Not required for a JVM - but required for us, if we want to handle inner classes
   too
  ***********************************/
  return 1;
}

int main(int argc, char **argv)
{
  u2 i, j, f;
  class *t = malloc(sizeof *t);

  printf("\n\n");

  if (argc < 2) {
    printf("not enough args");
    exit(-1);
  }

  if (!class_open(argv[1]))
    quit("class_open() failed");

  if (!read_class(t))
    quit("Invalid class file");

  if (argc > 2) {

    f = ACCESS_FLAGS(t);
    
    if (IS_INTERFACE(f)) {
      printf("public interface %s extends %s\n", get_self_name(t),
	     get_super_name(t));
    }
    else {
      if (IS_ABSTRACT(f)) {
	printf("%s abstract class %s extends %s\n", IS_PUBLIC(f) ? "public":"private",
	       get_self_name(t), get_super_name(t));
      }
      else {
	if (IS_FINAL(f))
	  printf("%s final class %s extends %s\n", IS_PUBLIC(f) ? "public":"private",
		 get_self_name(t), get_super_name(t));
	else
	  printf("%s class %s extends %s\n", IS_PUBLIC(f) ? "public":"private",
		 get_self_name(t), get_super_name(t));
      }
    }
    printf("Version %d:%d\n",MAJOR_VERSION(t), MINOR_VERSION(t));

    dump_interfaces(t);
    dump_methods(t);
    dump_fields(t);
    dump_cp(t);
  }
  else 
    emit_asm(t);


}
