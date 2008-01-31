# 1 "/local/scratch/floater-1.4.15/tclcode/floatert.deq"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "/local/scratch/floater-1.4.15/tclcode/floatert.deq"
# 35 "/local/scratch/floater-1.4.15/tclcode/floatert.deq"
# 1 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 1
# 57 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
if {[info tclversion] < 8.0} {
    puts stderr "You have compiled Floater with Tcl [info tclversion]"
    puts stderr "You must recompile with Tcl 8.0 or higher"
    exit 1
}
# 70 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
set debugstartup 0

if $debugstartup {


    proc line args {




        puts "Line: $args"

    }
} else {
    proc line args {}
}
# 145 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
# 1 "/local/scratch/floater-1.4.15/tclcode/gset.deq" 1
# 35 "/local/scratch/floater-1.4.15/tclcode/gset.deq"
proc tryset {a b} {
    if {[set x [string first "(" $a]] == -1} {
	global $a
    } else {
	global [string range $a 0 [incr x -1]]
    }

    if [catch {set $a}] {set $a $b}
}


proc gset {a {b salami_on_rye}} {
    if {[set x [string first "(" $a]] == -1} {
	global $a
    } else {
	global [string range $a 0 [incr x -1]]
    }

    if ![string compare $b salami_on_rye] {set $a} {set $a $b}
}


proc gunset {a} {
    if {[set x [string first "(" $a]] == -1} {
	global $a
    } else {
	global [string range $a 0 [incr x -1]]
    }

    unset $a
}
# 146 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after gset




gset floater_version {Floater 1.4.15}

# 1 "/local/scratch/floater-1.4.15/tclcode/errorhandle.deq" 1
# 34 "/local/scratch/floater-1.4.15/tclcode/errorhandle.deq"
proc bgerror {m} {catch {debugmsg $m}}
# 154 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after errorhandle
# 1 "/local/scratch/floater-1.4.15/tclcode/files.deq" 1
# 38 "/local/scratch/floater-1.4.15/tclcode/files.deq"
proc lseenhand {root number scoring} {
    global seenhands maxseenhand 	Nseen$scoring Sseen$scoring Eseen$scoring Wseen$scoring





    set seen seen
    foreach d {N S E W} {
	if [info exists $d$seen$scoring] {
	eval "set old $$d$seen$scoring"
	set $d$seen$scoring $old$number-

	}
    }
    set seenhands($root,$number) 1
    if [info exists maxseenhand($root)] 	{if {$maxseenhand($root) > $number} return}

    set maxseenhand($root) $number
}


