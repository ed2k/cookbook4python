#modified from Copyright Jon Berg , turtlemeat.com

import cgi
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
#import pri

upload_html = '''
<HTML><BODY>
<form method='POST' enctype='multipart/form-data' action='/'>
File to upload: <input type=file name=upfile>
<input type=submit value=Upload>
</form>
</BODY></HTML>
'''

class MyHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type',	'text/html')
        self.end_headers()
        self.wfile.write(upload_html)    
        return
                

    def do_POST(self):
        try:
            ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
            print ctype, pdict
            if ctype == 'multipart/form-data':
                query=cgi.parse_multipart(self.rfile, pdict)
            self.send_response(301)            
            self.end_headers()
	    #TODO, guess the file name
            upfilecontent = query.get('upfile')
            print "filecontent", len(upfilecontent[0])
            file('tmp','wb').write(upfilecontent[0])
            self.wfile.write("<HTML>"+str(len(upfilecontent[0]))+" Bytes uploaded</HTML>");
            
        except :
            print 'upload filed'
            self.wfile.write("upload filed");

def main():
    try:
        server = HTTPServer(('', 80), MyHandler)
        print 'started httpserver...'
        server.serve_forever()
    except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

if __name__ == '__main__':
    main()

