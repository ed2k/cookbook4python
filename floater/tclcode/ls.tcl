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
#     Lex Spoon <lex@cc.gatech.edu>
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

# ls.tcl -- Floater's login server, responsible for logging in users
#           and keeping track of the open tables


set lsfile $server_dir/serverfiles/loginserver.tcl
set writefile $lsfile\_wtest
set lsdir [file dirname $lsfile]
set rsdir $server_dir/floater_rs

set webdir $server_dir/www
set rawresultsdir $webdir/rawresults
set permdir $lsdir/perm
set tmpdir $lsdir/tmp

# set these intervals to 5 minutes
set check_tempdir_interval [expr 1000 * 60 * 5]  
set flush_interval [expr 1000 * 60 * 5]     

exec sh -c "cd $lsdir && make all || (echo make all failed | mail $bugmail)"

proc dumppw {} {
    global lsdir bugmail
    catch {exec sh -c "cd $lsdir && make dump || (echo dumppw failed | mail $bugmail)" &}
}

proc copyresultfiles {week} {
    global rsdir rawresultsdir
    catch {
	# puts stderr "copyresultfiles $week"
	set files [glob $rsdir/$week*]
	puts stderr "cp $files $rawresultsdir"
	eval exec cp $files $rawresultsdir
    }
}

