# 1 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
# 63 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
if {[info tclversion] < 8.0 || $tk_version < 8.0} {
    puts stderr "You have compiled Floater with Tcl [info tclversion]/Tk $tk_version."
    puts stderr "You must recompile with Tcl/Tk 8.0 or higher."
    exit 1
}


set debugstartup 0

if $debugstartup {


    proc line args {




        puts "Line: $args"

    }
} else {
    proc line args {}
}







proc fontfamilies {} {
    set normalfonts ""
    foreach f [font families] {
	if [string compare "" [string trim $f " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"]] {

        } else {
	set normalfonts [concat $normalfonts [list $f]]
	}
    }
    return $normalfonts
}



proc findfontfam {pattern} {
    foreach f [fontfamilies] {
	if [regexp -nocase $pattern $f] { return $f }
    }
    return ""
}






proc searchfont {fam size} {
    set fam [findfontfam $fam]
    if {$fam == ""} { set fam times }
    set r "*-times-medium-r-normal--*-[expr 10 * $size]-*-*-*-*-*-*"
    catch {
	set f [font create]
	font config $f -family $fam -size $size
	if {[font measure $f "MMMM"] != [font measure $f " "]} {
	set r [list $fam $size]
	}
    }
    catch {
	font delete $f
    }
    return $r
}

proc times {size} {
    foreach pat {{times.* roman} {new century schoolbook} palatino bookman} {
	if {[set t [searchfont $pat $size]] != ""} { return $t }
    }

    return "Times $size"
}


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
# 50 "/local/scratch/floater-1.4.15/tclcode/logo.deq"
proc about {{timeout 0}} {
    global logofilename about_text macintosh

    toplevel .welcome -bg black

    set f [times 18]
    set c "catch {destroy .welcome}"
    if ![info exists logofilename] { set logofilename Floaterlogo.gif }
    if [catch {
	if {![file readable $logofilename]} {
	set dirs /usr/local/lib/floater1.4.15
	set dirs "$dirs [file dirname [info nameofexecutable]]"
	foreach dir $dirs {
		gset logofilename [file join $dir Floaterlogo.gif]
		if [file readable $logofilename] break
	}
	}
	if [catch {image create photo im -file $logofilename}] 		{image create photo im -file [file rootname $logofilename]}

	label .welcome.l -image im
    }] then {
	label .welcome.l -text Floater
	
    }

    label .welcome.b -justify center -text "$about_text"

    foreach w {.welcome.l .welcome.b} {
	$w config -border 0 -font $f -bg black -fg white 		-highlightthickness 0 -relief flat

	bind $w <1> $c
    }


    pack .welcome.l .welcome.b

    if {$timeout > 0} { after $timeout {catch {destroy .welcome}} }
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



    global display_text_N

    if ![info exists display_text_N] {set display_text_N 0}
    set w .display_text[incr display_text_N]
    toplevel $w
    wm title $w $name
    set f [frame $w.f]
    text $f.text -relief raised -yscrollcommand "$f.scroll set" 	-width 77 -font $fixedfont

    $f.text insert end $text
    pack [scrollbar $f.scroll -command "$f.text yview"] 	-side right -fill y

    pack $f.text -side left -fill both -expand y
    button $w.b -text "Dismiss" -command "destroy $w"
    pack $w.b -side bottom -pady 2
    pack $f -side top -fill both -expand y

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
# 262 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
proc talkmsg {s} {
    global debugtext debugprinting showerrors

    if $debugprinting then { set w $debugtext } else { set w [talktext] }

    if {!$showerrors && [regexp -nocase error $s]} return

    catch {
	$w insert end "$s\n"
	$w yview -pickplace end
    }
}


proc floatererror {s} { talkmsg "ERROR: $s" }





if {[catch {source $startupfile} err] 	&& ![regexp -nocase {no such file} $err]} {




    puts stderr $err

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





gset screenheight [winfo screenheight .]
gset screenwidth [winfo screenwidth .]
gset effectiveheight [expr $screenheight - 100]

tryset geometry_specified 0
catch {wm title . "Floater"}
catch {wm minsize . 1 1}





set large [expr $effectiveheight > 750]


if $large {
    tryset _suitfont(l) "Symbol 24"
    tryset _cardfont(l) [times 24]
    tryset _bbfont(l) [times 18]
    tryset _NTfont(l) [times 18]
    tryset _suitfont(m) "Symbol 18"
    tryset _cardfont(m) [times 18]
    tryset _bbfont(m) [times 12]
    tryset _NTfont(m) [times 14]
    tryset _suitfont(s) "Symbol 12"
    tryset _cardfont(s) [times 12]
    tryset _bbfont(s) [times 8]
    tryset _NTfont(s) [times 10]
    tryset _namefont(l) "*-times-bold-r-normal--*-140-*-*-*-*-*-*"
    tryset _namefont(m) "*-times-bold-r-normal--*-140-*-*-*-*-*-*"
    tryset _namefont(s) "*-times-bold-r-normal--*-120-*-*-*-*-*-*"
} else {
    tryset _suitfont(l) "Symbol 18"
    tryset _cardfont(l) [times 18]
    tryset _bbfont(l) [times 18]
    tryset _NTfont(l) [times 14]
    tryset _suitfont(m) "Symbol 12"
    tryset _cardfont(m) [times 12]
    tryset _bbfont(m) [times 12]
    tryset _NTfont(m) [times 10]
    tryset _suitfont(s) "Symbol 10"
    tryset _cardfont(s) [times 10]
    tryset _bbfont(s) [times 8]
    tryset _NTfont(s) [times 8]
    tryset _namefont(l) "*-times-bold-r-normal--*-120-*-*-*-*-*-*"
    tryset _namefont(m) "*-times-bold-r-normal--*-120-*-*-*-*-*-*"
    tryset _namefont(s) "*-times-bold-r-normal--*-100-*-*-*-*-*-*"
}



proc refont {widget font} {
    global a_$font fonts widget_to_font


    if {[info exists widget_to_font($widget)] && 	$widget_to_font($widget) != $font} {

	set oldfont $widget_to_font($widget)
	global a_$oldfont

	catch {eval "unset a_$oldfont\($widget)"}
    }

    eval "set a_$font\($widget) 1"
    set widget_to_font($widget) $font
    set fonts($font) 1
}


proc updatefont {font} {
    global a_$font $font


    eval "set new $$font"
    foreach w [array names a_$font] {

	if [catch {$w configure -font $new}] {eval "unset a_$font\($w)"}
    }
}


proc updateallfonts {} {
    global fonts

    foreach font [array names fonts] {
	updatefont $font
    }
}
# 431 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
set talkfontsize 8
catch {
    if {$screenheight >= 600 || 	[regexp {.*x([0-9]+)} [wm geometry .] a wmheight] && 	$wmheight >= 500} { incr talkfontsize 1 }


    if {$screenheight >= 700 || 	[regexp {.*x([0-9]+)} [wm geometry .] a wmheight] && 	$wmheight >= 600} { incr talkfontsize 1 }


    if {$screenheight >= 800 || 	[regexp {.*x([0-9]+)} [wm geometry .] a wmheight] && 	$wmheight >= 700} { incr talkfontsize 2 }


    if {$screenheight >= 900 || 	[regexp {.*x([0-9]+)} [wm geometry .] a wmheight] && 	$wmheight >= 800} { incr talkfontsize 2 }


}



gset talkfontfam [findfontfam "New Century Schoolbook"]
if { $talkfontfam == "" } {
    gset talkfontfam [findfontfam "New Cent.* S"]
}
if { $talkfontfam == "" } {
    gset talkfontfam Times
}


proc updatetalkfont {} {
    global talkfont talkfontsize talkfontfam fontfam fontsiz 	debugtext cmdlinefont cmdlinelabelfont


    set talkfont [set f [list $talkfontfam $talkfontsize]]
    [talktext] configure -font $f
    [talktext] see end
    catch {
	$debugtext configure -font $f
	$debugtext see end
    }
    set cmdlinefont $f
    set cmdlinelabelfont $f
    updatefont cmdlinefont
    updatefont cmdlinelabelfont


    set fontfam $talkfontfam
    set fontsiz $talkfontsize
}

line 432

proc updatetalkfontsize {size} {
    global talkfontsize

    if ![regexp {[1-9][0-9]*} $size] {
	floatererror "`talkfontsize' expects one argument: a positive integer"
    } elseif {$talkfontsize != $size} {
	set talkfontsize $size
	updatetalkfont
    }
}

proc updatetalkfontfam {fam} {
    global talkfontfam

    if ![regexp {[a-zA-Z0-9]} $fam] return
    if {$talkfontfam != $fam} {
	set talkfontfam $fam
	updatetalkfont
    }
}

gset talkfont [list $talkfontfam $talkfontsize]




proc setfontsize {size} {
    global suitfont cardfont NTfont passfont doublefont redoublefont namefont 	auctionlabelfont auctionbbfont cmdlinefont cmdlinelabelfont 	_suitfont _cardfont _bbfont _NTfont _namefont talkfont 	buttoncardoptions buttonsuitoptions macintosh




    set namefont $_namefont($size)
    set suitfont $_suitfont($size)
    set cardfont $_cardfont($size)
    set bbfont $_bbfont($size)
    set NTfont $_NTfont($size)
    set passfont $NTfont
    set doublefont $cardfont
    set redoublefont $doublefont
    set auctionlabelfont $passfont
    set auctionbbfont $bbfont
    set cmdlinefont $talkfont
    set cmdlinelabelfont $cmdlinefont
    set buttoncardoptions "-font \{$cardfont\} -padx [px] -relief flat"
    set buttonsuitoptions "-font \{$suitfont\} -padx [pxx] -relief flat"
}


proc px {} {
    global macintosh
    expr $macintosh ? 3 : 1
}

proc pxx {} {
    global macintosh
    expr $macintosh ? 4 : 2
}


setfontsize m

tryset NTtext NT
tryset passtext Pass
tryset doubletext X
tryset redoubletext XX

tryset auctionlabel "The Bidding:"
tryset auctionnamewidth 12

gset suitchar(0) [set club "\247"]
gset suitchar(1) [set diamond "\250"]
gset suitchar(2) [set heart "\251"]
gset suitchar(3) [set spade "\252"]
proc s {} {global spade; return "$spade -fg black"}
proc h {} {global heart; return "$heart -fg red"}
proc d {} {global diamond; return "$diamond -fg red"}
proc c {} {global club; return "$club -fg black"}


tryset framesuitoptions ""




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

# 1 "/local/scratch/floater-1.4.15/tclcode/options_GUI.deq" 1
# 46 "/local/scratch/floater-1.4.15/tclcode/options_GUI.deq"
set separateTalk_ [set talk_is_sep 0]
proc separateTalk {{toggle 0}} {
    global talk_is_sep separateTalk_



    if $toggle {set separateTalk_ [expr !$separateTalk_]}
    if {$talk_is_sep == $separateTalk_} return
    set talk_is_sep $separateTalk_
    gui_separateTalk $talk_is_sep
}	





set hideMatrix_ [set matrix_hidden_during_auction 0]
proc hideMatrixDuringAuction {bool} {
    global hideMatrix_ matrix_hidden_during_auction needAuctionUpdate


    set hideMatrix_ $bool
    if {$hideMatrix_ == $matrix_hidden_during_auction} return
    set matrix_hidden_during_auction $hideMatrix_
    set needAuctionUpdate 1
}




gset entryLinesShown_ "talk"

proc showEntryLines {linesToShow} {
    global entryLinesShown_ toplevel

    set entryLinesShown_ $linesToShow
    set cmd $toplevel.cmd

    if [string equal $entryLinesShown_ talk] {
	set show_talk 1
	set show_cmd 0
    }
    if [string equal $entryLinesShown_ command] {
	set show_talk 0
	set show_cmd 1
    }
    if [string equal $entryLinesShown_ both] {
	set show_talk 1
	set show_cmd 1
    }
    if {! [info exists show_talk]} {
	talkmsg {argument should be "talk", "command", or "both"}
	return
    }

    pack forget $cmd.f.labelc $cmd.f.labelt
    pack forget $cmd.talk $cmd.cmdline

    if $show_cmd {
	pack $cmd.f.labelc -anchor e
	pack $cmd.cmdline -fill x
    }

    if $show_talk {
	pack $cmd.f.labelt -anchor e
	pack $cmd.talk -fill x
    }

    focus_cmdline
}


set hideCommandLine_ 0
proc hideCommandLine {{toggle 0}} {
    global hideCommandLine_

    if $toggle {set hideCommandLine_ [expr !$hideCommandLine_]}
    if $hideCommandLine_ {
	pack forget .cmd.cmdline
	pack forget .cmd.f.labelc
    } else {
        pack .cmd.cmdline -before .cmd.talk -fill x -side top -expand yes
	pack .cmd.f.labelc -before .cmd.f.labelt -side top -anchor e
    }
}






set deiconifyIfBeeped_ 1
proc deiconifyIfBeeped {{toggle 0}} {
    global deiconifyIfBeeped_


    if $toggle {set deiconifyIfBeeped_ [expr !$deiconifyIfBeeped_]}
}

proc Floater_deiconify {} {
    global deiconifyIfBeeped_

    catch {
	if {$deiconifyIfBeeped_ && [wm state .] == "iconic"} {wm deiconify .}
    }
}






if {$effectiveheight > 770} {
    set auction_hide_time -1
} else {
    set auction_hide_time 10
}





global gui_compact_
set gui_compact_ 1
# 576 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
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


proc tpcard {w} {

    if ![string match *ello* [$w config -fg]] {
	$w config -fg yellow
	
    } else {
	$w config -fg black
	
    }
}
# 105 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc suit {f cards suit} {
# 119 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
    global buttoncardoptions buttonsuitoptions removecard togglepassedaction

    set buttons {}
    for {set i [expr [string length $cards] - 1]} {$i >= 0} {incr i -1} {
	set card [string index $cards $i]
	set comm "-command \"command $suit$card\""
	set newbutton [eval "button $f.$suit.$suit$card 		$buttoncardoptions -text $card $comm"]

	refont $newbutton cardfont
	set buttons [linsert $buttons 0 $newbutton]
	set removecard([string toupper $suit$card]) "destroy $newbutton"
	set togglepassedaction([string toupper $suit$card]) "tpcard $newbutton"
    }





    refont [eval "label $f.$suit.suit $buttonsuitoptions -text [$suit]"] suitfont
    eval "pack $f.$suit.suit $buttons -side left"

}



proc hand {f s h d c} {

    global framesuitoptions


    if [winfo exists $f.name] {
	foreach i {name s h d c} {
	foreach child [winfo children $f.$i] {
		catch {destroy $child}
	}
	}
    } else {
	catch {destroy $f.name $f.s $f.h $f.d $f.c}
	frame $f.name
	frame $f.s
	frame $f.h
	frame $f.d
	frame $f.c
	pack $f.name -side top -anchor w
	eval "pack $f.s $f.h $f.d $f.c -side top -anchor w $framesuitoptions"
    }





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






	set path [matrix_widget].middle.box.$who
	catch {pack forget $path.suit $path.card}

    }
}




