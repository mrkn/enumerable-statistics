require 'spec_helper'
require 'enumerable/statistics'

require_relative 'shared/value_counts'

RSpec.describe Array do
  describe '#value_counts' do
    let(:receiver) { values }

    include_examples 'value_counts'
  end
end

RSpec.describe Hash do
  describe '#value_counts' do
    let(:receiver) do
      values.map.with_index {|x, i| [i, x] }.to_h
    end

    include_examples 'value_counts'
  end
end

RSpec.describe Enumerable do
  describe '#value_counts' do
    let(:receiver) { values.each }

    include_examples 'value_counts'
  end
end
