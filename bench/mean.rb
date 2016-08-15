require 'enumerable/statistics'
require 'benchmark/ips'

Benchmark.ips do |x|
  x.config(times: 5, warmup: 2)

  n = 1_000
  ary = Array.new(n) { rand }

  x.report('inject') do
    mean = ary.inject(:+) / n.to_f
  end

  x.report('while') do
    i = 0
    mean = 0
    while i < n
      mean += ary[i]
      i += 1
    end
    mean /= n.to_f
  end

  x.report('mean') do
    mean = ary.mean
  end
end