proc seen {root {silent 0}} {
    global seenhands maxseenhand youveseen

    if ![info exists maxseenhand($root)] {
	if {$youveseen && !$silent} 		{talkmsg "You haven't seen any hands from $root"}

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
# 91 "/local/scratch/floater-1.4.15/tclcode/files.deq"
set slash "/"


proc floater_mkdir {s} {
    file mkdir $s
}

proc floater_delete {s} {
    file delete $s
}

proc append_to_file {fileName line} {
    set file [open $fileName a]
    puts $file $line
    close $file
}

if [info exists env(FLOATER_DIR)] {
    tryset floaterdir $env(FLOATER_DIR)
}

tryset floaterdir [set env(HOME)]${slash}.floater

tryset startupfile $floaterdir${slash}startup.tcl
tryset seenfileroot $floaterdir${slash}seenhands

if {[file exists $floaterdir] == 0} {catch {floater_mkdir $floaterdir}}
if {[file exists $seenfileroot] == 0} {catch {floater_mkdir $seenfileroot}}



proc tclfiles {dir {recurse 1}} {
    global slash

    set file_bases [glob -nocomplain -path "$dir$slash" *.tcl]

    set files ""
    foreach base $file_bases {
        lappend files [file join $dir $base]
    }

    return $files
}

proc source_all_tclfiles {dir {recurse 1}} {
    global startupfile

    foreach file [tclfiles $dir $recurse] {
	if {$file != $startupfile} {
	if [catch {source $file} err] {
		puts stderr $err
	}
	}
    }
}



proc setprioruse {name} {
    global usedname startupfile

    if [info exists usedname($name)] return
    set "usedname($name)" 1
    catch {
        append_to_file $startupfile \"set usedname($name)\" 1"
    }
}





set seenname _everyone_

proc seenfile {root} {
    global seenfileroot seenname slash
    return $seenfileroot$slash$seenname$slash$root.tcl
}


proc seenhand {root number scoring} {
    global seenhands


    if [info exists seenhands($root,$number)] return
    lseenhand $root $number $scoring

    if [catch {append_to_file [seenfile $root] "lseenhand $root $number $scoring"}] {
	floatererror "Floater error: unable to make permanent note of what hands you've seen!"
	set e1 "Floater error: unable to write to file "
	set e2 [seenfile $root]
	floatererror "$e1$e2"
    }
}

proc loadseen {} {
    global seenhands maxseenhand seenname seenfileroot 	globaldate previousglobaldate slash


    debugmsg "loadseen with seenname=$seenname"


    if {[info exists globaldate] && [info exists previousglobaldate]} {
	cleanseen $seenfileroot${slash}$seenname $globaldate $previousglobaldate
	if {$seenname != "_everyone_"} {
	cleanseen $seenfileroot${slash}_everyone_ $globaldate $previousglobaldate
	}
    }

    catch {unset seenhands; unset maxseenhand}
    if {[file exists $seenfileroot${slash}_everyone_] == 0} 	{catch {floater_mkdir $seenfileroot${slash}_everyone_}}

    if {[file exists $seenfileroot${slash}$seenname] == 0} 	{catch {floater_mkdir $seenfileroot${slash}$seenname}}

    source_all_tclfiles $seenfileroot${slash}_everyone_


    source_all_tclfiles $seenfileroot 0

    if {$seenname != "_everyone_"} 	{source_all_tclfiles $seenfileroot${slash}$seenname}

}

proc cleanseen {dir except1 except2} {
    global slash
    foreach file [tclfiles $dir 0] {
	if {![string match "*$except1*" $file]
	&& ![string match "*$except2*" $file]} {
		floater_delete $file
	}
    }
}
# 231 "/local/scratch/floater-1.4.15/tclcode/files.deq"
proc validfile {filename} {
    global floaterdir slash

    regexp $floaterdir${slash}.* [file dirname $filename]${slash}
}





package require http

proc nop args {}

proc http_done {token} {
    global token_to_file token_to_cmd
    upvar #0 $token state
    catch { close $token_to_file($token) }
    catch { unset $token_to_file($token) }
    if {$state(status) == "ok"} {
	catch { eval $token_to_cmd($token) }
    }
    catch { unset $token_to_cmd($token) }
}




proc http_fetch_to_file {url finalname {tmpfile ""}} {
    global token_to_file token_to_cmd

    if {$tmpfile == ""} {
	set filename $finalname
    } else {
	set filename $tmpfile
    }
    set f [open $filename w]
    set token [::http::geturl $url -channel $f -command http_done]
    set token_to_filename($token) $f
    if {$tmpfile != ""} {
	set token_to_cmd($token) "file rename \{$tmpfile\} \{$finalname\}"
    }
}
# 156 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after files
# 1 "/local/scratch/floater-1.4.15/tclcode/connect.deq" 1
# 34 "/local/scratch/floater-1.4.15/tclcode/connect.deq"
gset conn_number 0

gset default_handshake "Floater 'shake"
gset you_may_host "you may host"
gset silent_handshake "Floater silent 'shake"
gset test_connection_succeeded 0


proc FloaterListen {{port 0}} {
    global localIPaddr0
    PortNumber [socket -server FloaterAcceptConnection $port]
}

proc FloaterAcceptConnection {sock addr port} {
    debugmsg "AcceptConnection $sock $addr $port"
    return [FloaterNewSocket $sock]
}

proc FloaterReadable {conn sock} {
    global expecting_handshake floater_silent default_handshake you_may_host

    debugmsg "FloaterReadable $conn $sock"
    set s [gets $sock]
    debugmsg "Got $s from $conn"
    if [info exists expecting_handshake($conn)] {
	debugmsg "expecting handshake"
	if {$s == $default_handshake} {
	
	unset expecting_handshake($conn)
	return
	} elseif {$s == $you_may_host} {
	
	unset expecting_handshake($conn)
	gset test_connection_succeeded 1
	return
	} else {
	if $floater_silent {
		global silent_handshake
		if {$s == $silent_handshake} {
		global floater_silent_conns
		set floater_silent_conns($conn) 1
		unset expecting_handshake($conn)
		return
		}
	}
	debugmsg "Expecting handshake but got $s"
	}
	
	FloaterClose $conn
	return
    }

    if {$s == "" && [eof $sock]} 	{FloaterClose $conn} 	{debugmsg "received $s"; floaterreceive $s $conn}


}


proc FloaterWritable {conn sock} {
    debugmsg "FloaterWritable $conn $sock"
}


proc FloaterConnect {addr port {handshake default}} {
    debugmsg "FloaterConnect $addr $port"
    FloaterNewSocket [socket $addr $port] $handshake
}



proc FloaterNewSocket {sock {handshake default}} {
    global sock_to_conn conn_to_sock conn_number expecting_handshake

    if {$handshake == "default"} {
	global default_handshake
	set handshake $default_handshake
    }
    debugmsg "NewSocket $sock $handshake"
    fconfigure $sock -blocking 0 -buffering line
    set conn [incr conn_number]
    set sock_to_conn($sock) $conn
    set conn_to_sock($conn) $sock
    set expecting_handshake($conn) 1

    fileevent $sock readable "FloaterReadable $conn $sock"
    if {$handshake != ""} {
	puts $sock $handshake
	debugmsg "sent handshake ($handshake) to $conn"
    }
    return $conn
}

proc PortNumber {sock} {
    lindex [fconfigure $sock -sockname] 2
}




tryset failedsendwait 3000



proc FloaterSend {to msg} {
    global conn_to_sock

    catch {set s $conn_to_sock($to)}






    debugmsg "Send $to ($s) $msg"

    if [catch {puts $s $msg}] {
	global failedsendwait

	after $failedsendwait 	debugmsg \"Closing $s due to failed send\"; 	catch \{close $s\}


    }
}



proc FloaterCloseName {name} {
    global name_to_conn

    set s "<none>"
    catch {set s $name_to_conn($name)}
    debugmsg "FloaterCloseName $name ($s)"
    if {$s != "<none>"} {
	catch {
	FloaterClose $s
	unset "name_to_conn($name)"
	}
    }
}
# 224 "/local/scratch/floater-1.4.15/tclcode/connect.deq"
proc bogusIP {s} {
    if {$s == "localhost"} { return 1 }
    if {$s == "localhost.localdomain"} { return 1 }
    if {$s == "127.0.0.1"} { return 1 }
    if {$s == "0.0.0.0"} { return 1 }
    if {$s == "255.255.255.255"} { return 1 }
    return 0
}

proc filter_and_join {s filter joiner} {
    set result ""
    foreach k $s {
	if ![$filter $k] { lappend result $k }
    }
    join $result $joiner
}

proc nothing {sock ipaddr port} {}


set localIPaddr 127.0.0.1

catch {
    set server [socket -server nothing 0]
    set socket [socket [info hostname] [PortNumber $server]]
    set localIPaddr0 [lindex [fconfigure $socket -peername] 0]
    set localIPaddr1 [lindex [fconfigure $socket -peername] 1]
    catch {close $socket}
    catch {close $server}
    set localIPaddr 	[filter_and_join "$localIPaddr0 $localIPaddr1" bogusIP !]


    if {$localIPaddr == ""} {
        set localIPaddr 127.0.0.1
    }

}
# 158 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after connect
# 1 "/local/scratch/floater-1.4.15/tclcode/mail.deq" 1
# 43 "/local/scratch/floater-1.4.15/tclcode/mail.deq"
set to_be_emailed_n 0


proc emailresult {result} {
    global resultparser errorstring to_be_emailed to_be_emailed_n

    if {[set q [what_to_send]] != ""} {set result "$result\nMagic cookie!$q"}
    if {$result == ""} {return 0}
    set r [

    pseudomail $result $resultparser



    ]
    if $r {
	
	set to_be_emailed([incr to_be_emailed_n]) $result
    } else {
	while {$to_be_emailed_n > 0} {
	
	
	
	
	
	set result $to_be_emailed($to_be_emailed_n)
	unset to_be_emailed($to_be_emailed_n)
	incr to_be_emailed_n -1
	if [emailresult $result] {return $r}
	}
    }
    return $r
}

proc emailseens {} {emailresult {}}


proc mail_bug {bug} {
    global bugmail errorstring


    pseudomail $bug $bugmail



}


proc pseudomail {what where} {
    global errorstring pseudomailaddr pseudomailport

    catch {
	set conn [FloaterConnect $pseudomailaddr $pseudomailport]
	FloaterSend $conn ozzie_and_harriet
	FloaterSend $conn "$where $what"
	FloaterClose $conn
    } errorstring
}
# 160 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after mail
# 1 "/local/scratch/floater-1.4.15/tclcode/seen.deq" 1
# 34 "/local/scratch/floater-1.4.15/tclcode/seen.deq"
proc query_have_seen {name set} {
    set x "$name $set"
    global $x
    array names $x
}


proc a_h_s {name set num} {
    global "have_seen_sets_$name"
    eval "set \"have_seen_sets_$name\($set)\" 1"

    set x "$name $set"
    global $x
    eval "set \{$x\($num)\} 1"
}

proc have_seen_sets {name} {
    global "have_seen_sets_$name"
    array names "have_seen_sets_$name"
}

proc discard_data_except_from {date} {
    global nameset

    foreach name $nameset {
	global "have_seen_sets_$name"
	foreach set [have_seen_sets $name] {
	if ![string match *$date* $set] {
		set x "$name $set"
		global $x
		unset $x

		eval "unset \"have_seen_sets_$name\($set)\""

	}
	}
    }
}



set to_be_sent_n 0

proc want_to_send {name set num} {
    global to_be_sent_n to_be_sent


    set to_be_sent([incr to_be_sent_n]) $name
    set to_be_sent([incr to_be_sent_n]) $set
    set to_be_sent([incr to_be_sent_n]) $num
}



proc what_to_send {} {
    global to_be_sent_n to_be_sent

    if {$to_be_sent_n == 0} {return ""}
    set s $to_be_sent(1)
    for {set i 2} {$i <= $to_be_sent_n} {incr i} {
	set s "$s	$to_be_sent($i)"
	unset to_be_sent($i)
    }
    set to_be_sent_n 0
    return $s
}
# 162 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after seen
# 1 "/local/scratch/floater-1.4.15/tclcode/logo.deq" 1
# 37 "/local/scratch/floater-1.4.15/tclcode/logo.deq"
global floater_version
tryset about_text "version [lrange $floater_version 1 end]

http:\/\/www.floater.org
This is free software."



proc about {{timeout 0}} {
    global about_text
    talkmsg "About Floater:\n$about_text"
}
# 164 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after logo
# 1 "/local/scratch/floater-1.4.15/tclcode/texts.deq" 1
# 34 "/local/scratch/floater-1.4.15/tclcode/texts.deq"
# 1 "/local/scratch/floater-1.4.15/tclcode/the_texts.deq" 1
# 38 "/local/scratch/floater-1.4.15/tclcode/the_texts.deq"
gset Confusing {
Floater relies on self-alerts. If playing formally, you are
expected to explain your bidding and carding agreements as carefully
as you would at a tournament. In any case, you alert by using the
commands `alert,' `redalert' and `explain.' You should alert
at the same time or before you take an alertable action. There are no
fixed rules as to what agreements are alertable---use your judgment
and do unto others as you would have them do unto you.

The easy way to alert an action is to type "!" (or "!!" for redalert)
on the same line as the command to perform that action---and
optionally put an explanation at the end. For example, you could type
"2D!weak two in either major" to bid and alert your own bid
simultaneously. See also the help on `alert,' `redalert' and
`explain.'

If you are using the graphical user interface, you may click on the
"Alert" or "Redalert" checkboxes BEFORE clicking on the call you wish
to make. For example, clicking on "1C" when the "Alert" checkbox is
highlighted will have the same effect as typing "1C!" on the keyboard.

Everyone at the table except partner sees your alerts.
}

gset Copyright {
Floater is released under either the Mozilla Public License 1.1, or the
GNU General Public License, at the user's choice. Both of these
licenses are below.



                          MOZILLA PUBLIC LICENSE
                                Version 1.1

                              ---------------

1. Definitions.

     1.0.1. "Commercial Use" means distribution or otherwise making the
     Covered Code available to a third party.

     1.1. "Contributor" means each entity that creates or contributes to
     the creation of Modifications.

     1.2. "Contributor Version" means the combination of the Original
     Code, prior Modifications used by a Contributor, and the Modifications
     made by that particular Contributor.

     1.3. "Covered Code" means the Original Code or Modifications or the
     combination of the Original Code and Modifications, in each case
     including portions thereof.

     1.4. "Electronic Distribution Mechanism" means a mechanism generally
     accepted in the software development community for the electronic
     transfer of data.

     1.5. "Executable" means Covered Code in any form other than Source
     Code.

     1.6. "Initial Developer" means the individual or entity identified
     as the Initial Developer in the Source Code notice required by Exhibit
     A.

     1.7. "Larger Work" means a work which combines Covered Code or
     portions thereof with code not governed by the terms of this License.

     1.8. "License" means this document.

     1.8.1. "Licensable" means having the right to grant, to the maximum
     extent possible, whether at the time of the initial grant or
     subsequently acquired, any and all of the rights conveyed herein.

     1.9. "Modifications" means any addition to or deletion from the
     substance or structure of either the Original Code or any previous
     Modifications. When Covered Code is released as a series of files, a
     Modification is:
          A. Any addition to or deletion from the contents of a file
          containing Original Code or previous Modifications.

          B. Any new file that contains any part of the Original Code or
          previous Modifications.

     1.10. "Original Code" means Source Code of computer software code
     which is described in the Source Code notice required by Exhibit A as
     Original Code, and which, at the time of its release under this
     License is not already Covered Code governed by this License.

     1.10.1. "Patent Claims" means any patent claim(s), now owned or
     hereafter acquired, including without limitation, method, process,
     and apparatus claims, in any patent Licensable by grantor.

     1.11. "Source Code" means the preferred form of the Covered Code for
     making modifications to it, including all modules it contains, plus
     any associated interface definition files, scripts used to control
     compilation and installation of an Executable, or source code
     differential comparisons against either the Original Code or another
     well known, available Covered Code of the Contributor's choice. The
     Source Code can be in a compressed or archival form, provided the
     appropriate decompression or de-archiving software is widely available
     for no charge.

     1.12. "You" (or "Your") means an individual or a legal entity
     exercising rights under, and complying with all of the terms of, this
     License or a future version of this License issued under Section 6.1.
     For legal entities, "You" includes any entity which controls, is
     controlled by, or is under common control with You. For purposes of
     this definition, "control" means (a) the power, direct or indirect,
     to cause the direction or management of such entity, whether by
     contract or otherwise, or (b) ownership of more than fifty percent
     (50%) of the outstanding shares or beneficial ownership of such
     entity.

2. Source Code License.

     2.1. The Initial Developer Grant.
     The Initial Developer hereby grants You a world-wide, royalty-free,
     non-exclusive license, subject to third party intellectual property
     claims:
          (a) under intellectual property rights (other than patent or
          trademark) Licensable by Initial Developer to use, reproduce,
          modify, display, perform, sublicense and distribute the Original
          Code (or portions thereof) with or without Modifications, and/or
          as part of a Larger Work; and

          (b) under Patents Claims infringed by the making, using or
          selling of Original Code, to make, have made, use, practice,
          sell, and offer for sale, and/or otherwise dispose of the
          Original Code (or portions thereof).

          (c) the licenses granted in this Section 2.1(a) and (b) are
          effective on the date Initial Developer first distributes
          Original Code under the terms of this License.

          (d) Notwithstanding Section 2.1(b) above, no patent license is
          granted: 1) for code that You delete from the Original Code; 2)
          separate from the Original Code; or 3) for infringements caused
          by: i) the modification of the Original Code or ii) the
          combination of the Original Code with other software or devices.

     2.2. Contributor Grant.
     Subject to third party intellectual property claims, each Contributor
     hereby grants You a world-wide, royalty-free, non-exclusive license

          (a) under intellectual property rights (other than patent or
          trademark) Licensable by Contributor, to use, reproduce, modify,
          display, perform, sublicense and distribute the Modifications
          created by such Contributor (or portions thereof) either on an
          unmodified basis, with other Modifications, as Covered Code
          and/or as part of a Larger Work; and

          (b) under Patent Claims infringed by the making, using, or
          selling of Modifications made by that Contributor either alone
          and/or in combination with its Contributor Version (or portions
          of such combination), to make, use, sell, offer for sale, have
          made, and/or otherwise dispose of: 1) Modifications made by that
          Contributor (or portions thereof); and 2) the combination of
          Modifications made by that Contributor with its Contributor
          Version (or portions of such combination).

          (c) the licenses granted in Sections 2.2(a) and 2.2(b) are
          effective on the date Contributor first makes Commercial Use of
          the Covered Code.

          (d) Notwithstanding Section 2.2(b) above, no patent license is
          granted: 1) for any code that Contributor has deleted from the
          Contributor Version; 2) separate from the Contributor Version;
          3) for infringements caused by: i) third party modifications of
          Contributor Version or ii) the combination of Modifications made
          by that Contributor with other software (except as part of the
          Contributor Version) or other devices; or 4) under Patent Claims
          infringed by Covered Code in the absence of Modifications made by
          that Contributor.

3. Distribution Obligations.

     3.1. Application of License.
     The Modifications which You create or to which You contribute are
     governed by the terms of this License, including without limitation
     Section 2.2. The Source Code version of Covered Code may be
     distributed only under the terms of this License or a future version
     of this License released under Section 6.1, and You must include a
     copy of this License with every copy of the Source Code You
     distribute. You may not offer or impose any terms on any Source Code
     version that alters or restricts the applicable version of this
     License or the recipients' rights hereunder. However, You may include
     an additional document offering the additional rights described in
     Section 3.5.

     3.2. Availability of Source Code.
     Any Modification which You create or to which You contribute must be
     made available in Source Code form under the terms of this License
     either on the same media as an Executable version or via an accepted
     Electronic Distribution Mechanism to anyone to whom you made an
     Executable version available; and if made available via Electronic
     Distribution Mechanism, must remain available for at least twelve (12)
     months after the date it initially became available, or at least six
     (6) months after a subsequent version of that particular Modification
     has been made available to such recipients. You are responsible for
     ensuring that the Source Code version remains available even if the
     Electronic Distribution Mechanism is maintained by a third party.

     3.3. Description of Modifications.
     You must cause all Covered Code to which You contribute to contain a
     file documenting the changes You made to create that Covered Code and
     the date of any change. You must include a prominent statement that
     the Modification is derived, directly or indirectly, from Original
     Code provided by the Initial Developer and including the name of the
     Initial Developer in (a) the Source Code, and (b) in any notice in an
     Executable version or related documentation in which You describe the
     origin or ownership of the Covered Code.

     3.4. Intellectual Property Matters
          (a) Third Party Claims.
          If Contributor has knowledge that a license under a third party's
          intellectual property rights is required to exercise the rights
          granted by such Contributor under Sections 2.1 or 2.2,
          Contributor must include a text file with the Source Code
          distribution titled "LEGAL" which describes the claim and the
          party making the claim in sufficient detail that a recipient will
          know whom to contact. If Contributor obtains such knowledge after
          the Modification is made available as described in Section 3.2,
          Contributor shall promptly modify the LEGAL file in all copies
          Contributor makes available thereafter and shall take other steps
          (such as notifying appropriate mailing lists or newsgroups)
          reasonably calculated to inform those who received the Covered
          Code that new knowledge has been obtained.

          (b) Contributor APIs.
          If Contributor's Modifications include an application programming
          interface and Contributor has knowledge of patent licenses which
          are reasonably necessary to implement that API, Contributor must
          also include this information in the LEGAL file.

               (c) Representations.
          Contributor represents that, except as disclosed pursuant to
          Section 3.4(a) above, Contributor believes that Contributor's
          Modifications are Contributor's original creation(s) and/or
          Contributor has sufficient rights to grant the rights conveyed by
          this License.

     3.5. Required Notices.
     You must duplicate the notice in Exhibit A in each file of the Source
     Code. If it is not possible to put such notice in a particular Source
     Code file due to its structure, then You must include such notice in a
     location (such as a relevant directory) where a user would be likely
     to look for such a notice. If You created one or more Modification(s)
     You may add your name as a Contributor to the notice described in
     Exhibit A. You must also duplicate this License in any documentation
     for the Source Code where You describe recipients' rights or ownership
     rights relating to Covered Code. You may choose to offer, and to
     charge a fee for, warranty, support, indemnity or liability
     obligations to one or more recipients of Covered Code. However, You
     may do so only on Your own behalf, and not on behalf of the Initial
     Developer or any Contributor. You must make it absolutely clear than
     any such warranty, support, indemnity or liability obligation is
     offered by You alone, and You hereby agree to indemnify the Initial
     Developer and every Contributor for any liability incurred by the
     Initial Developer or such Contributor as a result of warranty,
     support, indemnity or liability terms You offer.

     3.6. Distribution of Executable Versions.
     You may distribute Covered Code in Executable form only if the
     requirements of Section 3.1-3.5 have been met for that Covered Code,
     and if You include a notice stating that the Source Code version of
     the Covered Code is available under the terms of this License,
     including a description of how and where You have fulfilled the
     obligations of Section 3.2. The notice must be conspicuously included
     in any notice in an Executable version, related documentation or
     collateral in which You describe recipients' rights relating to the
     Covered Code. You may distribute the Executable version of Covered
     Code or ownership rights under a license of Your choice, which may
     contain terms different from this License, provided that You are in
     compliance with the terms of this License and that the license for the
     Executable version does not attempt to limit or alter the recipient's
     rights in the Source Code version from the rights set forth in this
     License. If You distribute the Executable version under a different
     license You must make it absolutely clear that any terms which differ
     from this License are offered by You alone, not by the Initial
     Developer or any Contributor. You hereby agree to indemnify the
     Initial Developer and every Contributor for any liability incurred by
     the Initial Developer or such Contributor as a result of any such
     terms You offer.

     3.7. Larger Works.
     You may create a Larger Work by combining Covered Code with other code
     not governed by the terms of this License and distribute the Larger
     Work as a single product. In such a case, You must make sure the
     requirements of this License are fulfilled for the Covered Code.

4. Inability to Comply Due to Statute or Regulation.

     If it is impossible for You to comply with any of the terms of this
     License with respect to some or all of the Covered Code due to
     statute, judicial order, or regulation then You must: (a) comply with
     the terms of this License to the maximum extent possible; and (b)
     describe the limitations and the code they affect. Such description
     must be included in the LEGAL file described in Section 3.4 and must
     be included with all distributions of the Source Code. Except to the
     extent prohibited by statute or regulation, such description must be
     sufficiently detailed for a recipient of ordinary skill to be able to
     understand it.

5. Application of this License.

     This License applies to code to which the Initial Developer has
     attached the notice in Exhibit A and to related Covered Code.

6. Versions of the License.

     6.1. New Versions.
     Netscape Communications Corporation ("Netscape") may publish revised
     and/or new versions of the License from time to time. Each version
     will be given a distinguishing version number.

     6.2. Effect of New Versions.
     Once Covered Code has been published under a particular version of the
     License, You may always continue to use it under the terms of that
     version. You may also choose to use such Covered Code under the terms
     of any subsequent version of the License published by Netscape. No one
     other than Netscape has the right to modify the terms applicable to
     Covered Code created under this License.

     6.3. Derivative Works.
     If You create or use a modified version of this License (which you may
     only do in order to apply it to code which is not already Covered Code
     governed by this License), You must (a) rename Your license so that
     the phrases "Mozilla", "MOZILLAPL", "MOZPL", "Netscape",
     "MPL", "NPL" or any confusingly similar phrase do not appear in your
     license (except to note that your license differs from this License)
     and (b) otherwise make it clear that Your version of the license
     contains terms which differ from the Mozilla Public License and
     Netscape Public License. (Filling in the name of the Initial
     Developer, Original Code or Contributor in the notice described in
     Exhibit A shall not of themselves be deemed to be modifications of
     this License.)

7. DISCLAIMER OF WARRANTY.

     COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS,
     WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
     WITHOUT LIMITATION, WARRANTIES THAT THE COVERED CODE IS FREE OF
     DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE OR NON-INFRINGING.
     THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED CODE
     IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT,
     YOU (NOT THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE
     COST OF ANY NECESSARY SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER
     OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS LICENSE. NO USE OF
     ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER THIS DISCLAIMER.

8. TERMINATION.

     8.1. This License and the rights granted hereunder will terminate
     automatically if You fail to comply with terms herein and fail to cure
     such breach within 30 days of becoming aware of the breach. All
     sublicenses to the Covered Code which are properly granted shall
     survive any termination of this License. Provisions which, by their
     nature, must remain in effect beyond the termination of this License
     shall survive.

     8.2. If You initiate litigation by asserting a patent infringement
     claim (excluding declatory judgment actions) against Initial Developer
     or a Contributor (the Initial Developer or Contributor against whom
     You file such action is referred to as "Participant") alleging that:

     (a) such Participant's Contributor Version directly or indirectly
     infringes any patent, then any and all rights granted by such
     Participant to You under Sections 2.1 and/or 2.2 of this License
     shall, upon 60 days notice from Participant terminate prospectively,
     unless if within 60 days after receipt of notice You either: (i)
     agree in writing to pay Participant a mutually agreeable reasonable
     royalty for Your past and future use of Modifications made by such
     Participant, or (ii) withdraw Your litigation claim with respect to
     the Contributor Version against such Participant. If within 60 days
     of notice, a reasonable royalty and payment arrangement are not
     mutually agreed upon in writing by the parties or the litigation claim
     is not withdrawn, the rights granted by Participant to You under
     Sections 2.1 and/or 2.2 automatically terminate at the expiration of
     the 60 day notice period specified above.

     (b) any software, hardware, or device, other than such Participant's
     Contributor Version, directly or indirectly infringes any patent, then
     any rights granted to You by such Participant under Sections 2.1(b)
     and 2.2(b) are revoked effective as of the date You first made, used,
     sold, distributed, or had made, Modifications made by that
     Participant.

     8.3. If You assert a patent infringement claim against Participant
     alleging that such Participant's Contributor Version directly or
     indirectly infringes any patent where such claim is resolved (such as
     by license or settlement) prior to the initiation of patent
     infringement litigation, then the reasonable value of the licenses
     granted by such Participant under Sections 2.1 or 2.2 shall be taken
     into account in determining the amount or value of any payment or
     license.

     8.4. In the event of termination under Sections 8.1 or 8.2 above,
     all end user license agreements (excluding distributors and resellers)
     which have been validly granted by You or any distributor hereunder
     prior to termination shall survive termination.

9. LIMITATION OF LIABILITY.

     UNDER NO CIRCUMSTANCES AND UNDER NO LEGAL THEORY, WHETHER TORT
     (INCLUDING NEGLIGENCE), CONTRACT, OR OTHERWISE, SHALL YOU, THE INITIAL
     DEVELOPER, ANY OTHER CONTRIBUTOR, OR ANY DISTRIBUTOR OF COVERED CODE,
     OR ANY SUPPLIER OF ANY OF SUCH PARTIES, BE LIABLE TO ANY PERSON FOR
     ANY INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES OF ANY
     CHARACTER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF GOODWILL,
     WORK STOPPAGE, COMPUTER FAILURE OR MALFUNCTION, OR ANY AND ALL OTHER
     COMMERCIAL DAMAGES OR LOSSES, EVEN IF SUCH PARTY SHALL HAVE BEEN
     INFORMED OF THE POSSIBILITY OF SUCH DAMAGES. THIS LIMITATION OF
     LIABILITY SHALL NOT APPLY TO LIABILITY FOR DEATH OR PERSONAL INJURY
     RESULTING FROM SUCH PARTY'S NEGLIGENCE TO THE EXTENT APPLICABLE LAW
     PROHIBITS SUCH LIMITATION. SOME JURISDICTIONS DO NOT ALLOW THE
     EXCLUSION OR LIMITATION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO
     THIS EXCLUSION AND LIMITATION MAY NOT APPLY TO YOU.

10. U.S. GOVERNMENT END USERS.

     The Covered Code is a "commercial item," as that term is defined in
     48 C.F.R. 2.101 (Oct. 1995), consisting of "commercial computer
     software" and "commercial computer software documentation," as such
     terms are used in 48 C.F.R. 12.212 (Sept. 1995). Consistent with 48
     C.F.R. 12.212 and 48 C.F.R. 227.7202-1 through 227.7202-4 (June 1995),
     all U.S. Government End Users acquire Covered Code with only those
     rights set forth herein.

11. MISCELLANEOUS.

     This License represents the complete agreement concerning subject
     matter hereof. If any provision of this License is held to be
     unenforceable, such provision shall be reformed only to the extent
     necessary to make it enforceable. This License shall be governed by
     California law provisions (except to the extent applicable law, if
     any, provides otherwise), excluding its conflict-of-law provisions.
     With respect to disputes in which at least one party is a citizen of,
     or an entity chartered or registered to do business in the United
     States of America, any litigation relating to this License shall be
     subject to the jurisdiction of the Federal Courts of the Northern
     District of California, with venue lying in Santa Clara County,
     California, with the losing party responsible for costs, including
     without limitation, court costs and reasonable attorneys' fees and
     expenses. The application of the United Nations Convention on
     Contracts for the International Sale of Goods is expressly excluded.
     Any law or regulation which provides that the language of a contract
     shall be construed against the drafter shall not apply to this
     License.

12. RESPONSIBILITY FOR CLAIMS.

     As between Initial Developer and the Contributors, each party is
     responsible for claims and damages arising, directly or indirectly,
     out of its utilization of rights under this License and You agree to
     work with Initial Developer and Contributors to distribute such
     responsibility on an equitable basis. Nothing herein is intended or
     shall be deemed to constitute any admission of liability.

13. MULTIPLE-LICENSED CODE.

     Initial Developer may designate portions of the Covered Code as
     "Multiple-Licensed". "Multiple-Licensed" means that the Initial
     Developer permits you to utilize portions of the Covered Code under
     Your choice of the NPL or the alternative licenses, if any, specified
     by the Initial Developer in the file described in Exhibit A.

EXHIBIT A -Mozilla Public License.

     ``The contents of this file are subject to the Mozilla Public License
     Version 1.1 (the "License"); you may not use this file except in
     compliance with the License. You may obtain a copy of the License at
     http:slashslashwww.mozilla.org/MPL/

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is ______________________________________.

     The Initial Developer of the Original Code is ________________________.
     Portions created by ______________________ are Copyright (C) ______
     _______________________. All Rights Reserved.

     Contributor(s): ______________________________________.

     Alternatively, the contents of this file may be used under the terms
     of the _____ license (the "[___] License"), in which case the
     provisions of [______] License are applicable instead of those
     above. If you wish to allow use of your version of this file only
     under the terms of the [____] License and not to allow others to use
     your version of this file under the MPL, indicate your decision by
     deleting the provisions above and replace them with the notice and
     other provisions required by the [___] License. If you do not delete
     the provisions above, a recipient may use your version of this file
     under either the MPL or the [___] License."

     [NOTE: The text of this Exhibit A may differ slightly from the text of
     the notices in the Source Code files of the Original Code. You should
     use the text of this Exhibit A rather than the text found in the
     Original Code Source Code for Your Modifications.]



		GNU GENERAL PUBLIC LICENSE
		Version 2, June 1991

 Copyright (C) 1989, 1991 Free Software Foundation, Inc.
     59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.

			Preamble

  The licenses for most software are designed to take away your
freedom to share and change it. By contrast, the GNU General Public
License is intended to guarantee your freedom to share and change free
software--to make sure the software is free for all its users. This
General Public License applies to most of the Free Software
Foundation's software and to any other program whose authors commit to
using it. (Some other Free Software Foundation software is covered by
the GNU Library General Public License instead.) You can apply it to
your programs, too.

  When we speak of free software, we are referring to freedom, not
price. Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
this service if you wish), that you receive source code or can get it
if you want it, that you can change the software or use pieces of it
in new free programs; and that you know you can do these things.

  To protect your rights, we need to make restrictions that forbid
anyone to deny you these rights or to ask you to surrender the rights.
These restrictions translate to certain responsibilities for you if you
distribute copies of the software, or if you modify it.

  For example, if you distribute copies of such a program, whether
gratis or for a fee, you must give the recipients all the rights that
you have. You must make sure that they, too, receive or can get the
source code. And you must show them these terms so they know their
rights.

  We protect your rights with two steps: (1) copyright the software, and
(2) offer you this license which gives you legal permission to copy,
distribute and/or modify the software.

  Also, for each author's protection and ours, we want to make certain
that everyone understands that there is no warranty for this free
software. If the software is modified by someone else and passed on, we
want its recipients to know that what they have is not the original, so
that any problems introduced by others will not reflect on the original
authors' reputations.

  Finally, any free program is threatened constantly by software
patents. We wish to avoid the danger that redistributors of a free
program will individually obtain patent licenses, in effect making the
program proprietary. To prevent this, we have made it clear that any
patent must be licensed for everyone's free use or not licensed at all.

  The precise terms and conditions for copying, distribution and
modification follow.

		GNU GENERAL PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. This License applies to any program or other work which contains
a notice placed by the copyright holder saying it may be distributed
under the terms of this General Public License. The "Program", below,
refers to any such program or work, and a "work based on the Program"
means either the Program or any derivative work under copyright law:
that is to say, a work containing the Program or a portion of it,
either verbatim or with modifications and/or translated into another
language. (Hereinafter, translation is included without limitation in
the term "modification".) Each licensee is addressed as "you".

Activities other than copying, distribution and modification are not
covered by this License; they are outside its scope. The act of
running the Program is not restricted, and the output from the Program
is covered only if its contents constitute a work based on the
Program (independent of having been made by running the Program).
Whether that is true depends on what the Program does.

  1. You may copy and distribute verbatim copies of the Program's
source code as you receive it, in any medium, provided that you
conspicuously and appropriately publish on each copy an appropriate
copyright notice and disclaimer of warranty; keep intact all the
notices that refer to this License and to the absence of any warranty;
and give any other recipients of the Program a copy of this License
along with the Program.

You may charge a fee for the physical act of transferring a copy, and
you may at your option offer warranty protection in exchange for a fee.

  2. You may modify your copy or copies of the Program or any portion
of it, thus forming a work based on the Program, and copy and
distribute such modifications or work under the terms of Section 1
above, provided that you also meet all of these conditions:

    a) You must cause the modified files to carry prominent notices
    stating that you changed the files and the date of any change.

    b) You must cause any work that you distribute or publish, that in
    whole or in part contains or is derived from the Program or any
    part thereof, to be licensed as a whole at no charge to all third
    parties under the terms of this License.

    c) If the modified program normally reads commands interactively
    when run, you must cause it, when started running for such
    interactive use in the most ordinary way, to print or display an
    announcement including an appropriate copyright notice and a
    notice that there is no warranty (or else, saying that you provide
    a warranty) and that users may redistribute the program under
    these conditions, and telling the user how to view a copy of this
    License. (Exception: if the Program itself is interactive but
    does not normally print such an announcement, your work based on
    the Program is not required to print an announcement.)

These requirements apply to the modified work as a whole. If
identifiable sections of that work are not derived from the Program,
and can be reasonably considered independent and separate works in
themselves, then this License, and its terms, do not apply to those
sections when you distribute them as separate works. But when you
distribute the same sections as part of a whole which is a work based
on the Program, the distribution of the whole must be on the terms of
this License, whose permissions for other licensees extend to the
entire whole, and thus to each and every part regardless of who wrote it.

Thus, it is not the intent of this section to claim rights or contest
your rights to work written entirely by you; rather, the intent is to
exercise the right to control the distribution of derivative or
collective works based on the Program.

In addition, mere aggregation of another work not based on the Program
with the Program (or with a work based on the Program) on a volume of
a storage or distribution medium does not bring the other work under
the scope of this License.

  3. You may copy and distribute the Program (or a work based on it,
under Section 2) in object code or executable form under the terms of
Sections 1 and 2 above provided that you also do one of the following:

    a) Accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of Sections
    1 and 2 above on a medium customarily used for software interchange; or,

    b) Accompany it with a written offer, valid for at least three
    years, to give any third party, for a charge no more than your
    cost of physically performing source distribution, a complete
    machine-readable copy of the corresponding source code, to be
    distributed under the terms of Sections 1 and 2 above on a medium
    customarily used for software interchange; or,

    c) Accompany it with the information you received as to the offer
    to distribute corresponding source code. (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form with such
    an offer, in accord with Subsection b above.)

The source code for a work means the preferred form of the work for
making modifications to it. For an executable work, complete source
code means all the source code for all modules it contains, plus any
associated interface definition files, plus the scripts used to
control compilation and installation of the executable. However, as a
special exception, the source code distributed need not include
anything that is normally distributed (in either source or binary
form) with the major components (compiler, kernel, and so on) of the
operating system on which the executable runs, unless that component
itself accompanies the executable.

If distribution of executable or object code is made by offering
access to copy from a designated place, then offering equivalent
access to copy the source code from the same place counts as
distribution of the source code, even though third parties are not
compelled to copy the source along with the object code.

  4. You may not copy, modify, sublicense, or distribute the Program
except as expressly provided under this License. Any attempt
otherwise to copy, modify, sublicense or distribute the Program is
void, and will automatically terminate your rights under this License.
However, parties who have received copies, or rights, from you under
this License will not have their licenses terminated so long as such
parties remain in full compliance.

  5. You are not required to accept this License, since you have not
signed it. However, nothing else grants you permission to modify or
distribute the Program or its derivative works. These actions are
prohibited by law if you do not accept this License. Therefore, by
modifying or distributing the Program (or any work based on the
Program), you indicate your acceptance of this License to do so, and
all its terms and conditions for copying, distributing or modifying
the Program or works based on it.

  6. Each time you redistribute the Program (or any work based on the
Program), the recipient automatically receives a license from the
original licensor to copy, distribute or modify the Program subject to
these terms and conditions. You may not impose any further
restrictions on the recipients' exercise of the rights granted herein.
You are not responsible for enforcing compliance by third parties to
this License.

  7. If, as a consequence of a court judgment or allegation of patent
infringement or for any other reason (not limited to patent issues),
conditions are imposed on you (whether by court order, agreement or
otherwise) that contradict the conditions of this License, they do not
excuse you from the conditions of this License. If you cannot
distribute so as to satisfy simultaneously your obligations under this
License and any other pertinent obligations, then as a consequence you
may not distribute the Program at all. For example, if a patent
license would not permit royalty-free redistribution of the Program by
all those who receive copies directly or indirectly through you, then
the only way you could satisfy both it and this License would be to
refrain entirely from distribution of the Program.

If any portion of this section is held invalid or unenforceable under
any particular circumstance, the balance of the section is intended to
apply and the section as a whole is intended to apply in other
circumstances.

It is not the purpose of this section to induce you to infringe any
patents or other property right claims or to contest validity of any
such claims; this section has the sole purpose of protecting the
integrity of the free software distribution system, which is
implemented by public license practices. Many people have made
generous contributions to the wide range of software distributed
through that system in reliance on consistent application of that
system; it is up to the author/donor to decide if he or she is willing
to distribute software through any other system and a licensee cannot
impose that choice.

This section is intended to make thoroughly clear what is believed to
be a consequence of the rest of this License.

  8. If the distribution and/or use of the Program is restricted in
certain countries either by patents or by copyrighted interfaces, the
original copyright holder who places the Program under this License
may add an explicit geographical distribution limitation excluding
those countries, so that distribution is permitted only in or among
countries not thus excluded. In such case, this License incorporates
the limitation as if written in the body of this License.

  9. The Free Software Foundation may publish revised and/or new versions
of the General Public License from time to time. Such new versions will
be similar in spirit to the present version, but may differ in detail to
address new problems or concerns.

Each version is given a distinguishing version number. If the Program
specifies a version number of this License which applies to it and "any
later version", you have the option of following the terms and conditions
either of that version or of any later version published by the Free
Software Foundation. If the Program does not specify a version number of
this License, you may choose any version ever published by the Free Software
Foundation.

  10. If you wish to incorporate parts of the Program into other free
programs whose distribution conditions are different, write to the author
to ask for permission. For software which is copyrighted by the Free
Software Foundation, write to the Free Software Foundation; we sometimes
make exceptions for this. Our decision will be guided by the two goals
of preserving the free status of all derivatives of our free software and
of promoting the sharing and reuse of software generally.

			NO WARRANTY

  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

		END OF TERMS AND CONDITIONS

	How to Apply These Terms to Your New Programs

  If you develop a new program, and you want it to be of the greatest
possible use to the public, the best way to achieve this is to make it
free software which everyone can redistribute and change under these terms.

  To do so, attach the following notices to the program. It is safest
to attach them to the start of each source file to most effectively
convey the exclusion of warranty; and each file should have at least
the "copyright" line and a pointer to where the full notice is found.

    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year> <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


Also add information on how to contact you by electronic and paper mail.

If the program is interactive, make it output a short notice like this
when it starts in an interactive mode:

    Gnomovision version 69, Copyright (C) year name of author
    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details.

The hypothetical commands `show w' and `show c' should show the appropriate
parts of the General Public License. Of course, the commands you use may
be called something other than `show w' and `show c'; they could even be
mouse-clicks or menu items--whatever suits your program.

You should also get your employer (if you work as a programmer) or your
school, if any, to sign a "copyright disclaimer" for the program, if
necessary. Here is a sample; alter the names:

  Yoyodyne, Inc., hereby disclaims all copyright interest in the program
  `Gnomovision' (which makes passes at compilers) written by James Hacker.

  <signature of Ty Coon>, 1 April 1989
  Ty Coon, President of Vice

This General Public License does not permit incorporating your program into
proprietary programs. If your program is a subroutine library, you may
consider it more useful to permit linking proprietary applications with the
library. If this is what you want to do, use the GNU Library General
Public License instead of this License.
}


gset help_texts {Copyright Confusing}
# 35 "/local/scratch/floater-1.4.15/tclcode/texts.deq" 2
proc display_text {name s} {
    global fixedfont

    set slash /
    regsub -all slashslash [string range $s 1 end] $slash$slash text

    talkmsg $text
# 60 "/local/scratch/floater-1.4.15/tclcode/texts.deq"
}
# 166 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
line after texts

foreach x [array names tcl_platform] { lappend platform $tcl_platform($x) }

gset floaterclock 0
gset table_arrival_time 0
gset snooze 0
# 188 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
gset ntalklines 0

gset dtalklines 0
		

gset talklineattop 0



gset showerrors 1
gset debugprinting 0


if $floater_silent {
    proc clearrect {x y} {puts stdout "clearrect $x $y"}
    proc anchor {l} {puts stdout "anchor $l"}
    proc down_and_anchor {{l 1}} {puts stdout "down_and_anchor $l"}
    proc right {{l 1}} {puts stdout "right $l"}
    proc str {l} {puts stdout "str `$l'"}
    proc ch {l} {puts stdout "ch $l"}
}

line 161

proc talkmsg {s {draw 1} {allowPrefix 1}} {
    global talklines ntalklines talkwidth debugprinting showerrors
    global dtalklines scrolllock talktop floater_silent floater_silent_conns

    if $floater_silent {
	puts $s
	global conn_to_sock
	foreach conn [array names floater_silent_conns] {
	catch {puts $conn_to_sock($conn) $s}
	}
	return
    }




    if $debugprinting return

    if {$talktop < 0} return
    if {!$showerrors && [regexp -nocase error $s]} return


    if [regexp "(.*)\n(.*)" $s whole a b] {
	talkmsg $a
	talkmsg $b
	return
    }

    if {[string length $s] > $talkwidth} {
	
	for {set i $talkwidth} {[incr i -1] > 0} {} {
	if {[string index $s $i] == " "} {
		incr i -1
		talkmsg [string range $s 0 $i] 0 0
		talkmsg [string range $s [expr $i + 2] end] $draw 0
		return
	}
	}
	
	talkmsg [string range $s 0 [expr $talkwidth - 1]] 0 0
	talkmsg [string range $s $talkwidth end] $draw 0
	return
    }

    set talklines($ntalklines) $s
    incr ntalklines
    if !$scrolllock {set dtalklines $ntalklines}
    if $draw {drawtalkregion}
}
# 276 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
proc floatererror {s} { talkmsg "ERROR: $s" }





if {[catch {source $startupfile} err] 	&& ![regexp -nocase {no such file} $err]} {


    talkmsg "ERROR: $err"



}


tryset loginservername "loginserver"
tryset loginserveraddr server.floater.org
tryset loginserverport "2210"
tryset resultservername "resultserver"
tryset resultserveraddr $loginserveraddr
tryset resultserverport "1430"
tryset pseudomailaddr $loginserveraddr
tryset pseudomailport "1440"
tryset resultparserprogram @FLOATER_SRC_DIR@/floatres/parsemail
tryset resultparser "floater@floater.org"
tryset bugmail "lex@cc.gatech.edu"

line 255


tryset defaultnote ""


tryset tricktime 2000






tryset autonewdeal_default 35



tryset autonewdeal_seconds $autonewdeal_default

tryset nokibitzers 0
tryset jointableservertree 1

tryset youveseen 1

tryset newbie [expr ![info exists usedname]]
# 566 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
gset tcl_interactive 1

set needAuctionUpdate 0


gset showingauction 0

# 1 "/local/scratch/floater-1.4.15/tclcode/options_common.deq" 1
# 46 "/local/scratch/floater-1.4.15/tclcode/options_common.deq"
set beepAtMyTurn_ 0
proc beepAtMyTurn {{toggle 0}} {
    global beepAtMyTurn_

    if $toggle {set beepAtMyTurn_ [expr !$beepAtMyTurn_]}
}
# 574 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2






# 1 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq" 1
# 38 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc redrawmatrixcards {} {}

proc togglepassedcard {suit card} {
    global togglepassedaction

    if [info exists togglepassedaction([string toupper $suit$card])] 	{catch $togglepassedaction([string toupper $suit$card])}

}

proc removecardfromhand {suit card} {
    global removecard

    if [info exists removecard([string toupper $suit$card])] 	{catch $removecard([string toupper $suit$card])}

}
# 69 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc rmcard {x y suit card} {
    set f "$x $y"
    anchor $f
    set suit [string toupper $suit]
    if {$suit == "S"} {rmcard2 $f $suit $card} {down_and_anchor}
    if {$suit == "H"} {rmcard2 $f $suit $card} {down_and_anchor}
    if {$suit == "D"} {rmcard2 $f $suit $card} {down_and_anchor}
    if {$suit == "C"} {rmcard2 $f $suit $card}
}

proc rmcard2 {f suit card} {
    global cursuit

    right 2
    set w [set "cursuit($f$suit)" [zap $card $cursuit($f$suit)]]
    str "$w "
}


proc zap {char text} {
    set i [string first $char $text]
    if {$i < 0} {
	return $text
    } elseif {$i == 0} {
	return [string range $text 1 end]
    } else {
	incr i -1
	set j [expr $i + 2]
	
	return "[string range $text 0 $i][string range $text $j end]"
    }
}




proc suit {f cards suit} {

    global cursuit removecard

    set suit [string toupper $suit]
    set cards [string toupper $cards]
    str "$suit $cards"
    down_and_anchor
    set "cursuit($f$suit)" $cards
    for {set i [expr [string length $cards] - 1]} {$i >= 0} {incr i -1} {
	set card [string index $cards $i]
	set removecard([string toupper $suit$card]) "rmcard $f $suit $card"
    }
# 140 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
}



proc hand {f s h d c} {
# 166 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
    global handwidth
    anchor $f
    clearrect $handwidth 4

    suit $f $s s
    suit $f $h h
    suit $f $d d
    suit $f $c c
}
# 206 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc fulldeal {s h d c LHOs LHOh LHOd LHOc 		Ps Ph Pd Pc RHOs RHOh RHOd RHOc} {

    global mframe

    hand $mframe(self) $s $h $d $c
    hand $mframe(pard) $Ps $Ph $Pd $Pc
    hand $mframe(lho) $LHOs $LHOh $LHOd $LHOc
    hand $mframe(rho) $RHOs $RHOh $RHOd $RHOc
}

