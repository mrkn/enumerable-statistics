require 'spec_helper'
require 'enumerable/statistics'
require 'delegate'

RSpec.describe Array do
  describe '#mean' do
    let(:ary) { [] }
    let(:block) { nil }

    subject(:mean) { ary.mean(&block) }

    with_array [] do
      it_is_float_equal(0.0)

      context 'with a conversion block' do
        it_is_float_equal(0.0)

        it 'does not call the block' do
          expect { |b|
            ary.mean(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      it_is_float_equal(3.0)

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_equal(6.0)
      end
    end

    with_array [3, 5] do
      it_is_float_equal(4.0)

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_equal(8.0)
      end
    end

    with_array [3, Rational(5), Complex(7, 3)] do
      it_is_complex_equal(Complex(5.0, 1.0))
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
      it_is_float_equal((large_number + small_number*10)/11.0)
    end

    with_array [Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal((large_number + small_number*10)/11.0)
    end

    with_array [small_number, Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal((Rational(large_number, 1) + small_number*11)/12.0)
    end
  end

  describe '#variance' do
    let(:ary) { [] }
    let(:block) { nil }

    subject(:variance) { ary.variance(&block) }

    with_array [] do
      it_is_float_nan

      context 'with a conversion block' do
        it_is_float_nan

        it 'does not call the block' do
          expect { |b|
            ary.variance(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      it_is_float_nan

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_nan
      end
    end

    with_array [3, 5] do
      it_is_float_equal(2.0)
    end

    with_array [3.0, 5.0] do
      it_is_float_equal(2.0)

      context 'with population: nil' do
        subject(:variance) { ary.variance(population: nil, &block) }
        it_is_float_equal(2.0)
      end

      context 'with population: false' do
        subject(:variance) { ary.variance(population: false, &block) }
        it_is_float_equal(2.0)
      end

      context 'with population: true' do
        subject(:variance) { ary.variance(population: true, &block) }
        it_is_float_equal(1.0)
      end
    end

    with_array [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    with_array [Object.new, Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    large_number = 100_000_000
    small_number = 1e-9
    until (large_number + small_number) == large_number
      small_number /= 10
    end

    ary = [large_number, *[small_number]*10]
    m = ary.mean
    s2 = ary.map { |x| (x - m)**2 }.sum
    var = s2 / (ary.length - 1).to_f

    with_array ary do
      it_is_float_equal(var)
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_array ary do
      it { is_expected.to be > 0.0 }
    end
  end

  describe '#mean_variance' do
    let(:ary) { [] }
    let(:block) { nil }

    subject(:mean_variance) { ary.mean_variance(&block) }

    with_array [] do
      specify do
        expect(subject[0]).to eq(0.0)
        expect(subject[1]).to be_nan
      end

      context 'with a conversion block' do
        it 'does not call the block' do
          expect { |b|
            ary.mean_variance(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      specify do
        expect(subject[0]).to eq(3.0)
        expect(subject[1]).to be_nan
      end

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        specify do
          expect(subject[0]).to eq(6.0)
          expect(subject[1]).to be_nan
        end
      end
    end

    with_array [3.0, 5.0] do
      context 'with population: nil' do
        subject(:mean_variance) { ary.mean_variance(population: nil, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(2.0)
        end
      end

      context 'with population: false' do
        subject(:mean_variance) { ary.mean_variance(population: false, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(2.0)
        end
      end

      context 'with population: true' do
        subject(:mean_variance) { ary.mean_variance(population: true, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(1.0)
        end
      end
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

    ary = [large_number, *[small_number]*10]
    m = ary.mean
    s2 = ary.map { |x| (x - m)**2 }.sum
    var = s2 / (ary.length - 1).to_f

    with_array ary do
      it { is_expected.to eq([m, var]) }
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_array ary do
      it { is_expected.to eq([ary.mean, ary.variance]) }
    end
  end

  describe '#mean_stdev' do
    let(:ary) { [] }
    let(:block) { nil }

    subject(:mean_stdev) { ary.mean_stdev(&block) }

    with_array [] do
      specify do
        expect(subject[0]).to eq(0.0)
        expect(subject[1]).to be_nan
      end

      context 'with a conversion block' do
        it 'does not call the block' do
          expect { |b|
            ary.mean_stdev(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      specify do
        expect(subject[0]).to eq(3.0)
        expect(subject[1]).to be_nan
      end

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        specify do
          expect(subject[0]).to eq(6.0)
          expect(subject[1]).to be_nan
        end
      end
    end

    with_array [3.0, 5.0] do
      context 'with population: nil' do
        subject(:mean_stdev) { ary.mean_stdev(population: nil, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(Math.sqrt(2.0))
        end
      end

      context 'with population: false' do
        subject(:mean_stdev) { ary.mean_stdev(population: false, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(Math.sqrt(2.0))
        end
      end

      context 'with population: true' do
        subject(:mean_stdev) { ary.mean_stdev(population: true, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(1.0)
        end
      end
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

    ary = [large_number, *[small_number]*10]
    m = ary.mean
    s2 = ary.map { |x| (x - m)**2 }.sum
    var = s2 / (ary.length - 1).to_f
    sd = Math.sqrt(var)

    with_array ary do
      it { is_expected.to eq([m, sd]) }
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_array ary do
      it { is_expected.to eq([ary.mean, Math.sqrt(ary.variance)]) }
    end
  end

  describe '#stdev' do
    let(:ary) { [] }
    let(:block) { nil }

    subject(:stdev) { ary.stdev(&block) }

    with_array [] do
      it_is_float_nan

      context 'with a conversion block' do
        it_is_float_nan

        it 'does not call the block' do
          expect { |b|
            ary.stdev(&b)
          }.not_to yield_control
        end
      end
    end

    with_array [3] do
      it_is_float_nan

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_nan
      end
    end

    with_array [3, 5] do
      it_is_float_equal(Math.sqrt(2.0))
    end

    with_array [3.0, 5.0] do
      it_is_float_equal(Math.sqrt(2.0))

      context 'with population: nil' do
        subject(:stdev) { ary.stdev(population: nil, &block) }
        it_is_float_equal(Math.sqrt(2.0))
      end

      context 'with population: false' do
        subject(:stdev) { ary.stdev(population: false, &block) }
        it_is_float_equal(Math.sqrt(2.0))
      end

      context 'with population: true' do
        subject(:stdev) { ary.stdev(population: true, &block) }
        it_is_float_equal(1.0)
      end
    end

    with_array [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    with_array [Object.new, Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    large_number = 100_000_000
    small_number = 1e-9
    until (large_number + small_number) == large_number
      small_number /= 10
    end

    ary = [large_number, *[small_number]*10]
    m = ary.mean
    s2 = ary.map { |x| (x - m)**2 }.sum
    var = s2 / (ary.length - 1).to_f
    sd = Math.sqrt(var)

    with_array ary do
      it_is_float_equal(sd)
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_array ary do
      it { is_expected.to be > 0.0 }
    end
  end
end
