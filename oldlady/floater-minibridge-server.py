#!/usr/bin/python

from floater_client import *
from sAi import *
from sbridge import *

MANAGERNAME = "tableserver"
MSG_SEPERATOR = '<f/>'

def nothingtodo(s):
    pass
import sAi
sAi.debug = nothingtodo

def nextStep(action, state, comps):
    rmsg = []
    if action == 'confirm_deal':
        # make usre NORTH always start the bid, so hand_id % 4 == 0
        state.hand_id += 4
        state.bid_status = BidStatus('')
        state.play_status = []
        state.deal = state.rubber.next_deal()
        for ai in comps: ai.new_deal(state.deal)

        rmsg.append(state.send_new_hand()[NORTH])
        # make up bids
        bidstatus = ['3n',' p',' p',' p']
        for fbid in bidstatus:
            bid = f2o_bid(fbid)
            for ai in comps: ai.bid_made(bid)
            state.deal.bid(bid)
        rmsg.append(state.encode_message('auction_status',[str(state.hand_id),''.join(bidstatus)]))
        state.bid_status = BidStatus(''.join(bidstatus))
        # opening player is at east
        card = comps[state.deal.player].play_self()
        a = [o2f_card(card)]
        rmsg.append(state.encode_message('play',[str(state.hand_id), convert_play2str(a)]))
        #rmsg.append(state.send_new_hand()[SOUTH])      
        
    return rmsg

def do_GET(msg):    
    # working as a live table, don't use st.deal as it is used to save original cards
    import pickle
    #data = urllib.unquote(self.path[start:])
    # read state from persistent storage
    st = None
    ais = None
    try:
        #TODO from history conversation recontruct state
        st, ais= pickle.load(open('floater-minibridge-state.pkl','rb'))
    except:
        st = State()
        st.hand_id = 1
        st.clientname = MANAGERNAME
        ais = [ComputerPlayer(seat) for seat in PLAYERS]
    data = msg
    #print 'r',[data]
    rsp = []
    while data != MSG_SEPERATOR:
        messages = table_handle(st,ais,data)
        selfdata = []
        #print '--messages',messages,"<br/>",MSG_SEPERATOR
        if messages is None:
            #print 'message should not be none'
            break; # why??? fixme
        for m in messages:
            if m is None: continue
            if len(m) == 2: # single client message
                #print 'p2p',m[0],m[1]
                rsp.append(m[1])
                selfdata.append(m[1])
            else:
                #print 's>',m
                rsp.append(m)
                selfdata.append(m)
        data = MSG_SEPERATOR.join(selfdata)+MSG_SEPERATOR
    #write state to storage
    pickle.dump((st,ais),open('floater-minibridge-state.pkl','wb'))
    return MSG_SEPERATOR+MSG_SEPERATOR.join(rsp)

def table_handle(state,ais,data):
   rmsg = []
   for line in data.split(MSG_SEPERATOR)[:-1]:
      message = decode_message(line)
      mname,mfrom,mid = message[:3]
      args = message[3:]
      if mname == 'S':
         username,seat,ip,port = args
         if state.deal is None: return nextStep('confirm_deal', state, ais)
         # client always north, south is always dummy, no bidding
         rmsg.append( state.send_new_hand()[NORTH])
      elif mname == 'p':
          tbdeal = ais[NORTH].deal
          dummy = tbdeal.dummy
          if tbdeal.trick is None: continue
          if mfrom != MANAGERNAME:
              # from user (has to be North or south), avoid user play anything 
              # when not its turn
              if dummy == SOUTH or dummy == NORTH:
                  # alow south and north
                  if tbdeal.player != NORTH and tbdeal.player != SOUTH: continue
              elif tbdeal.player != NORTH: continue
            
          print '--',dummy, 'is dummy, whose turn',tbdeal.player
          # todo, consider NORTH is dummy to exchange with SOUTH
          state.play_status = convert_str2play(args[1])

          if len(state.play_status) == 1:
              if dummy == NORTH: rmsg.append( state.send_new_hand()[SOUTH])
              else:rmsg.append( state.send_new_hand()[NORTH])
              for ai in ais:
                  ai.deal.hands[dummy] = state.deal.hands[dummy][:]
          elif len(state.play_status) == 52:
              return nextStep('confirm_deal', state, ais)

          card = f2o_card(state.play_status[-1])
          player = tbdeal.player
          for ai in ais: ai.deal.play_card(card)
          if tbdeal.trickCompleted():
              for ai in ais:
                  ai.trick_complete()                      
          print 'whose turn',tbdeal.player
          # allow user play south and north if he is the contrator
          if dummy == NORTH or dummy == SOUTH:
             if tbdeal.player == SOUTH or tbdeal.player==NORTH: continue
          elif tbdeal.player == NORTH: continue

          if tbdeal.player != dummy:
              card = ais[tbdeal.player].play_self ()
          else:
              card = ais[tbdeal.declarer].play_dummy()
          a = o2f_card(card)
          print 'playing',card
          state.play_status += [a]
          rmsg.append(state.encode_message('play',[str(state.hand_id), convert_play2str(state.play_status)]))
          
   return rmsg

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urllib

def httpResponse(msg):
    return "Content-type: text\n\n"+msg    

class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        start = len('/postit.yaws?flproxyB=')
        data = urllib.unquote(self.path[start:])
        #print 'r',[data]
        self.wfile.write(httpResponse(do_GET(data)))

        
TODO = """ a cgi server as mini bridge
 a client can use request/response mode to play bridge, instead of polling data.
 use cases, request/response from client/server
 1. client first connect, always seated at North
 2. generate a deal, adjust it (north has most points), decide a double blind result
    return hands for south and north, with the final artificial bid result
    east is always in the opening position
 3. 
 4. hand id, play/hand id, play history
 5. hand id, play/hand id, play history, new hand, score ...

"""

def mmm():
   import socket, select
   try:
        server = HTTPServer(('', 10101), MyHandler)
        print 'started httpserver...'
        server.serve_forever()
   except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

print "Content-type: text/plain\n"

import cgi
import cgitb; cgitb.enable()

form = cgi.FieldStorage()
msg = form.getvalue('flproxyB','')
print do_GET(msg)


