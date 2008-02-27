#! /usr/bin/env python

# Old Lady
# Copyright (C) 2007 Paul Kuliniewicz <paul@kuliniewicz.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1301, USA.


import sbridge
from sbridge import *
import bidding

sayc = bidding.Bidding ()

class ComputerPlayer:
    """
    A computer player.

    Currently, the AI is dumb as rocks, but it'll at least play by the rules.
    """
    bidState = None
    def __init__ (self, seat):
        self.seat = seat
        self.rubber = None
        self.deal = None
        self.history = []
    def new_deal (self, deal):
        """
        Reset the computer player's state for a new deal.

        Instances of this class are recycled between deals in case the AI
        eventually learns how the human plays and adapts its strategy
        accordingly....
        """

        #self.deal = deal
        self.deal = sbridge.Deal(deal.dealer)
        for p in sbridge.PLAYERS:
            if deal.hands[p] is not None:
                self.deal.hands[p] = deal.hands[p][:]
            else:
                self.deal.hands[p] = None
        self.bidState = AIBidStatus(self)
        self.history = []

    def bid_made (self, bid):
        """
        Called after each bid is placed.
        """
        self.bidState.evaluateBid(bid)
        self.deal.bid(bid)
        self.history.insert (0, bid)
        
    def trick_complete (self):
        """
        Called at the end of each trick.

        This should be used as part of keeping track of which cards have
        been played, but for now, does nothing.
        """

        self.deal.next_trick()

    def bid (self):
        """
        Place a bid during the bidding phase.

        Currently, this just passes all the time.  Eventually, it will use
        an actual, usable bidding strategy....
        """
        bid = sayc.choose_bid (self.deal.hands[self.seat], self.history)
        print 'prolog:',self.seat,bid
        self.bidState.evaluate_deal()
        return bid
        
    
    def play_self (self):
        """
        Play a card from the AI's own hand during a trick.

        Currently, this is really dumb, and doesn't bother trying to keep
        track of what cards have been played or anything.
        """

        return self.play_from_hand (self.seat)
        # normally player just play its own card
        # so we return what card played to table manager

    def play_dummy (self):
        """
        Play a card from the AI's partner's dummy hand during a trick.

        Currently, this is really dumb, treating playing the dummy's hand
        no differently than playing its own.
        """
        assert self.seat == self.deal.declarer
        return self.play_from_hand (self.deal.dummy)

    def play_from_hand (self, player):
        """
        Play a card during a trick from the specified player's hand.

        Currently, this is pretty dumb.  If leading a trick, it plays the
        highest-ranked card it has in an arbitrary suit.  Otherwise, it
        tries to play the lowest-ranked card that will take the trick for
        its team, or dump the lowest-ranked card in its hand.
        """

        hand = self.deal.hands[player][:]
        trick = self.deal.trick

        if trick.leader == player:
            # Lead the highest-ranked card in the hand.
            hand.sort (by_rank, None, True)
            return hand[0]

        elif sbridge.team (trick.winner) == sbridge.team (player):
            # Dump the lowest-ranked card, non-trump if possible.
            candidates = [card for card in hand if card.suit == trick.lead]
            if len (candidates) == 0:
                candidates = [card for card in hand if card.suit != self.deal.contract.denom]
            if len (candidates) == 0:
                candidates = hand
            candidates.sort (by_rank)
            return candidates[0]

        else:
            # Try to take the trick with the lowest-ranked card possible.
            in_suit = [card for card in hand if card.suit == trick.lead]
            trumps = [card for card in hand if card.suit == self.deal.contract.denom]
            garbage = [card for card in hand if card.suit != trick.lead and card.suit != self.deal.contract.denom]
            if len (in_suit) > 0:
                candidates = []
                if trick.cards[trick.winner].suit == trick.lead:
                    candidates = [card for card in in_suit if card.rank > trick.cards[trick.winner].rank]
                if len (candidates) == 0:
                    candidates = in_suit
                candidates.sort (by_rank)
                return candidates[0]
            elif len (trumps) > 0:
                candidates = []
                if trick.cards[trick.winner].suit == self.deal.contract.denom:
                    candidates = [card for card in trumps if card.rank > trick.cards[trick.winner].rank]
                if len (candidates) == 0:
                    candidates = trumps
                candidates.sort (by_rank)
                return candidates[0]
            else:
                garbage.sort (by_rank)
                return garbage[0]
    def player(self): return self.deal.player
    def partner(self): return partner(self.seat)
    def lho(self): return seat_next(self.seat)
    def rho(self): return seat_prev(self.seat)


def by_rank (x, y):
    """
    Sort function for comparing two cards strictly by rank.
    """

    return cmp (x.rank, y.rank)

#--------------- AI bidding stuff -------------
def hand2suits(hand):
   suits = {0:[],1:[],2:[],3:[]} # somwhow list of list doesn't work
   for c in hand:
      idx = c.suit
      suits[idx].append(c.rank)
   return suits


