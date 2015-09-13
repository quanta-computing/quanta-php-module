require 'json'

def request
  waiting_time = eval `curl 'http://localhost:3000/' -so /tmp/test.html -H 'X-QM3K-Profiler: true' -w '%{time_starttransfer} - %{time_connect}'`
  puts "WAITING TIME: #{(waiting_time * 1000).to_i}ms"
end

def get_php_json
  JSON.parse File.read `(ls -1t /tmp/quanta-* | head -1)`[0..-2]
end

QUANTA_PROFILER_DIV_BEGIN = "<div id='--quanta--profiler' style='display:none'>"
QUANTA_PROFILER_DIV_END = "</div>"

def get_magento_json
  magento_json = File.read '/tmp/test.html'
  begin
    idx = magento_json.index QUANTA_PROFILER_DIV_BEGIN
    magento_json = magento_json[idx + QUANTA_PROFILER_DIV_BEGIN.length..-1]
    idx = magento_json.index QUANTA_PROFILER_DIV_END
    magento_json = magento_json[0...idx]
  rescue
    raise RuntimeError.new "Quanta Profiler data not found"
  end
  JSON.parse magento_json
end

def put_timers timers, values, which
  puts ''
  puts "### #{which} TIMERS ###"
  values.reduce 0 do |i, value|
    puts "#{timers[i]} => #{value}"
    i + 1
  end
  puts ''
  puts "TOTAL: #{values.reduce 0, &:+}"
  puts ''
end

request
php_json = get_php_json
magento_json = get_magento_json
timers = php_json['profiling']['timers'].keys
put_timers timers, php_json['profiling']['timers'].values, 'PHP'
put_timers timers, magento_json['timers'], 'MAGENTO'
