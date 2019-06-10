require 'spec_helper'

RSpec.describe Array, '#percentile' do
  let(:ary) { [] }
  let(:args) { [0, 25, 50, 75, 100] }

  subject(:percentile) { ary.percentile(args) }

  with_array [] do
    specify do
      expect { percentile }.to raise_error(ArgumentError)
    end
  end

  with_array [1] do
    specify do
      expect(percentile).to eq([1.0, 1.0, 1.0, 1.0, 1.0])
    end
  end

  with_array [1, 2, 3] do
    specify do
      expect(percentile).to eq([1.0, 1.5, 2.0, 2.5, 3.0])
    end
  end

  with_array [1, 2] do
    let(:args) { 50 }
    specify do
      expect(percentile).to eq(1.5)
    end
  end

  with_array [1, 2] do
    let(:args) { [50] }
    specify do
      expect(percentile).to eq([1.5])
    end
  end

  with_array [1, 2, 3] do
    let(:args) { [100, 25, 0, 75, 50] }
    specify do
      expect(percentile).to eq([3.0, 1.5, 1.0, 2.5, 2.0])
    end
  end

  with_array [1, Float::NAN, 3] do
    let(:args) { [100, 25] }
    specify do
      expect(percentile).to match([be_nan, be_nan])
    end
  end

  with_array [1, nil, 3] do
    let(:args) { [100, 25] }
    specify do
      expect(percentile).to match([be_nan, be_nan])
    end
  end
end
