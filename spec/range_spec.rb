require 'spec_helper'
require 'enumerable/statistics'
require 'delegate'

RSpec.describe Enumerable do
  describe '#sum' do
    with_enum 1..0 do
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

    with_enum 3..3 do
      it_is_int_equal(3)

      with_init(0.0) do
        it_is_float_equal(3.0)
      end
    end

    with_enum 3..5 do
      it_is_int_equal(12)
    end

    with_enum 1..2 do
      with_init(10)do
        it_is_int_equal(13)

        with_conversion ->(v) { v * 2 }, 'v * 2' do
          it_is_int_equal(16)
        end
      end
    end

    it 'calls a block for each item once' do
      yielded = []
      range = 1..3
      expect(range.each.sum {|x| yielded << x; x * 2 }).to eq(12)
      expect(yielded).to eq(range.to_a)
    end

    with_enum :a..:b do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    with_enum "a".."c" do
      with_init("") do
        it { is_expected.to eq("abc") }
      end
    end
  end
end