gset tricktimeOK 1



proc startshowtricktimer {} {
    global tricktime tricktimeOK

    set tricktimeOK 0
    after $tricktime set tricktimeOK 1
}



proc delayedclearmatrix {} {
    global needtoerase

    set needtoerase 1
    after 5000 clearmatrixtimer
}

proc clearmatrixtimer {} {
    global needtoerase

    if $needtoerase {erasebidplay all}
}



proc erasebidplay {who} {
    global tricktimeOK

    if {!$tricktimeOK} {
	after 100 erasebidplay $who
    }
    if {$who == "all"} {
	global needtoerase

	set needtoerase 0
	erasebidplay lho
	erasebidplay rho
	erasebidplay pard
	erasebidplay self
    } else {

	global matrixtext
	
	anchor $matrixtext($who)
	clearrect 2 1




    }
}




proc showplay {player suit card} {

    global matrixtext

    anchor $matrixtext($player)
    if {$suit == "?"} {
	str "? "
    } else {
	str $suit$card
    }
# 304 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
}





proc showbid {player level strain} {


    global matrixtext

    anchor $matrixtext($player)
    if {$strain == "-"} {
	str " "
    } elseif {$strain == "?"} {
	str "? "
    } elseif {$level > 0} {
	str "$level$strain "
    } else {	
	str "$strain "
    }



}


