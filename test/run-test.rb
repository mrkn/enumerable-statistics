$VERBOSE = true

base_dir = File.expand_path("../..", __FILE__)
lib_dir = File.join(base_dir, "lib")
test_dir = File.join(base_dir, "test")

$LOAD_PATH.unshift(lib_dir)

require "test/unit"
require "enumerable/statistics"

exit Test::Unit::AutoRunner.run(true, test_dir)
