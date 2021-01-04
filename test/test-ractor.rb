class RactorTest < Test::Unit::TestCase
  def setup
    omit("require Ractor") unless defined? Ractor
  end

  test("Array#mean") do
    r = Ractor.new do
      [1, 2, 3, 4].mean
    end
    assert_equal(2.5, r.take)
  end
end
