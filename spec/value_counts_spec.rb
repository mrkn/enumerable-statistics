require 'spec_helper'
require 'enumerable/statistics'

RSpec.shared_examples_for 'value_counts' do
  matrix = [
    { normalize: false, sort: true,  ascending: false, dropna: true,
      result: {"g"=>11, "b"=>10, "f"=>6, "e"=>4, "a"=>3, "c"=>3, "d"=>3} },
    { normalize: false, sort: true,  ascending: false, dropna: false,
      result: {"g"=>11, "b"=>10, "f"=>6, "e"=>4, "a"=>3, "c"=>3, "d"=>3, nil=>3} },

    { normalize: false, sort: true,  ascending: true,  dropna: true,
      result: {"a"=>3, "c"=>3, "d"=>3, "e"=>4, "f"=>6, "b"=>10, "g"=>11} },
    { normalize: false, sort: true,  ascending: true,  dropna: false,
      result: {nil=>3, "a"=>3, "c"=>3, "d"=>3, "e"=>4, "f"=>6, "b"=>10, "g"=>11} },

    { normalize: false, sort: false, ascending: false, dropna: true,
      result: {"b"=>10, "g"=>11, "a"=>3, "f"=>6, "e"=>4, "c"=>3, "d"=>3} },
    { normalize: false, sort: false, ascending: false, dropna: false,
      result: {nil=>3, "b"=>10, "g"=>11, "a"=>3, "f"=>6, "e"=>4, "c"=>3, "d"=>3} },

    { normalize: false, sort: false, ascending: true,  dropna: true,
      result: {"b"=>10, "g"=>11, "a"=>3, "f"=>6, "e"=>4, "c"=>3, "d"=>3} },
    { normalize: false, sort: false, ascending: true,  dropna: false,
      result: {nil=>3, "b"=>10, "g"=>11, "a"=>3, "f"=>6, "e"=>4, "c"=>3, "d"=>3} },

    { normalize: true,  sort: true,  ascending: false, dropna: true,
      result: {"g"=>0.275, "b"=>0.250, "f"=>0.150, "e"=>0.100, "a"=>0.075, "c"=>0.075, "d"=>0.075} },
    { normalize: true,  sort: true,  ascending: false, dropna: false,
      result: {"g"=>11/43.0, "b"=>10/43.0, "f"=>6/43.0, "e"=>4/43.0, "a"=>3/43.0, "c"=>3/43.0, "d"=>3/43.0, nil=>3/43.0} },

    { normalize: true,  sort: true,  ascending: true,  dropna: true,
      result: {"a"=>0.075, "c"=>0.075, "d"=>0.075, "e"=>0.100, "f"=>0.150, "b"=>0.250, "g"=>0.275} },
    { normalize: true,  sort: true,  ascending: true,  dropna: false,
      result: {nil=>3/43.0, "a"=>3/43.0, "c"=>3/43.0, "d"=>3/43.0, "e"=>4/43.0, "f"=>6/43.0, "b"=>10/43.0, "g"=>11/43.0} },

    { normalize: true,  sort: false, ascending: false, dropna: true,
      result: {"b"=>0.250, "g"=>0.275, "a"=>0.075, "f"=>0.150, "e"=>0.100, "c"=>0.075, "d"=>0.075} },
    { normalize: true,  sort: false, ascending: false, dropna: false,
      result: {nil=>3/43.0, "b"=>10/43.0, "g"=>11/43.0, "a"=>3/43.0, "f"=>6/43.0, "e"=>4/43.0, "c"=>3/43.0, "d"=>3/43.0} },

    { normalize: true,  sort: false, ascending: true,  dropna: true,
      result: {"b"=>0.250, "g"=>0.275, "a"=>0.075, "f"=>0.150, "e"=>0.100, "c"=>0.075, "d"=>0.075} },
    { normalize: true,  sort: false, ascending: true,  dropna: false,
      result: {nil=>3/43.0, "b"=>10/43.0, "g"=>11/43.0, "a"=>3/43.0, "f"=>6/43.0, "e"=>4/43.0, "c"=>3/43.0, "d"=>3/43.0} },
  ]

  matrix.each do |params|
    param_values = params.values_at(:normalize, :sort, :ascending, :dropna)
    context "with normalize: %s, sort: %s, ascending: %s, dropna: %s" % param_values do
      specify do
        params = params.dup
        expected_result = params.delete(:result)
        expect(receiver.value_counts(**params).to_a).to eq(expected_result.to_a)
      end
    end
  end
end

RSpec.describe Array do
  describe '#value_counts' do
    let(:receiver) do
      'bggbafgeeebgbbaccdgdgdbbgbgffbaffggcegbf'.chars.tap do |ary|
        ary[5, 0] = nil
        ary[15, 0] = nil
        ary[20, 0] = nil
      end
    end

    include_examples 'value_counts'
  end
end

RSpec.describe Hash do
  describe '#value_counts' do
    let(:array) do
      'bggbafgeeebgbbaccdgdgdbbgbgffbaffggcegbf'.chars.tap do |ary|
        ary[5, 0] = nil
        ary[15, 0] = nil
        ary[20, 0] = nil
      end
    end

    let(:receiver) do
      array.map.with_index {|x, i| [i, x] }.to_h
    end

    include_examples 'value_counts'
  end
end

RSpec.describe Enumerable do
  describe '#value_counts' do
    let(:array) do
      'bggbafgeeebgbbaccdgdgdbbgbgffbaffggcegbf'.chars.tap do |ary|
        ary[5, 0] = nil
        ary[15, 0] = nil
        ary[20, 0] = nil
      end
    end

    let(:receiver) do
      array.each
    end

    include_examples 'value_counts'
  end
end
