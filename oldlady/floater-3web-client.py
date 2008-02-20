from floater_client import *
import random

website = '127.0.0.1:10101'
url = 'http://'+website+'/postit.yaws?flproxyB='

if __name__ == "__main__":

   if len(sys.argv) > 1: website = sys.argv[1]
   import urllib,time

   st = State()
   st.clientname = 'mfrom'
   st.table_seated[0] = st.clientname
   st.hand_id = 9
   message = st.encode_message('request_seat',[0])
   while True:      
      print 's>',message
      #if message is None:
      #   message = st.encode_message('request_seat',[0])
      try:
         data = urllib.urlopen(url+urllib.quote(message+'\r\n'),proxies={}).read()
      except IOError:
         # next turn could also be mine if I win in the last trick
         message = st.encode_message('request_seat',[0])
         continue
      # check message
      if data.find("Internal error") >= 0:
         print [data]
         time.sleep(1)
         continue
      #data = data[:-4]
      if data != 'nothing': print 'r',[data]

      handleData(st,data)

      message = handle_auction(st)

      time.sleep(random.random())
