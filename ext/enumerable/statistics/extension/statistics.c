#include <ruby/ruby.h>
#include <ruby/util.h>
#include <ruby/version.h>
#include <assert.h>
#include <math.h>

#if RUBY_API_VERSION_CODE >= 20400
/* for 2.4.0 or higher */
# define HAVE_ARRAY_SUM
# define HAVE_ENUM_SUM
# undef HAVE_RB_FIX_PLUS
# undef HAVE_RB_RATIONAL_PLUS
#elif RUBY_API_VERSION_CODE >= 20200
/* for 2.3.0 and 2.2.0 */
# undef HAVE_ARRAY_SUM
# undef HAVE_ENUM_SUM
# undef HAVE_RB_FIX_PLUS
# undef HAVE_RB_RATIONAL_PLUS
#endif

#ifdef HAVE_RB_ARITHMETIC_SEQUENCE_EXTRACT
# define HAVE_ARITHMETIC_SEQUENCE
#else
# undef HAVE_ARITHMETIC_SEQUENCE
#endif

#ifndef RB_INTEGER_TYPE_P
# define RB_INTEGER_TYPE_P(obj) enum_stat_integer_type_p(obj)
static inline int
enum_stat_integer_type_p(VALUE obj)
{
    return (FIXNUM_P(obj) ||
      (!SPECIAL_CONST_P(obj) &&
       BUILTIN_TYPE(obj) == RUBY_T_BIGNUM));
}
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

#define SET_MEAN(v) do { if (mean_ptr) *mean_ptr = (v); } while (0)
#define SET_VARIANCE(v) do { if (variance_ptr) *variance_ptr = (v); } while (0)

static VALUE half_in_rational;

static ID idPow, idPLUS, idMINUS, idSTAR, idDIV, idGE;
static ID id_eqeq_p, id_idiv, id_negate, id_to_f, id_cmp, id_nan_p;
static ID id_each, id_real_p, id_sum, id_population, id_closed, id_compare, id_edge;

static VALUE sym_left, sym_right;

static VALUE cHistogram;

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

#ifndef HAVE_RB_FIX_PLUS
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

#ifndef HAVE_RB_INT_PLUS
static VALUE
rb_int_plus(VALUE x, VALUE y)
{
  if (FIXNUM_P(x)) {
    return rb_fix_plus(x, y);
  }
  else if (RB_TYPE_P(x, T_BIGNUM)) {
    return rb_big_plus(x, y);
  }
  return rb_num_coerce_bin(x, y, '+');
}
#endif

#ifndef HAVE_RB_FIX_MINUS
static VALUE
rb_fix_minus(VALUE x, VALUE y)
{
  if (FIXNUM_P(y)) {
    long a, b, c;
    VALUE r;

    a = FIX2LONG(x);
    b = FIX2LONG(y);
    c = a - b;
    r = LONG2NUM(c);

    return r;
  }
  else if (RB_TYPE_P(y, T_BIGNUM)) {
    x = rb_int2big(FIX2LONG(x));
    return rb_big_minus(x, y);
  }
  else if (RB_TYPE_P(y, T_FLOAT)) {
    return DBL2NUM((double)FIX2LONG(x) - RFLOAT_VALUE(y));
  }
  else {
    return rb_num_coerce_bin(x, y, '-');
  }
}
#endif

#ifndef HAVE_RB_INT_MINUS
VALUE
rb_int_minus(VALUE x, VALUE y)
{
  if (FIXNUM_P(x)) {
    return rb_fix_minus(x, y);
  }
  else if (RB_TYPE_P(x, T_BIGNUM)) {
    return rb_big_minus(x, y);
  }
  return rb_num_coerce_bin(x, y, '-');
}
#endif

#ifndef HAVE_RB_INTEGER_FLOAT_CMP
static VALUE
rb_integer_float_cmp(VALUE x, VALUE y)
{
  double yd = RFLOAT_VALUE(y);
  double yi, yf;
  VALUE rel;

  if (isnan(yd))
    return Qnil;
  if (isinf(yd)) {
    if (yd > 0.0) return INT2FIX(-1);
    else return INT2FIX(1);
  }
  yf = modf(yd, &yi);
  if (FIXNUM_P(x)) {
#if SIZEOF_LONG * CHAR_BIT < DBL_MANT_DIG /* assume FLT_RADIX == 2 */
    double xd = (double)FIX2LONG(x);
    if (xd < yd)
      return INT2FIX(-1);
    if (xd > yd)
      return INT2FIX(1);
    return INT2FIX(0);
#else
    long xn, yn;
    if (yi < FIXNUM_MIN)
      return INT2FIX(1);
    if (FIXNUM_MAX+1 <= yi)
      return INT2FIX(-1);
    xn = FIX2LONG(x);
    yn = (long)yi;
    if (xn < yn)
      return INT2FIX(-1);
    if (xn > yn)
      return INT2FIX(1);
    if (yf < 0.0)
      return INT2FIX(1);
    if (0.0 < yf)
      return INT2FIX(-1);
    return INT2FIX(0);
#endif
  }
  y = rb_dbl2big(yi);
  rel = rb_big_cmp(x, y);
  if (yf == 0.0 || rel != INT2FIX(0))
    return rel;
  if (yf < 0.0)
    return INT2FIX(1);
  return INT2FIX(-1);
}
#endif

static VALUE
fix_ge(VALUE x, VALUE y)
{
  if (FIXNUM_P(y)) {
    if (FIX2LONG(x) >= FIX2LONG(y)) return Qtrue;
    return Qfalse;
  }
  else if (RB_TYPE_P(y, T_BIGNUM)) {
    return rb_big_cmp(y, x) != INT2FIX(+1) ? Qtrue : Qfalse;
  }
  else if (RB_TYPE_P(y, T_FLOAT)) {
    VALUE rel = rb_integer_float_cmp(x, y);
    return rel == INT2FIX(1) || rel == INT2FIX(0) ? Qtrue : Qfalse;
  }
  else {
    return rb_num_coerce_relop(x, y, idGE);
  }
}

#ifndef HAVE_RB_BIG_GE
static VALUE
rb_big_ge(VALUE x, VALUE y)
{
  VALUE rel;
  int n;

  if (RB_INTEGER_TYPE_P(y)) {
    rel = rb_big_cmp(x, y);
  }
  else if (RB_FLOAT_TYPE_P(y)) {
    rel = rb_integer_float_cmp(x, y);
  }
  else {
    return rb_num_coerce_relop(x, y, idGE);
  }

  if (NIL_P(rel)) return Qfalse;
  n = FIX2INT(rel);
  return n >= 0 ? Qtrue : Qfalse;
}
#endif