proc showplay {player suit card} {
# 285 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
    global cardfont suitfont

    set path [matrix_widget].middle.box.$player
    catch {pack forget $path.suit $path.card} c
    if {$c != ""} {
	
	refont [label $path.suit -font $suitfont -borderwidth 0] suitfont
	refont [label $path.card -font $cardfont -borderwidth 0] cardfont
    }
    pack $path.suit $path.card -side left

    if {$suit != "?"} {
	eval "$path.suit configure -text [$suit] -font \{$suitfont\}"
	refont $path.suit suitfont
    } else {
	$path.suit configure -text ""
    }
    $path.card configure -text [string toupper $card]

}





proc showbid {player level strain} {
# 326 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
    drawbid [matrix_widget].middle.box.$player $level $strain

}
# 359 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
proc suitcolor {suit} {
    if {$suit == "h"} {
        return red
    } elseif {$suit == "d"} {
        return red
    } else {
        return black
    }
}

proc drawbid {path level strain} {
    global NTtext NTfont cardfont suitfont 	passtext passfont doubletext doublefont redoubletext redoublefont 	drawnbids






    if {! [info exists drawnbids] } {
	set drawnbids(numbids) 0
    }
    set drawnbids($drawnbids(numbids),path) $path
    set drawnbids($drawnbids(numbids),level) $level
    set drawnbids($drawnbids(numbids),strain) $strain
    incr drawnbids(numbids)
	

    catch {pack forget $path.strain $path.level} c
    if {$c != ""} {
	
	refont [label $path.level -font $suitfont -borderwidth 0] suitfont
	canvas $path.strain -width 20 -height 14 -borderwidth 0 -relief flat -highlightthickness 0
    }
    pack $path.level $path.strain -side left

    $path.strain delete all
    pack forget $path.strain

    if {$strain == "x"} {
	$path.level configure -text $doubletext -font $doublefont -fg red -width [string length $passtext]
	refont $path.level doublefont
    } elseif {$strain == "xx"} {
	$path.level configure -text $redoubletext -font $redoublefont -fg blue -width [string length $passtext]
	refont $path.level redoublefont
    } elseif {$strain == "p"} {
	$path.level configure -text $passtext -font $passfont -fg black -width [string length $passtext]
	refont $path.level passfont
    } elseif {$strain == "-"} {
	$path.level configure -text ""
    } elseif {$strain == "?"} {
	$path.level configure -text "?" -fg black -width 1
    } else {
	$path.level configure -text $level -font $suitfont -fg [suitcolor $strain] -width [string length $level]
	refont $path.level suitfont
	$path.strain create image 0 0 -image "$strain-12" -anchor nw
	pack $path.strain -side left
    }
}

