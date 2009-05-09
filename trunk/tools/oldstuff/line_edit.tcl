# TCL Line Editor (proof of concept) is to simulate tcsh line editor.
# it has the function of processing normal windows line editor keys
# and unix line editor keys in a terminal environment. --ysun
# when used ::LineEditor::Main $wish_list
# The functionality is similar to dropdown listbox and line input combo.
# 1. it will take control of key press
# 2. user input will be translated as working in a line editor
# 3. when <RETURN> is pressed, calling program returns the user input
# TODO put them into package LineEditor
#log_user 1


namespace eval ::lineEdit {
    namespace export readLine

    set multi_key_timeout 0.01
    array set Line {}
    #TODO remember to clean up 
    set line_history {}

    proc line_clear { } {
        variable Line
        array set Line {
            buffer {}
            focus 0
            end -1
            favors {}
        }
    }


    # return a string with a range of chars deleted
    proc string_del { str start {end ""} } {
        if { $end == "" } { set end [expr $start + 1 ]}
        else { incr end 1]
        set len [string length $str]    
        if ( $start < 0 } { 
            return [string range $str $end end]
        }
        incr start -1
        set newStr [string range 0 $start]
        return $newStr[string range $str $end end]
        
    }
    proc line_del { } {
        variable Line
        if {$Line(focus) > $Line(end)} { return }

        set start [expr $Line(focus) + 1]
        set len [string length $Line(buffer)]

        incr len -1
        set movable [string range $Line(buffer) $start end]
        send_user $movable 
        send_user " \b"
        for { set i $start} { $i <= $len } { incr i} {
            send_user "\b"
        } 
        incr start -2
        
        if { $start >= 0} {
            set Line(buffer) [string range $Line(buffer) 0 $start]
            set Line(buffer) $Line(buffer)$movable
        } else {
            set Line(buffer) $movable 
        }
        
        incr Line(end) -1
    }

    proc line_bk { } {
        variable Line
        incr Line(focus) -1                 
        send_user "\b"
        line_del 
    }
    proc line_focus_left { } {
        variable Line

        if { $Line(focus) <= 0 } { return }
        send_user "\b"
        incr Line(focus) -1
    }
    proc line_focus_right { } {
        variable Line

        if { $Line(focus) >  $Line(end) } { return }
        send_user [string index $Line(buffer) $Line(focus)]
        incr Line(focus) 1
    }

    proc line_focus_begin { } {
        variable Line
        set Line(focus) 0
        send_user "\r"
    }

    proc line_focus_end { } {
        variable Line
        set len [string length $Line(buffer)]
        send_user  [string range $Line(buffer) $Line(focus) end] 

        set Line(focus) [incr len 1]
    }

    proc line_move_focus {  pos } {
        variable Line
        if { $pos < 0 || $pos > [expr $Line(end) + 1]} { return }
        if { $pos < $Line(focus) } { 
            for { set i $Line(focus) } { $i > $pos } { incr i -1} {
                send_user "\b"
            }
            set Line(focus) $pos
        } elseif { $pos > $Line(focus) } { 
            set prePos [expr pos -1]
            send_user [string range $Line(buffer) $Line(focus) $prePos]
            set Line(focus) $pos
        }
    }

    # input one char
    proc line_input { ch} {
        variable Line
        set focus $Line(focus)
        incr focus -1
        set movable [string range $Line(buffer) $Line(focus) end]
        set Line(buffer) [string range $Line(buffer) 0 $focus]$ch$movable

        send_user $ch$movable

        set moveto [expr $Line(focus) + 1]
        incr Line(end) 1
        set Line(focus) [expr $Line(end) +1]
        line_move_focus $moveto
    }

    proc line_input_str { str } {
        set len [string length $str]
        for { set i 0 } { $i < $len } {incr i} {
            line_input [string index $str $i]
        }
    }

    proc line_del_rest { } {
        variable Line
        
        while { $Line(focus) != [expr $Line(end) + 1] } {
            line_del
        }
    }

    ## ::tkcon::ExpandBestMatch - finds the best unique match in a list of names
    ## The extra $e in this argument allows us to limit the innermost loop a
    ## little further.  This improves speed as $l becomes large or $e becomes long.
    # ARGS:	l	- list to find best unique match in
    # 	e	- currently best known unique match
    # Returns:	longest unique match in the list
    # ex: l = {{abcd} {abdf} {abdd} }
    # return ab
    #     l =  {{abcd} {abdf} {abdd} {sss} }
    # return ""
    ## 
    proc ExpandBestMatch {l {e {}}} {
        set ec [lindex $l 0]
        if {[llength $l]>1} {
            set e  [string length $e]; incr e -1
            set ei [string length $ec]; incr ei -1
            foreach l $l {
                while {$ei>=$e && [string first $ec $l]} {
                    set ec [string range $ec 0 [incr ei -1]]
                }
            }
        }
        return $ec
    }

