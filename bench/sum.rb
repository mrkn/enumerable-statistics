require 'enumerable/statistics'
require 'benchmark/ips'

Benchmark.ips do |x|
  x.config(times: 5, warmup: 2)

  n = 1_000
  ary = Array.new(n) { rand }

  x.report('inject') do
    sum = ary.inject(:+)
  end

  x.report('while') do
    i = 0
    sum = 0
    while i < n
      sum += ary[i]
      i += 1
    end
  end

  x.report('sum') do
    sum = ary.sum
  end
end
