require "enumerable/statistics/extension"

module Enumerable
  module Statistics
    VERSION = Gem.loaded_specs['enumerable-statistics'].version.to_s
  end
end

require_relative 'statistics/array_ext'
require_relative 'statistics/enumerable_ext'