proc gui_newauction {} {
    global drawnbids max_auction_line

    foreach child [winfo children [auction_widget]] {
        if ![regexp name $child] {catch {destroy $child}}
    }

    set drawnbids(numbids) 0
    set max_auction_line -1
}




proc copybids {new_toplevel} {
    global toplevel drawnbids max_auction_line

    if {! [info exists drawnbids]} {
	
	return
    }


    set saved_toplevel $toplevel
    set toplevel $new_toplevel


    if [info exists max_auction_line] {
	set old_max_auction_line $max_auction_line
    } else {
	set old_max_auction_line -1
    }


    set bids(numbids) $drawnbids(numbids)
    for {set i 0} {$i < $drawnbids(numbids)} {incr i} {
	set bids($i,path) $drawnbids($i,path)
	set bids($i,level) $drawnbids($i,level)
	set bids($i,strain) $drawnbids($i,strain)
    }


    gui_newauction




    for {set row 0} {$row <= $old_max_auction_line} {incr row} {
	foreach column {0 1 2 3} {
	auction_cell $column $row
	}
    }



    for {set i 0} {$i < $bids(numbids)} {incr i} {
	set lastchar [expr [string length $saved_toplevel] - 1]
	set newpath [string replace $bids($i,path) 0 $lastchar $new_toplevel]

	drawbid $newpath $bids($i,level) $bids($i,strain)
    }



    set toplevel $saved_toplevel
}




proc setname {player compassdir name} {
# 504 "/local/scratch/floater-1.4.15/tclcode/matrix0.deq"
    global namefont mframe playername position
    set playername($player) $name
    set position($player) $compassdir
    set f $mframe($player).name.label
    if {[string first "(" $name] == -1} {set name "$name ($compassdir)"}
    catch {$f configure -text $name} c
    if {$c != ""} {
	
	refont [label $f -font $namefont -text $name] namefont
	pack $f -pady 0
    }

}
# 577 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
# 1 "/local/scratch/floater-1.4.15/tclcode/matrix.deq" 1
# 80 "/local/scratch/floater-1.4.15/tclcode/matrix.deq"
# 1 "/local/scratch/floater-1.4.15/tclcode/matrixsize.deq" 1
# 39 "/local/scratch/floater-1.4.15/tclcode/matrixsize.deq"
proc smallmatrix {c b v {m 160}} {
    global handx handy canv namex namey suitgap ourcardgap theircardgap vgap 	pardheight matrixheight matrixwidth lhowidth rhowidth 	self_matrix_gap bottom_cutoff rhomaxx





    gset pardheight 100
    gset matrixwidth 200
    gset lhowidth 193
    gset rhowidth $lhowidth
    gset self_matrix_gap 55
    gset suitgap 45
    gset ourcardgap 23
    gset theircardgap 19
    gset rhomaxx($c) 600
    gset cardrectvgap 4


    set bottom_cutoff $b
    set vgap $v
    set matrixheight $m
    set matrixtop $pardheight
    set matrixl $lhowidth
    set matrixbot [expr $pardheight + $matrixheight]
    set matrixr [expr $lhowidth + $matrixwidth]
    set canv($c,matrix) "$matrixl $matrixtop $matrixr $matrixbot"

    set handx($c,self) [expr $matrixl + $matrixwidth / 2]
    set handx($c,pard) $handx($c,self)
    set handy($c,pard) [expr $matrixtop - 50]
    set handy($c,self) [expr $matrixbot + $self_matrix_gap]
    set handx($c,lho) 50
    set handy($c,lho) [expr $pardheight + 40]
    set handx($c,rho) [expr $matrixr + 40]
    set handy($c,rho) $handy($c,lho)

    set canv($c,height) [expr $handy($c,self) + 35 - $bottom_cutoff]
    set canv($c,width) [expr $lhowidth + $matrixwidth + $rhowidth]

    foreach p {lho self pard rho} {
	set namex($p) $handx($c,$p)
	set namey($p) [expr $handy($c,$p) - 43]
    }
}

proc tinymatrix {c} {smallmatrix $c 32 26 148}


proc bigmatrix {c} {
    global handx handy canv namex namey suitgap ourcardgap theircardgap vgap 	pardheight matrixheight matrixwidth lhowidth rhowidth 	self_matrix_gap bottom_cutoff rhomaxx




    gset handx($c,self) 320
    gset handy($c,self) 370
    gset handx($c,pard) $handx($c,self)
    gset handy($c,pard) 50
    gset handx($c,lho) 50
    gset handy($c,lho) 150
    gset handx($c,rho) 490
    gset handy($c,rho) $handy($c,lho)
    gset rhomaxx($c) 650

    gset canv($c,matrix) "200 100 440 300"

    gset canv($c,height) 415
    gset canv($c,width) 660

    foreach p {lho self pard rho} {
	gset namex($p) $handx($c,$p)
	gset namey($p) [expr $handy($c,$p) - 43]
    }

    gset suitgap 50
    gset ourcardgap 25
    gset theircardgap 20
    gset vgap 38

    gset cardrectvgap 5
}







proc mdealtest {{n -1}} {
    set q(0) {AKQ AKQ AKQ AKQJ JT9 JT9 JT9 T987 876 876 876 6543 5432 5432 5432 2}
    set q(1) {AKQJT9 AKQJT98 AKQJT987 AKQJT9876 {} {} {} {} {} {} {} {} {} {} {} {}}
    set q(2) {{} {} {} {} AKQJT9 AKQJT98 AKQJT987 AKQJT9876 {} {} {} {} {} {} {} {}}
    set q(3) {{} {} {} {} {} {} {} {} AKQJT9 AKQJT98 AKQJT987 AKQJT9876 {} {} {} {}}
    set q(4) {{} {} {} {} {} {} {} {} {} {} {} {} AKQJT9 AKQJT98 AKQJT987 AKQJT9876}
    set q(5) {AKQJT987654 Q82 J J6 {} {} {} {} {} {} {} {} {} {} {} {}}
    set q(6) {{} {} {} {} AKQJT987654 Q82 J J6 {} {} {} {} {} {} {} {}}
    set q(7) {{} {} {} {} {} {} {} {} AKQJT987654 Q82 J J6 {} {} {} {}}
    set q(8) {{} {} {} {} {} {} {} {} {} {} {} {} AKQJT987654 Q82 J J6}
    if {$n >= 0} {catch {eval "fulldeal $q($n)"}} {
	for {set i 1} {$i <= 8} {incr i} {
	after [expr $i * 5000] fulldeal $q($i)
	}
    }
}


proc mt {{n 0}} {
    mdealtest $n

    showplay self c q
    showplay pard s k
    showplay lho d a
    showplay rho h j
    sillynames
}

proc mtest {{d {}}} {
    catch {destroy .tc}
    toplevel .tc
    canvsetup [canvas .tc.c]
    pack .tc.c -side top -expand yes -fill both
    fulldeal AKQ AKQ AKQ AKQJ JT9 JT9 JT9 T987 876 876 876 6543 5432 5432 5432 2
    eval "mdealtest $d"

    showplay self c q
    showplay pard s k
    showplay lho d a
    showplay rho h j
    sillynames
}
# 81 "/local/scratch/floater-1.4.15/tclcode/matrix.deq" 2

global matrix_showing


proc matrix_showcards {b who} {
    global matrixcards matrix_showing
    if {$b != $matrixcards([matrix_widget],$who)} {
	global canv
	set c [matrix_widget]
	if [set matrixcards([matrix_widget],$who) $b] {
	$c coords $canv([matrix_widget],frame,$who) $canv([matrix_widget],exilex) $canv([matrix_widget],exiley)
	} else {
	$c coords $canv([matrix_widget],frame,$who) $canv([matrix_widget],mx,$who) 		[expr $canv([matrix_widget],my,$who) + 		($matrix_showing([matrix_widget]) ? 0 : $canv([matrix_widget],YMatrixHide))]


	}	
    }
}

proc highlight_card {tag w x y} {
    global canv
    $w itemconfigure $tag -background #c0c0c0
    set canv([matrix_widget],highlighted) [$w find withtag $tag]
}

proc unhighlight_card {tag w x y} {
    global canv
    $w itemconfigure $tag -background white
    catch {unset canv([matrix_widget],highlighted)}
}

set last_click_card_time -1
proc click_card {tag w time x y} {
    global canv last_click_card_time last_click_card_x last_click_card_y


    if {$last_click_card_time > 0
        && [expr $time - $last_click_card_time] < 750
        && [expr abs($last_click_card_x - $x)] < 5
        && [expr abs($last_click_card_y - $y)] < 5} return
    set last_click_card_time $time
    set last_click_card_x $x
    set last_click_card_y $y

    if [info exists canv([matrix_widget],highlighted)] {
	if {$canv([matrix_widget],highlighted) == [$w find withtag $tag]} {
	command $canv([matrix_widget],item_to_card,[$w find withtag $tag])
	}
    }
}