proc drawbid {x y level strain} {
    global auctionx auctiony auctionbot



    if {[expr $auctiony + $y + 2] <= $auctionbot} {
	anchor "[expr $auctionx + 1 + 4 * $x] [expr $auctiony + $y + 2]"

	
	if {$strain == "x"} {
	set s "X "
	} elseif {$strain == "xx"} {
	set s "XX"
	} elseif {$strain == "p"} {
	set s "P "
	} elseif {$strain == "-"} {
	set s " "
	} elseif {$strain == "?"} {
	set s "? "
	} else {
	set s $level$strain
	}
	
	clearrect 2 1
	if {$s != " "} {str [string toupper $s]}
    }
}
# 488 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc setname {player compassdir name} {

    global namepos namewidth

    anchor $namepos($player)
    if {[string first "(" $name] == -1} {set name "$name ($compassdir)"}
    if {[string length $name] > $namewidth} {
	set name [string range $name 0 [expr $namewidth - 1]]
    }
    if {$player == "self" || $player == "pard"} {
	rightjustify $name $namewidth
    } else {
	clearrect $namewidth 1
    }
    str $name
# 516 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
}
# 581 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2


line 532




proc showauction {bool} {
    global auctionx auctiony auctionwidth auctionheight auctionright auctionbot
    global showingauction

    set showingauction $bool
    anchor "$auctionx $auctiony"
    clearrect $auctionwidth $auctionheight
    if $bool {
	hline . $auctionx $auctionright $auctiony
	vline . $auctionx $auctiony $auctionbot
    }
    textseated
}





