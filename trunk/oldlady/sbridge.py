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


import random

def _(A): return A

class gtk_ListStore:
    data = []
    def append(self):
        self.data.append({})
        return len(self.data)-1
    def set(self, row, a, b):
        self.data[row][a] = b
        #print row,':',a,b
    def foreach(self, func, a):
        for idx in xrange(len(self.data)):
            if func(self, None, idx, None): return
    def get_value(self, idx, a):
        if idx <0 or idx >= len(self.data):return None
        if a not in self.data[idx]: return None
        #print 'get',idx, a, self.data[idx][a]
        return self.data[idx][a]
    
        

RANKS = range (2, 15)
TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE = RANKS

SUITS = range (4)
DENOMINATIONS = range (5)
CLUBS, DIAMONDS, HEARTS, SPADES, NO_TRUMP = DENOMINATIONS

PASS = -1
DOUBLE = -2
REDOUBLE = -3

PLAYERS = range (4)
WEST, NORTH, EAST, SOUTH = PLAYERS
PLAYER_NAMES = [_("West"), _("North"), _("East"), _("South")]

TEAMS = range (2)
WEST_EAST, NORTH_SOUTH = TEAMS
TEAM_NAMES = [_("West-East"), _("North-South")]


def denomination_to_string (denom):
    """
    Produces a string representation of a suit or denomination.
    """

    #return ["\xe2\x99\xa3", "\xe2\x99\xa6", "\xe2\x99\xa5", "\xe2\x99\xa0", "NT"][denom]
    return ["c", "d", "h", "s", "n"][denom]


def rank_to_string (rank):
    """
    Produces a string representation of a rank.
    """

    if rank <= 10:
        return str (rank)
    else:
        return ["J", "Q", "K", "A"][rank - JACK]


def team (player):
    """
    Returns the team a player is on.
    """

    return player % 2


def partner (player):
    """
    Returns the partner of a player.
    """

    return (player + 2) % 4


class Card:
    """
    A single playing card.
    """

    def __init__ (self, suit, rank):
        self.suit = suit
        self.rank = rank

    def __str__ (self):
        return rank_to_string (self.rank) + denomination_to_string (self.suit)

    def __cmp__ (self, other):
        return self.suit.__cmp__ (other.suit) or self.rank.__cmp__ (other.rank)


class Bid:
    """
    A bid made during the opening phase of a deal.
    """

    def __init__ (self, level, denom=None):
        self.level = level
        self.denom = denom

    def __str__ (self):
        if self.is_pass ():
            return "Pass"
        elif self.is_double ():
            return "Double"
        elif self.is_redouble ():
            return "Redouble"
        else:
            return str (self.level) + denomination_to_string (self.denom)

    def is_contract (self):
        """
        Return whether this bid represents a possible contract.
        """

        return self.level > 0

    def is_pass (self):
        """
        Return whether this bid is a pass.
        """

        return self.level == PASS

    def is_double (self):
        """
        Return whether this bid is a double.
        """

        return self.level == DOUBLE

    def is_redouble (self):
        """
        Return whether this bid is a redouble.
        """

        return self.level == REDOUBLE


class Trick:
    """
    A single trick during a deal.
    """

    def __init__ (self, player, trump):
        self.player = player
        self.trump = trump
        self.winner = None
        self.cards = [None, None, None, None]
        self.lead = None
        self.leader = player

    def play_card (self, card):
        """
        Play a card during the trick.  Return whether the trick requires
        more cards before determining a winner.
        """

        self.cards[self.player] = card
        if self.lead is None:
            self.lead = card.suit
            self.winner = self.player
        elif (card.suit == self.cards[self.winner].suit and card.rank > self.cards[self.winner].rank) or \
             (card.suit == self.trump and self.cards[self.winner].suit != self.trump):
            self.winner = self.player
        self.player = (self.player + 1) % 4
        return self.cards[self.player] is None
    def __str__(self):
        r = []
        for c in self.cards:
            if c is None: r.append('-')
            else: r.append(str(c))
        return ' '.join(r)


