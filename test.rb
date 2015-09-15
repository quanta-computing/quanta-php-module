require 'base64'
require 'openssl'
require 'json'


def magento_decrypt data
  key = "-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDB/6ahcWVynlbXcHrn0hLfUe5iQB5tof/gsWDUBTpc6I9bQv6e
Ctubn5G54C6HYg/MCqc4RQOOgzPTfzbrRVRJc6SvQvU6gajlXxBxqRymh3kxBSRU
kn5cjY9L8CZ/y7WEHbYmD3Ti7ApsExA91224vxxCdSQy1c2aVPYcecCy0wIDAQAB
AoGBAJO7TjY0908weus20bGFjS+Y4dLaBxHvwr5l9Vmf3ZDw3HJf8yuBEzlQa5XD
vUsFhwgXuL2l+I5nJWlSwAyr0AGFM+ce4OwD4jtSUiT5cWCsxixJJAypAc5lT4DL
Ql4qn4tfIiV5UtMRjphBu95xQPj0Sp6VX7oZC2Gd1izHeDABAkEA/P37c4Hxcvzp
2EMVu9QcgHJ2vy/HLSZp5HLfP8Xf7yEHlWlFWbkOzHaCIlVOXuLwLPYOY/BWR9ae
F88QjoXIAQJBAMROHRHU3XyYuvVM5LKtMg9nUlt/ZQauPCai9N02w+WO5oFY3MXr
xw0jiWvVW1bf9Awq9YhyisfUJF8MsrUs2tMCQDBD6C/uL3knhRlsGv5dhh5/N+Zb
oB0eAf/CZpMNFh7R51d64O8wHgGl7kfMjIoPs5BwVOh+FofYCwsxxs/Z+AECQFUl
+NACQCxKZ1X9i3nBTGSIEZ4kjSKvsuGjb4kAt0o8iNl0PRcQpV+amYIPorX1IHui
qqwL2wzAhSJBdMbHdQcCQB6TLz0+INLn8PZnWPrWG9H7mI4LnIg/s2nF4g9QZIYZ
IYkEUrG9iyk/0+kzLYcx49hcJEPfL8OnFH0FZNpG23k=
-----END RSA PRIVATE KEY-----"

  enc_password, enc_iv, enc_data = data.split('#').map { |v| Base64.decode64(v) }
  priv_key = OpenSSL::PKey::RSA.new key
  password = priv_key.private_decrypt enc_password
  iv = priv_key.private_decrypt enc_iv
  decrypter = OpenSSL::Cipher::AES.new(128, :CBC)
  decrypter.decrypt
  decrypter.key = password
  decrypter.iv = iv
  decrypter.update(enc_data) + decrypter.final
end

def request
  waiting_time = eval `curl 'http://localhost:3000/' -so /tmp/test.html -H 'X-QM3K-Profiler: true' -w '%{time_starttransfer} - %{time_connect}'`
  puts "WAITING TIME: #{(waiting_time * 1000).to_i}ms"
  puts ''
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
  JSON.parse magento_decrypt magento_json
end

def put_timers timers, values, which
  puts "### #{which} PROFILING TIMERS ###"
  values.reduce 0 do |i, value|
    puts "#{timers[i]} => #{value}"
    i + 1
  end
  puts ''
  puts "TOTAL: #{values.reduce 0, &:+}"
  puts ''
end

def put_magento_block_names blocks
  puts "### MAGENTO BLOCKS ###"
  puts(blocks.map do |name, data|
    name
  end.join(', '))
  puts ''
end

def put_php_block_names blocks
  puts "### PHP BLOCKS ###"
  puts(blocks.map do |block|
    block['name']
  end.join(', '))
  puts ''
end

def put_blocks php_blocks, magento_blocks
  php_blocks.each do |php_block|
    magento_block = magento_blocks[php_block['name']]
    puts "##{php_block['name']}"
    puts "CLASS PHP: [#{php_block['class']}] MAGENTO: [#{magento_block[1]}]"
    puts "TEMPLATE PHP: [#{php_block['template']}] MAGENTO: [#{magento_block[3]}]"
    puts "GENERATE PHP: [#{php_block['generate']}] MAGENTO: [#{magento_block[5]}]"
    puts ''
  end
end

request
php_json = get_php_json
magento_json = get_magento_json
timers = php_json['profiling'].keys
put_timers timers, php_json['profiling'].values, 'PHP'
put_timers timers, magento_json['timers'], 'MAGENTO'
put_php_block_names php_json['blocks']
put_magento_block_names magento_json['blocks']
put_blocks php_json['blocks'], magento_json['blocks']
