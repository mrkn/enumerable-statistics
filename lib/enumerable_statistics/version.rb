module EnumerableStatistics
  VERSION = '1.1.0-dev'

  module Version
    numbers, TAG = VERSION.split('-', 2)
    MAJOR, MINOR, MICRO = numbers.split('.', 3).map(&:to_i)
    STRING = VERSION
  end
end
