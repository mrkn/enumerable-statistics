require 'enumerable/statistics'
require 'benchmark/ips'

Benchmark.ips do |x|
  x.config(times: 5, warmup: 2)

  n = 1_000
  ary = Array.new(n) { rand }
  mean = ary.inject(:+) / n.to_f

  x.report('inject') do
    var = ary.inject(0.0) { |sum, x|
      sum += (x - mean) ** 2
    } / (n - 1).to_f
  end

  x.report('while') do
    i = 0
    var = 0
    while i < n
      var += (ary[i] - mean) ** 2
      i += 1
    end
    mean /= (n - 1).to_f
  end

  x.report('variance') do
    var = ary.variance
  end
end
