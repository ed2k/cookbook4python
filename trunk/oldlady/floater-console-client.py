from floater_client import *
from sbridge import *
import os

def print_suits(suits):
   ''' print a hand, organized as 4 suit'''
   for i in xrange(4):
      j = 3 - i
      print 'CDHS'[j]+':',
      for c in suits[j]:
         print '0123456789TJQKA'[c],
      print


def solver(trump, currentTricks, deal):
   '''deal is N-W,S-C 4x4 list, shdc -> 0123 '''
   if trump != 4: trump = 3-trump
   print trump,
   for c in currentTricks: print str(c),
   print deal
   # the play to solve is always at 0 (North), so last player is always 3(West)
   first = (4 - len(currentTricks)) % 4
   if first == 2: first = 1
   test = [trump, first]
   for i in xrange(4):
      for j in xrange(4):
         n = 0
         for k in xrange(len(deal[i][j])):
            n = n | (1 << STR2RANK[deal[i][j][k]])
         test.append(n)
   for c in currentTricks:
      test.append(3-c.suit)
      test.append(c.rank)
   arg = ' '.join([str(x) for x in test])
   print arg
   r = os.popen('../ddsprogs/dds '+arg).read().splitlines()[1].split()
   print 'suit','shdc'[int(r[0])],'rank',r[1],'win tricks',r[3]    

def o2dstack_hand(hand):
   '''deal stack  has format AK QJ - T98765432, from Spade to Club'''
   h = o2pbn_hand(hand).split('.')
   h.reverse()
   for i in xrange(4):
      if h[i] == '': h[i] = '-'
   return ' '.join(h)
    
def DealGenerator(hands, bids, plays, ai):
    ''' assume it is my turn '''
    assert ai.seat != ai.deal.dummy
    myseat = ai.deal.player
    seat2 = ai.deal.dummy
    if seat2 == myseat: seat2 = partner(myseat)
    hand2 = o2dstack_hand(hands[seat2])
    mine = o2dstack_hand(hands[myseat])

    eargs = []
    hidden = []
    for i in xrange(4):
       if i == myseat: continue
       if i == seat2: continue
       hidden.append(i)
       if ai.deal.played_hands[i] != []:
          eargs.append(sbridge.PLAYER_NAMES[i].lower()+' gets')
       for c in ai.deal.played_hands[i]:
          eargs.append(str(c).upper())       
       eargs.append(';')
    cmd = './deal -i format/pbn -'+seat_str(seat2)+' "'+ hand2 + '" -'+seat_str(myseat)+' "'+mine+'" -e "'+' '.join(eargs)+'" 1'
    print cmd
    newdeal = os.popen(cmd).read().splitlines()[0].split('"')[1][2:].split()

    print str(ai.deal.trick)
    #move ai to North
    ddeal = [[],[],[],[]]
    currentTrick = []
    # put estimated deal in 4x4 format
    for i in xrange(4):
       d = newdeal[i].split('.')
       ddeal[i] = [[],[],[],[]]
       for j in xrange(4):
          ddeal[i][j] = list(set(d[j]))
    # remove played cards
    for i in xrange(4):
       seat = f2o(i)
       for c in ai.deal.played_hands[seat]:
          s = 3-c.suit
          ddeal[i][s].remove(str(c)[0])
    
    lenMine = len(ai.deal.played_hands[myseat])
    currentTrick = []
    seat = seat_prev(myseat)
    while (len(ai.deal.played_hands[seat]) > lenMine):
       currentTrick.insert(0,ai.deal.played_hands[seat][-1])
       seat = seat_prev(seat)

    sdeal = [[],[],[],[]]
    #move my seat to North for solver
    for i in xrange(4):
       d = ddeal[seat_move(myseat,i)]
       for j in xrange(4):
          sdeal[i].append(''.join(d[j]))

    solver(ai.deal.contract.denom, currentTrick, sdeal)
          
    

      
   
class ConsoleState(State):
   def handle_auction(self):
      ''' try use oldlady engine '''
      dealer = (self.hand_id-1) % 4
      ai = sAi.ComputerPlayer(dealer)
      ai.seat = self.own_seat()
      print 'dealer','NESW'[dealer],'my seat','NESW'[ai.seat]

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
      print 'myseat','NESW'[ai.seat],'player','NESW'[deal.player], 'dummy','NESW'[deal.dummy],'declarer','NESW'[deal.declarer]      
      #evaluate_deal(ai)
      # TODO, generate a deal based on bidding, and play history
      # use double dummy solver to find the best play.
      if deal.hands[deal.dummy] is not None:
         guess_deal = DealGenerator(self.deal.hands, self.bid_status, self.play_status, ai)
      # card = solver(guess_deal)      
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



