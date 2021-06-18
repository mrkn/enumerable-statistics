module EnumerableStatistics
  module ArrayExtension
    def find_max
      n = size
      return nil if n == 0

      imax, i = 0, 1
      while i < n
        imax = i if self[i] > self[imax]
        i += 1
      end
      [self[imax], imax]
    end

    def argmax
      find_max[1]
    end

    def find_min
      n = size
      return nil if n == 0

      imin, i = 0, 1
      while i < n
        imin = i if self[i] < self[imax]
        i += 1
      end
      [self[imin], imin]
    end

    def argmin
      find_min[1]
    end
  end

  Array.include ArrayExtension
end
