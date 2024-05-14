# 2.0.8

- Prohibit the use of both `nbins` and `edges` kwargs simultaneously in the `histogram` method.
- Support `skip_na` kwarg in `sum` and related methods.
- Support Ruby 3.4+.

# 2.0.7

- Fix the bug of histogram with bin range that is smaller than value range

# 2.0.6

- Add edges parameter in histogram
- Rename parameter in histogram to fix typo: `weight` to `weights`

# 2.0.5

- Add weighted histogram support

# 2.0.4

- Add `find_min`, `find_max`, `argmin`, `argmax` methods
- Fix `nbin=:auto` case in `histogram` method

# 2.0.3

- Ractor-safe version

# 2.0.2

- Support Ruby 3.0

# 2.0.1

- Fix a bug of `histogram` (#9)

# 2.0.0

- Add `value_counts` method in Array, Hash, and Enumerable
- Add `median` method in Array
- Add `percentile` method in Array
- Add `histogram` method in Array

# 1.0.1

- Add `mean_variance` method in Array class and Enumerable module
- Add optimized implementation of `mean_variance` method for a Hash
- Add the following methods, implemented by using `mean_variance`, in Array class and Enumerable module
    - `mean`
    - `variance`
    - `stddev`
    - `mean_stddev`
- Add `sum` method in Array class and Enumerable module when Ruby < 2.4, that is almost same as introduced in Ruby 2.4
- Add optimized implementation of `sum` method for a Range with integer ends and a Hash, that is almost same as introduced in Ruby 2.4

# 1.0.0

- This version was yanked due to documentation issue