#ifndef HAVE_RB_INT_GE
static VALUE
rb_int_ge(VALUE x, VALUE y)
{
  if (FIXNUM_P(x)) {
    return fix_ge(x, y);
  }
  else if (RB_TYPE_P(x, T_BIGNUM)) {
    return rb_big_ge(x, y);
  }
  return Qnil;
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

    return f_addsub(self, num, den, other, ONE, '+');
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

/* call-seq:
 *    ary.sum
 *
 * Calculate the sum of the values in `ary`.
 * This method utilizes
 * [Kahan summation algorithm](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
 * to compensate the result precision when the `ary` includes Float values.
 *
 * Note that This library does not redefine `sum` method introduced in Ruby 2.4.
 *
 * @return [Number] A summation value
 */
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

static void
calculate_and_set_mean(VALUE *mean_ptr, VALUE sum, long const n)
{
  if (RB_TYPE_P(sum, T_COMPLEX)) {
    VALUE real_mean, imag_mean;
    VALUE const real = RCOMPLEX(sum)->real;
    VALUE const imag = RCOMPLEX(sum)->imag;

    if (RB_FLOAT_TYPE_P(real))
      real_mean = DBL2NUM(RFLOAT_VALUE(real) / n);
    else
      real_mean = rb_funcall(real, idDIV, 1, DBL2NUM(n));

    if (RB_FLOAT_TYPE_P(imag))
      imag_mean = DBL2NUM(RFLOAT_VALUE(imag) / n);
    else
      imag_mean = rb_funcall(imag, idDIV, 1, DBL2NUM(n));

    SET_MEAN(complex_new(CLASS_OF(sum), real_mean, imag_mean));
  }
  else if (RB_FLOAT_TYPE_P(sum)) {
    SET_MEAN(DBL2NUM(RFLOAT_VALUE(sum) / n));
  }
  else
    SET_MEAN(rb_funcall(sum, idDIV, 1, DBL2NUM(n)));
}

static void
ary_mean_variance(VALUE ary, VALUE *mean_ptr, VALUE *variance_ptr, size_t ddof)
{
  long i;
  size_t n = 0;
  double m = 0.0, m2 = 0.0, f = 0.0, c = 0.0;

  SET_MEAN(DBL2NUM(0));
  SET_VARIANCE(DBL2NUM(NAN));

  if (RARRAY_LEN(ary) == 0)
    return;
  else if (RARRAY_LEN(ary) == 1) {
    VALUE e = RARRAY_AREF(ary, 0);
    if (rb_block_given_p())
      e = rb_yield(e);
    if (RB_TYPE_P(e, T_COMPLEX))
      SET_MEAN(e);
    else {
      e = rb_Float(e);
      SET_MEAN(e);
    }
    return;
  }

  if (variance_ptr == NULL) {
    VALUE init = DBL2NUM(0.0);
    VALUE const sum = ary_sum(1, &init, ary);
    long const n = RARRAY_LEN(ary);
    calculate_and_set_mean(mean_ptr, sum, n);
    return;
  }

  for (i = 0; i < RARRAY_LEN(ary); ++i) {
    double x, delta, y, t;
    VALUE e;

    n += 1;

    e = RARRAY_AREF(ary, i);
    if (rb_block_given_p())
      e = rb_yield(e);

    if (RB_FLOAT_TYPE_P(e))
      x = RFLOAT_VALUE(e);
    else if (FIXNUM_P(e))
      x = FIX2LONG(e);
    else if (RB_TYPE_P(e, T_BIGNUM))
      x = rb_big2dbl(e);
    else
      x = rb_num2dbl(e);

    y = x - c;
    t = f + y;
    c = (t - f) - y;
    f = t;

    delta = x - m;
    m += delta / n;
    m2 += delta * (x - m);
  }

  SET_MEAN(DBL2NUM(f / n));
  if (n >= 2) {
    assert(n > ddof);
    SET_VARIANCE(DBL2NUM(m2 / (n - ddof)));
  }
}

static int
opt_population_p(VALUE opts)
{
  VALUE population = Qfalse;

  if (!NIL_P(opts)) {
#ifdef HAVE_RB_GET_KWARGS
    ID kwargs = id_population;
    rb_get_kwargs(opts, &kwargs, 0, 1, &population);
#else
    VALUE val = rb_hash_aref(opts, ID2SYM(id_population));
    population = NIL_P(val) ? population : val;
#endif
  }

  return RTEST(population);
}

/* call-seq:
 *    ary.mean_variance(population: false)
 *
 * Calculate a mean and a variance of the values in `ary`.
 * The first element of the result array is the mean, and the second is the variance.
 *
 * When the `population:` keyword parameter is `true`,
 * the variance is calculated as a population variance (divided by $n$).
 * The default `population:` keyword parameter is `false`;
 * this means the variance is a sample variance (divided by $n-1$).
 *
 * This method scan values in `ary` only once,
 * and does not cache the values on memory.
 *
 * @return (mean, variance) Two element array consists of mean and variance values
 */
static VALUE
ary_mean_variance_m(int argc, VALUE* argv, VALUE ary)
{
  VALUE opts, mean, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  ary_mean_variance(ary, &mean, &variance, ddof);
  return rb_assoc_new(mean, variance);
}

/* call-seq:
 *    ary.mean
 *
 * Calculate a mean of the values in `ary`.
 * This method utilizes
 * [Kahan summation algorithm](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
 * to compensate the result precision when the `enum` includes Float values.
 *
 * @return [Number] A mean value
 */
static VALUE
ary_mean(VALUE ary)
{
  VALUE mean;
  ary_mean_variance(ary, &mean, NULL, 1);
  return mean;
}

/* call-seq:
 *    ary.variance(population: false)
 *
 * Calculate a variance of the values in `ary`.
 * This method scan values in `ary` only once,
 * and does not cache the values on memory.
 *
 * When the `population:` keyword parameter is `true`,
 * the variance is calculated as a population variance (divided by $n$).
 * The default `population:` keyword parameter is `false`;
 * this means the variance is a sample variance (divided by $n-1$).
 *
 * @return [Number] A variance value
 */
static VALUE
ary_variance(int argc, VALUE* argv, VALUE ary)
{
  VALUE opts, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  ary_mean_variance(ary, NULL, &variance, ddof);
  return variance;
}

#define ENUM_WANT_SVALUE() do { \
  e = rb_enum_values_pack(argc, argv); \
} while (0)

struct enum_sum_memo {
  VALUE v, r;
  long n, count;
  double f, c;
  int block_given;
  int float_value;
};

static void
sum_iter(VALUE e, struct enum_sum_memo *memo)
{
  int const unused = (assert(memo != NULL), 0);

  long n = memo->n;
  VALUE v = memo->v;
  VALUE r = memo->r;
  double f = memo->f;
  double c = memo->c;

  if (memo->block_given)
    e = rb_yield(e);

  memo->count += 1;

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
  (void)unused;
}