proc matrixsetup {c} {
    global canv cardwidth cardheight matrixcards cardrectvgap
    global tinymat smallmat screenheight screenwidth matrix_showing

    set tinymat [expr $screenheight <= 600]
    set smallmat [expr !$tinymat && ($screenheight <= 1000)]

    if {$tinymat || $smallmat} {
	if $tinymat {
	tryset fixedfont {Courier 8}
	tinymatrix $c
	} else {
	tryset fixedfont {Courier 10}
	smallmatrix $c 0 30
	}
    } else {
	bigmatrix $c
	tryset fixedfont {Courier 12}
    }

    set matrix_showing($c) 1


    catch {destroy $c}
    canvas $c -height $canv($c,height) -width $canv($c,width) -borderwidth 0 -highlightthickness 0

    set x [set canv($c,exilex) -2000]
    set y [set canv($c,exiley) 0]
    set canv($c,fg,s) black
    set canv($c,fg,h) red
    set canv($c,fg,d) red
    set canv($c,fg,c) black
    set canv($c,bg,s) white
    set canv($c,bg,h) white
    set canv($c,bg,d) white
    set canv($c,bg,c) white
    foreach suit {s h d c} {
	foreach card {a k q j t 9 8 7 6 5 4 3 2} {
	set n [$c create bitmap $x $y -tags livecard]
	$c itemconfigure $n -bitmap c_$card$suit 		-background $canv($c,bg,$suit) -foreground $canv($c,fg,$suit)

	set canv($c,fg_,$n) $canv($c,fg,$suit)
	set canv($c,item_to_card,$n) $suit$card
	set canv($c,$card$suit) $n
	set canv($c,[string toupper $card]$suit) $n
	set canv($c,$card[string toupper $suit]) $n
	set canv($c,[string toupper $card$suit]) $n
	}
    }


    $c bind livecard <Any-Enter> { highlight_card current %W %x %y }
    $c bind livecard <Any-Leave> { unhighlight_card current %W %x %y }
    $c bind livecard <ButtonRelease-1> { click_card current %W %t %x %y }

    set bbox [$c bbox $canv($c,as)]
    set cardwidth [expr [lindex $bbox 2] - [lindex $bbox 0]]
    set cardheight [expr [lindex $bbox 3] - [lindex $bbox 1]]


    global namefont namewid namex namey
    foreach p {self pard} {
	set namewid($c,$p) [
	$c create text $namex($p) $namey($p) -font $namefont -justify center
	]
    }
    foreach p {lho rho} {
	
	set x [expr $namex($p) - $cardwidth / 2]
	set namewid($c,$p) [
	$c create text $x $namey($p) -font $namefont -justify left -anchor w
	]
    }

    global lhomaxx handx handy
    eval "$c create rect $canv($c,matrix)"
    set matrixleft [set lhomaxx($c) [lindex $canv($c,matrix) 0]]
    set matrixright [lindex $canv($c,matrix) 2]
    set matrixtop [lindex $canv($c,matrix) 1]
    set matrixbot [lindex $canv($c,matrix) 3]
    set canv($c,YMatrixHide) [expr - ($matrixbot + 4)]
    set canv($c,matrixHiddenHeight) 	[expr $handy($c,self) - $matrixbot + $cardheight / 2 + 3]



    set canv($c,mx,lho) [expr $matrixleft + $cardwidth / 2 + 5]
    set canv($c,my,lho) [expr ($matrixtop + $matrixbot) / 2]
    set canv($c,mx,rho) [expr $matrixright - $cardwidth / 2 - 5]
    set canv($c,my,rho) [expr ($matrixtop + $matrixbot) / 2]
    set canv($c,mx,self) $handx($c,self)
    set canv($c,my,self) [expr $matrixbot - $cardheight / 2 - $cardrectvgap]
    set canv($c,mx,pard) $canv($c,mx,self)
    set canv($c,my,pard) [expr $matrixtop + $cardheight / 2 + $cardrectvgap]

    global suitfont cardfont

    foreach p {lho self rho pard} {
	set canv($c,matrixtext,$p) [
	$c create text $canv($c,mx,$p) $canv($c,my,$p) -font $cardfont
	]
    }


    foreach p {lho self rho pard} {
	set path [frame $c.f$p]





	set canv($c,frame,$p) 		[$c create window $canv($c,mx,$p) $canv($c,my,$p) -window $path]

	set matrixcards($c,$p) 0
    }




    foreach p {lho rho pard self} {gset matrixcards($c,$p) 0}
}



proc setname {player compassdir name} {
    global namefont playername position canv namewid

    set playername($player) $name
    set position($player) $compassdir
    set f $namewid([matrix_widget],$player)
    set w [matrix_widget]
    if {[string first "(" $name] == -1} {set name "$name ($compassdir)"}

    catch {$w itemconfigure $f -text $name} c
    if {$c != ""} {}
}

proc eraseallcards {} {
    erasebidplay all
    foreach suit {s h d c} {
	foreach card {a k q j t 9 8 7 6 5 4 3 2} {
	erasecard $suit $card
	}
    }
}


proc erasecard {suit card} {
    global canv

    drawcard $suit $card $canv([matrix_widget],exilex) $canv([matrix_widget],exiley)
    [matrix_widget] itemconfigure $canv([matrix_widget],$card$suit) -foreground $canv([matrix_widget],fg,$suit)
}

proc drawcard {suit card x y} {
    global canv

    set c [matrix_widget]
    showMatrix 1
    $c coords $canv([matrix_widget],$card$suit) $x $y
}



proc redrawmatrixcards {} {
    global canv

    foreach p {lho rho self pard} {
	if [info exists canv([matrix_widget],matrixcard,$p)] {
	eval "showplay $p $canv([matrix_widget],matrixcard,$p)"
	}
    }
}
# 321 "/local/scratch/floater-1.4.15/tclcode/matrix.deq"
proc redohand {who suit card} {
    global curhandx curhandy hands handsx handsy

    regsub -nocase $card [set o $hands($who,$suit)] {} n

    if [string compare $n $o] {
	erasecard $suit $card
	if {$who == "lho" || $who == "rho"} {
	
	set curhandx $handsx($who,$suit)
	set curhandy $handsy($who,$suit)
	suit $who $n $suit
	} else {
	set hands($who,$suit) $n
	hand $who 		$hands($who,s) $hands($who,h) $hands($who,d) $hands($who,c)

	}
    }
}


proc tprestore {} {
    global canv purple
    set c [matrix_widget]
    foreach t [array names purple] {
	unset purple($t)
	catch {talkmsg "Restoring color to $t"}
	$c itemconfig $t -foreground $canv([matrix_widget],fg_,$t)
    }
}

proc tpcard {what} {
    global canv purple
    set c [matrix_widget]
    set t $canv([matrix_widget],$what)
    if ![info exists purple($t)] {
	set purple($t) 1
	$c itemconfig $t -foreground purple
	lappend canv([matrix_widget],maybepass) $t
	
    } else {
	unset purple($t)
	$c itemconfig $t -foreground $canv([matrix_widget],fg_,$t)
	
    }
}

proc suit {who cards suit} {
    global removecard togglepassedaction cardgap curhandx curhandy 	lhomaxx rhomaxx cardwidth hands handsx handsy canv



    set hands($who,$suit) $cards
    set handsx($who,$suit) $curhandx
    set handsy($who,$suit) $curhandy
    set l [string length $cards]
    set spacing $cardgap
    set xmin [expr $cardwidth / 2]
    if {$who == "lho" || $who == "rho"} {
	if {$l > 6} {incr spacing -2}
	if {$l > 7} {incr spacing -3}
	if {$l > 8} {incr spacing -2}
	if {$l > 9} {incr spacing -3}
	if {$spacing < 10} {set spacing 10}
	if {$who == "lho"} {set xmax $lhomaxx([matrix_widget])} {set xmax $rhomaxx([matrix_widget])}
	while {[expr $curhandx + $cardwidth / 2 + ($l - 1) * $spacing + 3] 		> $xmax} {

	if {$curhandx > $xmin} {incr curhandx -3} {incr spacing -1}
	}
    }
    if {$curhandx < $xmin} {set curhandx xmin}
    for {set i 0} {$i < $l} {incr i} {
	set card [string index $cards $i]
	drawcard $suit $card $curhandx $curhandy
	set removecard([string toupper $suit$card]) "redohand $who $suit $card"
	set togglepassedaction([string toupper $suit$card]) "tpcard $card$suit"
	set canv([matrix_widget],dealt,[string tolower $suit$card]) 1
	incr curhandx $spacing
    }
}


