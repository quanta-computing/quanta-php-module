# Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS.
# It must be equal to the last entry ([x] = NULL) + 1

# put that in the optimal way
methods = {
  15 => 'Magento\Framework\App\Bootstrap::run',
  6 => 'Magento\Framework\App\Bootstrap::create',
  7 => 'Magento\Framework\App\Bootstrap::createApplication',
  5 => 'Magento\Framework\Interception\Config\Config::initialize',
  0 => 'Magento\Framework\App\Request\Http::getFrontName',
  16 => '',
  17 => '',
  13 => 'Magento\Framework\App\FrontController\Interceptor::dispatch',
  12 => 'Magento\Framework\App\Action\Action::dispatch',
  1 => 'Magento\Framework\View\Page\Builder::generateLayoutBlocks',
  14 => 'Magento\Framework\View\Result\Page\Interceptor::renderResult',
  18 => '',
  3 => 'Magento\Framework\App\Response\Http\Interceptor::sendResponse',
  19 => '',
  20 => '',
  4 => 'Magento\Framework\View\Layout::_renderBlock',
  9 => 'PDOStatement::execute',
  10 => 'Magento\Backend\Controller\Adminhtml\Cache\FlushAll::execute',
  8 => 'Magento\Framework\App\Cache\TypeList::cleanType',
  11 => 'Magento\Backend\Controller\Adminhtml\Cache\FlushSystem::execute',
  21 => '',
  22 => ''
}

def compute_functions methods
  methods.each_with_index.reduce Hash.new do |functions, ((order, method), index)|
    next functions if method.empty?
    klass, function_name = method.split '::'
    (functions[function_name] ||= []).push class_name: klass, index: index, order: order
    functions[function_name].sort_by { |function| function[:order] }
    functions
  end.sort_by do |function_name, functions|
    functions.min { |function| function[:order] }[:order]
  end
end

def insert_into_tree node, string, value
  i = 0
  n = string.length
  while i < n && node[:children][string[i]]
    node = node[:children][string[i]]
    i += 1
  end
  while i < n
    node = node[:children][string[i]] = {children: {}, value: nil}
    i += 1
  end
  node[:children]['\\0'] = {children: {}, value: value}
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

def compile_node node, depth = 0
  if node[:children].empty?
    # print depth, "++hp_globals.internal_match_counters.function;"
    print depth, "const char *class_name = hp_get_class_name(data TSRMLS_CC);";
    print depth, "if (!class_name) return -1;"
    node[:value].each do |value|
      print depth, "if (!strcmp(class_name, \"#{value[:class_name].gsub '\\', '\\\\\\\\'}\")) return #{value[:index]};"
    end
    # print depth, "++hp_globals.internal_match_counters.class_unmatched;"
  end
  node[:children].each do |letter, child|
    print depth, "if (function_name[#{depth}] == '#{letter}') {"
    compile_node child, depth + 1
    print depth, '}'
  end
  print depth, "return -1;"
end

def compile_tree tree
  puts "int hp_match_monitored_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {"
  # puts " ++hp_globals.internal_match_counters.total;"
  compile_node tree
  puts "}"
end

def compile_fill methods
  puts "#include \"quanta_mon.h\""
  puts
  puts "void hp_fill_monitored_functions(char **function_names) {"
  puts " if (function_names[0] != NULL) return;"
  methods.each_with_index do |(__, method), index|
    puts " function_names[#{index}] = \"#{method.gsub '\\', '\\\\\\\\'}\";";
  end
  puts " function_names[#{methods.length}] = NULL;"
  puts "}"
  puts
end

compile_fill methods
compile_tree create_tree compute_functions methods