static VALUE
enum_sum_i(RB_BLOCK_CALL_FUNC_ARGLIST(e, args))
{
  ENUM_WANT_SVALUE();
  sum_iter(e, (struct enum_sum_memo *) args);
  return Qnil;
}

static int
hash_sum_i(VALUE key, VALUE value, VALUE arg)
{
  sum_iter(rb_assoc_new(key, value), (struct enum_sum_memo *) arg);
  return ST_CONTINUE;
}

static void
hash_sum(VALUE hash, struct enum_sum_memo *memo)
{
  assert(RB_TYPE_P(hash, T_HASH));
  assert(memo != NULL);

  rb_hash_foreach(hash, hash_sum_i, (VALUE)memo);
}

static void
int_range_sum_count(VALUE beg, VALUE end, int excl,
                    VALUE init, VALUE *sum_ptr, long *count_ptr)
{
  if (excl) {
    if (FIXNUM_P(end))
      end = LONG2FIX(FIX2LONG(end) - 1);
    else
      end = rb_big_minus(end, LONG2FIX(1));
  }

  if (rb_int_ge(end, beg)) {
    VALUE a;
    a = rb_int_plus(rb_int_minus(end, beg), LONG2FIX(1));
    a = f_mul(a, rb_int_plus(end, beg));
    a = f_idiv(a, LONG2FIX(2));
    if (sum_ptr)
      *sum_ptr = rb_int_plus(init, a);
    if (count_ptr)
      *count_ptr = a;
    return;
  }

  if (sum_ptr)
    *sum_ptr = init;
  if (count_ptr)
    *count_ptr = 0;
}

static void
enum_sum_count(VALUE obj, VALUE init, VALUE *sum_ptr, long *count_ptr)
{
  struct enum_sum_memo memo;
  VALUE beg, end;
  int excl;

  memo.count = 0;
  memo.v = init;
  memo.block_given = rb_block_given_p();
  memo.n = 0;
  memo.r = Qundef;

  if ((memo.float_value = RB_FLOAT_TYPE_P(memo.v))) {
    memo.f = RFLOAT_VALUE(memo.v);
    memo.c = 0.0;
  }

  if (RTEST(rb_range_values(obj, &beg, &end, &excl))) {
    if (!rb_block_given_p() && !memo.float_value &&
        RB_INTEGER_TYPE_P(beg) && RB_INTEGER_TYPE_P(end)) {
      int_range_sum_count(beg, end, excl, memo.v, sum_ptr, count_ptr);
      return;
    }
  }

  if (RB_TYPE_P(obj, T_HASH) &&
      rb_method_basic_definition_p(CLASS_OF(obj), id_each))
    hash_sum(obj, &memo);
  else
    rb_block_call(obj, id_each, 0, 0, enum_sum_i, (VALUE)&memo);

  if (memo.float_value) {
    if (sum_ptr)
      *sum_ptr = DBL2NUM(memo.f);
  }
  else {
    if (memo.n != 0)
      memo.v = rb_fix_plus(LONG2FIX(memo.n), memo.v);
    if (memo.r != Qundef)
      memo.v = rb_rational_plus(memo.r, memo.v);
    if (sum_ptr)
      *sum_ptr = memo.v;
  }

  if (count_ptr)
    *count_ptr = memo.count;
}

#ifndef HAVE_ENUM_SUM
/* call-seq:
 *    enum.sum
 *
 * Calculate the sum of the values in `enum`.
 * This method utilizes
 * [Kahan summation algorithm](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
 * to compensate the result precision when the `enum` includes Float values.
 *
 * Note that This library does not redefine `sum` method introduced in Ruby 2.4.
 *
 * @return [Number] A summation value
 */
static VALUE
enum_sum(int argc, VALUE* argv, VALUE obj)
{
  VALUE sum, init;

  if (rb_scan_args(argc, argv, "01", &init) == 0)
    init = LONG2FIX(0);

  enum_sum_count(obj, init, &sum, NULL);

  return sum;
}
#endif

struct enum_mean_variance_memo {
  int block_given;
  size_t n;
  double m, m2, f, c;
};

static void
mean_variance_iter(VALUE e, struct enum_mean_variance_memo *memo)
{
  int const unused = (assert(memo != NULL), 0);

  double x, delta, y, t;

  long n = memo->n;
  double m = memo->m;
  double m2 = memo->m2;
  double f = memo->f;
  double c = memo->c;

  if (memo->block_given)
    e = rb_yield(e);

  n += 1;

  if (RB_FLOAT_TYPE_P(e))
    x = RFLOAT_VALUE(e);
  else if (FIXNUM_P(e))
    x = FIX2LONG(e);
  else if (RB_TYPE_P(e, T_BIGNUM))
    x = rb_big2dbl(e);
  else {
    x = rb_num2dbl(e);
  }

  y = x - c;
  t = f + y;
  c = (t - f) - y;
  f = t;

  delta = x - m;
  m += delta / n;
  m2 += delta * (x - m);

  memo->n = n;
  memo->m = m;
  memo->m2 = m2;
  memo->f = f;
  memo->c = c;
  (void)unused;
}

static VALUE
enum_mean_variance_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(e, args))
{
  struct enum_mean_variance_memo *memo = (struct enum_mean_variance_memo *)args;
  ENUM_WANT_SVALUE();
  mean_variance_iter(e, memo);
  return Qnil;
}

static int
hash_mean_variance_i(VALUE key, VALUE value, VALUE arg)
{
  mean_variance_iter(rb_assoc_new(key, value), (struct enum_mean_variance_memo *) arg);
  return ST_CONTINUE;
}

static void
hash_mean_variance(VALUE hash, struct enum_mean_variance_memo *memo)
{
  assert(RB_TYPE_P(hash, T_HASH));
  assert(memo != NULL);

  rb_hash_foreach(hash, hash_mean_variance_i, (VALUE)memo);
}

static void
enum_mean_variance(VALUE obj, VALUE *mean_ptr, VALUE *variance_ptr, size_t ddof)
{
  struct enum_mean_variance_memo memo;

  SET_MEAN(DBL2NUM(0));
  SET_VARIANCE(DBL2NUM(NAN));

  if (variance_ptr == NULL) {
    long n;
    VALUE sum;
    VALUE init = DBL2NUM(0.0);
    enum_sum_count(obj, init, &sum, &n);
    if (n > 0)
      calculate_and_set_mean(mean_ptr, sum, n);
    return;
  }

  memo.block_given = rb_block_given_p();
  memo.n = 0;
  memo.m = 0.0;
  memo.m2 = 0.0;
  memo.f = 0.0;
  memo.c = 0.0;

  if (RB_TYPE_P(obj, T_HASH) &&
      rb_method_basic_definition_p(CLASS_OF(obj), id_each))
    hash_mean_variance(obj, &memo);
  else
    rb_block_call(obj, id_each, 0, 0, enum_mean_variance_iter_i, (VALUE)&memo);

  if (memo.n == 0)
    return;
  else if (memo.n == 1)
    SET_MEAN(DBL2NUM(memo.f));
  else {
    SET_MEAN(DBL2NUM(memo.f / memo.n));

    assert(memo.n > ddof);
    SET_VARIANCE(DBL2NUM(memo.m2 / (double)(memo.n - ddof)));
  }
}