proc rightjustify {s width {r 1}} {
    while {[string length $s] < $width} {
	set s " $s"
	if $r {ch " "}
    }
    return $s
}



proc hline {c xlo xhi y} {
    anchor "$xlo $y"
    for {} {$xlo <= $xhi} {incr xlo} {ch $c}
}

proc vline {c x ylo yhi} {
    anchor "$x $ylo"
    for {} {$ylo <= $yhi} {incr ylo} {ch $c; down_and_anchor}
}


hline - 30 46 4
hline - 30 46 10
vline | 29 5 9
vline | 47 5 9


gset mframe(self) {30 11}
gset mframe(pard) {30 0}
gset mframe(lho) {15 6}
gset mframe(rho) {49 6}
gset handwidth 14

gset namewidth 14
gset namepos(self) {15 11}
gset namepos(pard) {15 0}
gset namepos(lho) {15 5}
gset namepos(rho) {49 5}

gset matrixtext(self) {37 9}
gset matrixtext(pard) {37 5}
gset matrixtext(lho) {31 7}
gset matrixtext(rho) {43 7}

gset auctionx 64
gset auctiony 5
gset auctionright 79
gset auctionbot 14
gset auctionwidth [expr $auctionright - $auctionx + 1]
gset auctionheight [expr $auctionbot - $auctiony + 1]
# 703 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
set x [expr ![catch {regexp -nocase "Apr.? 1 " [clock format [clock seconds]]} y]]
if !$x {set y 0}
if [expr $x && $y] {
 fulldeal AKQJ AKQJ AKQJ AK T98 T98 T98 QJT9 765 765 765 876 432 432 432 5432
 showbid self 8 n
} else {
 fulldeal AKQ AKQ AKQ AKQJ JT9 JT9 JT9 T987 876 876 876 6543 5432 5432 5432 2
 showbid self 7 n
}


