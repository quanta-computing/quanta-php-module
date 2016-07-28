require 'yaml'

def insert_into_tree node, string, value
  i = 0
  n = string.length
  while i < n && node[:children][string[i]]
    node = node[:children][string[i]]
    i += 1
  end
  while i < n
    node = node[:children][string[i]] = {children: {}, value: value}
    i += 1
  end
  node[:children]['\\0'] = {children: {}, value: value}
end

def compute_functions methods
  methods.each_with_index.reduce Hash.new do |functions, ((_, function), index)|
    klass, function_name = function['name'].split '::'
    (functions[function_name] ||= []).push class_name: klass,
      index: index,
      order: function['tree_order'].to_i
    functions[function_name].sort_by! { |tree_function| tree_function[:order] }
    functions
  end.sort_by do |function_name, functions|
    functions.min { |function| function[:order] }[:order]
  end
end

def create_tree functions
  functions.reduce({children: {}, value: nil}) do |tree, (function, klasses)|
    insert_into_tree tree, function, klasses
    tree
  end
end

def indent length
  ' ' * (length + 1)
end

def print depth, text
  puts "#{indent depth}#{text}"
end

def compile_node app_name, node, depth = 0
  if node[:children].empty?
    print depth, "++hp_globals.internal_match_counters.function;"
    print depth, "const char *class_name = hp_get_class_name(data TSRMLS_CC);";
    print depth, "if (!class_name) return NULL;"
    node[:value].each do |value|
      print depth, "if (!strcmp(class_name, \"#{value[:class_name].gsub '\\', '\\\\\\\\'}\"))"
      print depth + 1, "return &#{app_name}_profiled_functions[#{value[:index]}];"
    end
    print depth, "++hp_globals.internal_match_counters.class_unmatched;"
  end
  node[:children].each do |letter, child|
    print depth, "if (function_name[#{depth}] == '#{letter}') {"
    compile_node app_name, child, depth + 1
    print depth, '}'
  end
  print depth, "return NULL;"
end

def compile_tree app_name, tree
  puts "profiled_function_t *#{app_name}_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {"
  puts " ++hp_globals.internal_match_counters.total;"
  compile_node app_name, tree
  puts "}"
end

def compile_profiled_functions name, app
  puts "static profiled_function_t #{name}_profiled_functions[] = {"
  app['functions'].each_with_index do |(_, function), index|
    begin_cb = function['begin_callback'] || 'NULL'
    end_cb = function['end_callback'] || 'NULL'
    ignore_in_stack = function['ignore_in_stack'] ? 1 : 0
    min_level = 0 #TODO!
    puts "{\"#{function['name'].gsub '\\', '\\\\\\\\'}\", #{index}, {#{
      ignore_in_stack}, #{min_level}}, #{
      begin_cb}, #{end_cb}, {0, 0, 0, 0}, {0, 0, 0, 0}},"
  end
  puts "{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}}"
  puts "};"
  puts
end

def function_index function_name, app
  app['functions'].find_index do |name, _|
    function_name == name
  end
end

def compile_profiler_timers name, app
  puts "static const profiler_timer_t #{name}_profiler_timers[] = {"
  app['timers'].each do |timer_name, timer|
    start_idx = function_index timer['start']['function'], app
    end_idx = function_index timer['end']['function'], app
    puts "{"
    puts "  \"#{timer_name}\","
    puts "  {&#{name}_profiled_functions[#{start_idx}], PROF_#{timer['start']['timer'].upcase}},"
    puts "  {&#{name}_profiled_functions[#{end_idx}], PROF_#{timer['end']['timer'].upcase}}"
    puts "},"
  end
  puts "{NULL, {0, 0}, {0, 0}}"
  puts "};"
  puts
end

def compile_profiler_match_function name, app
  tree = create_tree compute_functions app['functions']
  compile_tree name, tree
  puts
end

def compile_profiled_application name, app
  first_idx = function_index app['first_function'], app
  last_idx = function_index app['last_function'], app
  puts "static profiled_application_t #{name}_profiled_application = {"
  puts "\"#{name}\","
  puts "#{name}_profiled_functions,"
  puts "#{app['functions'].length},"
  puts "#{name}_profiler_timers,"
  puts "#{app['timers'].length},"
  puts "&#{name}_profiled_functions[#{first_idx}],"
  puts "&#{name}_profiled_functions[#{last_idx}],"
  puts "NULL, NULL, NULL,"
  puts "#{app['create_context_callback'] || 'NULL'},"
  puts "#{app['cleanup_context_callback'] || 'NULL'},"
  puts "#{name}_match_function,"
  puts "#{app['send_metrics_callback'] || 'NULL'}"
  puts "};"
  puts
end

def header
  puts "#include \"quanta_mon.h\""
  puts
end

header()
apps = YAML.load_file(ARGV[0])['cms']
apps.each do |(name, app)|
  compile_profiled_functions name, app
  compile_profiler_timers name, app
  compile_profiler_match_function name, app
  compile_profiled_application name, app
end

#TMP until we detect application at runtime
puts "void register_application(void) {"
print 2, "hp_globals.profiled_application = &magento2_profiled_application;"
print 2, "init_profiled_application(&magento2_profiled_application);"
puts "}"
