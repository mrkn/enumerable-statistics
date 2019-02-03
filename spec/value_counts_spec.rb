require 'spec_helper'
require 'enumerable/statistics'

require_relative 'shared/value_counts'
require_relative 'shared/value_counts_int_bins'

RSpec.describe Array do
  describe '#value_counts' do
    let(:receiver) { values }

    context 'without bins:' do
      include_examples 'value_counts'
    end

    context 'with bins:' do
      context 'an integer bins:' do
        include_examples 'value_counts with an integer bins:'
      end
    end
  end
end

RSpec.describe Hash do
  describe '#value_counts' do
    let(:receiver) do
      values.map.with_index {|x, i| [i, x] }.to_h
    end

    context 'without bins:' do
      include_examples 'value_counts'
    end

    context 'with bins:' do
      context 'an integer bins:' do
        include_examples 'value_counts with an integer bins:'
      end
    end
  end
end

RSpec.describe Enumerable do
  describe '#value_counts' do
    let(:receiver) { values.each }

    context 'without bins:' do
      include_examples 'value_counts'
    end

    context 'with bins:' do
      context 'when bins: is an integer' do
        include_examples 'value_counts with an integer bins:'
      end
    end
  end
end
