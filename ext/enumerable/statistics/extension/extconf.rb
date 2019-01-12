require 'mkmf'

have_type('struct RRational')
have_func('rb_rational_new')
have_func('rb_rational_num')
have_func('rb_rational_den')
have_func('rb_rational_plus')

have_type('struct RComplex')
have_func('rb_complex_raw')
have_func('rb_complex_real')
have_func('rb_complex_imag')
have_func('rb_complex_plus')
have_func('rb_complex_div')
have_func('rb_dbl_complex_new')

create_makefile('enumerable/statistics/extension')
