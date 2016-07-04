# Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS.
# It must be equal to the last entry ([x] = NULL) + 1

methods = [
  'Magento\Framework\App\Bootstrap::run',
  'Magento\Framework\App\Bootstrap::create',
  'Magento\Framework\App\Bootstrap::createApplication',
  'Magento\Framework\Interception\Config\Config::initialize',
  'Magento\Framework\App\Request\Http::getFrontName',
  '',
  '',
  'Magento\Framework\App\FrontController\Interceptor::dispatch',
  'Magento\Framework\App\Action\Action::dispatch',
  'Magento\Framework\View\Page\Builder::generateLayoutBlocks',
  'Magento\Framework\View\Result\Page\Interceptor::renderResult',
  '',
  'Magento\Framework\App\Response\Http\Interceptor::sendResponse',
  '',
  '',
  'Magento\Framework\View\Layout::_renderBlock',
  'PDOStatement::execute',
  'Magento\Backend\Controller\Adminhtml\Cache\FlushAll::execute',
  'Magento\Framework\App\Cache\TypeList::cleanType',
  'Magento\Backend\Controller\Adminhtml\Cache\FlushSystem::execute',
  '',
  ''
]

def compute_functions methods
  methods.each_with_index.reduce Hash.new do |functions, (method, index)|
    next functions if method.empty?
    klass, function = method.split '::'
    (functions[function] ||= []).push class_name: klass, index: index
    functions
  end
end

def insert_into_tree node, string, value
  i = 0
  n = string.length
  while i < n
    if node[:children][string[i]]
      node = node[:children][string[i]]
      i += 1
    else
      break
    end
  end
  while i < n
    node = node[:children][string[i]] = {children: {}, value: value}
    i += 1
  end
end

def create_tree functions
  functions.reduce({children: {}, value: nil}) do |tree, (function, klasses)|
    insert_into_tree tree, function, klasses
    tree
  end
end

def indent length
  ' ' * length
end

def print depth, text
  puts "#{indent depth}#{text}"
end

def compile_node node, depth = 0
  if node[:children].empty?
    print depth, "const char *class_name = hp_get_class_name(data TSRMLS_CC);";
    print depth, "if (!class_name) return -1;"
    node[:value].each do |value|
      print depth, "if (!strcmp(class_name, \"#{value[:class_name].gsub '\\', '\\\\\\\\'}\")) return #{value[:index]};"
    end
    print depth, "return -1;"
  end
  node[:children].each do |letter, child|
    print depth, "if (function_name[#{depth}] == '#{letter}') {"
    compile_node child, depth + 1
    print depth, '}'
  end
end

def compile_tree tree
  puts "int hp_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {"
  compile_node tree, 1
  puts " return -1;"
  puts "}"
end

def compile_fill methods
  puts "#include \"quanta_mon.h\""
  puts
  puts "void hp_fill_monitored_functions(char **function_names) {"
  puts " if (function_names[0] != NULL) return;"
  methods.each_with_index do |method, index|
    puts " function_names[#{index}] = \"#{method}\"";
  end
  puts " function_names[#{methods.length}] = NULL;"
  puts "}"
  puts
end

compile_fill methods
compile_tree create_tree compute_functions methods