/* call-seq:
 *    enum.mean_variance(population: false)
 *
 * Calculate a mean and a variance of the values in `enum`.
 * The first element of the result array is the mean, and the second is the variance.
 *
 * When the `population:` keyword parameter is `true`,
 * the variance is calculated as a population variance (divided by $n$).
 * The default `population:` keyword parameter is `false`;
 * this means the variance is a sample variance (divided by $n-1$).
 *
 * This method scan values in `enum` only once,
 * and does not cache the values on memory.
 *
 * @return (mean, variance) Two element array consists of mean and variance values
 */
static VALUE
enum_mean_variance_m(int argc, VALUE* argv, VALUE obj)
{
  VALUE opts, mean, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  enum_mean_variance(obj, &mean, &variance, ddof);
  return rb_assoc_new(mean, variance);
}

/* call-seq:
 *    enum.mean
 *
 * Calculate a mean of the values in `enum`.
 * This method utilizes
 * [Kahan summation algorithm](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
 * to compensate the result precision when the `enum` includes Float values.
 *
 * @return [Number] A mean value
 */
static VALUE
enum_mean(VALUE obj)
{
  VALUE mean;
  enum_mean_variance(obj, &mean, NULL, 1);
  return mean;
}

/* call-seq:
 *    enum.variance(population: false)
 *
 * Calculate a variance of the values in `enum`.
 * This method scan values in `enum` only once,
 * and does not cache the values on memory.
 *
 * When the `population:` keyword parameter is `true`,
 * the variance is calculated as a population variance (divided by $n$).
 * The default `population:` keyword parameter is `false`;
 * this means the variance is a sample variance (divided by $n-1$).
 *
 * @return [Number] A variance value
 */
static VALUE
enum_variance(int argc, VALUE* argv, VALUE obj)
{
  VALUE opts, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  enum_mean_variance(obj, NULL, &variance, ddof);
  return variance;
}

static VALUE
sqrt_value(VALUE x)
{
  if (RB_INTEGER_TYPE_P(x) || RB_FLOAT_TYPE_P(x) || RB_TYPE_P(x, T_RATIONAL)) {
    double f = NUM2DBL(x);
    return DBL2NUM(sqrt(f));
  }

  return rb_funcall(x, idPow, 1, half_in_rational);
}

/* call-seq:
 *    enum.mean_stdev(population: false)
 *
 * Calculate a mean and a standard deviation of the values in `enum`.
 * The first element of the result array is the mean,
 * and the second is the standard deviation.
 *
 * This method is equivalent to:
 *
 * ```ruby
 * def mean_stdev(population: false)
 *   m, v = mean_variance(population: population)
 *   [m, Math.sqrt(v)]
 * end
 * ```
 *
 * @return (mean, stdev)
 */
static VALUE
enum_mean_stdev(int argc, VALUE* argv, VALUE obj)
{
  VALUE opts, mean, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  enum_mean_variance(obj, &mean, &variance, ddof);
  VALUE stdev = sqrt_value(variance);
  return rb_assoc_new(mean, stdev);
}

/* call-seq:
 *    enum.stdev(population: false)
 *
 * Calculate a standard deviation of the values in `enum`.
 *
 * This method is equivalent to:
 *
 * ```ruby
 * Math.sqrt(enum.variance(population: population))
 * ```
 *
 * @return [Number] A standard deviation value
 */
static VALUE
enum_stdev(int argc, VALUE* argv, VALUE obj)
{
  VALUE variance = enum_variance(argc, argv, obj);
  VALUE stdev = sqrt_value(variance);
  return stdev;
}

/* call-seq:
 *    ary.mean_stdev(population: false)
 *
 * Calculate a mean and a standard deviation of the values in `ary`.
 * The first element of the result array is the mean,
 * and the second is the standard deviation.
 *
 * This method is equivalent to:
 *
 * ```ruby
 * def mean_stdev(population: false)
 *   m, v = mean_variance(population: population)
 *   [m, Math.sqrt(v)]
 * end
 * ```
 *
 * @return (mean, stdev)
 */
static VALUE
ary_mean_stdev(int argc, VALUE* argv, VALUE ary)
{
  VALUE opts, mean, variance;
  size_t ddof = 1;

  rb_scan_args(argc, argv, "0:", &opts);
  if (opt_population_p(opts))
    ddof = 0;

  ary_mean_variance(ary, &mean, &variance, ddof);
  VALUE stdev = sqrt_value(variance);
  return rb_assoc_new(mean, stdev);
}

/* call-seq:
 *    ary.stdev(population: false)
 *
 * Calculate a standard deviation of the values in `ary`.
 *
 * This method is equivalent to:
 *
 * ```ruby
 * Math.sqrt(ary.variance(population: population))
 * ```
 *
 * @return [Number] A standard deviation value
 */
static VALUE
ary_stdev(int argc, VALUE* argv, VALUE ary)
{
  VALUE variance = ary_variance(argc, argv, ary);
  VALUE stdev = sqrt_value(variance);
  return stdev;
}

static inline int
is_na(VALUE v)
{
  if (NIL_P(v))
    return 1;

  if (RB_FLOAT_TYPE_P(v) && isnan(RFLOAT_VALUE(v)))
    return 1;

  if (rb_respond_to(v, id_nan_p) && RTEST(rb_funcall(v, id_nan_p, 0)))
    return 1;

  return 0;
}

static int
ary_percentile_sort_cmp(const void *ap, const void *bp, void *dummy)
{
  VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
  VALUE cmp;

  if (is_na(a)) {
    return -1;
  }
  else if (is_na(b)) {
    return 1;
  }

  /* TODO: optimize */
  cmp = rb_funcall(a, id_cmp, 1, b);
  return rb_cmpint(cmp, a, b);
}

static VALUE
ary_percentile_make_sorted(VALUE ary)
{
  long n, i;
  VALUE sorted;

  n = RARRAY_LEN(ary);
  sorted = rb_ary_tmp_new(n);
  for (i = 0; i < n; ++i) {
    rb_ary_push(sorted, RARRAY_AREF(ary, i));
  }
  RARRAY_PTR_USE(sorted, ptr, {
    ruby_qsort(ptr, n, sizeof(VALUE),
               ary_percentile_sort_cmp, NULL);
  });
  return sorted;
}