proc hand {who s h d c} {
    global handx handy curhandx curhandy suitgap cardgap hands vgap 	ourcardgap theircardgap



    if {$who == "self" || $who == "pard"} {
	set cardgap $ourcardgap
	set width [expr ([string length $s] + [string length $h] + [string length $d] + [string length $c]) * $cardgap + (([string length $s] > 0) + ([string length $h] > 0) + ([string length $d] > 0)) * $suitgap]
	set curhandx [expr $handx([matrix_widget],$who) - round(0.5 * $width)]
	set curhandy $handy([matrix_widget],$who)

	suit $who $s s
	if {$s != ""} {incr curhandx $suitgap}
	suit $who $h h
	if {$h != ""} {incr curhandx $suitgap}
	suit $who $d d
	if {$d != ""} {incr curhandx $suitgap}
	suit $who $c c
    } else {
	set cardgap $theircardgap
	set curhandx $handx([matrix_widget],$who)
	set curhandy $handy([matrix_widget],$who)
	suit $who $s s
	incr curhandy $vgap
	set curhandx $handx([matrix_widget],$who)
	incr curhandx 10
	suit $who $h h
	incr curhandy $vgap
	set curhandx $handx([matrix_widget],$who)
	suit $who $d d
	incr curhandy $vgap
	set curhandx $handx([matrix_widget],$who)
	incr curhandx 10
	suit $who $c c
    }
}

proc fulldeal_erase_straglers {} {
    global canv
    foreach suit {s h d c} {
	foreach card {a k q j t 9 8 7 6 5 4 3 2} {
	if !$canv([matrix_widget],dealt,$suit$card) { erasecard $suit $card }
	}
    }
}


proc fulldeal {s h d c LHOs LHOh LHOd LHOc 		Ps Ph Pd Pc RHOs RHOh RHOd RHOc} {

    global canv



    foreach suit {s h d c} {
	foreach card {a k q j t 9 8 7 6 5 4 3 2} {
	set canv([matrix_widget],dealt,$suit$card) 0
	}
    }

    tprestore


    if {"$Ps$Ph$Pd$Pc$LHOs$LHOh$LHOd$LHOc$RHOs$RHOh$RHOd$RHOc" != ""} {
	
	hand self $s $h $d $c
	hand pard $Ps $Ph $Pd $Pc
	hand lho $LHOs $LHOh $LHOd $LHOc
	hand rho $RHOs $RHOh $RHOd $RHOc
	fulldeal_erase_straglers
    } else {
	
	
	global matrix_showing
	set z $matrix_showing([matrix_widget])
	hand self $s $h $d $c
	hand pard $Ps $Ph $Pd $Pc
	hand lho $LHOs $LHOh $LHOd $LHOc
	hand rho $RHOs $RHOh $RHOd $RHOc
	fulldeal_erase_straglers
	showMatrix $z
    }
}


proc sillynames {} {
    setname self S {hairy dude}
    setname lho W {skinny dude}
    setname pard N {goofball}
    setname rho E {elephant water}
}

proc matrixtext {player s {options {}}} {
    global canv

    eval "[matrix_widget] itemconfigure $canv([matrix_widget],matrixtext,$player) -text \{$s\} 	$options"

}

proc showplay {player suit card} {
    global canv

    if {$suit == "?"} {
	matrixtext $player "?"
    } else {
	removecardfromhand $suit $card
	erasebidplay $player
	matrix_showcards 1 $player
	drawcard $suit $card $canv([matrix_widget],mx,$player) $canv([matrix_widget],my,$player)
	set canv([matrix_widget],matrixcard,$player) "$suit $card"
    }
}

proc showbid {player level strain} {
    global canv

    matrix_showcards 0 $player
    drawbid [matrix_widget].f$player $level $strain
}



proc erasebidplay {who} {
    global tricktimeOK

    if {!$tricktimeOK} {
	after 100 erasebidplay who
    }

    if {$who == "all"} {
	global needtoerase

	set needtoerase 0
	erasebidplay lho
	erasebidplay rho
	erasebidplay pard
	erasebidplay self
    } else {
	global canv
	matrixtext $who ""
	if [info exists canv([matrix_widget],matrixcard,$who)] {
	eval "erasecard $canv([matrix_widget],matrixcard,$who)"
	unset canv([matrix_widget],matrixcard,$who)
	}
	matrix_showcards 1 $who
    }
}


proc showMatrix {b} {
    global matrix_showing canv

    set c [matrix_widget]


    if {$b == $matrix_showing($c)} return

    if [set matrix_showing($c) $b] {
	$c move all 0 [expr - $canv($c,YMatrixHide)]
	$c configure -height $canv($c,height)
    } else {
	$c move all 0 $canv($c,YMatrixHide)
	$c configure -height $canv($c,matrixHiddenHeight)
    }
}




proc matrix_copycards {new_toplevel} {
    global canv hands toplevel


    set old_toplevel $toplevel
    set toplevel $new_toplevel


    foreach who {lho rho self pard} {
	hand $who $hands($who,s) $hands($who,h) $hands($who,d) $hands($who,c)
    }



    foreach p {lho rho self pard} {
	set canv($new_toplevel,matrixcard,$p) canv($old_toplevel,matrixcard,$p)
    }



    set toplevel $old_toplevel
}
# 578 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
# 1 "/local/scratch/floater-1.4.15/tclcode/gui.deq" 1
# 78 "/local/scratch/floater-1.4.15/tclcode/gui.deq"
global toplevel


if $mswindows {
    global background
    set background gray
} else {
    global background
    set background "lightgray"
}





proc status_widget {} {
    global toplevel
    return "$toplevel.info.stat"
}

proc info_widget {} {
    global toplevel
    return "$toplevel.info.infoline"
}

proc matrix_widget {} {
    global toplevel
    return $toplevel.play
}



proc auction_cell {col line} {
    global toplevel max_auction_line
    set name "$toplevel.auction.r.$col$line"
    if [winfo exists $name] {
    } else {
        frame $name
        grid $name -column $col -row [expr $line + 1]
    }

    if {! [info exists max_auction_line]} { set max_auction_line -1 }
    if {$line > $max_auction_line} {
	set max_auction_line $line
    }

    return $name
}

proc auction_widget {} {
    global toplevel
    return "$toplevel.auction.r"
}

proc talktext {} {
    global toplevel talk_is_sep

    if $talk_is_sep {
	return .floaterTalk.text
    } else {
	return $toplevel.talk.text
    }
}





proc focus_cmdline {} {
    global entryLinesShown_ toplevel

    if {$entryLinesShown_ == "talk"} {
	focus $toplevel.cmd.talk
    } else {
	focus $toplevel.cmd.cmdline
    }
}



proc auctionbbcommand {what} {
    global alert redalert

    command $what
    if $alert {command alert}
    if $redalert {command redalert}
    set alert 0
    set redalert 0
}



proc typedcommand {cmd} {
    if {$cmd == ""} {command {inullcommand c}} {command $cmd}
}

proc typedtalk {talk} {
    if {$talk == ""} {command {inullcommand t}} {talk $talk}
}

proc bindsetup {w tabto returnscript} {
    global toplevel

    bind $w <Any-Enter> "focus %W"
    if [string match "*.cmd.cmdline" $tabto] {
	bind $w <KeyPress-Tab> "focus_cmdline; break"
	bind $w <Control-KeyPress-i> "focus_cmdline; break"
    } else {
	bind $w <KeyPress-Tab> "focus $tabto; break"
	bind $w <Control-KeyPress-i> "focus $tabto; break"
    }
    bind $w <KeyPress-Return> $returnscript
    return $w
m}



proc major_pane {path} {
    frame $path -relief raised -borderwidth 3
}


proc talkframe {t} {
    global talkfont background

    scrollbar $t.scroll -command "$t.text yview" -takefocus false
    text $t.text -wrap word -yscrollcommand "$t.scroll set" -width 60 -height 15 -font $talkfont -relief flat -takefocus false -background $background
    refont $t.text talkfont
    pack $t.scroll -side right -fill y
    pack $t.text -side left -expand yes -fill both

    bind $t.text <Configure> "$t.text yview -pickplace end"
    bind $t.text <FocusIn> focus_cmdline
}


proc debugwindow {d} {
    major_pane $d
    gset debugtext [text $d.text -relief raised -yscrollcommand "$d.scroll set" -width 300 -height 10]
    scrollbar $d.scroll -command "$d.text yview"
    pack $d.scroll -side right -fill y
    pack $d.text -side left
}





proc cmdframe {cmd} {
    global cmdlinelabelfont cmdlinefont background

    major_pane $cmd
    frame $cmd.f
    refont [label $cmd.f.labelt -font $cmdlinelabelfont -text "Talk:"] cmdlinelabelfont
    refont [label $cmd.f.labelc -font $cmdlinelabelfont -text "Command:"] cmdlinelabelfont
    pack $cmd.f.labelc $cmd.f.labelt -anchor e
    refont [entry $cmd.cmdline -font $cmdlinefont -relief sunken -bd 2 -textvariable cmd -background $background] cmdlinefont
    refont [entry $cmd.talk -font $cmdlinefont -relief sunken -bd 2 -textvariable talk -background $background] cmdlinefont
    pack $cmd.f -side left
    pack $cmd.cmdline $cmd.talk -fill x -side top
}