line 901






gset previous_trick_index 0
gset previous_trick {}

proc reset_previous_trick {{index -999}} {
    global previous_trick previous_trick_index

    if {$index == -999 || $index == $previous_trick_index} {
	set previous_trick {}
    }
}

proc set_previous_trick {s {erase 1}} {
    global previous_trick previous_trick_index

    set previous_trick $s
    incr previous_trick_index


    if $erase {after 10000 "reset_previous_trick $previous_trick_index"}
}




gset oldpov S
gset oldseated 0

proc textseated {{seated -1} {pov S}} {
    global auctionx auctiony showingauction oldseated oldpov

    if {$seated == -1} {set seated $oldseated; set pov $oldpov}
    set oldseated $seated
    set oldpov $pov
    if !$showingauction return

    anchor "$auctionx $auctiony"
    down_and_anchor
    right 1
    if $seated {
	str "LHO Par RHO you"
    } else {
	if {$pov == "S"} {
	str "(W) (N) (E) (S)"
	} elseif {$pov == "N"} {
	str "(E) (S) (W) (N)"
	} elseif {$pov == "E"} {
	str "(S) (W) (N) (E)"
	} elseif {$pov == "W"} {
	str "(N) (E) (S) (W)"
	}
    }
}
# 783 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
set statusline {}
set infoline {}