def hcp(hand):
   h = 0
   for c in hand:
       h += c.hcp()
   return h
def shape(hand): return 0

sayc_opening= [['1n','hcp in 16..18, shape_type is balanced'],
    ['1s','hcp in 13..21, s > h, s >= 5'],
    ['1h','hcp in 13..21, h >= 5,h >= s'],
    ['1d','hcp in 13..21, s < 5,h < 5,d >= 3, d > c'],
    ['1c','hcp in 13..21, s < 5,h < 5,c >= 3, c >= d'],
    [' p','catchall']
    ]
sayc_opening2=[[' x','hcp >= 16'],
               [' x','hcp >= 12, suit < 4, unbid_major >= 4'],
               ['+1','hcp in 9..16, newsuit0 >= 5'],
               ['+2','hcp in 11..16, newsuit0 >= 5'],
               ['jump', 'hcp in 6..10, newsuit >= 6']
               ]
''' short means the length of shortest suit, long means the lenght of longest suit
      suit the one in opening bid
      newsuit0 means if bid only in same level (c -> dhs, d -> hs)
      '''
sayc_response1= [['+1','opening_suit_type is major, hcp in 6..10, suit >= 3'],
                 ['+2','opening_suit_type is major, hcp >= 13,suit >= 3'],
                 ['+3','opening_suit_type is major, hcp in 6..10, suit >= 4, shape_type is unbalanced'],
                 ['1d','opening is 1c, hcp in 6..18, d >= 4'],
                 ['1h','opening is 1c, hcp in 6..18, h >= 4'],
                 ['1s','opening is 1c, hcp in 6..18, s >= 4'],
                 ['1h','opening is 1d, hcp in 6..18, h >= 4'],
                 ['1s','opening is 1d, hcp in 6..18, s >= 4'],
                 ['1n','hcp in 6..10'],
                 ['2n','opening_suit_type is minor, hcp in 13..15, shape_type is balanced'],
                 ['3n','opening_suit_type is minor, hcp in 16..18, shape_type is balanced'],
                 ['1s','opening is 1h, hcp in 6..18, s >= 4'],
                 ['2c','opening_suit_type is major, hcp in 6..18, c >= 4'],
                 ['2d','opening_suit_type is major, hcp in 6..18, d >= 4'],
                 ['2h','opening is 1s, hcp in 11..18, h >= 5'],
                 ['jump','hcp >= 19'],
                 ['2d','opening is 1d, hcp in 6..10, d >= 4'],
                 ['2c','opening is 1c, hcp in 6..10, c >= 5'],
                 ['2d','opening is 1d, hcp >= 13, d >= 4'],
                 ['2c','opening is 1c, hcp >= 13, c >= 5'],                 
                 ]
saycOpenerNextBid = [['+1','response1_type is raise, bidseqs is 1+2, hcp+shortage in 13..15'],
                     ['+1','response1_type is raise, bidseqs is 1+1, hcp+shortage in 16..18'],
                     ['+2','response1_type is raise, bidseqs is 1+1, hcp+shortage in 19..21'],
                     ['+2','response1_type is raise, bidseqs is 1+2, hcp+shortage in 16..18'],
                     ['+3','response1_type is raise, bidseqs is 1+2, hcp+shortage in 19..21'],
                     ['+3','response1_type is raise, bidseqs is 1+2, hcp+shortage in 19..21'],
                     ['??','response1 is 1n, hcp in 13..15, shape is unbalanced'],
                     ['2n','response1 is 1n, hcp in 16..18'],
                     ['3_','response1 is 1n, hcp+shortage in 16..18'],
                     ['3n','response1 is 1n, hcp in 19..21'],
                     ['4_','response1 is 1n, hcp+shortage in 19..21'],
                     
                     [' p','catchall']]

