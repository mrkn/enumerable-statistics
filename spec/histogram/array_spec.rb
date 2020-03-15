require 'spec_helper'
require 'enumerable/statistics'

RSpec.describe Array, '#histogram' do
  let(:ary) { [] }
  let(:args) { [] }
  let(:kwargs) { {} }

  subject(:histogram) { ary.histogram(*args, **kwargs) }

  context 'dualing_histogram' do
    specify do

      a = [1,1,1, 5, 5, 5, 5, 10, 10, 10]
      b = [7, 7, 7, 12, 12, 12, 12, 20, 20, 20]
      expect(a.histogram.edge.first).to_not eq(b.histogram.edge.first)
      expect(a.histogram.edge.last).to_not eq(b.histogram.edge.last)

      expect(a.dual_histograms(compare: b).edge.first).to eq(b.dual_histograms(compare: a).edge.first)
      expect(a.dual_histograms(compare: b).edge.last).to eq(b.dual_histograms(compare: a).edge.last)
    end
  end


  with_array [] do
    context 'default' do
      specify do
        expect(histogram.edge).to eq([0.0])
        expect(histogram.weights).to eq([])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end

    context 'closed: :right' do
      let(:kwargs) { {closed: :right} }

      specify do
        expect(histogram.edge).to eq([0.0])
        expect(histogram.weights).to eq([])
        expect(histogram.closed).to eq(:right)
        expect(histogram.density?).to eq(false)
      end
    end
  end

  with_array [1] do
    context 'default' do
      specify do
        expect(histogram.edge).to eq([1.0, 2.0])
        expect(histogram.weights).to eq([1])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end

    context 'nbins: 5' do
      let(:args) { [5] }

      specify do
        expect(histogram.edge).to eq([1.0, 2.0])
        expect(histogram.weights).to eq([1])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end
  end

  with_array [1, 2] do
    context 'closed: :left' do
      let(:kwargs) { {closed: :left} }

      specify do
        expect(histogram.edge).to eq([1.0, 1.5, 2.0, 2.5])
        expect(histogram.weights).to eq([1, 0, 1])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end
  end

  with_array [1, 2, 3, 4, 5, 6, 7, 8, 9] do
    context 'default' do
      specify do
        expect(histogram.edge).to eq([0.0, 2.0, 4.0, 6.0, 8.0, 10.0])
        expect(histogram.weights).to eq([1, 2, 2, 2, 2])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end

    context 'closed: :right' do
      let(:kwargs) { {closed: :right} }

      specify do
        expect(histogram.edge).to eq([0.0, 2.0, 4.0, 6.0, 8.0, 10.0])
        expect(histogram.weights).to eq([2, 2, 2, 2, 1])
        expect(histogram.closed).to eq(:right)
        expect(histogram.density?).to eq(false)
      end
    end

    context 'nbins: 3' do
      let(:args) { [3] }

      specify do
        expect(histogram.edge).to eq([0.0, 5.0, 10.0])
        expect(histogram.weights).to eq([4, 5])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end
  end

  context "with 10,000 normal random values" do
    let(:ary) do
      random = Random.new(13)
      Array.new(10000) do
        # Box-Muller method
        x, y = random.rand, random.rand
        Math.sqrt(-2 * Math.log(x)) * Math.cos(2 * Math::PI * y)
      end
    end

    context "default" do
      specify do
        expect(histogram.edge).to eq([-4.0, -3.5, -3.0, -2.5, -2.0, -1.5,
                                      -1.0, -0.5,  0.0,  0.5,  1.0,  1.5,
                                       2.0,  2.5,  3.0,  3.5])
        expect(histogram.weights).to eq([2, 14, 50, 161, 451,
                                         884, 1508, 1880, 1966, 1538,
                                         893, 432, 160, 51, 10])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end

    context "closed: :right" do
      let(:kwargs) { {closed: :right} }

      specify do
        expect(histogram.edge).to eq([-4.0, -3.5, -3.0, -2.5, -2.0, -1.5,
                                      -1.0, -0.5,  0.0,  0.5,  1.0,  1.5,
                                       2.0,  2.5,  3.0,  3.5])
        expect(histogram.weights).to eq([2, 14, 50, 161, 451,
                                         884, 1508, 1880, 1966, 1538,
                                         893, 432, 160, 51, 10])
        expect(histogram.closed).to eq(:right)
        expect(histogram.density?).to eq(false)
      end
    end

    context "nbins: 5" do
      let(:args) { [5] }

      specify do
        expect(histogram.edge).to eq([-4.0, -2.0, 0.0, 2.0, 4.0])
        expect(histogram.weights).to eq([227, 4723, 4829, 221])
        expect(histogram.closed).to eq(:left)
        expect(histogram.density?).to eq(false)
      end
    end
  end
end
