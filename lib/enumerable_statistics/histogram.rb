module EnumerableStatistics
  class Histogram < Struct.new(:edge, :weights, :closed, :isdensity)
    alias density? isdensity
  end
end
