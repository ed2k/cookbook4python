puts {
<html><head><TITLE>Floater Tutorial</TITLE></head>
<body bgcolor="#ffffff" text="#000000" link="#CC0000" alink="#FF3300" vlink="#0000CC">

<P align=center>
<img BORDER=0 SRC="medwhitefloater.gif" ALT="Floater">

<center><H3>Tutorial</H3></center>
<p>

This tutorial was constructed using Floater 1.0beta in late 1996.  It
is still a good introduction to using Floater, however.<p>

For the purposes of this tutorial, we will use screen shots from the
Floater's textual user interface.  In real life, most people will use
the GUI instead.  Nonetheless, this should be useful to all.<p>

Here is what you see when you start up Floater:
}
include screen1

puts "
To get started, you either use [c login] or, if you are new,
[c newuser].  For demonstration purposes we use the latter;
[c login] is similar except that it queries you for your password
rather than for your email address.
"
include screen2

puts {Above, I have typed "newuser" and Floater has displayed what I
typed on "the command line."  After pressing return, I then enter my
chosen name and press return, and enter my full email address:}
# command newuser
include screen3

puts {After pressing return again:}
include screen4

puts {
Now I am "logged in."  Because in the future I must enter a password
to play as "Buzz," and only I will receive that password by email, no
one else can play as Buzz.  Also, now that I have logged in, Floater
will attribute my results to me.<p>}
puts "
I would like to play some bridge, so I use the [c tables]
command to what tables are being hosted by other players.  That is, on
the command line, I type <tt>tables</tt> followed by return or enter.
The result:
"
# command tables
include screen5

puts "
I see Jonathan has 3 players and seems to want someone to [c join], so I
go to his table by using the command"
command join Jonathan
puts ".  (If I'd wanted to, I could instead have used the
[c host] command to host a table of my own.)"
include screen6

puts "
I am now at Jonathan's table and I see Jonathan, Franco, and David
are seated.<p>

At this point, however, some of the information at the bottom of the
screen has scrolled away, and I'd like to go back to look at it.  In
the GUI, I could just use the scroll bar, but for the textual user
interface I need to use the arrow keys (or the [c scroll] command).
Just to illustrate, here is what I see after pressing the up arrow key:
"
include screen7

puts {
Notice that the text at the bottom of the screen has scrolled back one
line.  Also, on the "status bar"---the line just below "talk" and just
above the scrolling text region---notice that <tt>13-19/20</tt> indicates
I am now viewing lines 13 through 19 of the 20 lines that have
been presented to me thus far in the talk region.<p>

However, since we aren't playing bridge yet, I might as well switch to
the other view of the world that Floater's textual user interface
offers.  By typing Control-v I can toggle between a screen that
looks like the previous screens in the tutorial and a screen that
looks like:
}
include screen8

puts {
Now the scrolling text region that had just occupied a few lines at
the bottom of the screen is much larger---at the expense of not
showing the "newspaper bridge column" view of the action (also known
as "the matrix").  By the way, the arrow keys and the}; puts [c scroll]
puts {command also work here, but at the moment they are unnecessary because
everything that has been displayed thus far fits on the screen at once.
(Note that the Control-v feature is not implemented for the GUI.)<p>

Moving along: below we see that Jonathan and Franco have said hi, and
I'm about to ask "need 1?" by entering it on the "talk line."  To
switch between typing on the talk line and the command line, use the
tab key.
}
include screen9

puts "
The North seat is not spoken for, so I take it by using the
[c north] command.  In practice I typed <tt>/north</tt> on the
talk line, which is equivalent to typing <tt>north</tt> on the command
line.  (In general, anything on the talk line that starts with
<tt>/</tt> is interpreted as if it were typed on the command line.
Similarly, entering <tt>\"Hi there</tt> on the command line is
equivalent to entering <tt>Hi there</tt> on the talk line.)
"
# talk need 1?
# talk /north
include screen10

puts {I have taken the North seat, and some cursory discussion of
system ensues...}
# talk system, pard?
# talk ok
include screen11

