require 'spec_helper'
require 'enumerable/statistics'
require 'delegate'

RSpec.describe Array do
  describe '#sum' do
    with_array [] do
      it_is_int_equal(0)

      with_init(0.0) do
        it_is_float_equal(0.0)
      end

      context 'with a conversion block' do
        it 'does not call the conversion block' do
          expect { |b|
            ary.sum(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      it_is_int_equal(3)

      with_init(0.0) do
        it_is_float_equal(3.0)
      end
    end

    with_array [3, 5] do
      it_is_int_equal(8)
    end

    with_array [3, 5, 7] do
      it_is_int_equal(15)
    end

    with_array [3, Rational(5)] do
      it_is_rational_equal(Rational(8))
    end

    with_array [3, 5, 7.0] do
      it_is_float_equal(15.0)
    end

    with_array [3, Rational(5), 7.0] do
      it_is_float_equal(15.0)
    end

    with_array [3, Rational(5), Complex(0, 1)] do
      it_is_complex_equal(Complex(Rational(8), 1))
    end

    with_array [3, Rational(5), 7.0, Complex(0, 1)] do
      it_is_complex_equal(Complex(15.0, 1))
    end

    with_array [3.5, 5] do
      it_is_float_equal(8.5)
    end

    with_array [2, 8.5] do
      it_is_float_equal(10.5)
    end

    with_array [Rational(1, 2), 1] do
      it_is_rational_equal(Rational(3, 2))
    end

    with_array [Rational(1, 2), Rational(1, 3)] do
      it_is_rational_equal(Rational(5, 6))
    end

    with_array [2.0, Complex(0, 3.0)] do
      it_is_complex_equal(Complex(2.0, 3.0))
    end

    with_array [1, 2] do
      with_init(10)do
        it_is_int_equal(13)

        with_conversion ->(v) { v * 2 }, 'v * 2' do
          it_is_int_equal(16)
        end
      end
    end

    it 'calls a block for each item once' do
      yielded = []
      three = SimpleDelegator.new(3)
      ary = [1, 2.0, three]
      expect(ary.sum {|x| yielded << x; x * 2 }).to eq(12.0)
      expect(yielded).to eq(ary)
    end

    with_array [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    large_number = 100_000_000
    small_number = 1e-9
    until (large_number + small_number) == large_number
      small_number /= 10
    end

    with_array [large_number, *[small_number]*10] do
      it_is_float_equal(large_number + small_number*10)
    end

    with_array [Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal(large_number + small_number*10)
    end

    with_array [small_number, Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal(large_number + small_number*11)
    end

    with_array ["a", "b", "c"] do
      with_init("") do
        it { is_expected.to eq("abc") }
      end
    end

    with_array [[1], [[2]], [3]] do
      with_init([]) do
        it { is_expected.to eq([1, [2], 3]) }
      end
    end
  end
end
