# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Floater Bridge Network.
#
# The Initial Developer of the Original Code is
# Geoff Pike <pike@EECS.Berkeley.EDU>.
# Portions created by the Initial Developer are Copyright (C) 1996-2003
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the
# terms of either the GNU General Public License Version 2 or later
# (the "GPL"), in which case the provisions of the GPL are applicable
# instead of those above. If you wish to allow use of your version of
# this file only under the terms of the GPL, and not to allow others
# to use your version of this file under the terms of the MPL,
# indicate your decision by deleting the provisions above and replace
# them with the notice and other provisions required by the GPL. If
# you do not delete the provisions above, a recipient may use your
# version of this file under the terms of either the MPL or the GPL.
# ***** END LICENSE BLOCK *****
# pm.tcl, written 1/97 by Geoff Pike
# This is the pseudo-mailer.  It allows mail to be sent from clients
# that do not have a mail(1) program.
 
# If first line from this conn, first thing in s is address to send
# to; rest is first line of message.  Otherwise, s is just the next line
# of the message.
proc remail {s conn} {
    global partialmail destination

    if [info exists partialmail($conn)] {
	set partialmail($conn) "$partialmail($conn)\n$s"
    } else {
	regexp {([^ ]+) (.*)} $s all dest msg
	set destination($conn) $dest
	set partialmail($conn) $msg
    }
}

# Use the address and text collected by remail, above, to actually
# send a mail message.  Return 1 and set errorstring on failure.
proc remailnow {conn} {
    global destination partialmail bugmail \
	    resultparser resultparserprogram errorstring
    
    set r [catch {
	if {![string compare $resultparser $destination($conn)]} {
	    puts "result: $partialmail($conn)\n\n"
	    exec echo $partialmail($conn) | $resultparserprogram
	} elseif {![string compare $bugmail $destination($conn)]} {
	    puts "to: $destination($conn)\n$partialmail($conn)\n\n"
	    exec echo $partialmail($conn) | mail $destination($conn)
	} else {
	    error "Attempt to mail to $destination($conn)"
	}
    } errorstring]

    catch {unset destination($conn)}
    catch {unset partialmail($conn)}
    return $r
}
    

