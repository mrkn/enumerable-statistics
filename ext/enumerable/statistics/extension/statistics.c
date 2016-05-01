#include <ruby/ruby.h>
#include <ruby/version.h>
#include <assert.h>

#if RUBY_API_VERSION_CODE >= 20400
/* for 2.4.0 or higher */
# define HAVE_ARRAY_SUM
# undef HAVE_RB_FIX_PLUS
# undef HAVE_RB_RATIONAL_PLUS
#elif RUBY_API_VERSION_CODE >= 20200
/* for 2.3.0 and 2.2.0 */
# undef HAVE_ARRAY_SUM
# undef HAVE_RB_FIX_PLUS
# undef HAVE_RB_RATIONAL_PLUS
#endif

#ifndef HAVE_TYPE_STRUCT_RRATIONAL
struct RRational {
    struct RBasic basic;
    const VALUE num;
    const VALUE den;
};
#endif

#ifndef RRATIONAL
# define RRATIONAL(obj) (R_CAST(RRational)(obj))
#endif

#ifndef RRATIONAL_SET_NUM
# define RRATIONAL_SET_NUM(rat, n) RB_OBJ_WRITE((rat), &((struct RRational *)(rat))->num,(n))
#endif

#ifndef RRATIONAL_SET_DEN
# define RRATIONAL_SET_DEN(rat, d) RB_OBJ_WRITE((rat), &((struct RRational *)(rat))->den,(d))
#endif

#ifndef HAVE_TYPE_STRUCT_RCOMPLEX
struct RComplex {
    struct RBasic basic;
    const VALUE real;
    const VALUE imag;
};
#endif

#ifndef RCOMPLEX
# define RCOMPLEX(obj) (R_CAST(RComplex)(obj))
#endif

#ifndef RCOMPLEX_SET_REAL
# define RCOMPLEX_SET_REAL(cmp, r) RB_OBJ_WRITE((cmp), &((struct RComplex *)(cmp))->real,(r))
#endif

#ifndef RCOMPLEX_SET_IMAG
# define RCOMPLEX_SET_IMAG(cmp, i) RB_OBJ_WRITE((cmp), &((struct RComplex *)(cmp))->imag,(i))
#endif

#ifndef MUL_OVERFLOW_SIGNED_INTEGER_P
# define MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, min, max) ( \
  (a) == 0 ? 0 : \
  (a) == -1 ? (b) < -(max) : \
  (a) > 0 ? \
    ((b) > 0 ? (max) / (a) < (b) : (min) / (a) > (b)) : \
    ((b) > 0 ? (min) / (a) < (b) : (max) / (a) > (b)))
#endif

#ifndef MUL_OVERFLOW_LONG_P
# define MUL_OVERFLOW_LONG_P(a, b) MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, LONG_MIN, LONG_MAX)
#endif

static ID idPow, idPLUS, idMINUS, idSTAR, id_eqeq_p, id_idiv, id_negate, id_to_f, id_cmp;
static ID id_each, id_real_p;

inline static VALUE
f_add(VALUE x, VALUE y)
{
  if (FIXNUM_P(y) && FIX2LONG(y) == 0)
    return x;
  else if (FIXNUM_P(x) && FIX2LONG(x) == 0)
    return y;
  return rb_funcall(x, idPLUS, 1, y);
}

inline static VALUE
f_sub(VALUE x, VALUE y)
{
  if (FIXNUM_P(y) && FIX2LONG(y) == 0)
    return x;
  return rb_funcall(x, idMINUS, 1, y);
}

inline static VALUE
f_real_p(VALUE x)
{
  if (FIXNUM_P(x) || RB_TYPE_P(x, T_BIGNUM) || RB_TYPE_P(x, T_RATIONAL) || RB_FLOAT_TYPE_P(x))
    return Qtrue;
  else if (RB_TYPE_P(x, T_COMPLEX))
    return Qfalse;
  else
    return rb_funcall(x, id_real_p, 0);
}

