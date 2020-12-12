require "bundler/gem_tasks"
require "rake/extensiontask"
require "rspec/core/rake_task"

task :default => :spec

Rake::ExtensionTask.new('enumerable/statistics/extension')

directory 'lib/enumerable/statistics'

RSpec::Core::RakeTask.new(:spec)

task :spec => :compile

task :bench do
  puts "# sum\n"
  system('benchmark-driver bench/sum.yml')

  puts "# mean\n"
  system('benchmark-driver bench/mean.yml')

  puts "# variance\n"
  system('benchmark-driver bench/variance.yml')
end