    # find all strings start as $start from $candidates list
    # ex {abc abcd acc} start = abc
    #    return abc
    #    {abc abcd acc oper} start = o
    #    return oper
    # test code ----------------------
    # set a [list abcd abcf abccd abcefg defgh]
    # set b [FindMatch $a a]
    # send_user [ExpandBestMatch $b d]
    # return ; #------------------
    proc FindMatch {candidates start} {
        set r {}
        foreach one $candidates {
            if { [string first $start $one] == 0} {
                #send_user "$r\n"
                lappend r $one
            }
        }
        return $r
    }

    proc auto_complete {  } {
        variable Line
        
        #get chars from beginning to just before cursor
        set word [string rang $Line(buffer) 0 [expr $Line(focus) -1]]
        set b [FindMatch $Line(favors) $word]
        set b [ExpandBestMatch $b $word]
        # debug
        #send_user "\nai: $b\n"

        # only output extra part, ex b="abcde" word="ab"
        # output cde
        if {[string length $b] > [string length $word] } {
            set rest [string rang $b [string length $word] end]
            for { set i [string length $word] } {$i < [string length $b]} {incr i} {
                line_input [string index $b $i]
            }
        }
    }

    proc process_vkey { key } {
        variable Line

        # if is normal key 0x1c - 0x7e (i.e A-Za-z 0-9, signs)
        # put it in line/edit buffer, move cursor
        # else do special process

        # print out key value for debugging.
        #send_user "<map:$key>"

        switch $key {
            "INS" {
                #send_user "\x1b\[5;4;"
            }
            "DEL" {
                line_del 
            }
            "^d" { line_del }
            "LEFT" {
                line_focus_left 
            }
            "RIGHT" {
                line_focus_right 
            }
            "^c" {
            } 
            "BK" {  ; # end of if ESC
                #DEL E[3~
                set TER_ESC "\x1b"
                set TER_DEL "${TER_ESC}\[3~"
                line_bk 
            } 
            "RT" {  
                send_user "\r\n"
                # Add current line to bottom of  history list 
                # if different from previous cmd:
                #            line_add_history 
                set ::line_history $Line(buffer)
            } 
            UP {
                # current history line update
                #history list point moves up
                #clear display line buffer
                line_focus_begin; line_del_rest;
                # output history list point line
                line_input_str $::line_history
                
            }
            "TAB" { auto_complete }
            "HOME" { line_focus_begin }
            "^a" {  
                line_focus_begin 
                # ESC\[2A" cursor up 2 rows
            } 
            "END" {line_focus_end }
            "^e" {  
                line_focus_end 
            } 
            "^k" { line_del_rest }
            default {
                if { $key >= "\x20" && $key <= "\x7e" && [string length $key] == 1} { 
                    # put normal key in the end, otherwise such as
                    # "BK" also falls into this category

                    # put into user input buffer
                    line_input  $key
                }  
            }
        };# end of switch
    }

    proc test { } {
        fileevent stdin readable ""
        set k  [::vtermKey::getkey stdin]
        if {$k == "^c" } {
            exec stty -raw echo  
            global stop
            set stop 0
            return
        }
       # flush stdout
        process_vkey $k

        fileevent stdin readable ::lineEdit::test
    }
    # retun input line
    proc readLine { favors } {
        variable Line
        # intercept key press
        # TODO save stty settings

        line_clear
        set Line(favors) $favors

        
        flush stdout
        exec stty raw -echo 

        fconfigure stdin -buffering none -blocking 0
        fconfigure stdout -buffering none -blocking 0
        fileevent stdin readable ::lineEdit::test
        
        # TODO restore stty settings

        # clear line buffer
        set r $Line(buffer)
        line_clear


        return $r
    }


    proc send_user { str } {
        puts -nonewline $str
    }

}

# ---------------------- start of testing -------------------
set a 0
proc timer {} { 
    global a
    incr a
    after 5000 timer; 
    dsp_text 10 0 -----------------timeout$a 
    dsp_text 24 0 ""
    puts -nonewline "$::lineEdit::Line(buffer)  "
    dsp_text 24 [expr $::lineEdit::Line(focus) +1] ""
}

proc dsp_text { row col str} {
    puts -nonewline "\u001b\[$row;${col}H$str"
}

proc test { } {
    puts "start ---"
    after 1000 timer

    set favors {a b ccd defg deee}
    set line [::lineEdit::readLine $favors]   

    puts $line\n
}

source vtermkey.tcl
test
set stop 0
vwait stop