static inline VALUE
ary_percentile_single_sorted(VALUE sorted, long n, double d)
{
  VALUE x0, x1;
  double i, f;
  long l;

  assert(RB_TYPE_P(sorted, T_ARRAY));
  assert(n == RARRAY_LEN(sorted));
  assert(n > 0);

  if (d < 0 || 100 < d) {
    rb_raise(rb_eArgError, "percentile out of bounds");
  }

  if (is_na(RARRAY_AREF(sorted, 0))) {
    return DBL2NUM(nan(""));
  }

  n = RARRAY_LEN(sorted);
  if (n == 1) {
    return RARRAY_AREF(sorted, 0);
  }

  d = (n - 1) * d / 100.0;
  f = modf(d, &i);
  l = (long)i;

  x0 = RARRAY_AREF(sorted, l);
  if (f == 0 || l == n - 1) {
    return x0;
  }

  x0 = rb_funcall(x0, idSTAR, 1, DBL2NUM(1 - f));
  x1 = RARRAY_AREF(sorted, l + 1);
  x1 = rb_funcall(x1, idSTAR, 1, DBL2NUM(f));

  return rb_funcall(x0, idPLUS, 1, x1);
}

static VALUE
ary_percentile_single(VALUE ary, VALUE q)
{
  long n;
  double d;
  VALUE qf, sorted;

  assert(RB_TYPE_P(ary, T_ARRAY));

  n = RARRAY_LEN(ary);
  assert(n > 0);

  switch (TYPE(q)) {
    case T_FIXNUM:
      d = (double)FIX2LONG(q);
      break;
    case T_BIGNUM:
      d = rb_big2dbl(q);
      break;

    case T_RATIONAL:
      /* fall through */
    default:
      qf = NUM2DBL(q);
      goto float_percentile;

    case T_FLOAT:
      qf = q;
float_percentile:
      d = RFLOAT_VALUE(qf);
      break;
  }

  if (n == 1) {
    return RARRAY_AREF(ary, 0);
  }

  sorted = ary_percentile_make_sorted(ary);

  return ary_percentile_single_sorted(sorted, n, d);
}

/* call-seq:
 *    ary.percentile(q) -> float
 *
 * Calculate specified percentiles of the values in `ary`.
 *
 * @param [Number, Array] percentile or array of percentiles to compute,
 *   which must be between 0 and 100 inclusive.
 *
 * @return [Float, Array] A percentile value(s)
 */
static VALUE
ary_percentile(VALUE ary, VALUE q)
{
  long n, m, i;
  double d;
  VALUE qf, qs, sorted, res;

  n = RARRAY_LEN(ary);
  if (n == 0) {
    rb_raise(rb_eArgError, "unable to compute percentile(s) for an empty array");
  }

  qs = rb_check_convert_type(q, T_ARRAY, "Array", "to_ary");
  if (NIL_P(qs)) {
    return ary_percentile_single(ary, q);
  }

  m = RARRAY_LEN(qs);
  res = rb_ary_new_capa(m);

  if (m == 1) {
    q = RARRAY_AREF(qs, 0);
    rb_ary_push(res, ary_percentile_single(ary, q));
  }
  else {
    sorted = ary_percentile_make_sorted(ary);

    for (i = 0; i < m; ++i) {
      VALUE x;

      q = RARRAY_AREF(qs, i);
      switch (TYPE(q)) {
        case T_FIXNUM:
          d = (double)FIX2LONG(q);
          break;
        case T_BIGNUM:
          d = rb_big2dbl(q);
          break;

        case T_RATIONAL:
          /* fall through */
        default:
          qf = NUM2DBL(q);
          goto float_percentile;

        case T_FLOAT:
          qf = q;
float_percentile:
          d = RFLOAT_VALUE(qf);
          break;
      }

      x = ary_percentile_single_sorted(sorted, n, d);
      rb_ary_push(res, x);
    }
  }

  return res;
}

/* call-seq:
 *    ary.median -> float
 *
 * Calculate a median of the values in `ary`.
 *
 * @return [Float] A median value
 */
static VALUE
ary_median(VALUE ary)
{
  long n;
  VALUE sorted, a0, a1;

  n = RARRAY_LEN(ary);
  switch (n) {
    case 0:
      goto return_nan;
    case 1:
      return RARRAY_AREF(ary, 0);
    case 2:
      a0 = RARRAY_AREF(ary, 0);
      a1 = RARRAY_AREF(ary, 1);
      goto mean_two;
    default:
      break;
  }

  sorted = ary_percentile_make_sorted(ary);

  a0 = RARRAY_AREF(sorted, 0);
  if (is_na(a0)) {
return_nan:
    return DBL2NUM(nan(""));
  }

  a1 = RARRAY_AREF(sorted, n / 2);
  if (n % 2 == 1) {
    return a1;
  }
  else {
    a0 = RARRAY_AREF(sorted, n / 2 - 1);

mean_two:
    a0 = rb_funcall(a0, idPLUS, 1, a1); /* TODO: optimize */
    if (RB_INTEGER_TYPE_P(a0) || RB_FLOAT_TYPE_P(a0) || RB_TYPE_P(a0, T_RATIONAL)) {
      double d = NUM2DBL(a0);
      return DBL2NUM(d / 2.0);
    }

    return rb_funcall(a0, idDIV, 1, DBL2NUM(2.0));
  }
}

struct value_counts_opts {
  int normalize_p;
  int sort_p;
  int ascending_p;
  int dropna_p;
};

static inline void
value_counts_extract_opts(VALUE kwargs, struct value_counts_opts *opts)
{
  assert(opts != NULL);

  /* default values */
  opts->normalize_p = 0;
  opts->sort_p = 1;
  opts->ascending_p = 0;
  opts->dropna_p = 1;

  if (!NIL_P(kwargs)) {
    enum { kw_normalize, kw_sort, kw_ascending, kw_dropna };
    static ID kwarg_keys[4];
    VALUE kwarg_vals[4];

    if (!kwarg_keys[0]) {
      kwarg_keys[kw_normalize] = rb_intern("normalize");
      kwarg_keys[kw_sort]      = rb_intern("sort");
      kwarg_keys[kw_ascending] = rb_intern("ascending");
      kwarg_keys[kw_dropna]    = rb_intern("dropna");
    }

    rb_get_kwargs(kwargs, kwarg_keys, 0, 4, kwarg_vals);
    opts->normalize_p = (kwarg_vals[kw_normalize] != Qundef) && RTEST(kwarg_vals[kw_normalize]);
    opts->sort_p      = (kwarg_vals[kw_sort]      != Qundef) && RTEST(kwarg_vals[kw_sort]);
    opts->ascending_p = (kwarg_vals[kw_ascending] != Qundef) && RTEST(kwarg_vals[kw_ascending]);
    opts->dropna_p    = (kwarg_vals[kw_dropna]    != Qundef) && RTEST(kwarg_vals[kw_dropna]);
  }
}