class Deal:
    """
    A single deal of contract bridge.
    """
    def shuffle(self):
        deck = []
        for suit in SUITS:
            for rank in RANKS:
                deck.append (Card (suit, rank))
        random.shuffle (deck)

        self.hands = [deck[0:13], deck[13:26], deck[26:39], deck[39:52]]
        for hand in self.hands:
            hand.sort ()
            hand.reverse()
            
    def __init__ (self, dealer):
        self.played_hands = [[],[],[],[]]
        self.hands = [None]*4
        self.dealer = dealer
        self.player = dealer
        self.contract = None
        self.passes = 0
        self.contractor = None
        self.doubler = None
        self.redoubler = None
        self.curcard = None # -yisu

        self.declarer = None
        self.dummy = None
        self.trick = None
        self.opening_lead = False
        self.tricks_taken = [0, 0]

        #self.model = gtk.ListStore (gobject.TYPE_PYOBJECT,
        #                            gobject.TYPE_PYOBJECT,
        #                            gobject.TYPE_PYOBJECT,
        #                            gobject.TYPE_PYOBJECT)
        self.model = gtk_ListStore()
        if dealer != WEST:
            self.iter = self.model.append ()
        else:
            self.iter = None

    def legal_bids (self):
        """
        Returns a list of the legal bids available to the next bidder.  If
        bidding is over, the list is empty.
        """

        if (self.contract is not None and self.passes == 3) or self.passes == 4:
            return []

        legal = [Bid (PASS)]
        if self.contract is not None:
            for denom in range (self.contract.denom + 1, NO_TRUMP + 1):
                legal.append (Bid (self.contract.level, denom))
            for level in range (self.contract.level + 1, 8):
                for denom in range (NO_TRUMP + 1):
                    legal.append (Bid (level, denom))
        else:
            for level in range (1, 8):
                for denom in range (NO_TRUMP + 1):
                    legal.append (Bid (level, denom))

        if self.contract is not None:
            if self.doubler is None and self.contractor != team (self.player):
                legal.append (Bid (DOUBLE))
            elif self.doubler is not None and self.redoubler is None and self.doubler != team (self.player):
                legal.append (Bid (REDOUBLE))

        return legal

    def bid (self, bid):
        """
        Place the current bidder's bid.  Returns whether bidding continues.
        """

        if bid.is_pass ():
            self.passes += 1
        elif bid.is_double ():
            self.passes = 0
            self.doubler = team (self.player)
        elif bid.is_redouble ():
            self.passes = 0
            self.redoubler = team (self.player)
        else:
            self.contract = bid
            self.passes = 0
            self.contractor = team (self.player)
            self.doubler = None
            self.redoubler = None

        if self.player == WEST:
            self.iter = self.model.append ()
            # new round in a graphic sense
            print
        self.model.set (self.iter, self.player, bid)
        print 'WNES'[self.player],bid,
        self.player = (self.player + 1) % 4

        if self.passes < 3 or (self.passes == 3 and self.contract is None):
            return True
        else:
            print
            #-yisu here we found auction is over,
            self.prepare_trick_taking ()
            #TODO get to see the dummy hand for each player, but not now, when?
            return False

    def prepare_trick_taking (self):
        """
        Transition from the bidding phase of the deal to the trick-taking
        phase.
        """

        def find_declarer (model, path, iter, data):
            for player in PLAYERS:
                bid = model.get_value (iter, player)
                if team (player) == team (self.contractor) and bid is not None and bid.denom == self.contract.denom:
                    self.declarer = player
                    return True
            return False

        if self.contract is not None:
            self.model.foreach (find_declarer, None)
            self.dummy = partner (self.declarer)
            self.player = (self.declarer + 1) % 4
            self.trick = Trick (self.player, self.contract.denom)
            print 'dealer','WNES'[self.dealer],'player','WNES'[self.player], 'dummy','WNES'[self.dummy],'declarer','WNES'[self.declarer]
        else:
            self.contract = Bid (PASS)

    def legal_card (self, card):
        """
        Check whether a card is legal to play in the current trick.
        """

        return self.trick.lead is None or (self.trick.lead == card.suit) or len ([c for c in self.hands[self.player] if c.suit == self.trick.lead]) == 0

    def play_card (self, card):
        """
        Play a card in the current trick.
        """
        if self.hands[self.player] != None:
            self.hands[self.player].remove (card)
            
        self.trick.play_card (card)
        self.played_hands[self.player].append(card)
        
        self.player = (self.player + 1) % 4
        self.opening_lead = True
        #-yisu a hack to save which is played for query
        # as ai.play_self() called play_card at so many place
        self.curcard = card

    def next_trick (self):
        """
        Advance to the next trick, if there is one.
        """

        self.tricks_taken[team (self.trick.winner)] += 1
        if self.tricks_taken[WEST_EAST] + self.tricks_taken[NORTH_SOUTH] < 13:
            self.player = self.trick.winner
            self.trick = Trick (self.trick.winner, self.contract.denom)
        else:
            self.trick = None


