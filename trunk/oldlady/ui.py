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


import oldlady.ai
import oldlady.bridge
import oldlady.defs

from gettext import gettext as _
import gobject
import gtk
from gtk import gdk
from gtk import glade
import os
import pango


PLAY_CARD, CONFIRM_TRICK, CONFIRM_DEAL, CONFIRM_GAME, CONFIRM_RUBBER = range (5)


class App:
    """
    The main application winodw for Old Lady.
    """

    def __init__ (self):
        self.xml = glade.XML (os.path.join (oldlady.defs.PKG_DATA_DIR, "oldlady.glade"))
        self.xml.signal_autoconnect (self)

        self.show_all_cards = False

        self.legal_bids = gtk.ListStore (gobject.TYPE_PYOBJECT)
        renderer = gtk.CellRendererText ()
        choose_bid = self.xml.get_widget ("choose-bid")
        choose_bid.set_model (self.legal_bids)
        choose_bid.pack_start (renderer)
        choose_bid.set_cell_data_func (renderer, self.render_bid, 0)
        choose_bid.set_active (0)

        self.ais = [oldlady.ai.ComputerPlayer (seat) for seat in oldlady.bridge.PLAYERS[oldlady.bridge.WEST:oldlady.bridge.SOUTH]]
        self.start_next_rubber ()

        self.cards = gdk.pixbuf_new_from_file (os.path.join (oldlady.defs.PKG_DATA_DIR, "bonded.svg"))
        self.card_width = self.cards.get_width () / 13
        self.card_height = self.cards.get_height () / 5

        self.hand_renderers = [HandRenderer (self, player) for player in oldlady.bridge.PLAYERS]

        bids = self.xml.get_widget ("bids")

        for text, player in [(_("West"), oldlady.bridge.WEST), (_("North"), oldlady.bridge.NORTH),
                             (_("East"), oldlady.bridge.EAST), (_("South"), oldlady.bridge.SOUTH)]:
            column = gtk.TreeViewColumn (text, renderer)
            column.set_cell_data_func (renderer, self.render_bid, player)
            bids.append_column (column)

        self.xml.get_widget ("app").show ()

    def start_next_deal (self):
        """
        Prepare for the next deal of cards.
        """

        self.deal = self.rubber.next_deal ()
        for ai in self.ais:
            ai.new_deal (self.deal)
        self.messages = []
        self.play_for_ais ()
        self.update_scores ()

    def start_next_rubber (self):
        """
        Start playing for a new rubber.
        """

        self.rubber = oldlady.bridge.Rubber (oldlady.bridge.SOUTH)
        for ai in self.ais:
            ai.new_rubber (self.rubber)
        self.start_next_deal ()

    def populate_legal_bids (self):
        """
        Fill in the bidding widget with the currently legal bids.
        """

        self.legal_bids.clear ()
        for legal_bid in self.deal.legal_bids ():
            iter = self.legal_bids.append ()
            self.legal_bids.set (iter, 0, legal_bid)

    def update_scores (self):
        """
        Update the displays of scores and tricks taken.
        """

        self.xml.get_widget ("we-vulnerable").set_label (self.rubber.vulnerable[oldlady.bridge.WEST_EAST] and _("Yes") or _("No"))
        self.xml.get_widget ("ns-vulnerable").set_label (self.rubber.vulnerable[oldlady.bridge.NORTH_SOUTH] and _("Yes") or _("No"))

        self.xml.get_widget ("we-above").set_label ("%d" % self.rubber.above[oldlady.bridge.WEST_EAST])
        self.xml.get_widget ("ns-above").set_label ("%d" % self.rubber.above[oldlady.bridge.NORTH_SOUTH])

        self.xml.get_widget ("we-below").set_label ("%d" % self.rubber.below[oldlady.bridge.WEST_EAST])
        self.xml.get_widget ("ns-below").set_label ("%d" % self.rubber.below[oldlady.bridge.NORTH_SOUTH])

        self.xml.get_widget ("we-tricks").set_label ("%d" % self.deal.tricks_taken[oldlady.bridge.WEST_EAST])
        self.xml.get_widget ("ns-tricks").set_label ("%d" % self.deal.tricks_taken[oldlady.bridge.NORTH_SOUTH])

    def play_for_ais (self):
        """
        Have the AIs make their moves, continuing until it is the human
        player's turn again.  If the human is dummy, he will still need
        to confirm each trick.
        """

        while True:
            if self.deal.contract is not None and self.deal.contract.is_pass ():
                self.xml.get_widget ("choose-bid").set_sensitive (False)
                self.xml.get_widget ("make-bid").set_sensitive (False)
                self.messages = [_("Deal abandoned; all players passed")]
                self.action = CONFIRM_DEAL
                self.xml.get_widget ("tableau").queue_draw ()
                return
            elif self.deal.trick is None:
                if self.deal.player == oldlady.bridge.SOUTH:
                    self.xml.get_widget ("bids").set_model (self.deal.model)
                    self.populate_legal_bids ()
                    self.xml.get_widget ("choose-bid").set_sensitive (True)
                    self.xml.get_widget ("choose-bid").set_active (0)
                    self.xml.get_widget ("make-bid").set_sensitive (True)
                    self.xml.get_widget ("tableau").queue_draw ()
                    self.action = None
                    return
                bid = self.ais[self.deal.player].bid ()
                for ai in self.ais:
                    ai.bid_made (bid)
                self.deal.bid (bid)
            else:
                self.xml.get_widget ("choose-bid").set_sensitive (False)
                self.xml.get_widget ("make-bid").set_sensitive (False)
                if self.deal.trick.cards[self.deal.player] is not None:
                    self.action = CONFIRM_TRICK
                    self.update_scores ()
                    self.xml.get_widget ("tableau").queue_draw ()
                    return
                if self.deal.player == oldlady.bridge.SOUTH and self.deal.player != self.deal.dummy:
                    self.action = PLAY_CARD
                    self.update_scores ()
                    self.xml.get_widget ("tableau").queue_draw ()
                    return
                if self.deal.player == oldlady.bridge.NORTH and self.deal.player == self.deal.dummy:
                    self.action = PLAY_CARD
                    self.update_scores ()
                    self.xml.get_widget ("tableau").queue_draw ()
                    return
                if self.deal.player != self.deal.dummy:
                    self.ais[self.deal.player].play_self ()
                else:
                    self.ais[oldlady.bridge.partner (self.deal.player)].play_dummy ()

    ##########################################################################
    #
    # Tableau callbacks
    #
    ##########################################################################

    def tableau_expose_event_cb (self, tableau, event):
        if len (self.messages) > 0:
            layout = pango.Layout (tableau.get_pango_context ())
            layout.set_text ("\n".join (self.messages))
            gc = tableau.window.new_gc ()
            tableau.window.draw_layout (gc, 50, 50, layout)
        else:
            for renderer in self.hand_renderers:
                renderer.refresh ()
            self.render_trick ()

    def tableau_button_release_event_cb (self, tableau, event):
        if self.action == PLAY_CARD:
            if self.deal.trick is not None:
                card = self.hand_renderers[self.deal.player].card_at (event.x, event.y)
                if card is not None and self.deal.legal_card (card):
                    self.deal.play_card (card)
                    if self.deal.trick.cards[self.deal.player] is not None:
                        self.action = CONFIRM_TRICK
                    self.update_scores ()
                    tableau.queue_draw ()
        elif self.action == CONFIRM_TRICK:
            for ai in self.ais:
                ai.trick_complete ()
            self.deal.next_trick ()
            if self.deal.trick is None:
                self.messages = self.rubber.score_deal ()
                self.action = CONFIRM_DEAL
            else:
                self.action = PLAY_CARD
            self.update_scores ()
            tableau.queue_draw ()
        elif self.action == CONFIRM_DEAL:
            if not self.deal.contract.is_pass ():
                self.messages = self.rubber.score_game ()
                self.update_scores ()
                if len (self.messages) > 0:
                    self.action = CONFIRM_GAME
                    tableau.queue_draw ()
                else:
                    self.start_next_deal ()
                    self.action = None
            else:
                self.start_next_deal ()
                self.action = None
        elif self.action == CONFIRM_GAME:
            self.messages = self.rubber.score_rubber ()
            self.update_scores ()
            if len (self.messages) > 0:
                self.action = CONFIRM_RUBBER
                tableau.queue_draw ()
            else:
                self.start_next_deal ()
                self.action = None
        elif self.action == CONFIRM_RUBBER:
            self.start_next_rubber ()
            self.action = None

        if self.action is None or self.action == PLAY_CARD:
            self.play_for_ais ()

    def render_trick (self):
        """
        Draw the cards played in the current trick.
        """

        trick = self.deal.trick
        if trick is None:
            return
        tableau = self.xml.get_widget ("tableau")
        size = tableau.window.get_size ()

        for player in range (trick.leader, len (oldlady.bridge.PLAYERS)) + range (trick.leader):
            card = trick.cards[player]
            if card is not None:
                dst_x = (size[0] - self.card_width) / 2
                dst_y = (size[1] - self.card_height) / 2
                if player == oldlady.bridge.WEST:
                    dst_x -= self.card_width / 2
                elif player == oldlady.bridge.NORTH:
                    dst_y -= self.card_height / 2
                elif player == oldlady.bridge.EAST:
                    dst_x += self.card_width / 2
                else:
                    dst_y += self.card_height / 2
                src_x = (card.rank != oldlady.bridge.ACE) and (card.rank - 1) or 0
                src_y = card.suit
                tableau.window.draw_pixbuf (None, self.cards,
                                            src_x * self.card_width, src_y * self.card_height,
                                            dst_x, dst_y,
                                            self.card_width, self.card_height,
                                            gdk.RGB_DITHER_NONE, 0, 0)

    ##########################################################################
    #
    # Menu and toolbar callbacks
    #
    ##########################################################################

    def game_new_cb (self, widget):
        self.rubber = oldlady.bridge.Rubber (oldlady.bridge.SOUTH)
        self.start_next_deal ()
        self.update_scores ()

    def game_quit_cb (self, widget):
        gtk.main_quit ()

    def show_all_toggled_cb (self, widget):
        self.show_all_cards = widget.active
        self.xml.get_widget ("tableau").queue_draw ()

    def help_about_cb (self, widget):
        dialog = self.xml.get_widget ("about")
        dialog.set_name (_("Old Lady"))
        dialog.set_version (oldlady.defs.VERSION)
        dialog.present ()

    ##########################################################################
    #
    # App window callbacks
    #
    ##########################################################################

    def make_bid_cb (self, button):
        choose_bid = self.xml.get_widget ("choose-bid")
        iter = choose_bid.get_active_iter ()
        bid = self.legal_bids.get_value (iter, 0)

        for ai in self.ais:
            ai.bid_made (bid)
        self.deal.bid (bid)
        self.play_for_ais ()

    def delete_event_cb (self, window, event):
        gtk.main_quit ()

    ##########################################################################
    #
    # About dialog callbacks
    #
    ##########################################################################

    def about_delete_event_cb (self, dialog, event):
        dialog.hide ()
        return True

    def about_response_cb (self, dialog, response):
        dialog.hide ()

    ##########################################################################
    #
    # Other callbacks
    #
    ##########################################################################

    def render_bid (self, column, renderer, model, iter, player):
        bid = model.get_value (iter, player)
        renderer.set_property ("text", bid is not None and str (bid) or "")


