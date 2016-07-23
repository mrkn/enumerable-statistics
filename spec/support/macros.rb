module Enumerable
  module Statistics
    module RSpecMacros
      def self.included(mod)
        mod.module_eval do
          extend ExampleGroupMethods
        end
      end

      module ExampleGroupMethods
        def with_array(given_ary, &example_group_block)
          describe "for #{given_ary.inspect}" do
            let(:ary) { given_ary }
            module_eval(&example_group_block)
          end
        end

        def with_enum(given_enum, description=given_enum.inspect, &example_group_block)
          if given_enum.is_a? Array
            given_enum = given_enum.each
            description += '.each'
          end

          describe "for #{description}" do
            let(:enum) { given_enum }
            let(:init) { 0 }
            let(:block) { nil }
            subject(:sum) { enum.sum(init, &block) }

            module_eval(&example_group_block)
          end
        end

        def with_init(init_value, &example_group_block)
          context "with init=#{init_value.inspect}" do
            let(:init) { init_value }

            module_eval(&example_group_block)
          end
        end

        def with_conversion(conversion_block, description, &example_group_block)
          context "with conversion `#{description}`" do
            let(:block) { conversion_block }

            module_eval(&example_group_block)
          end
        end

        def it_equals_with_type(x, type)
          it { is_expected.to be_an(type) }
          it { is_expected.to eq(x) }
        end

        def it_is_int_equal(n)
          it_equals_with_type(n, Integer)
        end

        def it_is_rational_equal(n)
          it_equals_with_type(n, Rational)
        end

        def it_is_float_equal(n)
          it_equals_with_type(n, Float)
        end

        def it_is_float_nan
          it { is_expected.to be_an(Float) }
          it { is_expected.to be_nan }
        end

        def it_is_complex_equal(n)
          it_equals_with_type(n, Complex)
        end
      end
    end
  end
end

RSpec.configure do |c|
  c.include Enumerable::Statistics::RSpecMacros
end