gset leftwidth 14
gset rightwidth 30
gset rightpos 50

proc strinfield {s x y width} {
    anchor "$x $y"
    clearrect $width 1
    if {[string length $s] > $width} 	{set s [string range $s 0 [expr $width - 1]]}

    str $s
}

strinfield $floater_version 0 0 15


proc connstat {{s {}}} {
    global leftwidth
    strinfield $s 0 1 $leftwidth
}


proc displayhandname {{s {}}} {
    global leftwidth
    strinfield $s 0 2 $leftwidth
}

proc statushandvul {{s {}}} {
    global leftwidth
    strinfield $s 0 3 $leftwidth
}

proc statushanddlr {{s {}}} {
    global leftwidth
    strinfield $s 0 4 $leftwidth
}

proc statuscontract {{s {}}} {
    global rightpos rightwidth
    strinfield $s $rightpos 0 $rightwidth
}

proc statustolead {{s {}}} {
    global rightpos rightwidth
    strinfield $s $rightpos 1 $rightwidth
}

proc displaytrickswon {{s {}}} {
    global rightpos rightwidth
    strinfield $s $rightpos 2 $rightwidth
}


proc statusclaim {{s {}}} {
    global rightpos rightwidth
    strinfield $s $rightpos 3 $rightwidth
}

proc statusresult {{s {}}} {
    global rightpos rightwidth
    strinfield $s $rightpos 4 $rightwidth
}


proc statusscore {{s {}}} {
    global leftwidth

    set x 0
    set y 5

    if {$s == ""} {set s " ; ; ; ; "}

    while {[regexp {([^;]*); (.*)} $s whole t s]} {
	strinfield $t $x $y $leftwidth
	incr y
    }
    strinfield $s $x $y $leftwidth
}


line 1064


gset oldntalklines 0
gset scrolllock 0




proc drawtalkregion {{must_redraw 0}} {
    global talklines dtalklines talktop talklineattop talkbottom oldntalklines
    global scrolllock ntalklines

    draw_on_current_display +

    set talksize [expr $talkbottom - $talktop + 1]

    if {($dtalklines >= $ntalklines) || ($ntalklines < $talksize)} {
	set dtalklines $ntalklines
	set scrolllock 0
    }

    set want_to_redraw 	[expr ($dtalklines - $talklineattop) > $talksize]

    if {$must_redraw || ($want_to_redraw && !$scrolllock)} {
	
	set y $talktop
	set i [set talklineattop [expr $dtalklines - $talksize]]
	if {$i < 0} {
	if $scrolllock {set dtalklines $talksize}
	set i 0
	}
	for {set talklineattop $i} 	{($y <= $talkbottom) && ($i < $dtalklines) && ($i < $ntalklines)} 	{incr i; incr y} {


		drawtalkline $y $talklines($i)
	}
	
	
	if {$i == $ntalklines} {set scrolllock 0}
    } elseif !$scrolllock {
	
	for {set y $talktop; set i $talklineattop} 		{$y <= $talkbottom && $i < $dtalklines} 		{incr i; incr y} {


	if {$i >= $oldntalklines} {drawtalkline $y $talklines($i)}
	}
    }
    set oldntalklines $ntalklines
    reset_cursor_position
    draw_on_current_display -
}

proc talkscroll {n} {
    global scrolllock dtalklines

    incr dtalklines $n
    set scrolllock 1
    drawtalkregion 1
}

proc turn_off_scrolllock {} {
    talkscroll 1000000
}

proc talkregion {top bottom} {
    global talktop talkbottom talklineattop scrolllock

    set talktop $top
    set talkbottom $bottom
    drawtalkregion 1
}

proc drawtalkline {y s} {
    anchor "0 $y"
    str "$s\n"
}






proc debugmsg {s} {
    global debugprinting floater_silent

    set old $debugprinting
    set debugprinting 1
    talkmsg $s
    set debugprinting $old

}





proc setcursor {cursor w} {
# 972 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
}

line 1173

proc unsetcursor {w} {
# 988 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
}

proc patientcursor {} {
    global cursorlevel

    if {[incr cursorlevel] == 1} {setcursor watch .}
}

proc normalcursor {} {
    global cursorlevel

    if {[incr cursorlevel -1] == 0} {unsetcursor .}
}
set cursorlevel 0
	
# 1022 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
tryset tabletimeout 600



tryset tablereannounce 90





set receiveiamalivelist {}
set sendiamalivelist {}


tryset sendiamaliveinterval 40000


tryset receiveiamaliveinterval 20000





tryset iamalivetimeout 450


proc shouldreceiveiamalive {conn} {
    global receiveiamalivelist

    set receiveiamalivelist [linsert $receiveiamalivelist 0 $conn]
}

proc shouldnotreceiveiamalive {conn} {
    global receiveiamalivelist

    catch {
	set i [lsearch $receiveiamalivelist $conn]
	set receiveiamalivelist [lreplace $receiveiamalivelist $i $i]
    }
}

proc shouldsendiamalive {conn} {
    global sendiamalivelist

    set sendiamalivelist [linsert $sendiamalivelist 0 $conn]
}

proc shouldnotsendiamalive {conn} {
    global sendiamalivelist

    catch {
	set i [lsearch $sendiamalivelist $conn]
	set sendiamalivelist [lreplace $sendiamalivelist $i $i]
    }
}

proc sendiamalives {} {
    global sendiamalivelist sendiamaliveinterval

    after $sendiamaliveinterval sendiamalives
    foreach conn $sendiamalivelist {
	debugmsg "Sending iamalive to $conn"
	catch {FloaterSend $conn *alive*}
    }
}

proc checkreceiveiamalive {conn} {
    global iamalivetimeout timeofmostrecent floaterclock


    catch {
	debugmsg "seconds since most recent msg on $conn: [expr ($floaterclock - $timeofmostrecent($conn))]"
	if [expr ($floaterclock - $timeofmostrecent($conn)) > $iamalivetimeout] 		{floatertimeout $conn}

    }
}

proc checkreceiveiamalives {} {
    global receiveiamalivelist receiveiamaliveinterval

    after $receiveiamaliveinterval checkreceiveiamalives
    foreach conn $receiveiamalivelist { checkreceiveiamalive $conn }
}

sendiamalives
checkreceiveiamalives







gset MyTurnTimer -99
tryset MyTurnTimerCountdown 20
proc startMyTurnTimer {} {
    global MyTurnTimerCountdown MyTurnTimer
    set MyTurnTimer $MyTurnTimerCountdown
}

proc MyTurnTimerRing {} {
    global showingauction
    if $showingauction {
	showauction 0
	showauction 1
	startMyTurnTimer
    }
}

proc stopMyTurnTimer {} {
    global MyTurnTimer
    set MyTurnTimer -99
}

line 1335



proc floaterclockbump {} {
    global floaterclock MyTurnTimer

    incr floaterclock
    if {$MyTurnTimer > 0} {if {[incr MyTurnTimer -1] == 0} MyTurnTimerRing}
    after 1000 floaterclockbump


    if {[expr $floaterclock % 3] == 0} {command {}}
}

after 1000 floaterclockbump


proc countdown {x} {
    global $x

    if {[set $x] > 0} then "after 1000 \"countdown $x\"" else return
    incr $x -1
}

proc reset_rejoinnow {} {
    global rejoinclock rejoinclockincrement

    set rejoinclock 0
    set rejoinclockincrement 1
}

proc rejoinnow {} {
    global rejoinclock rejoinclockincrement

    if {$rejoinclock <= 0} then {
	if {$rejoinclockincrement < 1800} 		{set rejoinclockincrement [expr 2 * $rejoinclockincrement]}

	set rejoinclock $rejoinclockincrement
	countdown rejoinclock
	return 1
    } else {return 0}
}

proc reset_find_rho {} {
    global rhoclock rhoclockincrement

    set rhoclock 0
    set rhoclockincrement 1
}

proc findrhonow {} {
    global rhoclock rhoclockincrement

    if {$rhoclock <= 0} then {
	if {$rhoclockincrement < 1800} 		{set rhoclockincrement [expr 2 * $rhoclockincrement]}

	set rhoclock $rhoclockincrement
	countdown rhoclock
	return 1
    } else {return 0}
}

reset_find_rho
reset_rejoinnow


gset autodealing 0

proc autonewdeal {} {
    global autonewdeal_seconds autodealing

    if $autodealing return
    if {$autonewdeal_seconds >= 0} {
	set autodealing 1
	after [expr 1000 * $autonewdeal_seconds] {
	global autodealing
	
	if $autodealing {
		set autodealing 0
		if {$autonewdeal_seconds >= 0} {command autodeal_now}
	} else {





	}
	}
    }
}





global updateloc_seconds
tryset updateloc_seconds 300

proc updateloc {} {
    global updateloc_seconds

    after [expr 1000 * $updateloc_seconds] updateloc
    catch {command iupdatelocation}
}

