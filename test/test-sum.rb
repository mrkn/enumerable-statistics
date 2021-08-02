require "delegate"

class SumTest < Test::Unit::TestCase
  large_number = 100_000_000
  small_number = 1e-9
  until (large_number + small_number) == large_number
    small_number /= 10
  end

  generic_test_data = [
    [ [], 0, 0 ],
    [ [], 0.0, 0.0 ],
    [ [3], 0, 3 ],
    [ [3], 0.0, 3.0 ],
    [ [3, 5], 0, 8 ],
    [ [3, 5, 7], 0, 15 ],
    [ [3, Rational(5)], 0, Rational(8) ],
    [ [3, 5, 7.0], 0, 15.0 ],
    [ [3, Rational(5), 7.0], 0, 15.0 ],
    [ [3, Rational(5), Complex(0, 1)], 0, Complex(Rational(8), 1) ],
    [ [3, Rational(5), 7.0, Complex(0, 1)], 0, Complex(15.0, 1) ],
    [ [3.5, 5], 0, 8.5 ],
    [ [2, 8.5], 0, 10.5 ],
    [ [Rational(1, 2), 1], 0, Rational(3, 2) ],
    [ [Rational(1, 2), Rational(1, 3)], 0, Rational(5, 6) ],
    [ [2.0, Complex(0, 3.0)], 0, Complex(2.0, 3.0) ],
    [ [1, 2], 10, 13],
    [ [large_number, *[small_number]*10], 0, large_number + small_number*10 ],
    [ [Rational(large_number), *[small_number]*10], 0, large_number + small_number*10 ],
    [ [small_number, Rational(large_number), *[small_number]*10], 0, large_number + small_number*11 ],
    [ ["a", "b", "c"], "", "abc"],
    [ [[1], [[2]], [3]], [], [1, [2], 3]],
  ].map { |recv, init, expected_result|
    [ "#{recv.inspect}.sum(#{init.inspect}) == #{expected_result.inspect}", [recv, init, expected_result]]
  }.to_h

  sub_test_case("Array#sum") do
    data(generic_test_data)
    def test_generic_case(data)
      ary, init, expected_result, conversion = data
      actual_result = ary.sum(init, &conversion)
      assert_equal({
                     value: expected_result,
                     class: expected_result.class
                   },
                   {
                     value: actual_result,
                     class: actual_result.class
                   })
    end

    def test_sum_with_block
      ary = [1, 2, SimpleDelegator.new(3)]
      yielded = []
      result = ary.sum(0) {|x| yielded << x; 2*x }
      assert_equal({ result: 12,     yielded: ary },
                   { result: result, yielded: yielded })
    end

    def test_skip_na_false
      ary = [1, 2, nil, SimpleDelegator.new(3)]
      assert_raise(TypeError) do
        ary.sum(0, skip_na: false)
      end
    end

    def test_skip_na_true
      ary = [1, 2, nil, SimpleDelegator.new(3)]
      result = ary.sum(0, skip_na: true)
      assert_equal(6, result)
    end

    def test_skip_na_true_with_block
      ary = [1, 2, nil, SimpleDelegator.new(3)]
      result = ary.sum(0, skip_na: true) {|x| x || 10 }
      assert_equal(16, result)
    end

    def test_type_error
      assert_raise(TypeError) do
        [Object.new].sum(0)
      end
    end
  end

  sub_test_case("Enumerable#sum") do
    test_data = generic_test_data.map {|key, value|
      [ key.sub(/\.sum/, ".each.sum"), value ]
    }.to_h
    data(test_data)
    def test_generic_case(data)
      ary, init, expected_result, conversion = data
      actual_result = ary.each.sum(init, &conversion)
      assert_equal({
                     value: expected_result,
                     class: expected_result.class
                   },
                   {
                     value: actual_result,
                     class: actual_result.class
                   })
    end

    def test_sum_with_block
      ary = [1, 2, SimpleDelegator.new(3)]
      yielded = []
      result = ary.each.sum(0) {|x| yielded << x; 2*x }
      assert_equal({ result: 12,     yielded: ary },
                   { result: result, yielded: yielded })
    end

    def test_skip_na_false
      ary = [1, 2, nil, SimpleDelegator.new(3)]
      assert_raise(TypeError) do
        ary.each.sum(0, skip_na: false)
      end
    end

    def test_skip_na_true
      ary = [1, 2, nil, SimpleDelegator.new(3)]
      result = ary.each.sum(0, skip_na: true)
      assert_equal(6, result)
    end

    def test_type_error
      assert_raise(TypeError) do
        [Object.new].each.sum(0)
      end
    end
  end

  sub_test_case("Hash#sum") do
    def test_skip_na_false
      hash = {
        a: 1,
        b: 2,
        c: nil,
        d: 3
      }
      assert_raise(TypeError) do
        hash.sum(0, skip_na: false, &:last)
      end
    end

    def test_skip_na_true
      hash = {
        a: 1,
        b: 2,
        c: nil,
        d: 3
      }
      result = hash.sum(0, skip_na: true, &:last)
      assert_equal(6, result)
    end
  end
end
