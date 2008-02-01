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
# files.tcl

proc setprioruse {name} {
    global usedname startupfile

    if [info exists usedname($name)] return
    set "usedname($name)" 1
    set u "\"usedname("
    catch {exec cat << "set $u$name)\" 1\n" >> $startupfile}
}

############################################################################
# stuff to keep track of what hands we've seen
############################################################################

set seenname _everyone_

proc seenfile {root} {
    global seenfileroot seenname
    return $seenfileroot/$seenname/$root.tcl
}

#record (not on disk---just for now) that we've seen this hand
proc lseenhand {root number} {
    global seenhands maxseenhand

#    if ![info exists seenhands($root,$number)] \
#	    {puts stderr "seen $root $number"}
    set seenhands($root,$number) 1
    if [info exists maxseenhand($root)] \
	    {if {$maxseenhand($root) > $number} return}
    set maxseenhand($root) $number
}

# record once and forever that we've seen this hand
proc seenhand {root number} {
    global seenhands

    if [info exists seenhands($root,$number)] return
    lseenhand $root $number
    if [catch {exec cat << "lseenhand $root $number\n" >> \
	    [seenfile $root]}] {
	floatererror "Floater error: unable to make permanent note of what hands you've seen!"
	set e1 "Floater error: unable to write to file "
	set e2 [seenfile $root]
	floatererror "$e1$e2"
    }
}

# what hands have we seen?
proc seen {root {silent 0}} {
    global seenhands maxseenhand youveseen

    if ![info exists maxseenhand($root)] {
	if {$youveseen && !$silent} \
		{talkmsg "You haven't seen any hands from $root"}
	return ""
    }
    set m $maxseenhand($root)
    set s ""
    set t ""
    for {set i 1} {$i <= $m} {incr i} {
	if [info exists seenhands($root,$i)] {
	    append s "$i-"
	    append t "$i "
	}
    }
    if {$youveseen && !$silent} {talkmsg "From set $root, you've seen: $t"}
    if {$silent} {return $t} {return $s}
}

proc loadseen {} {
    global seenhands maxseenhand seenname seenfileroot

    debugmsg "loadseen with seenname=$seenname"

    catch {unset seenhands; unset maxseenhand}
    if {[file exists $seenfileroot/_everyone_] == 0} \
	    {catch {floater_mkdir $seenfileroot/_everyone_}}
    if {[file exists $seenfileroot/$seenname] == 0} \
	    {catch {floater_mkdir $seenfileroot/$seenname}}
    source_all_tclfiles $seenfileroot/_everyone_

    # for backward compatibility with a1
    source_all_tclfiles $seenfileroot 0

    if {$seenname != "_everyone_"} \
	    {source_all_tclfiles $seenfileroot/$seenname}
}

#############################################################################
# Check if a file is in our part of the file system
#############################################################################

# The file must lie in floaterdir or a subdirectory
proc validfile {filename} {
    global floaterdir

    regexp $floaterdir/.* [file dirname $filename]/
}