proc auctionbb {narrow bb w {what same} {rank infer}} {
    global auctionbbfont bb_buttons_onrow

    if {$what == "same"} {set what $w}

    if {$rank == "infer"} {
        set rank [string index $w 0]
    }
    if $narrow {
       set row $rank
    } else {
       set row [expr ($rank + 1) / 2]
    }
    if [expr $row < 0] {
        set row alerts
    }

    refont [button $bb.$row.$w -text $what 	-command "auctionbbcommand $what" 	-font $auctionbbfont] auctionbbfont



    pack $bb.$row.$w -side left
}





proc auctionframe {auction {narrow 0}} {
    global auctionbbfont auctionlabelfont auctionlabel namefont auctionnamewidth

    major_pane $auction

    refont [label $auction.l -font $auctionlabelfont -text $auctionlabel] auctionlabelfont
    frame $auction.r

    if $narrow {
        set headervar position
	set namewidth 6
    } else {
        set headervar playername
	set namewidth $auctionnamewidth
    }

    refont [label $auction.r.name0 -font $namefont -textvariable [format "%s(lho)" $headervar] -width $namewidth -padx [px]] namefont



    refont [label $auction.r.name1 -font $namefont -textvariable [format "%s(pard)" $headervar] -width $namewidth -padx [px]] namefont



    refont [label $auction.r.name2 -font $namefont -textvariable [format "%s(rho)" $headervar] -width $namewidth -padx [px]] namefont



    refont [label $auction.r.name3 -font $namefont -textvariable [format "%s(self)" $headervar] -width $namewidth -padx [px]] namefont



    if $narrow {
	pack $auction.l $auction.r -side top -anchor n
    } else {
	pack $auction.l $auction.r -side left -anchor n
    }


    grid $auction.r.name0 -column 0 -row 0
    grid $auction.r.name1 -column 1 -row 0
    grid $auction.r.name2 -column 2 -row 0
    grid $auction.r.name3 -column 3 -row 0
}



proc bbframe {bb narrow} {
    global auctionbbfont auctionlabelfont auctionlabel namefont auctionnamewidth

    major_pane $bb

    if $narrow {
	
        set headerside top
    } else {
        set headerside left
    }
    refont [label $bb.header -font $auctionlabelfont -text "Your Bid:"] auctionlabelfont
    pack $bb.header -side $headerside -anchor n



    frame $bb.alerts
    frame $bb.0
    frame $bb.1
    frame $bb.2
    frame $bb.3
    frame $bb.4
    if $narrow {
	frame $bb.5
	frame $bb.6
	frame $bb.7
    }




    if $narrow {
	set alert alerts
    } else {
	set alert 0
    }




    refont [checkbutton $bb.$alert.alert -text "Alert" -variable alert 		-command {set redalert 0} -font $auctionbbfont 		-relief flat -wraplength 1000 -padx 10 ] auctionbbfont


    pack $bb.$alert.alert -side left
    refont [checkbutton $bb.$alert.redalert -text "Red Alert" 		-variable redalert -command {set alert 0} 		-font $auctionbbfont -relief flat -wraplength 1000 -padx 10] auctionbbfont


    pack $bb.$alert.redalert -side right




    auctionbb $narrow $bb pass Pass 0
    auctionbb $narrow $bb x Double 0
    auctionbb $narrow $bb xx Redouble 0

    auctionbb $narrow $bb 1C
    auctionbb $narrow $bb 1D
    auctionbb $narrow $bb 1H
    auctionbb $narrow $bb 1S
    auctionbb $narrow $bb 1N
    auctionbb $narrow $bb 2C
    auctionbb $narrow $bb 2D
    auctionbb $narrow $bb 2H
    auctionbb $narrow $bb 2S
    auctionbb $narrow $bb 2N
    auctionbb $narrow $bb 3C
    auctionbb $narrow $bb 3D
    auctionbb $narrow $bb 3H
    auctionbb $narrow $bb 3S
    auctionbb $narrow $bb 3N
    auctionbb $narrow $bb 4C
    auctionbb $narrow $bb 4D
    auctionbb $narrow $bb 4H
    auctionbb $narrow $bb 4S
    auctionbb $narrow $bb 4N
    auctionbb $narrow $bb 5C
    auctionbb $narrow $bb 5D
    auctionbb $narrow $bb 5H
    auctionbb $narrow $bb 5S
    auctionbb $narrow $bb 5N
    auctionbb $narrow $bb 6C
    auctionbb $narrow $bb 6D
    auctionbb $narrow $bb 6H
    auctionbb $narrow $bb 6S
    auctionbb $narrow $bb 6N
    auctionbb $narrow $bb 7C
    auctionbb $narrow $bb 7D
    auctionbb $narrow $bb 7H
    auctionbb $narrow $bb 7S
    auctionbb $narrow $bb 7N

    if $narrow {
	pack $bb.alerts $bb.0 $bb.1 $bb.2 $bb.3 $bb.4 $bb.5 $bb.6 $bb.7 -side top
    } else {
	pack $bb.0 $bb.1 $bb.2 $bb.3 $bb.4 -side top
    }
}







proc gui_setup {toplevel} {
    global floater_version talk_is_sep

    frame $toplevel


    major_pane $toplevel.info
    label $toplevel.info.version -text $floater_version
    label $toplevel.info.stat -text " "
    label $toplevel.info.infoline -text " "
    matrixsetup $toplevel.play
    major_pane $toplevel.talk
    talkframe $toplevel.talk
    major_pane $toplevel.talkfiller
    debugwindow $toplevel.debug
    cmdframe $toplevel.cmd
    auctionframe $toplevel.auction [string equal $toplevel .bigmain]
    bbframe $toplevel.bb [string equal $toplevel .bigmain]



    set padx 2
    set pady 2

    pack $toplevel.info.version -side top -fill x
    pack $toplevel.info.stat -side top -fill x
    pack $toplevel.info.infoline -side top -fill x

    if [string equal $toplevel ".bigmain"] {

	frame $toplevel.rightcolumn
	lower $toplevel.rightcolumn
	pack $toplevel.rightcolumn -side right -fill y

	$toplevel.info.stat configure -text " \n\n\n "


	pack $toplevel.info -in $toplevel.rightcolumn -side top -fill x -padx $padx -pady $pady

	pack $toplevel.bb -in $toplevel.rightcolumn -side top -fill x -anchor n -padx $padx -pady $pady


	pack $toplevel.auction -in $toplevel.rightcolumn -side top -fill x -padx $padx -pady $pady

	major_pane $toplevel.rightcolumn.iconholder
	pack $toplevel.rightcolumn.iconholder -side top -fill both -expand yes -padx $padx -pady $pady
	canvas $toplevel.rightcolumn.iconholder.i -width 48 -height 48 -borderwidth 0 -relief flat -highlightthickness 0
	$toplevel.rightcolumn.iconholder.i create image 0 0 -image floater-48 -anchor nw
	pack $toplevel.rightcolumn.iconholder.i -side top -expand yes
    } else {
	pack $toplevel.info -side top -fill x -padx $padx -pady $pady
    }
    major_pane $toplevel.playholder
    lower $toplevel.playholder
    pack $toplevel.play -in $toplevel.playholder -side top -padx 3 -pady 3
    pack $toplevel.playholder -side top -padx $padx -pady $pady -fill x




    pack $toplevel.cmd -side bottom -fill x -padx $padx -pady $pady




    bindsetup $toplevel.cmd.cmdline $toplevel.cmd.talk 	{global cmd toplevel; if {$cmd == ""} {typedcommand $cmd} else {typedcommand $cmd; $toplevel.cmd.cmdline delete 0 end}}


    bindsetup $toplevel.cmd.talk $toplevel.cmd.cmdline 	{global talk toplevel; if {$talk == ""} {typedtalk $talk} else {typedtalk $talk; $toplevel.cmd.talk delete 0 end}}

}




proc showauction {bool} {
    global showingauction toplevel

    set showingauction $bool

    if [string equal $toplevel .bigmain] {

        return
    }

    if $bool {
	pack $toplevel.auction -side top -after $toplevel.playholder
    } else {
	pack forget $toplevel.auction
    }
}



proc showButtonBar {b} {
    global toplevel showingbb

    set showingbb $b

    if [string equal $toplevel .bigmain] {

        return
    }

    if $b {
        pack $toplevel.bb -after $toplevel.auction
    } else {
	pack forget $toplevel.bb
    }
}




proc split_line {line min_nls} {
    global toplevel

    if [string equal $line ""] { set line " " }

    if [string equal $toplevel ".bigmain"] {
        set line [string map "| {\n}" $line]
	set num_nls [llength [split $line "\n"]]
	for {set x 0} {$x < [expr $min_nls - $num_nls]} {incr x} {
	set line "$line\n "
	}
    } else {
	set line [string map "| {; }" $line]
    }

    return $line
}


proc gui_setinfo {newinfo} {
    global infoline
    set infoline $newinfo

    [info_widget] configure -text [split_line $newinfo 3]
}



