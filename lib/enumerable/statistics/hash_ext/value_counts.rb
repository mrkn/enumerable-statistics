class Hash
  def value_counts(*args, **kwargs)
    each_value.value_counts(*args, **kwargs)
  end
end
