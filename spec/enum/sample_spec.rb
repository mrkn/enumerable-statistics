require 'spec_helper'
require 'enumerable/statistics'

RSpec.describe Enumerable, '#sample' do
  let(:random) { Random.new }
  let(:n) { 20 }

  let(:replace) { nil }
  let(:weights) { nil }
  let(:opts) { {} }

  before do
    opts[:replace] = replace if replace
    opts[:weights] = weights if weights
  end

  context 'when the receiver has 1 item' do
    let(:enum) { 1.upto(1) }

    shared_examples_for '1-item enumerable' do
      context 'without replacement' do
        specify { expect(opts).not_to include(:replace) }

        specify do
          expect(enum.sample(**opts)).to eq(1)

          expect(enum.sample(10, **opts)).to eq([1])
          expect(enum.sample(20, **opts)).to eq([1])
        end
      end

      context 'with replacement' do
        let(:replace) { true }

        specify { expect(opts).to include(replace: true) }

        specify do
          expect(enum.sample(10, **opts)).to eq(Array.new(10, 1))
          expect(enum.sample(20, **opts)).to eq(Array.new(20, 1))
        end
      end
    end

    context 'without weights' do
      specify { expect(opts).not_to include(:weights) }

      include_examples '1-item enumerable'
    end

    # TODO: weights
    xcontext 'with weights' do
      let(:weights) do
        { 1 => 1.0 }
      end

      specify { expect(opts).to include(weights: weights) }

      include_examples '1-item enumerable'
    end
  end

  context 'when the receiver has 2 item' do
    let(:enum) { 1.upto(2) }

    shared_examples_for 'sample from 2-item enumerable without replacement' do
      specify { expect(opts).not_to include(:replace) }

      specify do
        expect(Array.new(100) { enum.sample(**opts) }).to all(eq(1).or eq(2))

        expect(enum.sample(10, **opts)).to contain_exactly(1, 2)
        expect(enum.sample(20, **opts)).to contain_exactly(1, 2)
      end
    end

    context 'without weights' do
      context 'without replacement' do
        it_behaves_like 'sample from 2-item enumerable without replacement'
      end

      context 'with replacement' do
        let(:replace) { true }

        specify { expect(opts).to include(replace: true) }

        specify do
          expect(enum.sample(10, **opts)).to have_attributes(length: 10).and all(eq(1).or eq(2))
          expect(enum.sample(20, **opts)).to have_attributes(length: 20).and all(eq(1).or eq(2))
        end
      end
    end

    # TODO: weights
    xcontext 'with weights' do
      specify { expect(opts).to include(weights: weights) }

      context 'without replacement' do
        it_behaves_like 'sample from 2-item enumerable without replacement'
      end

      context 'with replacement' do
        let(:replace) { true }

        specify { expect(opts).to include(replace: true) }
      end
    end
  end

  context 'without weight' do
    let(:enum) { 1.upto(100000) }

    context 'without size' do
      context 'without rng' do
        context 'without weight' do
          specify do
            result = enum.sample
            expect(result).to be_an(Integer)
            other_results = Array.new(100) { enum.sample }
            expect(other_results).not_to be_all {|i| i == result }
          end
        end
      end

      context 'with rng' do
        specify do
          save_random = random.dup
          result = enum.sample(random: random)
          expect(result).to be_an(Integer)
          other_results = Array.new(100) { enum.sample(random: save_random.dup) }
          expect(other_results).to be_all {|i| i == result }
        end
      end
    end

    context 'with size (== 1)' do
      context 'without rng' do
        context 'without weight' do
          specify do
            result = enum.sample(1)
            expect(result).to be_an(Integer)
            other_results = Array.new(100) { enum.sample(1) }
            expect(other_results).not_to be_all {|i| i == result }
          end
        end
      end

      context 'with rng' do
        specify do
          save_random = random.dup
          result = enum.sample(1, random: random)
          expect(result).to be_an(Integer)
          other_results = Array.new(100) { enum.sample(1, random: save_random.dup) }
          expect(other_results).to be_all {|i| i == result }
        end
      end
    end

    context 'with size (> 1)' do
      context 'without replacement' do
        context 'without rng' do
          subject(:result) { enum.sample(n) }

          specify do
            result = enum.sample(n)
            expect(result).to be_an(Array)
            expect(result.length).to eq(n)
            expect(result.uniq.length).to eq(n)
            other_results = Array.new(100) { enum.sample(n) }
            expect(other_results).not_to be_all {|i| i == result }
          end
        end

        context 'with rng' do
          subject(:result) { enum.sample(n, random: random) }

          specify do
            save_random = random.dup
            result = enum.sample(n, random: random)
            expect(result).to be_an(Array)
            expect(result.length).to eq(n)
            expect(result.uniq.length).to eq(n)
            other_results = Array.new(100) { enum.sample(n, random: save_random.dup) }
            expect(other_results).to be_all {|i| i == result }
          end
        end
      end

      context 'with replacement' do
        pending
      end
    end
  end

  context 'with weight' do
    pending
  end
end