static int
value_counts_result_to_assoc_array_i(VALUE key, VALUE val, VALUE ary)
{
  VALUE assoc = rb_ary_tmp_new(2);
  rb_ary_push(assoc, key);
  rb_ary_push(assoc, val);
  rb_ary_push(ary, assoc);
  return ST_CONTINUE;
}

static int
value_counts_sort_cmp_asc(const void *ap, const void *bp, void *dummy)
{
  VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
  VALUE av, bv, cmp;

  av = RARRAY_AREF(a, 1);
  bv = RARRAY_AREF(b, 1);

  /* TODO: optimize */
  cmp = rb_funcall(av, id_cmp, 1, bv);
  return rb_cmpint(cmp, av, bv);
}

static int
value_counts_sort_cmp_desc(const void *ap, const void *bp, void *dummy)
{
  VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
  VALUE av, bv, cmp;

  av = RARRAY_AREF(a, 1);
  bv = RARRAY_AREF(b, 1);

  /* TODO: optimize */
  cmp = rb_funcall(bv, id_cmp, 1, av);
  return rb_cmpint(cmp, bv, av);
}

static VALUE
value_counts_sort_result(VALUE result, const int dropna_p, const int ascending_p)
{
  VALUE na_count = Qundef, ary, sorted;
  long i;

  if (RHASH_SIZE(result) < 1) {
    return result;
  }

  if (!dropna_p) {
    na_count = rb_hash_lookup2(result, Qnil, Qundef);
    if (na_count != Qundef) {
      rb_hash_delete(result, Qnil);
    }
  }

  const long len = (long)RHASH_SIZE(result);
  ary = rb_ary_tmp_new(len);
  rb_hash_foreach(result, value_counts_result_to_assoc_array_i, ary);
  if (ascending_p) {
    RARRAY_PTR_USE(ary, ptr, {
      ruby_qsort(ptr, RARRAY_LEN(ary), sizeof(VALUE),
                 value_counts_sort_cmp_asc, NULL);
    });
  }
  else {
    RARRAY_PTR_USE(ary, ptr, {
      ruby_qsort(ptr, RARRAY_LEN(ary), sizeof(VALUE),
                 value_counts_sort_cmp_desc, NULL);
    });
  }

#ifdef HAVE_RB_HASH_NEW_WITH_SIZE
  sorted = rb_hash_new_with_size(len);
#else
  sorted = rb_hash_new();
#endif

  if (na_count != Qundef && ascending_p) {
    rb_hash_aset(sorted, Qnil, na_count);
  }

  for (i = 0; i < len; ++i) {
    VALUE a = RARRAY_AREF(ary, i);
    VALUE k = RARRAY_AREF(a, 0);
    VALUE v = RARRAY_AREF(a, 1);
    rb_hash_aset(sorted, k, v);
  }

  if (na_count != Qundef && !ascending_p) {
    rb_hash_aset(sorted, Qnil, na_count);
  }

  return sorted;
}

struct value_counts_normalize_params {
  VALUE result;
  long total;
};

static int
value_counts_normalize_i(VALUE key, VALUE val, VALUE arg)
{
  struct value_counts_normalize_params *params = (struct value_counts_normalize_params *)arg;
  double new_val;

  new_val = NUM2DBL(val) / params->total;
  rb_hash_aset(params->result, key, DBL2NUM(new_val));

  return ST_CONTINUE;
}

struct value_counts_memo {
  int dropna_p;
  long total;
  long na_count;
  VALUE result;
};

static VALUE
any_value_counts(int argc, VALUE *argv, VALUE obj,
                 void (* counter)(VALUE, struct value_counts_memo *))
{
  VALUE kwargs;
  struct value_counts_opts opts;
  struct value_counts_memo memo;

  rb_scan_args(argc, argv, ":", &kwargs);
  value_counts_extract_opts(kwargs, &opts);

  memo.result = rb_hash_new();
  memo.total = 0;
  memo.na_count = 0;
  memo.dropna_p = opts.dropna_p;

  if (!opts.dropna_p) {
    rb_hash_aset(memo.result, Qnil, INT2FIX(0)); // reserve the room for NA
  }

  counter(obj, &memo);

  if (!opts.dropna_p) {
    if (memo.na_count == 0)
      rb_hash_delete(memo.result, Qnil);
    else
      rb_hash_aset(memo.result, Qnil, LONG2NUM(memo.na_count));
  }

  if (opts.sort_p) {
    memo.result = value_counts_sort_result(memo.result, opts.dropna_p, opts.ascending_p);
  }

  if (opts.normalize_p) {
    struct value_counts_normalize_params params;
    params.result = memo.result;
    params.total = memo.total - (opts.dropna_p ? memo.na_count : 0);
    rb_hash_foreach(memo.result, value_counts_normalize_i, (VALUE)&params);
  }

  return memo.result;
}

static VALUE
enum_value_counts_without_sort_i(RB_BLOCK_CALL_FUNC_ARGLIST(e, args))
{
  struct value_counts_memo *memo = (struct value_counts_memo *)args;

  ENUM_WANT_SVALUE();

  if (is_na(e)) {
    ++memo->na_count;
  }
  else {
    VALUE cnt = rb_hash_lookup2(memo->result, e, INT2FIX(0));
    rb_hash_aset(memo->result, e, rb_int_plus(cnt, INT2FIX(1)));
  }

  ++memo->total;

  return Qnil;
}

static void
enum_value_counts_without_sort(VALUE obj, struct value_counts_memo *memo)
{
  rb_block_call(obj, id_each, 0, 0, enum_value_counts_without_sort_i, (VALUE)memo);
}

static VALUE
enum_value_counts(int argc, VALUE* argv, VALUE obj)
{
  return any_value_counts(argc, argv, obj, enum_value_counts_without_sort);
}

static void
ary_value_counts_without_sort(VALUE ary, struct value_counts_memo *memo)
{
  const VALUE zero = INT2FIX(0);
  const VALUE one = INT2FIX(1);
  long i, na_count = 0;
  long const n = RARRAY_LEN(ary);

  for (i = 0; i < n; ++i) {
    VALUE val = RARRAY_AREF(ary, i);

    if (is_na(val)) {
      ++na_count;
    }
    else {
      VALUE cnt = rb_hash_lookup2(memo->result, val, zero);
      rb_hash_aset(memo->result, val, rb_int_plus(cnt, one));
    }
  }

  memo->total = n;
  memo->na_count = na_count;
}