class OneHand:
   '''
   define: balanced, semi-balance, un-balance
   opening bids,
   hcp=22+, 2NT hcp=22-24 shape=balanced
        3NT hcp=25-27 shape=balanced
        2C else
   hcp=0-12, 2x hcp=6-10, len=6x (x=DHS)
         3x (up to 5D), len=7+ (rule of two and three?)
         Pass else
   hcp=13-21, 1NT hcp=16-18 shap=balanced
          1H/S, len=5+, the longest, (if the same len, 1H)
          1D, len=4+
          1C, len=2+
          
   hcp=9+, len=5+ Overcall
   takeout double, hcp=12+, len(enemysuit)=short
   
   '''
   def __init__(self, ai):
      self.ai = ai
      self.hand = ai.deal.hands[ai.seat]
      self.suits = hand2suits(self.hand)
   def hcp(self): return hcp(self.hand)
   def n_s(self): return len(self.suits[3])
   def n_h(self): return len(self.suits[2])
   def n_d(self): return len(self.suits[1])
   def n_c(self): return len(self.suits[0])   
   def opening(self):
       for rule in sayc_opening:
           if self.check(rule[1]): return rule[0]
       return None
   def opening2(self,openbid):
       self.opening = openbid
       for rule in sayc_opening2:
           if self.check(rule[1]): return rule[0]
       return None
   def response1(self, openbid):
      ''' short means the length of shortest suit, long means the lenght of longest suit
      suit the one in opening bid
      '''
      self.opening = openbid
      print 'openbid',openbid
      response = ''
      for rule in sayc_response1:
          if self.check(rule[1]):
              response = rule[0]
              break
      if response == '': print 'not defined response rule'
      return response
   def newsuit(self):
       r = []
       for suit in xrange(4):
           if suit == self.ai.bidState.opening[1].denom: continue
           r.append(len(self.suits[suit]))
       return max(r)
   def newsuit0(self):
       r = []
       for suit in xrange(self.ai.bidState.currentBid[1].denom+1,4):
           if suit == self.ai.bidState.currentBid[1].denom: continue
           r.append(len(self.suits[suit]))
       return max(r)   
   def check(self, ruleseqs):
      if ruleseqs == 'catchall': return True
      for r in ruleseqs.split(','):
         left,op,right = r.split()
         left = self.get(left)
         right = self.get(right)
         if not self.op(left,right,op): return False
      print 'got',ruleseqs   
      return True
   def get(self, symbol):
       if symbol == 'hcp': return self.hcp()
       if symbol == 'opening_suit_type':return self.ai.bidState.opening[1].suit_type()
       if symbol == 'opening':return str(self.ai.bidState.opening[1])
       if symbol == 'newsuit':return self.newsuit()
       if symbol == 'newsuit0': return self.newsuit0()
       if symbol in 'cdhs':
           return  len(self.suits[KIDX[symbol]])
       if symbol.find('..') > 0:
           minv, maxv = symbol.split('..')
           return (int(minv),int(maxv))
       try:
           return int(symbol)
       except: return symbol
   def op(self, left, right, opcode):
      if opcode == '<': return (left < right)
      if opcode == '>': return (left > right)
      if opcode == '>=': return (left >= right)
      if opcode == '<=': return (left <= right)
      if opcode == 'in':
          minv,maxv = right
          return (left >= minv) and (left <= maxv)
      if opcode == 'is': return left == right
      print 'unknown op',left,opcode,right
         

class AIBidStatus:
    ''' Another class record bid history, but interpret as hcp, shape etc. to help bidding and playing'''
    opening = None
    # not pass, double
    currentBid = None
    def __init__(self, ai):
        self.ai = ai
        self.handsEval = []
        for p in PLAYERS:
            self.handsEval.append(HandEvaluation())
        self.hand = OneHand(ai)
    def setOpening(self):
        if self.ai.history == []: return None
        if self.opening is not None: return self.opening
        player = self.ai.deal.dealer
        bids = self.ai.history[:]
        bids.reverse()
        for bid in bids:
            if not bid.is_pass():
                self.opening = (player, bid)
                return (player, bid)
            player = seat_next(player)
        return None
    def evaluateBid(self, bid):
        openbid = self.setOpening()
        if not bid.is_pass() and not bid.is_double() and not bid.is_redouble():
            self.curretnBidLevel = (self.ai.deal.player, bid)
        if openbid is None:
            for rule in sayc_opening:
                if str(bid) == rule[0]: print rule[1]

    def evaluate_deal(self):
       ''' find the proper bid
        hand, in suit order
       print hcp, shape, estimate partner and opponents
       '''
       deal = self.ai.deal

       #mysuits = hand2suits(hand)
       if deal.trick is None:
          print 'HCP', self.hand.hcp()
          openbid =  self.opening
          bid = 'pass'
          if openbid is None: bid = self.hand.opening()
          elif openbid[0] == partner(self.ai.seat):
              bid = self.hand.response1(openbid[1])
          elif openbid[0] == seat_prev(self.ai.seat):
              bid = self.hand.opening2(openbid[1])
          print 'ai bid',bid
          return                
            
class HandEvaluation:
    hcp = ''

r='''
      if self.check(['opening == major','hcp in 6..10','suit >= 3']): return '+1'
      if self.check(['opening == minor','hcp in 6..10','suit >= 5','major < 4']): return '+1'
      if self.chekc(['opening == major','hcp in 11..12','suit >= 3']): return '+2'
      if self.check(['opening == minor','hcp in 11..12','suit >= 5','major < 4']): return '+2'
      if self.check(['hcp <= 9', 'long >= 5', 'short <= 1']): return '1'
      if self.check(['hcp >= 6','c < 5','d >= 4','h < 4','s < 4','opening == 1c']): return 'id'

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
