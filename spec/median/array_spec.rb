require 'spec_helper'

RSpec.describe Array, '#median' do
  let(:ary) { [] }

  subject(:median) { ary.median }

  with_array [] do
    it_is_float_nan
  end

  with_array [1] do
    it_is_int_equal(1)
  end

  with_array [0, 1] do
    it_is_float_equal(0.5)
  end

  with_array [0.0444502, 0.0463301, 0.141249, 0.0606775] do
    it_is_float_equal((0.0463301 + 0.0606775) / 2.0)
  end

  with_array [0.0463301, 0.0444502, 0.141249] do
    it_is_float_equal(0.0463301)
  end

  with_array [0.0444502, 0.141249, 0.0463301] do
    it_is_float_equal(0.0463301)
  end

  with_array [0.0444502, Float::NAN, 0.0463301] do
    it_is_float_nan
  end

  with_array [0.0444502, nil, 0.0463301] do
    it_is_float_nan
  end
end
