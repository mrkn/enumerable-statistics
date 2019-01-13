require_relative 'array_ext/value_counts'

module Enumerable::Statistics::ArrayExtension
  ::Array.include ValueCounts
end