# keep everything before the first open paren
proc parenstrip {s} {
    if {[set i [string first \( $s]] == -1} {return $s}
    string range $s 0 [incr i -1]
}

proc tryget {a} {
    global [parenstrip $a]

    if [info exists $a] {return [set $a]} {return ""}
}

#Try to find an element of arr whose index is v, ignoring case differences
proc tryget_nocase {arr v} {
    global $arr

    set v [string toupper $v]
    foreach i [array names $arr] {
	if ![string compare $v [string toupper $i]] {
	    return [tryget $arr\($i\)]
	}
    }
    return ""
}

proc s {a b} {global [parenstrip $a]; set $a $b}
if [file exists $lsfile] {source $lsfile}
global pw pwx
foreach i [array names pw] {
    set "pwx([string toupper $i])" $i
}

proc sendmail {to subject {replyto "floater@floater.org"}} {
    set f [open "| /usr/lib/sendmail $to" w]
    puts $f "From: Floater <$replyto>"
    puts $f "To: $to"
    puts $f "Reply-To: $replyto"
    puts $f "X-URL: http://www.floater.org/"
    puts $f "Subject: $subject"
    puts $f ""
    return $f
}

proc welcomeemail {name addr password} {
    set e [sendmail $addr "Welcome to Floater!"]
    puts $e "Thanks for playing Floater!"
    puts $e ""
    puts $e "Your account name is: $name"
    puts $e "Your initial account password is: $password"
    puts $e ""
    puts $e "(Be sure to enter upper case letters as upper case, and lower"
    puts $e "case letters as lower case.)"
    puts $e ""
    puts $e "You may change your Floater password at any time by using"
    puts $e "the Change Pasword command of the File menu.  We recommend"

    puts $e "using a password that is different from passwords that you"
    puts $e "may be using for other purposes."
    puts $e ""
    puts $e "We hope you enjoy playing the game!"
    puts $e ""
    puts $e "Best Wishes,"
    puts $e ""
    puts $e "The Floater Team"
    puts $e ""
    puts $e "P.S. This is an automatically generated form letter.  However,"
    puts $e "     if you send us email real people do read it and respond!"
    close $e
}

proc permdo {script} {
    global writefile

    eval $script
    exec cat << "$script\n" >> $writefile
}

proc permset {a b} {
    global [parenstrip $a] writefile

    puts stdout "permset $a $b"
    set $a $b
    exec cat << "set \{$a\} \{$b\}\n" >> $writefile
}

proc newaccount {name addr password} {
    global email pw pwx

    permset "email($name)" $addr
    permset "pw($name)" $password
    set pwx([string toupper $name]) $name
    catch {welcomeemail $name $addr $password}
}

proc password_fetch {name} {
    global pw pwx
    if [info exists pw($name)] {return $pw($name)}
    return ""
#    set upname [string toupper $name]
#    if {"$upname" != "$name"} {
#	if [info exists pwx($upname)] {
#	    set r [password_fetch $pwx($upname)]
#	    if {$r != ""} {return $r}
#	}
#    }
#    tryget_nocase pw $name
}

proc name_fetch {name} {
    global pw pwx
    if [info exists pw($name)] {return $name}
    set upname [string toupper $name]
    if {"$upname" != "$name"} {
	if [info exists pwx($upname)] {
	    return $pwx($upname)
	}
    }
    foreach i [array names pw] {
	if ![string compare $upname [string toupper $i]] {return $i}
    }
    
    # shouldn't get here
    return $name
}

#############################################################################

proc safeglob {pattern} {
    if [catch {set s [glob $pattern]}] {return ""} {return $s}
}

proc seen_startup {date1 date2} {
    global permdir tmpdir flush_interval

    #remove files that aren't *$date1* or *$date2* or directories
    set junkdir $permdir/junk
    foreach f [safeglob $permdir/*] {
	if {![string match *$date1* $f] && ![string match *$date2* $f] &&
		    ![file isdirectory $f]} {
	    if ![file exists $junkdir] {catch {floater_mkdir $junkdir}}
	    catch {exec mv $f $junkdir}
	}
    }

    #load all remaining files from permdir (they'll match date1 or date2)
    foreach f [safeglob $permdir/*] {
	if ![file isdirectory $f] {
	    uplevel #0 "source $f"
	    talkmsg "Loaded $f"
	}
    }

    check_tempdir
    after [expr $flush_interval / 2] flush_permseenfiles
}

proc check_tempdir {} {
    global check_tempdir_interval tmpdir

    after $check_tempdir_interval check_tempdir

    # 10 seconds should allow files being written to be closed
    after 10000 "read_tempfiles \{[safeglob $tmpdir/*]\}"
}

gset checknewweek 1

set to_be_deleted ""

proc read_tempfiles {files} {
    global to_be_deleted

    foreach file $files {
	talkmsg "reading $file"
	catch {uplevel #0 "source $file"}
    }
    if {$files != ""} {set to_be_deleted "$to_be_deleted $files"}
}

#Make permanent note of the fact that $name has seen $num from $set.
proc permanent_seen {name set num} {
    global permseenfiles permdir

    a_h_s $name $set $num

    #if not logged in, return
    if [regexp \\* $name] return

    if ![info exists permseenfiles($set)] {
	set permseenfiles($set) [open $permdir/$set a]
    }
    puts $permseenfiles($set) "a_h_s \{$name\} $set $num"
}

proc flush_permseenfiles {} {
    global permseenfiles flush_interval to_be_deleted tmpdir

    puts "flushing permseenfiles"
    foreach f [array names permseenfiles] {catch {flush $permseenfiles($f)}}
    if {$to_be_deleted != ""} {
	puts "Deleting files from $tmpdir"
	catch {eval "file delete $to_be_deleted"}
	set to_be_deleted ""
    }
    after $flush_interval flush_permseenfiles
}

#############################################################################

proc remove_stale_locations {} {
    global stale_locations_seconds lastlocation lastlocation_timestamp \
	    floaterclock

    after [expr 1000 * $stale_locations_seconds] remove_stale_locations

    foreach i [array names lastlocation_timestamp] {
	if {[expr $floaterclock - $lastlocation_timestamp($i)] >
					        $stale_locations_seconds} \
		{
		    unset lastlocation_timestamp($i)
		    unset lastlocation($i)
		}
    }
}

tryset stale_locations_seconds 500
remove_stale_locations

proc bgerror {m} {talkmsg $m}

#############################################################################

gset testConnTime 15

# Return whether a test connection to the given IPaddr:port succeeds.
# Don't allow any test to take longer than testConnTime seconds.
proc testConn {IPaddr port} {
    # if IPaddr is foo!bar then recursively test foo and bar
    if {[string first ! $IPaddr] >= 0} {
	foreach part [split $IPaddr !] {
	    if {[testConn $part $port] == "y"} {return "y"}
	}
	return "n"
    }

    # max time per addr/port pair (in ms)
    global testConnTime default_handshake
    set timeout [expr $testConnTime * 10000]
    # frequency to check if socket creation failed (in ms)
    set timestep 500
    # min delay between reading lines from telnet
    set linetimestep 10

    if [catch {set f [open "|telnet $IPaddr $port"]} err] {
	set q 1
    } else {
	set q [catch {
	    fconfigure $f -blocking 0
	    set happy 0
	    for {set i 0} {!$happy && $i < $timeout} {incr i $timestep} {
		set s [gets $f]
		puts "testConn $IPaddr: $s"
		if {$s == "" && [fblocked $f]} {
		    after $timestep
		} else {
		    if [regexp $default_handshake $s] {
			set happy 1
		    } elseif {[eof $f]} {
			return "eof"
		    } else {
			after $linetimestep
		    }
		}
	    }
	    if !$happy { return "timeout" }
	} err]
    }
    catch { close $f }
    if $q {
	puts "testing conn $IPaddr $port: failed ($err)"
	return "n"
    } else {
	puts "testing conn $IPaddr $port: succeeded"
	return "y"
    }
}

# timeout in seconds
gset testConnInBackgroundTime 120

# The new, preferred alternative to testConn (above).
# Works with Floater 1.3b1 on.
proc testConnInBackground {IPaddr port} {
    # if IPaddr is foo!bar then recursively test foo and bar
    if {[string first ! $IPaddr] >= 0} {
	foreach part [split $IPaddr !] {
	    testConnInBackground $part $port
	}
	return
    }
    
    global you_may_host testConnInBackgroundTime 
    set maxseconds $testConnInBackgroundTime
    if [catch {
	set s [socket -async $IPaddr $port] 
	fconfigure $s -blocking 0
	puts $s $you_may_host
	flush $s
	after $maxseconds "catch { close $s }"
    } x] {
	puts "Error in testConnInBackground: $x"
    }
}
    
#############################################################################


tryset tables ""

proc notetables {s time} {
    global tables

    puts stderr "notetables"
    if {$tables == $s} return
    set tables $s
    construct_activity_webpage $time
}

gset minlogin 1
gset maxlogin 0

proc notelogin {name time} {
    global logins maxlogin logins_timestamp floaterclock

    incr maxlogin
    set logins($maxlogin) "$time: <b>$name</b> </br>"
    set logins_timestamp($maxlogin) $floaterclock
    construct_activity_webpage $time
}

tryset login_trim_seconds 3600

proc trim_logins {} {
    global logins maxlogin minlogin logins_timestamp floaterclock \
	    login_trim_seconds

    # puts stderr trim_logins
    while {$minlogin <= $maxlogin} {
	if {[expr $logins_timestamp($minlogin) + $login_trim_seconds]
		< $floaterclock} {
	    # puts stderr "trimming $logins($minlogin)"
	    unset logins_timestamp($minlogin)
	    unset logins($minlogin)
	    incr minlogin
	} else return
    }
}

gset last_construction_of_activity_webpage -9999

# time is the textual time stamp to put on the page.  retry is whether
# to try again later if it turns out that right now is too soon to
# create a new version.
proc construct_activity_webpage {time {retry 1}} {
    global logins maxlogin minlogin logins_timestamp tables floaterclock \
	    last_construction_of_activity_webpage

    # minimum number of seconds between updates
    set waittime 60

    puts stderr "construct_activity_webpage $time"
    if {[expr $floaterclock - $last_construction_of_activity_webpage]
            < $waittime} {
	# puts stderr "too soon"
        if $retry {
	    after [expr 1000 * ($waittime + 5 - \
		($floaterclock - $last_construction_of_activity_webpage))] \
		"construct_activity_webpage \{$time\} 0"
	}
	return
    }

    trim_logins
    set recent_logins ""
    for {set i $minlogin} {$i <= $maxlogin} {incr i} {
	set recent_logins $logins($i)$recent_logins
    }	
    
    update_activity_webpage $recent_logins $tables $time
}



tryset saved_recent_logins ""
tryset saved_tables ""

set activity_webpage_filename $webdir/activity.html

proc update_activity_webpage {recent_logins tables creation_time} {
    global saved_recent_logins saved_tables activity_webpage_filename \
	    last_construction_of_activity_webpage floaterclock

    set last_construction_of_activity_webpage $floaterclock
     puts stderr update_activity_webpage

    if {$saved_recent_logins == $recent_logins && $saved_tables == $tables} {
	puts stderr "update_activity_webpage: no change, aborting"
	    return
    }

    set saved_recent_logins $recent_logins
    set saved_tables $tables

    set page "<html><head><TITLE>Floater Activity</TITLE></head>
<body bgcolor=\"#ffffff\" text=\"#000000\" link=\"#CC0000\" alink=\"#FF3300\" vlink=\"#0000CC\">

<h2>Floater Activity: $creation_time</h2>

Tables:

$tables
<p>
Recent Logins:
<p>
$recent_logins
<p>
This page is automatically updated to reflect recent Floater activity.
All times are local to Atlanta, Georgia, USA.
This version was created at $creation_time.  New versions are created
whenever there is new information, but at most once per minute.  You may
have to reload to get the latest info.
<p>

<hr>
<A HREF=http://www.floater.org>Floater</A>
"

     puts stderr "creating $activity_webpage_filename"
    if [catch {set f [open $activity_webpage_filename w]}] return

    puts $f $page
    close $f
}