class Rubber:
    """
    Keeps dealing hands until one team wins two games; then tallies up points
    to determine who wins.
    """

    def __init__ (self, dealer):
        self.deal = None
        self.hands = None
        self.dealer = dealer
        self.above = [0, 0]
        self.below = [0, 0]
        self.vulnerable = [False, False]
        self.rubber = False

    def next_deal (self):
        """
        Start another deal, returning the deal for convenience.
        """

        self.deal = Deal (self.dealer)
        self.deal.shuffle()
        self.dealer = (self.dealer + 1) % 4
        self.hands = [hand[:] for hand in self.deal.hands]

        return self.deal

    def score_deal (self):
        """
        Tally up the scores for the current deal only, and return a list of
        what points were earned and how.
        """

        scoring = []
        level = self.deal.contract.level
        denom = self.deal.contract.denom
        offense = team (self.deal.declarer)
        defense = 1 - offense

        if self.deal.tricks_taken[offense] >= self.deal.contract.level + 6:
            if denom == CLUBS or denom == DIAMONDS:
                points = 20 * level
            elif denom == HEARTS or denom == SPADES:
                points = 30 * level
            else:
                points = 30 * level + 10

            if self.deal.redoubler is not None:
                points *= 4
            elif self.deal.doubler is not None:
                points *= 2

            scoring.append (_("%s earns %d points below for making contract") % (TEAM_NAMES[offense], points))
            self.below[offense] += points

            overtricks = self.deal.tricks_taken[offense] - 6 - level
            if overtricks > 0:
                if self.deal.redoubler is not None and self.vulnerable[offense]:
                    points = 400 * overtricks
                elif self.deal.redoubler is not None:
                    points = 200 * overtricks
                elif self.deal.doubler is not None and self.vulnerable[offense]:
                    points = 200 * overtricks
                elif self.deal.doubler is not None:
                    points = 100 * overtricks
                elif denom == CLUBS or denom == DIAMONDS:
                    points = 20 * overtricks
                else:
                    points = 30 * overtricks
                scoring.append (_("%s earns %d points above for %d overtricks") % (TEAM_NAMES[offense], points, overtricks))
                self.above[offense] += points
                if self.deal.doubler is not None:
                    if self.deal.redoubler is not None:
                        points = 100
                    else:
                        points = 50
                    scoring.append (_("%s earns %d points above for the insult") % (TEAM_NAMES[offense], points))
                    self.above[offense] += points

            if level == 7:
                if self.vulnerable[offense]:
                    points = 1500
                else:
                    points = 1000
                scoring.append (_("%s earns %d points above for the grand slam") % (TEAM_NAMES[offense], points))
                self.above[offense] += points
            elif level == 6:
                if self.vulnerable[offense]:
                    points = 750
                else:
                    points = 500
                scoring.append (_("%s earns %d points above for the slam") % (TEAM_NAMES[offense], points))
                self.above[offense] += points
        else:
            undertricks = 6 + level - self.deal.tricks_taken[offense]
            if self.deal.doubler is not None:
                if self.vulnerable[offense]:
                    points = 200 + 300 * (undertricks - 1)
                else:
                    if undertricks >= 4:
                        points = 100 + 200 + 200 + 300 * (undertricks - 3)
                    elif undertricks >= 2:
                        points = 100 + 200 * (undertricks - 1)
                    else:
                        points = 100
                if self.deal.redoubler is not None:
                    points *= 2
            else:
                if self.vulnerable[offense]:
                    points = 100 * undertricks
                else:
                    points = 50 * undertricks
            scoring.append (_("%s earns %d points above for %d undertricks") % (TEAM_NAMES[defense], points, undertricks))
            self.above[defense] += points

        # TODO: Verify that these points never get (re)doubled.
        for player in PLAYERS:
            if denom == NO_TRUMP:
                aces = [card for card in self.hands[player] if card.rank == ACE]
                if len (aces) == 4:
                    scoring.append (_("%s earns 150 points above for %s having all four aces") % (TEAM_NAMES[team (player)], PLAYER_NAMES[player]))
                    self.above[team (player)] += 150
            else:
                honors = [card for card in self.hands[player] if card.suit == denom and card.rank >= TEN]
                if len (honors) == 5:
                    scoring.append (_("%s earns 150 points above for %s having all five top trump honors") % (TEAM_NAMES[team (player)], PLAYER_NAMES[player]))
                    self.above[team (player)] += 150
                elif len (honors) == 4:
                    scoring.append (_("%s earns 100 points above for %s having four of the five top trump honors") % (TEAM_NAMES[team (player)], PLAYER_NAMES[player]))
                    self.above[team (player)] += 100

        return scoring

    def score_game (self):
        """
        Tally up the scores for the end of the game, if the end of the game
        has indeed been reached.  Returns a list of what points were earned
        and how as a result.
        """

        scoring = []
        offense = team (self.deal.declarer)
        defense = 1 - offense

        if self.below[offense] >= 100:
            scoring.append (_("%s wins a game") % TEAM_NAMES[offense])
            if self.vulnerable[offense]:
                self.rubber = True
                if self.vulnerable[defense]:
                    scoring.append (_("%s earns 500 points above for ending the rubber in three games") % TEAM_NAMES[offense])
                    self.above[offense] += 500
                else:
                    scoring.append (_("%s earns 700 points above for ending the rubber in two games") % TEAM_NAMES[offense])
                    self.above[offense] += 700
            else:
                self.vulnerable[offense] = True
            for pair in TEAMS:
                self.above[pair] += self.below[pair]
                self.below[pair] = 0
            scoring.append (_("Scores below get moved above"))

        return scoring


    def score_rubber (self):
        """
        Tally up the final scores to see who wins the rubber, if the rubber
        has in fact been decided.  Returns a list of what happened.
        """

        scoring = []

        if self.rubber:
            if self.above[WEST_EAST] == self.above[NORTH_SOUTH]:
                scoring.append (_("Both teams tie the rubber!"))
            else:
                if self.above[WEST_EAST] > self.above[NORTH_SOUTH]:
                    victor = WEST_EAST
                else:
                    victor = NORTH_SOUTH
                scoring.append (_("%s wins the rubber!") % TEAM_NAMES[victor])

        return scoring

class HandEvaluation:
    def __init__(self, hand):
        self.hand = hand
        
