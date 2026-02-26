#!/bin/env python3

import http.server

ADDRESS = "0.0.0.0"
PORT = 1255

class HelloWorldHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(b'Hello World')

server_address = (ADDRESS, PORT)
httpd = http.server.HTTPServer(server_address, HelloWorldHandler)

print(f"HTTPS server running on http://{ADDRESS}:{PORT}")
httpd.serve_forever()