#ifndef HAVE_RB_FIX_PLUS
static VALUE
complex_new(VALUE klass, VALUE real, VALUE imag)
{
  assert(!RB_TYPE_P(real, T_COMPLEX));

  NEWOBJ_OF(obj, struct RComplex, klass, T_COMPLEX | (RGENGC_WB_PROTECTED_COMPLEX ? FL_WB_PROTECTED : 0));

  RCOMPLEX_SET_REAL(obj, real);
  RCOMPLEX_SET_IMAG(obj, imag);

  return (VALUE)obj;
}

static VALUE
complex_caonicalize_new(VALUE klass, VALUE real, VALUE imag)
{
  if (f_real_p(real) && f_real_p(imag))
    return complex_new(klass, real, imag);
  else if (f_real_p(imag)) {
    VALUE new_imag;

    new_imag = f_add(RCOMPLEX(real)->imag, imag);

    return complex_new(klass, RCOMPLEX(real)->real, new_imag);
  }
  else {
    VALUE new_real, new_imag;

    new_real = f_sub(RCOMPLEX(real)->real, RCOMPLEX(imag)->imag);
    new_imag = f_add(RCOMPLEX(real)->imag, RCOMPLEX(imag)->real);

    return complex_new(klass, new_real, new_imag);
  }
}

static VALUE
complex_add(VALUE self, VALUE other)
{
  if (RB_TYPE_P(other, T_COMPLEX)) {
    VALUE real, imag;

    real = f_add(RCOMPLEX(self)->real, RCOMPLEX(other)->real);
    imag = f_add(RCOMPLEX(self)->imag, RCOMPLEX(other)->imag);

    return complex_new(CLASS_OF(self), real, imag);
  }
  else if (rb_obj_is_kind_of(other, rb_cNumeric) && RTEST(f_real_p(other))) {
    VALUE real;

    real = f_add(RCOMPLEX(self)->real, other);

    return complex_new(CLASS_OF(self), real, RCOMPLEX(other)->imag);
  }
  return rb_num_coerce_bin(self, other, idPLUS);
}

static VALUE
rb_fix_plus(VALUE x, VALUE y)
{
  if (FIXNUM_P(y)) {
    long a, b, c;
    VALUE r;

    a = FIX2LONG(x);
    b = FIX2LONG(y);
    c = a + b;
    r = LONG2NUM(c);

    return r;
  }
  else if (RB_TYPE_P(y, T_BIGNUM)) {
    return rb_big_plus(y, x);
  }
  else if (RB_FLOAT_TYPE_P(y)) {
    return DBL2NUM((double)FIX2LONG(x) + RFLOAT_VALUE(y));
  }
  else if (RB_TYPE_P(y, T_COMPLEX)) {
    return complex_add(y, x);
  }
  else {
    return rb_num_coerce_bin(x, y, '+');
  }
}
#endif

#ifndef HAVE_RB_RATIONAL_PLUS
# define ZERO INT2FIX(0)
# define ONE  INT2FIX(1)

# define f_boolcast(x) ((x) ? Qtrue : Qfalse)
#define rb_raise_zerodiv() rb_raise(rb_eZeroDivError, "divided by 0")

inline static VALUE
f_cmp(VALUE x, VALUE y)
{
  if (FIXNUM_P(x) && FIXNUM_P(y)) {
    long c = FIX2LONG(x) - FIX2LONG(y);
    if (c > 0)
      c = 1;
    else if (c < 0)
      c = -1;
    return INT2FIX(c);
  }
  return rb_funcall(x, id_cmp, 1, y);
}

inline static VALUE
f_negative_p(VALUE x)
{
  if (FIXNUM_P(x))
    return f_boolcast(FIX2LONG(x) < 0);
  return rb_funcall(x, '<', 1, ZERO);
}

