class ArrayTest < Test::Unit::TestCase
  sub_test_case("find_max and argmax") do
    data(:case, [
      { array: [3, 6, 2, 4, 9, 1, 2, 9], result: [9, 4] },
      { array: [3, 6, 2, 4, 3, 1, 2, 8], result: [8, 7] },
      { array: [7, 6, 2, 4, 3, 1, 2, 6], result: [7, 0] }
    ])
    def test_find_max(data)
      array, result = data[:case].values_at(:array, :result)
      assert_equal(result, array.find_max)
    end

    def test_argmax(data)
      array, result = data[:case].values_at(:array, :result)
      assert_equal(result[1], array.argmax)
    end
  end

  sub_test_case("find_min and argmin") do
    data(:case, [
      { array: [3, 6, 1, 4, 9, 1, 2, 9], result: [1, 2] },
      { array: [3, 6, 3, 4, 4, 8, 3, 2], result: [2, 7] },
      { array: [3, 6, 5, 4, 3, 6, 8, 6], result: [3, 0] }
    ])
    def test_find_min(data)
      array, result = data[:case].values_at(:array, :result)
      assert_equal(result, array.find_min)
    end

    def test_argmin(data)
      array, result = data[:case].values_at(:array, :result)
      assert_equal(result[1], array.argmin)
    end
  end
end
