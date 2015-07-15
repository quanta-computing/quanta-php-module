require 'json'

waiting_time = eval `curl 'http://localhost:3000/' -so /dev/null -w '%{time_starttransfer} - %{time_connect}'`
file = `(ls -1t /tmp/quanta-* | head -1)`

json = File.read file[0..-2]

puts "### JSON ###"
puts json
puts ''

json = JSON.parse json

sum = json['profiling']['timers'].reduce 0 do |sum, (name, value)|
  sum + value
end

puts "WAITING TIME: #{(waiting_time * 1000).to_i}ms"
puts "MAGENTO TIME: #{(sum * 1000).to_i}ms"
