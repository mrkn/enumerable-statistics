#include <ruby/ruby.h>

static VALUE
ary_find_max(VALUE ary)
{
  const long n = RARRAY_LEN(ary);
  if (n == 0) {
    return Qnil;
  }

  long imax = 0;
  VALUE max = RARRAY_AREF(ary, imax);

  long i;
  for (i = 1; i < n; ++i) {
    VALUE v = RARRAY_AREF(ary, i);
    if (RTEST(rb_funcall(v, '>', 1, max))) {
      imax = i;
      max = v;
    }
  }

  return rb_assoc_new(max, LONG2NUM(imax));
}

static VALUE
ary_find_min(VALUE ary)
{
  const long n = RARRAY_LEN(ary);
  if (n == 0) {
    return Qnil;
  }

  long imin = 0;
  VALUE min = RARRAY_AREF(ary, imin);

  long i;
  for (i = 1; i < n; ++i) {
    VALUE v = RARRAY_AREF(ary, i);
    if (RTEST(rb_funcall(v, '<', 1, min))) {
      imin = i;
      min = v;
    }
  }

  return rb_assoc_new(min, LONG2NUM(imin));
}

void
Init_array_extension(void)
{
  VALUE mEnumerableStatistics = rb_const_get_at(rb_cObject, rb_intern("EnumerableStatistics"));
  VALUE mArrayExtension = rb_const_get_at(mEnumerableStatistics, rb_intern("ArrayExtension"));

  rb_undef_method(mArrayExtension, "find_max");
  rb_define_method(mArrayExtension, "find_max", ary_find_max, 0);

  rb_undef_method(mArrayExtension, "find_min");
  rb_define_method(mArrayExtension, "find_min", ary_find_min, 0);
}
