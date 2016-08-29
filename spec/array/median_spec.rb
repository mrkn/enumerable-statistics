require 'spec_helper'
require 'enumerable/statistics'

RSpec.describe Array, '#median' do
  let(:ary) { [] }
  let(:init) { nil }
  let(:block) { nil }

  subject(:median) { ary.median(&block) }
end
