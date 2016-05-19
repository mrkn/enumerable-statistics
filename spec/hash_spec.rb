require 'spec_helper'
require 'enumerable/statistics'
require 'delegate'

RSpec.describe Hash do
  describe '#sum' do
    with_enum({}) do
      it_is_int_equal(0)

      with_init(0.0) do
        it_is_float_equal(0.0)
      end

      context 'with a conversion block' do
        it 'does not call the conversion block' do
          expect { |b|
            enum.sum(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum({ a: 3 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_int_equal(3)

        with_init(0.0) do
          it_is_float_equal(3.0)
        end
      end
    end

    with_enum({ a: 3, b: 5 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_int_equal(8)
      end
    end

    with_enum({ a: 3, b: 5, c: 7 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_int_equal(15)
      end
    end

    with_enum({ a: 3, b: Rational(5) }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_rational_equal(Rational(8))
      end
    end

    with_enum({ a: 3, b: 5, c: 7.0 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(15.0)
      end
    end

    with_enum({ a: 3, b: Rational(5), c: 7.0 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(15.0)
      end
    end

    with_enum({ a: 3, b: Rational(5), c: Complex(0, 1) }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_complex_equal(Complex(8, 1))
      end
    end

    with_enum({ a: 3, b: Rational(5), c: 7.0, d: Complex(0, 1) }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_complex_equal(Complex(15.0, 1))
      end
    end

    with_enum({ a: 3.5, b: 5 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(8.5)
      end
    end

    with_enum({ a: 2, b: 8.5 }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(10.5)
      end
    end

    with_enum({ a: Rational(1, 2), b: Rational(1, 3) }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_rational_equal(Rational(5, 6))
      end
    end

    with_enum({ a: 2.0, b: Complex(0, 3.0) }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_complex_equal(Complex(2.0, 3.0))
      end
    end

    it 'calls a block for each item once' do
      yielded = []
      three = SimpleDelegator.new(3)
      hash = { a: 1, b: 2.0, c: three }
      expect(hash.sum {|k, x| yielded << x; x * 2 }).to eq(12.0)
      expect(yielded).to eq(hash.values)
    end

    with_enum({ a: Object.new }) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        specify do
          expect { subject }.to raise_error(TypeError)
        end
      end
    end

    large_number = 100_000_000
    small_number = 1e-9
    until (large_number + small_number) == large_number
      small_number /= 10
    end

    
    with_enum Hash[[*:a..:k].zip([large_number, *[small_number]*10])] do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(large_number + small_number*10)
      end
    end

    with_enum Hash[[*:a..:k].zip([Rational(large_number, 1), *[small_number]*10])] do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(large_number + small_number*10)
      end
    end

    with_enum Hash[[*:a..:l].zip([small_number, Rational(large_number, 1), *[small_number]*10])] do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        it_is_float_equal(large_number + small_number*11)
      end
    end

    with_enum({ a: "a", b: "b", c: "c"}) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        with_init("") do
          it { is_expected.to eq("abc") }
        end
      end
    end

    with_enum({ a: [1], b: [[2]], c: [3]}) do
      with_conversion ->(k, v) { v }, '(k, v) -> v' do
        with_init([]) do
          it { is_expected.to eq([1, [2], 3]) }
        end
      end
    end

    with_enum({ 1 => 2, 3 => 4, 5 => 6 }) do
      with_conversion ->(k, v) { k * v }, '(k, v) -> k * v' do
        it_is_int_equal(44)
      end
    end
  end
end
