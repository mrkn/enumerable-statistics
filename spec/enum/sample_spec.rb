require 'spec_helper'
require 'enumerable/statistics'

RSpec.describe Enumerable, '#sample' do
  let(:random) { Random.new }
  let(:n) { 20 }

  context 'without weight' do
    let(:enum) { 1.upto(100000) }

    context 'without size' do
      context 'without rng' do
        context 'without weight' do
          specify do
            result = enum.sample
            expect(result).to be_an(Integer)
            other_results = Array.new(100) { enum.sample }
            expect(other_results).not_to be_all(eq result)
          end
        end
      end

      context 'with rng' do
        specify do
          save_random = random.dup
          result = enum.sample(random: random)
          expect(result).to be_an(Integer)
          other_results = Array.new(100) { enum.sample(random: save_random.dup) }
          expect(other_results).to be_all(eq result)
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
            expect(other_results).not_to be_all(eq result)
          end
        end
      end

      context 'with rng' do
        specify do
          save_random = random.dup
          result = enum.sample(1, random: random)
          expect(result).to be_an(Integer)
          other_results = Array.new(100) { enum.sample(1, random: save_random.dup) }
          expect(other_results).to be_all(eq result)
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
            other_results = Array.new(100) { enum.sample(n) }
            expect(other_results).not_to be_all(eq result)
          end
        end

        context 'with rng' do
          subject(:result) { enum.sample(n, random: random) }

          specify do
            save_random = random.dup
            result = enum.sample(n, random: random)
            expect(result).to be_an(Array)
            expect(result.length).to eq(n)
            other_results = Array.new(100) { enum.sample(n, random: random) }
            expect(other_results).to be_all(eq result)
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
