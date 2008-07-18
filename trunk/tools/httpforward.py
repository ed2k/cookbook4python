from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urllib

    
class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        print self.path
	url = 'http://142.133.118.126:10101'+self.path
	data = urllib.urlopen(url,proxies={}).read()
        self.wfile.write(data)


if __name__ == "__main__":
   
   import socket, select
   try:
        server = HTTPServer(('', 10101), MyHandler)
        print 'started httpserver...'
        server.serve_forever()
   except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

