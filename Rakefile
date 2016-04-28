require "bundler/gem_tasks"
require "rake/extensiontask"
require "rspec/core/rake_task"

task :default => :spec

Rake::ExtensionTask.new('enumerable/statistics/extension')

RSpec::Core::RakeTask.new(:spec)