puts "
At this point, I check on both our convention card and the opponents'.
Normally, the [c ccdump] command displays our convention card; by
specifying a direction, I can see their convention card (in this
case, <tt>ccdump EW</tt>).  Here is the result of entering the two
commands <tt>ccdump</tt> and <tt>ccdump EW</tt> in succession:
"
# command ccdump
# command ccdump EW
include screen12

puts "
Apparently we don't have a convention card set (though we have
verbally agreed to play SA); the opponents are playing 2/1 with
various gadgets.<p>

Now, the host of the table has decided to start play. (He issued the
[c deal] command.  Only the host may do so.)  I see:
"
include screen13

puts {
My cards and other relevant information are visible on the status bar,
but since we're playing now, I decide to use Control-v again to switch back to
the other view.
}
include screen14

puts {Now the auction begins...}
include screen15

puts {...and the auction continues...}
include screen16

puts {...and then RHO takes a bid...}
include screen17

puts {
OK, my turn to bid!  This is indicated on the status bar and by the
<tt>?</tt>'s in the obvious places.  Pass seems clear, so I enter <tt>p</tt>
on the command line.
}
# command p
include screen18

puts {
My pass appeared in the matrix and in the auction summary on the right,
just as expected.
}
include screen19

puts {Ooops! Partner typed <tt>p</tt> on the talk line when he wanted
to pass.  No harm done, but Floater didn't interpret it as a call, so
partner had to re-enter <tt>p</tt> on the command line (or <tt>/p</tt>
on the talk line).  Skipping ahead a bit...}
include screen20

puts "Here we see that Franco has alerted.  The fact that we see
<tt>Franco!</tt> rather than <tt>Franco:</tt> in front of <tt>Alert!
4th suit forcing</tt> indicates that Franco's partner did not see
anything.  <b>In Floater, one is expected to alert one's own bids!</b>
Several mechanisms are provided for doing.  One may either enter, for
example, <tt>3c!4th suit forcing</tt>, or one may use the
[c alert] command.  And for particularly unusual bidding or
carding agreements, one may use <tt>3c!!4th suit forcing</tt> or the
[c redalert] command.  Deciding what merits an alert and what
merits a <font color=\"#FF0000\">red alert</font> is completely up to you.<p>

(By the way, just because you are expected to alert your own bids
doesn't mean you shouldn't alert partner's bids as well.  Try to be helpful!)
<p>
OK.  Later in the auction, I see:
"
# command p
# command p
# command p
include screen21

puts {And after the auction is over:}
include screen22

puts "At this point I'm wondering what 4N and 5C meant in this
auction, so I use the [c opp] command to send a message that only
the opponents see.  I don't want partner to see what I'm asking,
because that is unauthorized information that he neither needs nor
wants to know.  I enter <tt>opp what is 5C?</tt> on the
command line."
# command opp what is 5C?
include screen23

puts {The main thing to see here is that an <tt>=</tt> after a
player's name rather than a <tt>:</tt> indicates that the message went
only to that player's opponents; a <tt>-</tt> indicates that the
message went only to me.<p>

Now, as it turns out, the hand is cold for 13 tricks, so declarer claims.
}
include screen24

puts "I use the [c accept] command."
# command accept
include screen26

puts {
Apparently partner has accepted the claim as well, and we now see both
our result and the results from other tables, if any.  In this case no
one else has played the hand yet, so we just see our result.  Later,
after the hands for 15Sep96 are no longer in play, the results that
came after ours as well as those that came before will be scored at
duplicate and posted to the Floater web page.<p>

The first hand is over.  I see my cards for the next deal here:
}
include screen27

puts {
That ends this brief Floater tutorial.  To learn more, look at the
rest of the <a href=index.html>Floater documentation</a>.<p>
Last modified }
include |date
puts {
.
<HR>        
<address>
<A HREF=http://www.cs.berkeley.edu/~pike>Geoff Pike</A>,<A HREF=mailto:floater@priam.cs.berkeley.edu>
&lt;pike@cs.berkeley.edu&gt;</A>, is the author of
<a href=../core.html>Floater</a> and maintainer of this WWW page.
</address>
}