proc gui_setstatus {newstatus} {
    global statusline toplevel

    set statusline $newstatus

    [status_widget] configure -text [split_line $newstatus 1]
}





proc copytext {old new} {
    catch {$new insert end [$old get 0.0 end-1c]}
}




proc gui_separateTalk {sep {win auto}} {
    global toplevel talk_is_sep talkfont

    if {$win == "auto" } {
	set win $toplevel
    }

    set talk_is_sep $sep


    set padx 2
    set pady 2



    if $talk_is_sep {
	catch {destroy .floaterTalk}

	toplevel .floaterTalk
	wm title .floaterTalk "Floater Talk"

	talkframe .floaterTalk

	.floaterTalk.text yview -pickplace end
	copytext $win.talk.text .floaterTalk.text
	$win.talk.text delete 0.0 end
	pack forget $win.talk
	pack $toplevel.talkfiller -side top -fill both -expand yes -padx $padx -pady $pady
    } else {
	copytext .floaterTalk.text $win.talk.text
	catch {destroy .floaterTalk}
	pack forget $toplevel.talkfiller
	pack $toplevel.talk -side top -fill both -expand yes -padx $padx -pady $pady
    }
}



proc gui_iscompact {} {
    global toplevel
    return [string equal $toplevel ".smallmain"]
}



proc gui_compact {compact} {
    global toplevel showingauction showingbb talk_is_sep statusline infoline entryLinesShown_ gui_compact_

    set gui_compact_ $compact

    set saved_talktext [[talktext] get 0.0 end-1c]

    set old_toplevel $toplevel

    if $compact {
	set new_toplevel .smallmain
    } else {
	set new_toplevel .bigmain
    }

    if {$old_toplevel == $new_toplevel} { return }

    gui_setup $new_toplevel




    matrix_copycards $new_toplevel
    copybids $new_toplevel



    destroy $old_toplevel
    pack $new_toplevel -expand yes -fill both
    set toplevel $new_toplevel




    if [info exists showingauction] {
	showauction $showingauction
    }
    if [info exists showingbb] {
	showButtonBar $showingbb
    }
    if [info exists talk_is_sep] {
	gui_separateTalk $talk_is_sep
    }
    if [info exists entryLinesShown_] {
	showEntryLines $entryLinesShown_
    }


    [talktext] delete 0.0 end
    [talktext] insert end $saved_talktext
    if [info exists statusline] { gui_setstatus $statusline }
    if [info exists infoline] { gui_setinfo $infoline }




    foreach player {lho rho self pard} {
	global playername position
	setname $player $position($player) $playername($player)
    }


    hideMatrixDuringAuction [string equal $toplevel .smallmain]



    focus_cmdline




    gui_setWindowSize
}




proc gui_setWindowSize {} {
    global screenheight screenwidth effectiveheight toplevel macintosh geometry_specified

    if $geometry_specified { return }

    if {$toplevel == ".bigmain"} {
	set w 850
    } else {
	set w 600
    }
    if $macintosh {
	set w [expr $w + 100]
    }
    if {$w > [expr $screenwidth - 35]} {
	set w [expr $screenwidth - 35]
    }

    set h 700
    if {$h > $effectiveheight} {
	set h $effectiveheight
    }

    wm geometry . [join "$w $h" x]





    wm minsize . 500 500
}
# 579 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2




line 532
# 669 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
if {$effectiveheight > 600} {
    global gui_compact_
    gui_setup .bigmain
    pack .bigmain -expand yes -fill both
    set toplevel .bigmain
    set gui_compact_ 0
} else {
    global gui_compact_
    gui_setup .smallmain
    pack .smallmain -expand yes -fill both
    set toplevel .smallmain
    set gui_compact_ 1
}


showEntryLines talk
gui_separateTalk 0
focus_cmdline
gui_setWindowSize





menu .menu -tearoff 0
# 1 "/local/scratch/floater-1.4.15/tclcode/menu.deq" 1
# 39 "/local/scratch/floater-1.4.15/tclcode/menu.deq"
   	





global m


proc simple {label {command <none>}} {
    global m

    if {"<none>" == $command} {set command $label}
    $m add command -label $label -command "command \"$command\""

}



proc addrb {var label {command <none>} {value <none>}} {
    global m

    if {"<none>" == $command} {set command $label}
    if {"<none>" == $value} {set value $label}
    $m add radiobutton -label $label -command "command \"$command\"" 	-variable $var -value "$value"

}


proc cascade {label {subcommands {}}} {
    global m

    $m add cascade -label $label -menu [set m2 "$m.cas$label"]
    menu $m2 -tearoff no
    set oldm $m
    set m $m2
    foreach sub $subcommands {
	eval "simple $sub"
    }
    set m $oldm
    return $m2
}


proc rcascade {label var {subcommands {}}} {
    global m

    $m add cascade -label $label -menu [set m2 "$m.cas[join $label _]"]
    menu $m2 -tearoff no
    set oldm $m
    set m $m2
    foreach sub $subcommands {
	eval "addrb $var $sub"
    }
    set m $oldm
    return $m2
}


set m [menu .menu.file -tearoff no]
.menu add cascade -menu $m -label File -underline 0
# 114 "/local/scratch/floater-1.4.15/tclcode/menu.deq"
simple "Login..." login
simple "Change Password..." password
simple "Load CC..." ccload
simple "Save CC..." ccsave



set needseated_fmenu_entries {2 3}
$m add separator
simple Quit



set m [menu .menu.join -tearoff no]
.menu add cascade -menu $m -label Join -underline 0

proc no_tables_to_join {b} {
    if $b {
	global join_menu_length join_menu
	.menu.join delete 0 end
	.menu.join add command -label "(none)" -state disabled
	.menu.join add separator
	.menu.join add command -label "Check for tables" 		-command "command tables"

	set join_menu_length 0
	foreach i [array names join_menu] { unset join_menu($i) }
    } else {
	catch {.menu.join delete "(none)"}
    }
}

no_tables_to_join 1

proc join_menu_add_table {name fullname} {
    global join_menu_length join_menu

    if [info exists join_menu($name)] {
	if {$fullname != $join_menu($name)} {
	.menu.join entryconfigure $join_menu($name) -label $fullname
	set join_menu($name) $fullname
	}
	return
    }
    if {[incr join_menu_length] == 1} {no_tables_to_join 0}
    set join_menu($name) $fullname
    .menu.join insert 0 command -label $fullname -command "command \"join $name\""
}

proc join_menu_remove_table {name} {
    global join_menu_length join_menu

    if ![info exists join_menu($name)] return
    .menu.join delete $join_menu($name)
    if {[incr join_menu_length -1] == 0} {
	no_tables_to_join 1
    } else {
	unset join_menu($name)
    }
}



set m [menu .menu.table -tearoff no]
.menu add cascade -menu $m -label Table -underline 0

simple Host
$m add separator
cascade Sit {North South East West}
simple Stand
$m add separator
cascade Kibitz
$m.casKibitz configure -postcommand {update_kibitz_menu}
simple Spec
$m add separator
cascade Unseat
$m.casUnseat configure -postcommand {update_unseat_menu}
cascade Communications {Disconnect {"Show Parent" parent} 	{"Show Children" children} {"Show Net Location" ip}}

simple "Show who's here" who
simple "Beep everyone" beep
set needtable_tmenu_entries {2 3 5 6 8 9 10 11 12}




proc playername_atcompass {compassdir} {
    global playername position
    foreach p {lho rho pard self} {
        if [info exists position($p)] {
            if [info exists playername($p)] {
                if [string equal $position($p) $compassdir] {
                    set name $playername($p)
                    if {[string first "(" $name] == -1} {
                        return $name
                    }
                }
            }
        }
    }
    return ""
}


proc compassdir_occupied {compassdir} {
    if [string equal [playername_atcompass $compassdir] ""] {
        return 0
    } else {
        return 1
    }
}




proc compass_description {compassdir} {
    set name [playername_atcompass $compassdir]
    if [ string equal $name "" ] {
        if [ string equal $compassdir "N" ] { return "North" }
        if [ string equal $compassdir "S" ] { return "South" }
        if [ string equal $compassdir "E" ] { return "East" }
        if [ string equal $compassdir "W" ] { return "West" }
        floaterror "$compassdir is an invalid compass direction"
    } else {
        return "$name ($compassdir)"
    }
}

proc update_kibitz_menu {} {
    global m

    set m .menu.table.casKibitz

    $m delete 0 end

    simple None {kibitz 0}
    foreach dir {N S E W} {
        simple [compass_description $dir] "kibitz $dir"
    }
}


proc update_unseat_menu {} {
    global m

    set m .menu.table.casUnseat

    $m delete 0 end

    foreach dir {N S E W} {
        if [compassdir_occupied $dir] {
            simple [compass_description $dir] "unseat $dir"
        }
    }
}



set m [menu .menu.bridge -tearoff no]
.menu add cascade -menu $m -label Bridge -underline 0

