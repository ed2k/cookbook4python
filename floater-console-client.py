from floater_client import *

def rank(n): return (n%13) + 2
def suit(n): return n/13
def hcp(hand):
   h = 0
   for c in hand:
      if rank(c) > 10:
         h += (rank(c)-10)
   return h
def shape(hand): return 0

def print_suits(suits):
   ''' print a hand, organized as 4 suit'''
   for i in xrange(4):
      j = 3 - i
      print 'CDHS'[j]+':',
      for c in suits[j]:
         print '0123456789TJQKA'[c],
      print
def hand2suits(hand):
   suits = {0:[],1:[],2:[],3:[]} # somwhow list of list doesn't work
   for c in hand:
      idx = suit(c)
      suits[idx].append(rank(c))
   return suits
      
def evaluate_deal(ai):
   #hand, in suit order
   #print hcp, shape, estimate partner and opponents
   deal = ai.deal
   hand = o2f_hand(deal.hands[ai.seat])

   mysuits = hand2suits(hand)
   if deal.trick is None:
      print
      print_suits(mysuits)
      print 'HCP', hcp(hand)
      return
   dummyhand = o2f_hand(deal.hands[deal.dummy])
   dummysuits = hand2suits(dummyhand)
   print 'dummy'
   print_suits(dummysuits)
   print
   print_suits(mysuits)
   
class ConsoleState(State):
   def handle_auction(self):
      ''' try use oldlady engine '''
      dealer = f2o((self.hand_id-1) % 4)
      ai = sAi.ComputerPlayer(dealer)
      ai.seat = f2o(self.own_seat())
      print 'dealer','WNES'[dealer],'my seat','WNES'[ai.seat]

      deal = sbridge.Deal(dealer)
      for p in sbridge.PLAYERS:
         if self.deal.hands[p] is None:
            deal.hands[p] = None
         else: deal.hands[p] = self.deal.hands[p][:]

      ai.deal = deal
      print 'myhand',
      for c in ai.deal.hands[ai.seat]: print c,
      print
      for i in xrange(len(self.bid_status)):
         ai.bid_made(f2o_bid(self.bid_status[i]))
      
      if ai.deal.trick is None:
         bid =  o2f_bid(ai.bid())
         # evaluate hand, provide reasoning, learn/discard human suggestion???
         self.ai = None
         evaluate_deal(ai)
         print 'AI suggest bid', bid
         user = raw_input('>>>> ')
         if user == '': return bid
         return user

      if ai.seat == deal.dummy: return None
      # first lead is over, everybody else see the dummy's hand
      
      for c in self.play_status:
         deal.play_card(f2o_card(c))
         if deal.trick.cards[deal.player] is not None:
            ai.trick_complete()

      if (ai.seat != deal.declarer) and (deal.player != ai.seat): return None
      if (ai.seat == deal.declarer) and (deal.player != deal.dummy) and (deal.player != deal.declarer):
         return None         
      print 'myseat','WNES'[ai.seat],'player','WNES'[deal.player], 'dummy','WNES'[deal.dummy],'declarer','WNES'[deal.declarer]      
      evaluate_deal(ai)
      card = None
      if (deal.player == deal.dummy) and (deal.declarer == ai.seat):
         card = ai.play_dummy()
      if deal.player == ai.seat: card = ai.play_self()

      print 'Ai suggest play', str(card)
      if self.ai == 'auto': return  o2f_card(card)
      user = raw_input('>>>> ')
      if user == '': return  o2f_card(card)
      if user == 'auto':
         self.ai = 'auto'
         return  o2f_card(card)
      return pbn2f_card(user)
   
if __name__ == "__main__":
   one_client(ConsoleState())