/* call-seq:
 *    ary.value_counts(normalize: false, sort: true, ascending: false, dropna: true) -> hash
 *
 * Returns a hash that contains the counts of values in `ary`.
 *
 * This method treats `nil` and NaN, the objects who respond `true` to `nan?`,
 * as the same thing, and stores the count of them as the value for `nil`.
 *
 * @param [false,true] normalize  If `true`, the result contains the relative
 *                                frequencies of the unique values.
 * @param [true,false] sort  Sort by values.
 * @param [false,true] ascending  Sort in ascending order.
 * @param [true,false] dropna  Don't include counts of NAs.
 *
 * @return [Hash] A hash consists of the counts of the values
 */
static VALUE
ary_value_counts(int argc, VALUE* argv, VALUE ary)
{
  return any_value_counts(argc, argv, ary, ary_value_counts_without_sort);
}

static int
hash_value_counts_without_sort_i(VALUE key, VALUE val, VALUE arg)
{
  struct value_counts_memo *memo = (struct value_counts_memo *)arg;

  if (is_na(val)) {
    ++memo->na_count;

    if (memo->dropna_p) {
      return ST_CONTINUE;
    }
  }
  else {
    VALUE cnt = rb_hash_lookup2(memo->result, val, INT2FIX(0));
    rb_hash_aset(memo->result, val, rb_int_plus(cnt, INT2FIX(1)));
  }

  return ST_CONTINUE;
}

static void
hash_value_counts_without_sort(VALUE hash, struct value_counts_memo *memo)
{
  rb_hash_foreach(hash, hash_value_counts_without_sort_i, (VALUE)memo);
  memo->total = RHASH_SIZE(hash);
}

/* call-seq:
 *    hash.value_counts(normalize: false, sort: true, ascending: false, dropna: true) -> hash
 *
 * Returns a hash that contains the counts of values in `hash`.
 *
 * This method treats `nil` and NaN, the objects who respond `true` to `nan?`,
 * as the same thing, and stores the count of them as the value for `nil`.
 *
 * @param [false,true] normalize  If `true`, the result contains the relative
 *                                frequencies of the unique values.
 * @param [true,false] sort  Sort by values.
 * @param [false,true] ascending  Sort in ascending order.
 * @param [true,false] dropna  Don't include counts of NAs.
 *
 * @return [Hash] A hash consists of the counts of the values
 */
static VALUE
hash_value_counts(int argc, VALUE* argv, VALUE hash)
{
  return any_value_counts(argc, argv, hash, hash_value_counts_without_sort);
}

static long
histogram_edge_bin_index(VALUE edge, VALUE rb_x, int left_p)
{
  double x, y;
  long lo, hi, mid;

  x = NUM2DBL(rb_x);

  lo = -1;
  hi = RARRAY_LEN(edge);

  if (left_p) {
    while (hi - lo > 1) {
      mid = lo + (hi - lo)/2;
      y = NUM2DBL(RARRAY_AREF(edge, mid));
      if (y <= x) {
        lo = mid;
      }
      else {
        hi = mid;
      }
    }
    return lo;
  }
  else {
    while (hi - lo > 1) {
      mid = lo + (hi - lo)/2;
      y = NUM2DBL(RARRAY_AREF(edge, mid));
      if (y < x) {
        lo = mid;
      }
      else {
        hi = mid;
      }
    }
    return hi - 1;
  }
}

static void
histogram_weights_push_values(VALUE weights, VALUE edge, VALUE values, int left_p)
{
  VALUE x, cur;
  long i, n, bi;

  n = RARRAY_LEN(values);
  for (i = 0; i < n; ++i) {
    x = RARRAY_AREF(values, i);

    bi = histogram_edge_bin_index(edge, x, left_p);

    cur = rb_ary_entry(weights, bi);
    if (NIL_P(cur)) {
      cur = INT2FIX(1);
    }
    else {
      cur = rb_funcall(cur, idPLUS, 1, INT2FIX(1));
    }

    rb_ary_store(weights, bi, cur);
  }
}

static int
opt_closed_left_p(VALUE opts)
{
  int left_p = 1;

  if (!NIL_P(opts)) {
    VALUE closed;
#ifdef HAVE_RB_GET_KWARGS
    ID kwargs = id_closed;
    rb_get_kwargs(opts, &kwargs, 0, 1, &closed);
#else
    closed = rb_hash_lookup2(opts, ID2SYM(id_closed), sym_left);
#endif
    left_p = (closed != sym_right);
    if (left_p && closed != sym_left) {
      rb_raise(rb_eArgError, "invalid value for :closed keyword "
               "(%"PRIsVALUE" for :left or :right)", closed);
    }
  }

  return left_p;
}

static VALUE
opt_compare_array(VALUE opts)
{

  if (!NIL_P(opts)) {
    VALUE compare;
#ifdef HAVE_RB_GET_KWARGS
    ID kwargs = id_compare;
    rb_get_kwargs(opts, &kwargs, 0, 1, &compare);
#else
    compare = rb_hash_lookup2(opts, ID2SYM(id_compare), sym_left);
#endif
  return compare;
  }
}

static inline long
sturges(long n)
{
  if (n == 0) return 1L;
  return (long)(ceil(log2(n)) + 1);
}

static VALUE
ary_histogram_calculate_edge_lo_hi(const double lo, const double hi, const long nbins, const int left_p)
{
  VALUE edge;
  double bw, lbw, start, step, divisor, r;
  long i, len;

  if (hi == lo) {
    start = hi;
    step = 1;
    divisor = 1;
    len = 1;
  }
  else {
    bw = (hi - lo) / nbins;
    lbw = log10(bw);
    if (lbw >= 0) {
      step = pow(10, floor(lbw));
      r = bw / step;
      if (r <= 1.1) {
        /* do nothing */
      }
      else if (r <= 2.2) {
        step *= 2;
      }
      else if (r <= 5.5) {
        step *= 5;
      }
      else {
        step *= 10;
      }
      divisor = 1.0;
      start = step * floor(lo / step);
      len = (long)ceil((hi - start) / step);
    }
    else {
      divisor = pow(10, -floor(lbw));
      r = bw * divisor;
      if (r <= 1.1) {
        /* do nothing */
      }
      else if (r <= 2.2) {
        divisor /= 2;
      }
      else if (r <= 5.5) {
        divisor /= 5;
      }
      else {
        divisor /= 10;
      }
      step = 1.0;
      start = floor(lo * divisor);
      len = (long)ceil(hi * divisor - start);
    }
  }

  if (left_p) {
    while (lo < start/divisor) {
      start -= step;
    }
    while ((start + (len - 1)*step)/divisor <= hi) {
      ++len;
    }
  }
  else {
    while (lo <= start/divisor) {
      start -= step;
    }
    while ((start + (len - 1)*step)/divisor < hi) {
      ++len;
    }
  }

  edge = rb_ary_new_capa(len);
  for (i = 0; i < len; ++i) {
    rb_ary_push(edge, DBL2NUM(start/divisor));
    start += step;
  }

  return edge;
}

