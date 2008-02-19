from floater_client import *

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urllib

st = State()
st.clientname = "tableserver"

class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        start = len('/postit.yaws?flproxyB=')
        data = urllib.unquote(self.path[start:])
        print 'r',[data]
        messages = table_handle(st,data)
        send_messages(self.wfile,messages)   
        #self.wfile.write("nothing\r\n\r\n")

def get_current_seated_msg(state):
   msg = []
   for seat in xrange(4):
      name = state.table_seated[seat]
      if name == '': continue
      m = state.encode_message('seated',[name,name_dict[seat],'ip','port'])
      msg.append(m)
   return msg   

   
def table_handle(state,data):
   rmsg = []
   for line in data.split('\r\n')[:-1]:
      message = decode_message(line)
      mname,mfrom,mid = message[:3]
      args = message[3:]
      if mname == 'S':
         username,seat,ip,port = args
         if state.deal is None:
             state.hand_id += 1
             state.deal = state.rubber.next_deal()            
         #if state.client_seated(IDX[seat[0]], username):
         #rmsg += get_current_seated_msg(state)
         # assume client always north
         rmsg += [state.send_new_hand()[0]]
      elif mname == 'a':
         print args
         state.bid_status = BidStatus(args[1])
         #TODO check if it is the right bidder
         state.deal.bid(f2o_bid(state.bid_status[-1]))
         rmsg.append(line)
         print [state.bid_status.data]
         if state.bid_status.data == ' p'*4:
            print 'all pass'
            state.deal = None
            state.bid_status = BidStatus('')
      elif mname == 'p':
         state.play_status = convert_str2play(args[1])
         rmsg.append(line)
         if len(state.play_status) == 1:
            rmsg += state.send_new_hand()
         if len(state.play_status) == 52:
            state.deal = None
            state.play_status = []
            state.bid_status = BidStatus('')
      return rmsg

def send_messages(conn,mm):
   if mm is None: return
   print mm
   for m in mm:
      if m is None: continue
      if len(m) == 2: # single client message
         print 'p2p',m[0],m[1]
         conn.write(m[1]+'\r\n')
      else:
         print 's',m
         conn.write(m+'\r\n')

TODO = """  work on a server that take care of table manager and 3 player
 a client can use request/response mode to play bridge, instead of polling data.
 use cases, request/response from client/server
 1. seated at North/hand id, client cards, hand id
 2. hand id, bid/hand id bid history, ...
 3. hand id, pass/hand id bid history, leading play, dummy (assume client is not leader)
 4. hand id, play/hand id, play history
 5. hand id, play/hand id, play history, new hand, score ...
"""
if __name__ == "__main__":
   
   import socket, select
   try:
        server = HTTPServer(('', 10101), MyHandler)
        print 'started httpserver...'
        server.serve_forever()
   except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()


