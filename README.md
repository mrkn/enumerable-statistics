# Enumerable::Statistics

Welcome to your new gem! In this directory, you'll find the files you need to be able to package up your Ruby library into a gem. Put your Ruby code in the file `lib/enumerable/statistics`. To experiment with that code, run `bin/console` for an interactive prompt.

TODO: Delete this and the text above, and describe your gem

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'enumerable-statistics'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install enumerable-statistics

## Usage

TODO: Write usage instructions here

## Performance

```
$ bundle exec rake bench
# sum
Warming up --------------------------------------
              inject     1.545k i/100ms
               while     2.342k i/100ms
                 sum    11.009k i/100ms
Calculating -------------------------------------
              inject     15.016k (± 9.6%) i/s -     75.705k in   5.098723s
               while     22.238k (±16.2%) i/s -    107.732k in   5.068156s
                 sum    112.992k (± 6.9%) i/s -    572.468k in   5.091868s
# mean
Warming up --------------------------------------
              inject     1.578k i/100ms
               while     2.057k i/100ms
                mean     9.855k i/100ms
Calculating -------------------------------------
              inject     15.347k (± 8.6%) i/s -     77.322k in   5.076009s
               while     21.669k (±14.5%) i/s -    106.964k in   5.074312s
                mean    108.861k (± 8.9%) i/s -    542.025k in   5.021786s
# variance
Warming up --------------------------------------
              inject   586.000  i/100ms
               while   826.000  i/100ms
            variance     8.475k i/100ms
Calculating -------------------------------------
              inject      6.187k (± 6.7%) i/s -     31.058k in   5.043418s
               while      8.597k (± 7.4%) i/s -     42.952k in   5.024587s
            variance     84.702k (± 8.5%) i/s -    423.750k in   5.039936s
```

![](./images/benchmark.png)

## Development

After checking out the repo, run `bin/setup` to install dependencies. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/[USERNAME]/enumerable-statistics.

