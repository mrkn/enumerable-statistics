require 'spec_helper'
require 'enumerable/statistics'

RSpec.describe Array do
  describe '#value_counts' do
    let(:receiver) do
      'bggbafgeeebgbbaccdgdgdbbgbgffbaffggcegbf'.chars.tap do |ary|
        ary[5, 0] = nil
        ary[15, 0] = nil
        ary[20, 0] = nil
      end
    end

    # ["b", "g", "a", "f", "e", "c", "d"]
    # {"b"=>10, "g"=>11, "a"=>3, "f"=>6, "e"=>4, "c"=>3, "d"=>3}
    matrix = [
      { normalize: false, sort: true,  ascending: false, dropna: true,
        result: {"g"=>11, "b"=>20, "f"=>6, "e"=>4, "a"=>3, "c"=>3, "d"=>3} },
      { normalize: false, sort: true,  ascending: false, dropna: false,
        result: {"g"=>11, "b"=>20, "f"=>6, "e"=>4, "a"=>3, "c"=>3, "d"=>3, nil=>3} },

      { normalize: false, sort: true,  ascending: true,  dropna: true,
        result: {"d"=>3, "c"=>3, "a"=>3, "e"=>4, "f"=>6, "b"=>10, "g"=>11} },
      { normalize: false, sort: true,  ascending: true,  dropna: false,
        result: {nil=>3, "d"=>3, "c"=>3, "a"=>3, "e"=>4, "f"=>6, "b"=>10, "g"=>11} },

      { normalize: false, sort: false, ascending: false, dropna: true,
        result: {"g"=>11, "e"=>4, "f"=>6, "b"=>10, "d"=>3, "c"=>3, "a"=>3} },
      { normalize: false, sort: false, ascending: false, dropna: false,
        result: {nil=>3, "g"=>11, "e"=>4, "f"=>6, "b"=>10, "d"=>3, "c"=>3, "a"=>3} },

      { normalize: false, sort: false, ascending: true,  dropna: true,
        result: {"g"=>11, "e"=>4, "f"=>6, "b"=>10, "d"=>3, "c"=>3, "a"=>3} },
      { normalize: false, sort: false, ascending: true,  dropna: false,
        result: {nil=>3, "g"=>11, "e"=>4, "f"=>6, "b"=>10, "d"=>3, "c"=>3, "a"=>3} },

      { normalize: true,  sort: true,  ascending: false, dropna: true,
        result: {"g"=>0.275, "b"=>0.250, "f"=>0.150, "e"=>0.100, "a"=>0.075, "c"=>0.075, "d"=>0.075} },
      { normalize: true,  sort: true,  ascending: false, dropna: false,
        result: {"g"=>0.255814, "b"=>0.232558, "f"=>0.139535, "e"=>0.093023, "a"=>0.069767, "c"=>0.069767, "d"=>0.069767, nil=>0.069767} },

      { normalize: true,  sort: true,  ascending: true,  dropna: true,
        result: {"d"=>0.075, "c"=>0.075, "a"=>0.075, "e"=>0.100, "f"=>0.150, "b"=>0.250, "g"=>0.275} },
      { normalize: true,  sort: true,  ascending: true,  dropna: false,
        result: {nil=>0.069767, "d"=>0.069767, "c"=>0.069767, "a"=>0.069767, "e"=>0.093023, "f"=>0.139535, "b"=>0.232558, "g"=>0.255814} },

      { normalize: true,  sort: false, ascending: false, dropna: true,
        result: {"g"=>0.275, "e"=>0.100, "f"=>0.150, "b"=>0.250, "d"=>0.075, "c"=>0.075, "a"=>0.075} },
      { normalize: true,  sort: false, ascending: false, dropna: false,
        result: {nil=>0.069767, "g"=>0.255814, "e"=>0.093023, "f"=>0.139535, "b"=>0.232558, "d"=>0.069767, "c"=>0.069767, "a"=>0.069767} },

      { normalize: true,  sort: false, ascending: true,  dropna: true,
        result: {"g"=>0.275, "e"=>0.100, "f"=>0.150, "b"=>0.250, "d"=>0.075, "c"=>0.075, "a"=>0.075} },
      { normalize: true,  sort: false, ascending: true,  dropna: false,
        result: {nil=>0.069767, "g"=>0.255814, "e"=>0.093023, "f"=>0.139535, "b"=>0.232558, "d"=>0.069767, "c"=>0.069767, "a"=>0.069767} },
    ]

    matrix.each do |params|
      param_values = params.values_at(:normalize, :sort, :ascending, :dropna)
      context "with normalize: %s, sort: %s, ascending: %s, dropna: %s" % param_values do
        specify do
          params = params.dup
          expected_result = params.delete(:result)
          expect(receiver.value_counts(**params)).to eq(expected_result)
        end
      end
    end
  end
end

RSpec.describe Hash do
  describe '#value_counts' do
    pending 'TODO'
  end
end

RSpec.describe Enumerable do
  describe '#value_counts' do
    let(:receiver) do
      'bggbafgeeebgbbaccdgdgdbbgbgffbaffggcegbf'.each_char
    end

    pending 'TODO'
  end
end
