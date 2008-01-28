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


class OneHand:   
   def __init__(self, hand):
      self.hand = hand
      self.suits = hand2suits(hand)
   def hcp(self): return hcp(self.hand)
   def n_s(self): return len(self.suits[3])
   def n_h(self): return len(self.suits[2])
   def n_d(self): return len(self.suits[1])
   def n_c(self): return len(self.suits[0])   
   def opening(self): 
      if self.check(['hcp >= 13', 's > h','s >= 5']): return '1s'
      if self.check(['hcp >= 13', 'h >= 5','h >= s']): return '1h'
      if self.check(['hcp >= 13', 's < 5','h < 5','d >= 3', 'd > c']): return '1d'
      if self.check(['hcp >= 13', 's < 5','h < 5','c >= 3', 'c >= d']): return '1c'
      if self.check(['hcp < 13']): return ' p'
      print 'leakage'
      return None
   def response_1(self):
      ''' short means the length of shortest suit, long means the lenght of longest suit
      suit the one in opening bid
      '''
      if self.check(['opening == major','hcp in 6..10','suit >= 3']): return '+1'
      if self.check(['opening == minor','hcp in 6..10','suit >= 5','major < 4']): return '+1'
      if self.chekc(['opening == major','hcp in 11..12','suit >= 3']): return '+2'
      if self.check(['opening == minor','hcp in 11..12','suit >= 5','major < 4']): return '+2'
      if self.check(['hcp <= 9', 'long >= 5', 'short <= 1']): return '1'
      if self.check(['hcp >= 6','c < 5','d >= 4','h < 4','s < 4','opening == 1c']): return 'id'
      r='''
       opening == minor, hcp >= 6, h == 4, h >= s -> 1h
       opening == minor, hcp >= 6, h > 4, h > s -> 1h
       opening == minor, hcp >= 6, s >= 4, s >= h -> 1s
       opening == minor, hcp >= 6, s == 4, h < 4 -> 1s
       opening == 1h, hcp >= 6, h < 3, s >= 4 -> 1s
       deny-opener-support openning ==  major, suit < 3
               or suit < 5
       denom_lt opening == 1, hcp >= 11, deny-opener-support, long >= 4 -> new at 2
       denom_lt opening == 1, hcp >= 19, deny-opener-support, long > 5 -> new at 2
       hcp in 6..10, deny_opener_support long < 4 -> 1n
       hcp >= 13 -> 2n jacoby_2n
       hcp in 15..17, balanced, suit >= 2 -> 3n
       hcp < 6 -> p

       rebid: support is the suit partner bid previously
       hcp in 13..16, support >= 4 -> +1
       hcp in 17..18, support >= 4 -> +1
       
      '''
   def check(self, ruleseqs):
      for r in ruleseqs:
         left,op,right = r.split()
         left = self.get(left)
         right = self.get(right)
         if self.op(left,right,op): return True
   def get(self, symbol):
      if symbol == 'hcp': return self.hcp()
      if symbol in 'cdhs':
         return  len(self.suits[KIDX[symble]])
   def op(self, left, right, op):
      if op == '<': return (left < right)
      if op == '>': return (left > right)
      if op == '>=': return (left >= right)
      if op == '<=': return (left <= right)
         
      
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
      print OneHand(hand).opening()
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