class HandRenderer:
    """
    Helper class that handles rendering and hit detecting of a single player's
    hand.
    """

    def __init__ (self, app, player):
        self.app = app
        self.player = player
        self.rect = None

    def refresh (self):
        """
        Recompute the bounding rectangle and redraw the hand on the tableau.
        """

        tableau = self.app.xml.get_widget ("tableau")
        size = tableau.window.get_size ()
        hand = self.app.deal.hands[self.player]

        if len (hand) == 0:
            self.rect = None
            return
        self.rect = gdk.Rectangle ()

        if oldlady.bridge.team (self.player) == oldlady.bridge.WEST_EAST:
            self.rect.width = self.app.card_width
            self.rect.height = self.app.card_height + self.app.card_height * (len (hand) - 1) / 4
            self.rect.y = (size[1] - self.rect.height) / 2
        else:
            self.rect.width = self.app.card_width + self.app.card_width * (len (hand) - 1) / 5
            self.rect.height = self.app.card_height
            self.rect.x = (size[0] - self.rect.width) / 2

        if self.player == oldlady.bridge.WEST:
            self.rect.x = 10
        elif self.player == oldlady.bridge.NORTH:
            self.rect.y = 10
        elif self.player == oldlady.bridge.EAST:
            self.rect.x = size[0] - 10 - self.rect.width
        else:
            self.rect.y = size[1] - 10 - self.rect.height

        dst_x = self.rect.x
        dst_y = self.rect.y
        for card in hand:
            if self.player == oldlady.bridge.SOUTH or (self.app.deal.opening_lead and self.player == self.app.deal.dummy) or self.app.show_all_cards:
                src_x = (card.rank != oldlady.bridge.ACE) and (card.rank - 1) or 0
                src_y = card.suit
            else:
                src_x = 2
                src_y = 4
            tableau.window.draw_pixbuf (None, self.app.cards,
                                        src_x * self.app.card_width, src_y * self.app.card_height,
                                        dst_x, dst_y,
                                        self.app.card_width, self.app.card_height,
                                        gdk.RGB_DITHER_NONE, 0, 0)
            if oldlady.bridge.team (self.player) == oldlady.bridge.WEST_EAST:
                dst_y += self.app.card_height / 4
            else:
                dst_x += self.app.card_width / 5

    def card_at (self, x, y):
        """
        Return the card rendered at coordinates (x,y) on the tableau, if any.
        """

        if self.rect is None or \
           x < self.rect.x or x >= self.rect.x + self.rect.width or \
           y < self.rect.y or y >= self.rect.y + self.rect.height:
               return None

        if oldlady.bridge.team (self.player) == oldlady.bridge.WEST_EAST:
            index = (int (y) - self.rect.y) / (self.app.card_height / 4)
        else:
            index = (int (x) - self.rect.x) / (self.app.card_width / 5)

        hand = self.app.deal.hands[self.player]
        if index < len (hand):
            return hand[index]
        else:
            return hand[-1]
