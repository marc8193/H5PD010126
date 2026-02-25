#!/bin/env python3
#
# Use below command to check if certificate works.
# curl --cacert cert.pem -X GET https://localhost:1255

import http.server
import ssl

PORT = 1255

Handler = http.server.SimpleHTTPRequestHandler

# Set up the server
server_address = ('localhost', PORT)
httpd = http.server.HTTPServer(server_address, Handler)

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain('cert.pem', 'key.pem')
httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

print(f"HTTPS server running on https://localhost:{PORT}")
httpd.serve_forever()