global claimmenu
set claimmenu [cascade Claim]
foreach i {13 12 11 10 9 8 7 6 5 4 3 2 1 0} {
    $claimmenu add command -label "$i tricks" -command "GUIclaim $i"
}
simple "Accept claim" accept
simple "Reject claim" reject
simple "Retract claim" retract
simple Review
simple "Show initial cards" cards
simple "Show last trick" last
simple "Show EW CC" {ccdump EW}
simple "Show NS CC" {ccdump NS}
$m add separator
set scoringmenu [cascade Scoring]
$scoringmenu add radiobutton -label IMP -command "command \"score IMP\"" 	-variable radioscoring

$scoringmenu add radiobutton -label MP -command "command \"score MP\"" 	-variable radioscoring

$scoringmenu add radiobutton -label Rubber 	-command "command \"score Rubber\"" 	-variable radioscoring -value RUBBER


$scoringmenu add radiobutton -label Hearts 	-command "command \"score Hearts\"" 	-variable radioscoring -value HEARTS


$scoringmenu add separator
$scoringmenu add radiobutton -label Competitive 	-command "command competitive" -variable radiocompetitive

$scoringmenu add radiobutton -label Noncompetitive 	-command "command noncompetitive" -variable radiocompetitive

.menu.bridge entryconfigure 10 -state disabled
proc menus_tablehost {b {scoring {}} {competitive {}}} {
    global radiocompetitive radioscoring scoringmenu

    set radioscoring $scoring
    set radiocompetitive $competitive
    if $b {set setting normal} {set setting disabled}
    .menu.bridge entryconfigure 10 -state $setting
    .menu.bridge entryconfigure 11 -state $setting
}
simple "Deal next hand" deal
.menu.bridge entryconfigure 11 -state disabled
simple "Show previous deal" previous
set needbridge_bmenu_entries {0 1 2 3 4 5 6}
set needtable_bmenu_entries {7 8}
.menu.bridge entryconfigure 12 -state disabled
proc menus_enable_previous {} { .menu.bridge entryconfigure 12 -state normal }



set m [menu .menu.options -tearoff no]
.menu add cascade -menu $m -label Options -underline 0

$m add checkbutton -label "Compact GUI" -command {gui_compact $gui_compact_} -variable gui_compact_

$m add checkbutton -label "Separate talk window" -command {command "separateTalk $separateTalk_"} -variable separateTalk_
$m add checkbutton -label "Beep at my turn" -command {command "beepAtMyTurn $beepAtMyTurn_"} -variable beepAtMyTurn_
$m add checkbutton -label "Deiconify if I'm beeped" -command {command "deiconifyIfBeeped $deiconifyIfBeeped_"} -variable deiconifyIfBeeped_
rcascade "Entry lines shown" entryLinesShown_ {{"talk" "showEntryLines talk" talk}

   		{"command" "showEntryLines command" command}
			{"both" "showEntryLines both" both}}
rcascade "Hide auction" auction_hide_time 			{{"after first trick" "hideAuction -1" -1}

			{"end of auction + 5 seconds" "hideAuction 5" 5}
			{"end of auction + 10 seconds" "hideAuction 10" 10}}

$m add separator
$m add command -label "Bridge Font" -state disabled
tryset radiofont Medium
$m add radiobutton -label "Large" -command "command \"font large\"" -variable radiofont
$m add radiobutton -label "Medium" -command "command \"font medium\"" -variable radiofont
$m add radiobutton -label "Small" -command "command \"font small\"" -variable radiofont



set m [menu .menu.font -tearoff no]
.menu add cascade -menu $m -label Font -underline 2
foreach f [lsort [fontfamilies]] {

    set firstchar [string index $f 0]
    if {![info exists prev] || $firstchar != $prev} {
	set submenu [cascade [set prev $firstchar]]
    }
    $submenu add radiobutton -label $f -command "updatetalkfontfam \{$f\}" 	-variable fontfam






}
set fontfam [lindex $talkfont 0]



set m [menu .menu.fontsize -tearoff no]
.menu add cascade -menu $m -label Fontsize -underline 4
foreach s {8 9 10 11 12 14 18 24 28 32 36} {
    $m add radiobutton -label $s -command "updatetalkfontsize $s" 	-variable fontsiz

}
set fontsiz [lindex $talkfont 1]



set m [menu .menu.help -tearoff no]
if $macintosh { set h Docs } else { set h Help }
.menu add cascade -menu $m -label $h -underline 0

simple "About" about
$m add separator

proc prevchar {c} {
    scan $c %c i
    format %c [incr i -1]
}

proc menus_helpcommands {commands} {
    global m help_texts

    set m .menu.help
    set endchars \[fq\]
    set begin a
    foreach s $commands {
	if {![string match "*(*" $s] && ![string match "*)*" $s]} {
	if {[regexp $endchars [set first [string range $s 0 0]]] &&
	![info exists doneit($first)]} {
		set doneit($first) 1
		cascade "$begin-[prevchar $first]" $w
		set begin $first
		set w {}
	}
	lappend w "$s \"help $s\""
	}
    }
    if {$w != {}} {cascade $begin-z $w}

    $m add separator
    foreach text $help_texts {
	simple $text
    }
}




proc menus_noclaim {} {
    foreach i {1 2 3} {
	.menu.bridge entryconfigure $i -state disabled
    }
}

proc menus_defclaim {} {
    foreach i {1 2} {
	.menu.bridge entryconfigure $i -state normal
    }
}

proc menus_declclaim {} {
    .menu.bridge entryconfigure 3 -state normal
}




proc claimable {n setting} {
    global claimmenu

    set n [expr 13 - $n]
    if $setting {set setting normal} {set setting disabled}
    $claimmenu entryconfigure $n -state $setting
}


proc GUIclaim {n} {
    global contract_tricks


    if ![info exists contract_tricks] return
    if {$n >= $contract_tricks} {
	command "make [expr $n - 6]"
    } else {
	command "down [expr $contract_tricks - $n]"
    }
}

proc update_claimmenu {decltricks deftricks} {

    set max [expr 13 - $deftricks]
    for {set i 0} {$i <= 13} {incr i} {
	if {$i < $decltricks || $i > $max} {
	claimable $i 0
	} else {
	claimable $i 1
	}
    }
}


proc menus_declaring {b} {
    if $b {set setting normal} {set setting disabled}
    .menu.bridge entryconfigure 0 -state $setting
}



set bridge_menus_state 1

proc activate_bridge_menus {b} {
    global bridge_menus_state needbridge_bmenu_entries

    if {$b == $bridge_menus_state} return

    if [set bridge_menus_state $b] {set setting normal} {set setting disabled}
    foreach n $needbridge_bmenu_entries {
	.menu.bridge entryconfigure $n -state $setting
    }

}

proc menus_newhand {} {
    activate_bridge_menus 1
    menus_noclaim
    menus_declaring 0
}

proc menus_nobridge {} {activate_bridge_menus 0; menus_noclaim}

menus_nobridge


set seated_menus_state 1

proc activate_seated_menus {b} {
    global seated_menus_state needseated_fmenu_entries

    if {$b == $seated_menus_state} return

    if [set seated_menus_state $b] {set setting normal} {set setting disabled}
    foreach n $needseated_fmenu_entries {
	
	.menu.file entryconfigure $n -state $setting
    }
    if !$b {menus_noclaim; menus_declaring 0}
}

activate_seated_menus 0


set table_menus_state 1

proc activate_table_menus {b} {
    global table_menus_state needtable_bmenu_entries needtable_tmenu_entries

    if {$b == $table_menus_state} return
    if !$b {activate_seated_menus 0; menus_tablehost 0}

    if [set table_menus_state $b] {set setting normal} {set setting disabled}
    foreach n $needtable_bmenu_entries {
	.menu.bridge entryconfigure $n -state $setting
    }
    foreach n $needtable_tmenu_entries {
	.menu.table entryconfigure $n -state $setting
    }
}

activate_table_menus 0
# 695 "/local/scratch/floater-1.4.15/tclcode/floater.deq" 2
. configure -menu .menu







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
# 865 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
line 1064
# 945 "/local/scratch/floater-1.4.15/tclcode/floater.deq"
proc turn_off_scrolllock {} {}


proc debugmsg {s} {
    global debugprinting floater_silent

    set old $debugprinting
    set debugprinting 1
    talkmsg $s
    set debugprinting $old

}





proc setcursor {cursor w} {


    if {$w == ".menu" || $w == ".#menu"} return
    global oldcursor

    set oldcursor($w) [lindex [$w configure -cursor] 4]
    $w configure -cursor $cursor
    foreach child [winfo children $w] {setcursor $cursor $child}

}

line 1173

proc unsetcursor {w} {


    global oldcursor

    if [info exists oldcursor($w)] {
	catch {
	$w configure -cursor $oldcursor($w)
	foreach child [winfo children $w] {unsetcursor $child}
	}
    }

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
	


proc configall {w c} {
    eval "$w configure $c"
    foreach child [winfo children $w] {configall $child $c}
}
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