inline static VALUE
f_zero_p(VALUE x)
{
  if (FIXNUM_P(x)) {
    return f_boolcast(FIX2LONG(x) == 0);
  }
  else if (RB_TYPE_P(x, T_BIGNUM)) {
    return Qfalse;
  }
  else if (RB_TYPE_P(x, T_RATIONAL)) {
    VALUE num = RRATIONAL(x)->num;
    return f_boolcast(FIXNUM_P(num) && FIX2LONG(num) == 0);
  }
  return rb_funcall(x, id_eqeq_p, 1, ZERO);
}

inline static VALUE
f_negate(VALUE x)
{
  return rb_funcall(x, id_negate, 0);
}

inline static VALUE
f_to_f(VALUE x)
{
  if (RB_TYPE_P(x, T_STRING))
    return DBL2NUM(rb_str_to_dbl(x, 0));
  return rb_funcall(x, id_to_f, 0);
}

inline static long
i_gcd(long x, long y)
{
  if (x < 0)
    x = -x;
  if (y < 0)
    y = -y;

  if (x == 0)
    return y;
  if (y == 0)
    return x;

  while (x > 0) {
    long t = x;
    x = y % x;
    y = t;
  }
  return y;
}

inline static VALUE
f_imul(long a, long b)
{
  VALUE r;

  if (a == 0 || b == 0)
    return ZERO;
  else if (a == 1)
    return LONG2NUM(b);
  else if (b == 1)
    return LONG2NUM(a);

  if (MUL_OVERFLOW_LONG_P(a, b))
    r = rb_big_mul(rb_int2big(a), rb_int2big(b));
  else
    r = LONG2NUM(a * b);

  return r;
}

inline static VALUE
f_mul(VALUE x, VALUE y)
{
  if (FIXNUM_P(y)) {
    long iy = FIX2LONG(y);
    if (iy == 0) {
      if (FIXNUM_P(x) || RB_TYPE_P(x, T_BIGNUM))
        return ZERO;
    }
    else if (iy == 1)
      return x;
  }
  if (FIXNUM_P(x)) {
    long ix = FIX2LONG(x);
    if (ix == 0) {
      if (FIXNUM_P(y) || RB_TYPE_P(y, T_BIGNUM))
        return ZERO;
    }
    else if (ix == 1)
      return y;
  }
  return rb_funcall(x, '*', 1, y);
}

inline static VALUE
f_idiv(VALUE x, VALUE y)
{
  return rb_funcall(x, id_idiv, 1, y);
}

inline static VALUE
f_mod(VALUE x, VALUE y)
{
  return rb_funcall(x, '%', 1, y);
}

inline static VALUE
f_gcd_normal(VALUE x, VALUE y)
{
  VALUE z;

  if (FIXNUM_P(x) && FIXNUM_P(y))
    return LONG2NUM(i_gcd(FIX2LONG(x), FIX2LONG(y)));

  if (f_negative_p(x))
    x = f_negate(x);
  if (f_negative_p(y))
    y = f_negate(y);

  if (f_zero_p(x))
    return y;
  if (f_zero_p(y))
    return x;

  for (;;) {
    if (FIXNUM_P(x)) {
      if (FIX2LONG(x) == 0)
        return y;
      if (FIXNUM_P(y))
        return LONG2NUM(i_gcd(FIX2LONG(x), FIX2LONG(y)));
    }
    z = x;
    x = f_mod(y, x);
    y = z;
  }
  /* NOTREACHED */
}

inline static VALUE
f_gcd(VALUE x, VALUE y)
{
  return f_gcd_normal(x, y);
}

inline static VALUE
nurat_s_new_internal(VALUE klass, VALUE num, VALUE den)
{
  NEWOBJ_OF(obj, struct RRational, klass, T_RATIONAL | (RGENGC_WB_PROTECTED_RATIONAL ? FL_WB_PROTECTED : 0));
  RRATIONAL_SET_NUM(obj, num);
  RRATIONAL_SET_DEN(obj, den);
  return (VALUE)obj;
}

