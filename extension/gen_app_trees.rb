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
      order: function['tree_order'].to_i,
      app: function['app']
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
    min_level = function['min_profiling_level'] || 'QUANTA_MON_MODE_APP_PROFILING'
    puts "{\"#{function['name'].gsub '\\', '\\\\\\\\'}\", #{index}, {#{
      ignore_in_stack}, #{min_level}}, #{
      begin_cb}, #{end_cb}, {0, 0, 0, 0}},"
  end
  puts "{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}}"
  puts "};"
  puts
end

def function_index function_name, app
  app['functions'].find_index do |name, _|
    function_name == name
  end
end

def compile_profiler_timers_stop name, app, stop
  app['timers'].each do |timer_name, timer|
    puts "static const profiler_timer_function_t #{name}_profiler_timer_#{timer_name}_#{stop}[] = {"
    timer[stop] = timer[stop].is_a?(Hash) ? [timer[stop]] : timer[stop]
    timer[stop].each do |stop_timer|
      idx = function_index stop_timer['function'], app
      puts "  {&#{name}_profiled_functions[#{idx}], PROF_#{stop_timer['timer'].upcase}},"
    end
    puts "  {NULL, 0}"
    puts "};"
  end
end

def compile_profiler_timers name, app
  ['start', 'end'].each do |stop|
    compile_profiler_timers_stop name, app, stop
  end
  puts "static const profiler_timer_t #{name}_profiler_timers[] = {"
  app['timers'].each do |timer_name, timer|
    puts "{"
    puts "  \"#{timer_name}\","
    puts "  #{name}_profiler_timer_#{timer_name}_start, #{timer['start'].length},"
    puts "  #{name}_profiler_timer_#{timer_name}_end, #{timer['end'].length},"
    puts "  {#{timer['ignore_sql'] ? 1 : 0}}"
    puts "},"
  end
  puts "{NULL, NULL, 0, NULL, 0, {0}}"
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
  puts "{NULL, NULL},"
  puts "NULL,"
  puts "#{app['create_context_callback'] || 'NULL'},"
  puts "#{app['cleanup_context_callback'] || 'NULL'},"
  puts "#{name}_match_function,"
  puts "#{app['send_metrics_callback'] || 'NULL'}"
  puts "};"
  puts
end

def compile_app_first_node node, depth = 0
  if node[:children].empty?
    print depth, "++hp_globals.internal_match_counters.function;"
    print depth, "const char *class_name = hp_get_class_name(data TSRMLS_CC);";
    print depth, "if (!class_name) return NULL;"
    node[:value].each do |value|
      print depth, "if (!strcmp(class_name, \"#{value[:class_name].gsub '\\', '\\\\\\\\'}\"))"
      print depth + 1, "return &#{value[:app]}_profiled_application;"
    end
    print depth, "++hp_globals.internal_match_counters.class_unmatched;"
  end
  node[:children].each do |letter, child|
    print depth, "if (function_name[#{depth}] == '#{letter}') {"
    compile_app_first_node child, depth + 1
    print depth, '}'
  end
  print depth, "return NULL;"
end

def compile_app_first_tree tree
  puts "profiled_application_t *qm_match_first_app_function(const char* function_name,"
  puts "zend_execute_data* data TSRMLS_DC) {"
  puts " ++hp_globals.internal_match_counters.total;"
  compile_app_first_node tree
  puts "}"
end

def compile_profiler_match_app_first_function apps
  functions = apps.reduce Hash.new do |_functions, (name, app)|
    _functions.merge(app['first_function'] => {
      'name' => app['functions'][app['first_function']]['name'],
      'tree_order' => 0,
      'app' => name})
  end
  tree = create_tree compute_functions functions
  compile_app_first_tree tree
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
compile_profiler_match_app_first_function apps
