module EnumerableStatistics
  class Histogram < Struct.new(:edges, :weights, :closed, :isdensity)
    alias edge edges
    alias density? isdensity
  end
end