inline static VALUE
nurat_s_canonicalize_internal_no_reduce(VALUE klass, VALUE num, VALUE den)
{
  switch (FIX2INT(f_cmp(den, ZERO))) {
    case -1:
      num = f_negate(num);
      den = f_negate(den);
      break;
    case 0:
      rb_raise_zerodiv();
      break;
  }

  return nurat_s_new_internal(klass, num, den);
}

inline static VALUE
f_addsub(VALUE self, VALUE anum, VALUE aden, VALUE bnum, VALUE bden, int k)
{
  VALUE num, den;

  if (FIXNUM_P(anum) && FIXNUM_P(aden) && FIXNUM_P(bnum) && FIXNUM_P(bden)) {
    long an = FIX2LONG(anum);
    long ad = FIX2LONG(aden);
    long bn = FIX2LONG(bnum);
    long bd = FIX2LONG(bden);
    long ig = i_gcd(ad, bd);

    VALUE g = LONG2NUM(ig);
    VALUE a = f_imul(an, bd / ig);
    VALUE b = f_imul(bn, ad / ig);

    VALUE c;
    if (k == '+')
      c = f_add(a, b);
    else
      c = f_sub(a, b);

    b = f_idiv(aden, g);
    g = f_gcd(c, g);
    num = f_idiv(c, g);
    a = f_idiv(bden, g);
    den = f_mul(a, b);
  }
  else {
    VALUE g = f_gcd(aden, bden);
    VALUE a = f_mul(anum, f_idiv(bden, g));
    VALUE b = f_mul(bnum, f_idiv(aden, g));

    VALUE c;
    if (k == '+')
      c = f_add(a, b);
    else
      c = f_sub(a, b);

    b = f_idiv(aden, g);
    g = f_gcd(c, g);
    num = f_idiv(c, g);
    a = f_idiv(bden, g);
    den = f_mul(a, b);
  }

  return nurat_s_canonicalize_internal_no_reduce(CLASS_OF(self), num, den);
}

static VALUE
rb_rational_plus(VALUE self, VALUE other)
{
  if (RB_TYPE_P(other, T_FIXNUM) || RB_TYPE_P(other, T_BIGNUM)) {
    VALUE num = RRATIONAL(self)->num;
    VALUE den = RRATIONAL(self)->den;

    return f_addsub(self, num, den, other, ONE, idPLUS);
  }
  else if (RB_TYPE_P(other, T_FLOAT)) {
    return f_add(f_to_f(self), other);
  }
  else if (RB_TYPE_P(other, T_RATIONAL)) {
    VALUE anum = RRATIONAL(self)->num;
    VALUE aden = RRATIONAL(self)->den;
    VALUE bnum = RRATIONAL(other)->num;
    VALUE bden = RRATIONAL(other)->den;

    return f_addsub(self, anum, aden, bnum, bden, '+');
  }
  else {
    return rb_num_coerce_bin(self, other, idPLUS);
  }
}
#endif

