module Enumerable
  def value_counts(normalize: false, sort: true, ascending: false, dropna: true)
    length = 0
    na_count = 0
    result = self.inject(Hash.new(0)) do |acc, val|
      length += 1
      if val.nil? || (val.is_a?(Float) && val.nan?)
        na_count += 1
      else
        acc[val] += 1
      end
      acc
    end
    if normalize
      n = length.to_f
      n -= na_count if dropna
      result.transform_values! {|v| v / n }
      na_count /= n unless dropna
    end
    if sort
      s = ascending ? 1 : -1
      result = result.sort_by {|k, v| s * v }
      unless dropna
        if ascending
          result.unshift [nil, na_count]
        else
          result.push [nil, na_count]
        end
      end
    else
      result = result.to_a
      result.unshift [nil, na_count] unless dropna
    end
    result.to_h
  end
end
