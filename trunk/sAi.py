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
import bidding

sayc = bidding.Bidding ()

class ComputerPlayer:
    """
    A computer player.

    Currently, the AI is dumb as rocks, but it'll at least play by the rules.
    """

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
            if self.seat == p:
                self.deal.hands[p] = deal.hands[self.seat][:]
            else:
                self.deal.hands[p] = None
            
        self.history = []

    def bid_made (self, bid):
        """
        Called after each bid is placed.
        """
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
        return sayc.choose_bid (self.deal.hands[self.seat], self.history)
    
    def play_self (self):
        """
        Play a card from the AI's own hand during a trick.

        Currently, this is really dumb, and doesn't bother trying to keep
        track of what cards have been played or anything.
        """

        self.play_from_hand (self.seat)
        # normally player just play its own card
        # so we return what card played to table manager
        return self.deal.curcard

    def play_dummy (self):
        """
        Play a card from the AI's partner's dummy hand during a trick.

        Currently, this is really dumb, treating playing the dummy's hand
        no differently than playing its own.
        """
        assert self.seat == self.deal.declarer
        self.play_from_hand (self.deal.dummy)
        return self.deal.curcard

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
            self.deal.play_card (hand[0])

        elif sbridge.team (trick.winner) == sbridge.team (player):
            # Dump the lowest-ranked card, non-trump if possible.
            candidates = [card for card in hand if card.suit == trick.lead]
            if len (candidates) == 0:
                candidates = [card for card in hand if card.suit != self.deal.contract.denom]
            if len (candidates) == 0:
                candidates = hand
            candidates.sort (by_rank)
            self.deal.play_card (candidates[0])

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
                self.deal.play_card (candidates[0])
            elif len (trumps) > 0:
                candidates = []
                if trick.cards[trick.winner].suit == self.deal.contract.denom:
                    candidates = [card for card in trumps if card.rank > trick.cards[trick.winner].rank]
                if len (candidates) == 0:
                    candidates = trumps
                candidates.sort (by_rank)
                self.deal.play_card (candidates[0])
            else:
                garbage.sort (by_rank)
                self.deal.play_card (garbage[0])

def by_rank (x, y):
    """
    Sort function for comparing two cards strictly by rank.
    """

    return cmp (x.rank, y.rank)
