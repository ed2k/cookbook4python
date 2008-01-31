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
#   Lex Spoon <lex@cc.gatech.edu>
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
proc clearrect {x y} {}
proc anchor {l} {}
proc down_and_anchor {{l 1}} {}
proc right {{l 1}} {}
proc str {l} {}
proc ch {l} {}

#############################################################################

proc gotresult {setname handname} {
    global sets
    set sets($setname) 1
    global hands_$setname
    set hands_$setname\($handname) 1
}

gset months {Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec}

proc parseset {setname} {
    global setdate setday setmonth setyear months setscoremethod

    regexp {([0-9]+)([a-zA-Z]+)([0-9]+)(.*)} $setname all setday setmonth setyear setscoring
    set setdate "$setday$setmonth$setyear"
    set setmonth [lsearch $months $setmonth]
    if {$setscoring == "IMP"} {set setscoremethod 0} {set setscoremethod 1}
}

# convert two-digit year into "real" year
proc yearadjust {y} {
    if {$y < 96} {expr $y + 2000} {expr $y + 1900}
}

proc setsort {a b} {
    global months

    regexp {([0-9]+)([a-zA-Z]+)([0-9]+)(.*)} $a all aday amonth ayear ascore
    regexp {([0-9]+)([a-zA-Z]+)([0-9]+)(.*)} $b all bday bmonth byear bscore
    if {$ayear != $byear} {
	    return [expr [yearadjust $ayear] - [yearadjust $byear]]}
    set amonth [lsearch $months $amonth]
    set bmonth [lsearch $months $bmonth]
    if {$amonth != $bmonth} {return [expr $amonth - $bmonth]}
    if {$aday != $bday} {return [expr $aday - $bday]}
    string compare $ascore $bscore
}
    
gset sets_sorted ""
proc firstset {} {
    global setiter sets sets_sorted

    if {$sets_sorted == ""} {
	set sets_sorted [
            lsort -command setsort -decreasing [array names sets]]
    }
    set setiter $sets_sorted
    nextset
}

proc nextset {} {
    global setiter

    if {$setiter == ""} return ""
    set r [lindex $setiter 0]
    set setiter [lrange $setiter 1 end]
    return $r
}

proc gotnames {e w n s} {
    global names

    set names($n) 1
    set names($s) 1
    set names($e) 1
    set names($w) 1
}
    
proc nsort_ {a b} {
    string compare [string toupper $a] [string toupper $b]
}

proc nsort {l} {
    lsort -command nsort_ $l
}

gset names_sorted ""
proc firstname {} {
    global nameiter names_sorted names

    if {$names_sorted == ""} {set names_sorted [nsort [array names names]]}
    set nameiter $names_sorted
    nextname
}

proc nextname {} {
    global nameiter

    if {$nameiter == ""} return ""
    set r [lindex $nameiter 0]
    set nameiter [lrange $nameiter 1 end]
    return $r
}

proc firstsubname {first last} {
    global nameiter names_sorted names subdone namesublast

    if {$names_sorted == ""} {set names_sorted [nsort [array names names]]}
    set nameiter $names_sorted
    while {"[nextname]" != "$first"} {}
    set subdone 0
    set namesublast $last
    return $first
}

proc nextsubname {} {
    global nameiter subdone namesublast

    if $subdone {return ""}
    set r [lindex $nameiter 0]
    set nameiter [lrange $nameiter 1 end]
    if {$r == $namesublast} {set subdone 1}
    return $r
}

proc firsthand {s} {
    global handiter hands_$s

    set handiter [array names hands_$s]
    nexthand
}

proc nexthand {} {
    global handiter

    if {$handiter == ""} return ""
    set r [lindex $handiter 0]
    set handiter [lrange $handiter 1 end]
    return $r
}
###########################################################################
proc maxcol {} {
    upvar COL col
    global maxrow

    set maxcol 0
    set maxrow 0
    foreach z [array names col] {
	if {[lindex [set s [split $z ,]] 1] > $maxrow} {
		set maxrow [lindex $s 1]}
	if {[lindex $s 0] > $maxcol} {
		set maxcol [lindex $s 0]}
    }
    return $maxcol
}

proc colget {i j} {
    upvar COL col

    if [info exists col($i,$j)] {
	set r $col($i,$j)
	unset col($i,$j)
	return $r
    } else {
	return " "
    }
}