updateloc
# 1253 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
gset should_defer 0


proc command args {
    global should_defer

    if $should_defer {deferpush "commandn $args"} else {eval "commandn $args"}
}
# 1272 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
proc floaterreceive {msg conn} {
    global should_defer timeofmostrecent floaterclock

    set timeofmostrecent($conn) $floaterclock

    if {$msg == "*alive*"} return

    if $should_defer {
	deferpush "floaterreceiven {$msg} {$conn}"
    } else {
	floaterreceiven $msg $conn
    }
}

proc talk args {
    global should_defer

    if $should_defer {deferpush "talkn $args"} else {eval "talkn $args"}
}

proc FloaterClose args {
    global should_defer

    if $should_defer {deferpush "FloaterClosen $args"} 	else {eval "FloaterClosen $args"}

}
# 1308 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
proc floatertimeout args {
    global should_defer

    if $should_defer {deferpush "floatertimeoutn $args"} 	else {eval "floatertimeoutn $args"}

}







proc defer {n} {
    global should_defer

    if {[incr should_defer $n] == 0} {
	while {![deferempty]} {eval [deferpop]}
    }
}

gset deferqueuelo 0
gset deferqueuehi 0

proc deferempty {} {
    global deferqueuehi deferqueuelo

    return [expr $deferqueuelo == $deferqueuehi]
}

proc deferpush {s} {
    global deferqueue deferqueuehi

    set deferqueue($deferqueuehi) $s
    incr deferqueuehi



}

proc deferpop {} {
    global deferqueue deferqueuelo

    set temp $deferqueue($deferqueuelo)
    unset deferqueue($deferqueuelo)
    incr deferqueuelo



    return $temp
}

line 1548
# 1369 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
set executing_index 0

proc Floater_execute {file} {
    global executing_index executing_command
    if {[set n [gets $file s]] >= 0} {
	if {$n > 0} {
	deferpush "show_executing [incr executing_index]; Floater_execute $file"
	set executing_command($executing_index) $s
	} else {
	
	Floater_execute $file
	}
	return
    }
    catch {close $file}
}

proc show_executing {n} {
    global executing_command

    talkmsg "Execute: $executing_command($n)"
    commandn $executing_command($n)
    unset executing_command($n)
}




proc untabify {s} {
    if [regexp {([^\t]*)\t(.*)} $s whole left right] {
	set i [string length $left]
	while 1 {
	set right " $right"
	incr i
	if [expr $i % 8 == 0] {return [untabify $left$right]}
	}
    } else {return $s}
}

proc truncate {s {n 80}} {
    if {[string length $s] > $n} {
	return [string range $s 0 [expr $n - 1]]
    } else {
	return $s
    }
}

proc unbraceclean {s} {
    regsub -all {\\(\[|\]|\{|\})} $s {\1} x
    return $x
}

proc beginnewcc {direction} {
    global newcc newccline newccignoring

    set newccline 0
    set newccignoring 0
    set newcc $direction
}

proc fetchnewcc {ccloadfile} {
    while {[gets $ccloadfile s] >= 0} {addnewcc $s 0}
}

proc addnewcc {s {bracecleaned 1}} {
    global newcc newccline cc newccignoring

    set s [untabify [truncate $s]]
    if $bracecleaned {set s [unbraceclean $s]}
    if {$newccline == 40} {set newccignoring 1; return}
    set cc($newcc,[incr newccline]) $s
}

proc endnewcc {} {
    global newcc newccline newccignoring cclines

    set cclines($newcc) $newccline

    if $newccignoring {
	return "Warning: Ignored lines beyond the first 40"
    } else {
	return ""
    }
}



proc ccstr {direction} {
    global cc cclines

    set s ""
    catch {
	if {$cclines($direction) < 1} {return ""}
	set s $cc($direction,1)
	for {set i 2} {$i <= $cclines($direction)} {incr i} {
	set s "$s\t$cc($direction,$i)"
	}
    }
    return $s
}

gset lastrange ""
proc inrange {n range} {
    global lastrange lastrangelow lastrangehigh

    if {$range != $lastrange} {
	set lastrange $range
	if [regexp {^([0-9]+)-([0-9]+)$} $range x lastrangelow lastrangehigh] {
	
	} elseif [regexp {^([0-9]+)$} $range lastrangelow] {
	set lastrangehigh $lastrangelow
	} elseif [regexp {^([0-9]+)-$} $range x lastrangelow] {
	set lastrangehigh 1000000
	} elseif [regexp {^-([0-9]+)$} $range x lastrangehigh] {
	set lastrangelow -1000000
	} else {error "Invalid range: $range"}
    }
    expr ($n >= $lastrangelow) && ($n <= $lastrangehigh)
}

proc ccdump {direction {range 1-}} {
    global cc cclines

    for {set i 1} {$i <= $cclines($direction)} {incr i} {
	if [inrange $i $range] {
	talkmsg $cc($direction,$i)
	}
    }
}

proc ccsave {file direction} {
    global cc cclines

    for {set i 1} {$i <= $cclines($direction)} {incr i} {
	puts $file $cc($direction,$i)
    }
    close $file
}

proc getccline {direction line} {
    global cc cclines

    if ![info exists cclines($direction)] {return ""}
    if {$line <= $cclines($direction)} {
	return $cc($direction,$line)
    } else {
	return ""
    }
}

line 1697







proc reverse_init {} {
    global reverse_n

    set reverse_n 0
}

proc reverse_print {s} {
    global reverse_n reverse_lines

    set reverse_lines($reverse_n) $s
    incr reverse_n
}

proc reverse_done {} {
    global reverse_n reverse_lines

    while {[incr reverse_n -1] >= 0} {
	talkmsg $reverse_lines($reverse_n)
	unset reverse_lines($reverse_n)
    }
}


proc Floater_login {} {
    global loginname loginpassword newbie

    toplevel .login

    frame .login.left
    frame .login.right
    frame .login.bottom

    button .login.bottom.cancel -text "Cancel" 	-command {set loginname ""; set loginpassword ""; destroy .login}

    button .login.bottom.clear -text "Clear" 	-command {set loginname ""; set loginpassword ""; focus .login.right.n}

    button .login.bottom.ok -text "OK" 	-command {destroy .login}


    proc newbietr {name el op} {
	global pw_or_email newbie

	if $newbie {set pw_or_email "Email address: "} 		{set pw_or_email "Password: "}

    }

    checkbutton .login.new -text "New User" -variable newbie
    trace variable newbie w newbietr
    if [info exists newbie] {set newbie $newbie} {set newbie 0}

    label .login.left.n -text "Name: "
    label .login.left.p -textvariable pw_or_email -width 13

    entry .login.right.n -bd 2 -relief sunken -textvariable loginname
    focus .login.right.n
    entry .login.right.p -bd 2 -relief sunken -textvariable loginpassword

    pack .login.bottom.cancel .login.bottom.clear .login.bottom.ok 	-side left -expand yes -fill x -padx 3m -pady 2m

    pack .login.left.n .login.left.p
    pack .login.right.n .login.right.p
    pack .login.bottom -side bottom
    pack .login.new -side bottom -pady 2m
    pack .login.left -side left -fill x -expand yes
    pack .login.right .login.right -side right -fill x -expand yes
    wm title .login "Floater login"

    bindsetup .login.right.n .login.right.p {focus .login.right.p}
    bindsetup .login.right.p .login.right.n {destroy .login}
    bind .login.right.n \\ {set loginname ""}

    grab set .login
    tkwait window .login
    trace vdelete newbie w newbietr
    set loginname [string trim $loginname]
    catch focus_cmdline
    if $newbie {return "N$loginname\\$loginpassword"} 	{return "O$loginname\\$loginpassword"}

}

proc Floater_changepw {} {
    global changepwname oldpassword newpassword

    toplevel .changepw

    frame .changepw.left
    frame .changepw.right
    frame .changepw.bottom


    button .changepw.bottom.cancel -text "Cancel" 	-command {set changepwname ""; set oldpassword ""; 	set newpassword ""; destroy .changepw}


    button .changepw.bottom.clear -text "Clear" 	-command {set changepwname ""; set oldpassword ""; 	set newpassword ""; focus .changepw.right.n}


    button .changepw.bottom.ok -text "OK" 	-command {destroy .changepw}


    label .changepw.left.n -text "Name: "
    label .changepw.left.o -text "Old password: "
    label .changepw.left.p -text "New password: "

    entry .changepw.right.n -bd 2 -relief sunken -textvariable changepwname
    entry .changepw.right.o -bd 2 -relief sunken -textvariable oldpassword
    entry .changepw.right.p -bd 2 -relief sunken -textvariable newpassword

    pack .changepw.bottom.cancel .changepw.bottom.clear .changepw.bottom.ok 	-side left -expand yes -fill x -padx 3m -pady 2m

    pack .changepw.left.n .changepw.left.o .changepw.left.p
    pack .changepw.right.n .changepw.right.o .changepw.right.p
    pack .changepw.bottom -side bottom
    pack .changepw.left -side left -fill x -expand yes
    pack .changepw.right .changepw.right -side right -fill x -expand yes
    wm title .changepw "change password"

    bindsetup .changepw.right.n .changepw.right.o {focus .changepw.right.o}
    bindsetup .changepw.right.o .changepw.right.p {focus .changepw.right.p}
    bindsetup .changepw.right.p .changepw.right.n {destroy .changepw}
    bind .changepw.right.n \\ {set changepwname ""}

    grab set .changepw
    tkwait window .changepw
    catch {focus .cmd; focus .cmd.talk}
    return "$changepwname\\$oldpassword\\$newpassword"
}



proc Floater_bell {} { catch { bell } }

showEntryLines both

line bottom
# 35 "/local/scratch/floater-1.4.15/tclcode/floatert.deq" 2
