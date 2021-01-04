require 'spec_helper'
require 'enumerable/statistics'
require 'delegate'

RSpec.describe Enumerable do
  describe '#mean' do
    subject(:mean) { enum.mean(&block) }
    let(:block) { nil }

    with_enum [] do
      it_is_float_equal(0.0)

      context 'with a conversion block' do
        it_is_float_equal(0.0)

        it 'does not call the block' do
          expect { |b|
            enum.mean(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum [3] do
      it_is_float_equal(3.0)

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_equal(6.0)
      end
    end

    with_enum [3, 5] do
      it_is_float_equal(4.0)

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_equal(8.0)
      end
    end

    with_enum [3, Rational(5), Complex(7, 3)] do
      it_is_complex_equal(Complex(5.0, 1.0))
    end

    with_enum [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    large_number = 100_000_000
    small_number = 1e-9
    until (large_number + small_number) == large_number
      small_number /= 10
    end

    with_enum [large_number, *[small_number]*10] do
      it_is_float_equal((large_number + small_number*10)/11.0)
    end

    with_enum [Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal((large_number + small_number*10)/11.0)
    end

    with_enum [small_number, Rational(large_number, 1), *[small_number]*10] do
      it_is_float_equal((Rational(large_number, 1) + small_number*11)/12.0)
    end
  end

  describe '#variance' do
    subject(:variance) { enum.variance(&block) }
    let(:enum) { [].to_enum }
    let(:block) { nil }

    with_enum [] do
      it_is_float_nan

      context 'with a conversion block' do
        it_is_float_nan

        it 'does not call the block' do
          expect { |b|
            enum.variance(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum [3] do
      it_is_float_nan

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_nan
      end
    end

    with_enum [3, 5] do
      it_is_float_equal(2.0)

      context 'with population: nil' do
        subject(:variance) { enum.variance(population: nil, &block) }
        it_is_float_equal(2.0)
      end

      context 'with population: false' do
        subject(:variance) { enum.variance(population: false, &block) }
        it_is_float_equal(2.0)
      end

      context 'with population: true' do
        subject(:variance) { enum.variance(population: true, &block) }
        it_is_float_equal(1.0)
      end
    end

    with_enum [3.0, 5.0] do
      it_is_float_equal(2.0)
    end

    with_enum [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    with_enum [Object.new, Object.new] do
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

    with_enum ary do
      it_is_float_equal(var)
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_enum ary do
      it { is_expected.to be > 0.0 }
    end
  end

  describe '#mean_variance' do
    subject(:mean_variance) { enum.mean_variance(&block) }
    let(:enum) { [].each }
    let(:block) { nil }


    with_enum [] do
      specify do
        expect(subject[0]).to eq(0.0)
        expect(subject[1]).to be_nan
      end

      context 'with a conversion block' do
        it 'does not call the block' do
          expect { |b|
            enum.mean_variance(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum [3] do
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

    with_enum [3.0, 5.0] do
      context 'with population: nil' do
        subject(:mean_variance) { enum.mean_variance(population: nil, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(2.0)
        end
      end

      context 'with population: false' do
        subject(:mean_variance) { enum.mean_variance(population: false, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(2.0)
        end
      end

      context 'with population: true' do
        subject(:mean_variance) { enum.mean_variance(population: true, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(1.0)
        end
      end
    end

    with_enum [Object.new] do
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

    with_enum ary do
      it { is_expected.to eq([m, var]) }
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_enum ary do
      it { is_expected.to eq([ary.mean, ary.variance]) }
    end
  end

  describe '#stdev' do
    subject(:stdev) { enum.stdev(&block) }
    let(:enum) { [].to_enum }
    let(:block) { nil }

    with_enum [] do
      it_is_float_nan

      context 'with a conversion block' do
        it_is_float_nan

        it 'does not call the block' do
          expect { |b|
            enum.stdev(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum [3] do
      it_is_float_nan

      with_conversion ->(v) { v * 2 }, 'v * 2' do
        it_is_float_nan
      end
    end

    with_enum [3, 5] do
      it_is_float_equal(Math.sqrt(2.0))

      context 'with population: nil' do
        subject(:stdev) { enum.stdev(population: nil, &block) }
        it_is_float_equal(Math.sqrt(2.0))
      end

      context 'with population: false' do
        subject(:stdev) { enum.stdev(population: false, &block) }
        it_is_float_equal(Math.sqrt(2.0))
      end

      context 'with population: true' do
        subject(:stdev) { enum.stdev(population: true, &block) }
        it_is_float_equal(1.0)
      end
    end

    with_enum [3.0, 5.0] do
      it_is_float_equal(Math.sqrt(2.0))
    end

    with_enum [Object.new] do
      specify do
        expect { subject }.to raise_error(TypeError)
      end
    end

    with_enum [Object.new, Object.new] do
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

    with_enum ary do
      it_is_float_equal(sd)
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_enum ary do
      it { is_expected.to be > 0.0 }
    end
  end

  describe '#mean_stdev' do
    subject(:mean_stdev) { enum.mean_stdev(&block) }
    let(:enum) { [].each }
    let(:block) { nil }


    with_enum [] do
      specify do
        expect(subject[0]).to eq(0.0)
        expect(subject[1]).to be_nan
      end

      context 'with a conversion block' do
        it 'does not call the block' do
          expect { |b|
            enum.mean_stdev(&b)
          }.not_to yield_control
        end
      end
    end

    with_enum [3] do
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

    with_enum [3.0, 5.0] do
      context 'with population: nil' do
        subject(:mean_stdev) { enum.mean_stdev(population: nil, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(Math.sqrt(2.0))
        end
      end

      context 'with population: false' do
        subject(:mean_stdev) { enum.mean_stdev(population: false, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(Math.sqrt(2.0))
        end
      end

      context 'with population: true' do
        subject(:mean_stdev) { enum.mean_stdev(population: true, &block) }
        specify do
          expect(subject[0]).to eq(4.0)
          expect(subject[1]).to eq(1.0)
        end
      end
    end

    with_enum [Object.new] do
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

    with_enum ary do
      it { is_expected.to eq([m, sd]) }
    end

    while true
      ary = Array.new(4) { 1.0 + rand*1e-6 }
      x = ary.map { |e| e**2 }.sum / ary.length.to_f
      y = (ary.sum / ary.length.to_f) ** 2
      break if x < y
    end

    with_enum ary do
      it { is_expected.to eq([ary.mean, Math.sqrt(ary.variance)]) }
    end
  end
end