#ifndef HAVE_ARRAY_SUM
static VALUE
ary_sum(int argc, VALUE* argv, VALUE ary)
{
  VALUE e, v, r;
  long i, n;
  int block_given;

  if (rb_scan_args(argc, argv, "01", &v) == 0)
    v = LONG2FIX(0);

  block_given = rb_block_given_p();

  if (RARRAY_LEN(ary) == 0)
    return v;

  n = 0;
  r = Qundef;
  for (i = 0; i < RARRAY_LEN(ary); i++) {
    e = RARRAY_AREF(ary, i);
    if (block_given)
      e = rb_yield(e);
    if (FIXNUM_P(e)) {
      n += FIX2LONG(e); /* should not overflow long type */
      if (!FIXABLE(n)) {
        v = rb_big_plus(LONG2NUM(n), v);
        n = 0;
      }
    }
    else if (RB_TYPE_P(e, T_BIGNUM))
      v = rb_big_plus(e, v);
    else if (RB_TYPE_P(e, T_RATIONAL)) {
      if (r == Qundef)
        r = e;
      else
        r = rb_rational_plus(r, e);
    }
    else
      goto not_exact;
  }

  if (n != 0)
    v = rb_fix_plus(LONG2FIX(n), v);
  if (r != Qundef)
    v = rb_rational_plus(r, v);
  return v;

not_exact:
  if (n != 0)
    v = rb_fix_plus(LONG2FIX(n), v);
  if (r != Qundef)
    v = rb_rational_plus(r, v);

  if (RB_FLOAT_TYPE_P(e)) {
    /* Kahan's compensated summation algorithm */
    double f, c;

    f = NUM2DBL(v);
    c = 0.0;
    goto has_float_value;
    for (; i < RARRAY_LEN(ary); i++) {
      double x, y, t;
      e = RARRAY_AREF(ary, i);
      if (block_given)
        e = rb_yield(e);
      if (RB_FLOAT_TYPE_P(e))
        has_float_value:
          x = RFLOAT_VALUE(e);
      else if (FIXNUM_P(e))
        x = FIX2LONG(e);
      else if (RB_TYPE_P(e, T_BIGNUM))
        x = rb_big2dbl(e);
      else if (RB_TYPE_P(e, T_RATIONAL))
        x = rb_num2dbl(e);
      else
        goto not_float;

      y = x - c;
      t = f + y;
      c = (t - f) - y;
      f = t;
    }
    return DBL2NUM(f);

  not_float:
    v = DBL2NUM(f);
  }

  goto has_some_value;
  for (; i < RARRAY_LEN(ary); i++) {
    e = RARRAY_AREF(ary, i);
    if (block_given)
      e = rb_yield(e);
  has_some_value:
    v = rb_funcall(v, idPLUS, 1, e);
  }

  return v;
}
#endif

#define ENUM_WANT_SVALUE() do { \
  e = rb_enum_values_pack(argc, argv); \
} while (0)

struct enum_sum_memo {
  VALUE v, r;
  long n;
  double f, c;
  int block_given;
  int float_value;
};

static VALUE
enum_sum_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(e, args))
{
  struct enum_sum_memo *memo = (struct enum_sum_memo *)args;
  long n = memo->n;
  VALUE v = memo->v;
  VALUE r = memo->r;
  double f = memo->f;
  double c = memo->c;

  ENUM_WANT_SVALUE();

  if (memo->block_given)
    e = rb_yield(e);

  if (memo->float_value)
    goto float_value;

  if (FIXNUM_P(v) || RB_TYPE_P(v, T_BIGNUM) || RB_TYPE_P(v, T_RATIONAL)) {
    if (FIXNUM_P(e)) {
      n += FIX2LONG(e); /* should not overflow long type */
      if (!FIXABLE(n)) {
        v = rb_big_plus(LONG2NUM(n), v);
        n = 0;
      }
    }
    else if (RB_TYPE_P(e, T_BIGNUM))
      v = rb_big_plus(e, v);
    else if (RB_TYPE_P(e, T_RATIONAL)) {
      if (r == Qundef)
        r = e;
      else
        r = rb_rational_plus(r, e);
    }
    else {
      if (n != 0) {
        v = rb_fix_plus(LONG2FIX(n), v);
        n = 0;
      }
      if (r != Qundef) {
        v = rb_rational_plus(r, v);
        r = Qundef;
      }
      if (RB_FLOAT_TYPE_P(e)) {
        f = NUM2DBL(v);
        c = 0.0;
        memo->float_value = 1;
        goto float_value;
      }
      else
        goto some_value;
    }
  }
  else if (RB_FLOAT_TYPE_P(v)) {
    /* Kahan's compensated summation algorithm */
    double x, y, t;

  float_value:
    if (RB_FLOAT_TYPE_P(e))
      x = RFLOAT_VALUE(e);
    else if (FIXNUM_P(e))
      x = FIX2LONG(e);
    else if (RB_TYPE_P(e, T_BIGNUM))
      x = rb_big2dbl(e);
    else if (RB_TYPE_P(e, T_RATIONAL))
      x = rb_num2dbl(e);
    else {
      v = DBL2NUM(f);
      memo->float_value = 0;
      goto some_value;
    }

    y = x - c;
    t = f + y;
    c = (t - f) - y;
    f = t;
  }
  else {
  some_value:
    v = rb_funcall(v, idPLUS, 1, e);
  }

  memo->v = v;
  memo->n = n;
  memo->r = r;
  memo->f = f;
  memo->c = c;

  return Qnil;
}

