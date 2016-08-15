require "bundler/gem_tasks"
require "rake/extensiontask"
require "rspec/core/rake_task"

task :default => :spec

Rake::ExtensionTask.new('enumerable/statistics/extension')

RSpec::Core::RakeTask.new(:spec)

task :bench do
  puts "# sum\n"
  system('ruby bench/sum.rb')

  puts "# mean\n"
  system('ruby bench/mean.rb')

  puts "# variance\n"
  system('ruby bench/variance.rb')
end