static VALUE
ary_histogram_calculate_edge(VALUE ary, const long nbins, const int left_p)
{
  long n;
  VALUE minmax;
  VALUE edge = Qnil;
  double lo, hi;

  Check_Type(ary, T_ARRAY);
  n = RARRAY_LEN(ary);

  if (n == 0 && nbins < 0) {
    rb_raise(rb_eArgError, "nbins must be >= 0 for an empty array, got %ld", nbins);
  }
  else if (n > 0 && nbins < 1) {
    rb_raise(rb_eArgError, "nbins must be >= 1 for a non-empty array, got %ld", nbins);
  }
  else if (n == 0) {
    edge = rb_ary_new_capa(1);
    rb_ary_push(edge, DBL2NUM(0.0));
    return edge;
  }

  minmax = rb_funcall(ary, rb_intern("minmax"), 0);
  lo = NUM2DBL(RARRAY_AREF(minmax, 0));
  hi = NUM2DBL(RARRAY_AREF(minmax, 1));

  edge = ary_histogram_calculate_edge_lo_hi(lo, hi, nbins, left_p);

  return edge;
}

/* call-seq:
 *    ary.histogram(nbins=:auto, closed: :left)
 *
 * @param [Integer] nbins  The approximate number of bins
 * @param [:left, :right] closed
 *   If :left (the default), the bin interval are left-closed.
 *   If :right, the bin interval are right-closed.
 *
 * @return [EnumerableStatistics::Histogram] The histogram struct.
 */
static VALUE
ary_histogram(int argc, VALUE *argv, VALUE ary)
{
  VALUE arg0, opts, edge, weights;
  int left_p;
  long nbins, nweights, i;

  rb_scan_args(argc, argv, "01:", &arg0, &opts);
  if (NIL_P(arg0)) {
    nbins = sturges(RARRAY_LEN(ary));
  }
  else {
    nbins = NUM2LONG(arg0);
  }
  left_p = opt_closed_left_p(opts);

  edge = ary_histogram_calculate_edge(ary, nbins, left_p);

  nweights = RARRAY_LEN(edge) - 1;
  weights = rb_ary_new_capa(nweights);
  for (i = 0; i < nweights; ++i) {
    rb_ary_store(weights, i, INT2FIX(0));
  }

  histogram_weights_push_values(weights, edge, ary, left_p);

  return rb_struct_new(cHistogram, edge, weights,
                       left_p ? sym_left : sym_right,
                       Qfalse);
}
static VALUE
ary_dual_histograms(int argc, VALUE *argv, VALUE ary)
{
  VALUE arg0, opts;
  int left_p;

  VALUE ary_edge, compare_ary_edge;
  VALUE ary_nbins, compare_ary_nbins;
  long nbins, nweights, i;
  VALUE compare_ary;

  rb_scan_args(argc, argv, "01:", &arg0, &opts);

  left_p = opt_closed_left_p(opts);
  compare_ary = opt_compare_array(opts);
  ary_nbins = sturges(RARRAY_LEN(ary));
  ary_edge = ary_histogram_calculate_edge(ary, ary_nbins, left_p);

  /* compare_ary_nbins = sturges(RARRAY_LEN(ary)); */
  /* compare_edge = ary_histogram_calculate_edge(compare_ary, compare_ary_nbins, left_p); */

  return ary_edge;
}


void
Init_extension(void)
{
  VALUE mEnumerableStatistics;

#ifndef HAVE_ENUM_SUM
  rb_define_method(rb_mEnumerable, "sum", enum_sum, -1);
#endif

  rb_define_method(rb_mEnumerable, "mean_variance", enum_mean_variance_m, -1);
  rb_define_method(rb_mEnumerable, "mean", enum_mean, 0);
  rb_define_method(rb_mEnumerable, "variance", enum_variance, -1);
  rb_define_method(rb_mEnumerable, "mean_stdev", enum_mean_stdev, -1);
  rb_define_method(rb_mEnumerable, "stdev", enum_stdev, -1);
  rb_define_method(rb_mEnumerable, "value_counts", enum_value_counts, -1);

#ifndef HAVE_ARRAY_SUM
  rb_define_method(rb_cArray, "sum", ary_sum, -1);
#endif
  rb_define_method(rb_cArray, "mean_variance", ary_mean_variance_m, -1);
  rb_define_method(rb_cArray, "mean", ary_mean, 0);
  rb_define_method(rb_cArray, "variance", ary_variance, -1);
  rb_define_method(rb_cArray, "mean_stdev", ary_mean_stdev, -1);
  rb_define_method(rb_cArray, "stdev", ary_stdev, -1);
  rb_define_method(rb_cArray, "percentile", ary_percentile, 1);
  rb_define_method(rb_cArray, "median", ary_median, 0);
  rb_define_method(rb_cArray, "value_counts", ary_value_counts, -1);

  rb_define_method(rb_cHash, "value_counts", hash_value_counts, -1);

  half_in_rational = nurat_s_new_internal(rb_cRational, INT2FIX(1), INT2FIX(2));
  rb_gc_register_mark_object(half_in_rational);

  mEnumerableStatistics = rb_const_get_at(rb_cObject, rb_intern("EnumerableStatistics"));
  cHistogram = rb_const_get_at(mEnumerableStatistics, rb_intern("Histogram"));

  rb_define_method(rb_cArray, "histogram", ary_histogram, -1);
  rb_define_method(rb_cArray, "dual_histograms", ary_dual_histograms, -1);

  idPLUS = '+';
  idMINUS = '-';
  idSTAR = '*';
  idDIV = '/';
  idPow = rb_intern("**");
  idGE = rb_intern(">=");
  id_eqeq_p = rb_intern("==");
  id_idiv = rb_intern("div");
  id_negate = rb_intern("-@");
  id_to_f = rb_intern("to_f");
  id_cmp = rb_intern("<=>");
  id_nan_p = rb_intern("nan?");
  id_each = rb_intern("each");
  id_real_p = rb_intern("real?");
  id_sum = rb_intern("sum");
  id_population = rb_intern("population");
  id_closed = rb_intern("closed");
  id_compare = rb_intern("compare");
  id_edge = rb_intern("edge");

  sym_left = ID2SYM(rb_intern("left"));
  sym_right = ID2SYM(rb_intern("right"));
}