static VALUE
enum_stat_sum(int argc, VALUE* argv, VALUE obj)
{
  struct enum_sum_memo memo;

  if (rb_scan_args(argc, argv, "01", &memo.v) == 0)
    memo.v = LONG2FIX(0);

  memo.block_given = rb_block_given_p();

  memo.n = 0;
  memo.r = Qundef;

  if ((memo.float_value = RB_FLOAT_TYPE_P(memo.v))) {
    memo.f = RFLOAT_VALUE(memo.v);
    memo.c = 0.0;
  }

  rb_block_call(obj, id_each, 0, 0, enum_sum_iter_i, (VALUE)&memo);

  if (memo.float_value) {
    return DBL2NUM(memo.f);
  }
  else {
    if (memo.n != 0)
      memo.v = rb_fix_plus(LONG2FIX(memo.n), memo.v);
    if (memo.r != Qundef)
      memo.v = rb_rational_plus(memo.r, memo.v);
    return memo.v;
  }
}

static VALUE
enum_stat_mean_variance(VALUE obj)
{
  return rb_assoc_new(INT2FIX(0), INT2FIX(0));
}

static VALUE
enum_stat_mean(VALUE obj)
{
  VALUE ary = enum_stat_mean_variance(obj);
  return RARRAY_AREF(ary, 0);
}

static VALUE
enum_stat_variance(VALUE obj)
{
  VALUE ary = enum_stat_mean_variance(obj);
  return RARRAY_AREF(ary, 1);
}

static VALUE
enum_stat_mean_stddev(VALUE obj)
{
  VALUE ary = enum_stat_mean_variance(obj);
  VALUE var = RARRAY_AREF(ary, 1);
  VALUE stddev = rb_funcall(var, idPow, 1, DBL2NUM(0.5));
  RARRAY_ASET(ary, 1, stddev);
  return ary;
}

static VALUE
enum_stat_stddev(VALUE obj)
{
  VALUE ary = enum_stat_mean_stddev(obj);
  return RARRAY_AREF(ary, 1);
}

void
Init_extension(void)
{
  rb_define_method(rb_mEnumerable, "sum", enum_stat_sum, -1);

#if 0
  rb_define_method(rb_mEnumerable, "mean_variance", enum_stat_mean_variance, 0);
  rb_define_method(rb_mEnumerable, "mean", enum_stat_mean, 0);
  rb_define_alias(rb_mEnumerable, "average", "mean");
  rb_define_method(rb_mEnumerable, "variance", enum_stat_variance, 0);
  rb_define_alias(rb_mEnumerable, "var", "variance");
  rb_define_method(rb_mEnumerable, "mean_stddev", enum_stat_mean_stddev, 0);
  rb_define_method(rb_mEnumerable, "stddev", enum_stat_stddev, 0);
#endif

#ifndef HAVE_ARRAY_SUM
  rb_define_method(rb_cArray, "sum", ary_sum, -1);
#endif

  idPLUS = '+';
  idMINUS = '-';
  idSTAR = '*';
  idPow = rb_intern("**");
  id_eqeq_p = rb_intern("==");
  id_idiv = rb_intern("div");
  id_negate = rb_intern("-@");
  id_to_f = rb_intern("to_f");
  id_cmp = rb_intern("<=>");
  id_each = rb_intern("each");
  id_real_p = rb_intern("real?");
}
