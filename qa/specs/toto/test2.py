#!/usr/bin/env python
import sys
import json
import socket

def main():
    (_, host, ports, context) = sys.argv
    print("CONTEXT: {}".format(json.dumps(json.loads(context), indent=2)))
    print("HOST: {}".format(host))
    print("PORTS: {}".format(json.dumps(json.loads(ports), indent=2)))
    ports = json.loads(ports)
    context = json.loads(context)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(ports['8080/tcp'])))
    s.sendall('SAlut les connards'.encode('utf-8'))
    s.close()

if __name__ == "__main__":
    main()
