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

# Terminal key maps ---------------------------------------------
# \x20 SP
# \x21 ! ... \x30 0 ... \x3a : ... \x41 A ... \x5b [ ... \x61 a \x7b { ... \x7e ~ 
# \x0 NULL, \x1 - \x1a ^A - ^Z, \x1b ESC \x7f (^ means Ctrl-)
# \x9 TAB, \x7f BK (Backspace), \xd RETURN
# \x1c \x1b ^[ or ^3 \x1c \x1d ^] \x1e \x1f ^/
# ^3 - ^7 \x1c - \x1f \x1f ^9
# UP ~[A, DOWN ~[B, RIGHT ~[C, LEFT ~[D (~ means Escape or ESC)
# F1 - F4 ~OP - ~OS, F5 E[15~ F6-F8 E[17~-E[19~ 
# F9 E[20~, F10 E[21~, F11 E[23~, F12 E[24~
# INS E[2~ (E means ESC, ~ is the real ~)
# HOME ~[H
# PAGEUP E[5~
# PAGEDOWN E[6~
# DEL E[3~
# END ~[F
# end of Termninal key maps ------------------------------------

# solaris map 
# <ctrl-m>, <ctrl-j> and <return> map to 0x0a
# <ctrl-h>  and <backspace> mapt to 0x08

namespace eval ::vtermKey {
    namespace export getkey vkey
    variable channel vkey
    
    proc getkey { channel_to_read } {
        variable channel
        set channel $channel_to_read
        
        set k [readByte]
        if { $k == "" } { return ""}
        return [map_to_vkey $k]
    }

    set ascii_map { 
        \x0 x0 \x1 x1 \x2 x2 \x3 x3 \x4 x4 \x5 x5 \x6 x6 \x7 x7
        \x8 x8 \x9 x9 \xa RT \xb xb \xc xc \xd xd \xe xe \xf xf
        \x10 x10 \x11 x11 \x12 x12 \x13 x13 \x14 x14 \x15 x15 \x16 x16 \x17 x17
        \x18 x18 \x19 x19 \x1a x1a \x1b x1b \x1c x1c \x1d x1d \x1e x1e \x1f x1f
        \x7f x7f
    }

    # ?pc keyboard map
    set pc_ascii_map { 
        \x0 x0 \x1 ^a \x2 ^b \x3 ^c \x4 ^d \x5 ^e \x6 ^f \x7 ^g
        \x8 ^h \x9 TAB \xa ^j \xb ^k \xc xc \xd RT \xe xe \xf xf
        \x10 x10 \x11 x11 \x12 x12 \x13 x13 \x14 x14 \x15 x15 \x16 x16 \x17 x17
        \x18 x18 \x19 x19 \x1a x1a \x1b ESC \x1c x1c \x1d x1d \x1e x1e \x1f x1f
        \x7f BK
    }

    # pc keyboard map
    proc pc_map_one_key {  key } {
        switch $key {
            "\x0" { return x0 }
            "\x1" { return ^a }
            "\x2" { return ^b }
            "\x3" { return ^c }
            "\x4" { return ^d }
            "\x5" { return ^e }
            "\x6" { return ^f }  
            "\x7" { return ^g }
            "\x8" { return ^h } ;#unix Backspace
            "\x9" { return TAB }
            "\xa" { return ^j }
            "\xb" { return ^k }
            "\xc" { return xc }
            "\xd" { return xd }
            "\xe" { return ^n }
            "\xf" { return ^o }
            "\x10" { return ^p }
            "\x11" { return ^q }
            "\x12" { return ^r }
            "\x13" { return ^s }
            "\x14" { return ^t }
            "\x15" { return ^u }
            "\x16" { return x16 }
            "\x17" { return ^w }
            "\x18" { return ^x }
            "\x19" { return ^y }
            "\x1a" { return ^z }
            "\x1b" { return ESC }
            "\x1c" { return x1c }
            "\x1d" { return x1d }
            "\x1e" { return x1e }
            "\x1f" { return x1f }
            "\x7f" { return BK }
            default { return $key }
        }
    }

    # solaris  keymap
    proc map_one_key {  key } {
        switch $key {
            "\x0" { return x0 }
            "\x1" { return ^a }
            "\x2" { return ^b }
            "\x3" { return ^c }
            "\x4" { return ^d }
            "\x5" { return ^e }
            "\x6" { return ^f }  
            "\x7" { return ^g }
            "\x8" { return BK } 
            "\x9" { return TAB }
            "\xa" { return RT } 
            "\xb" { return ^k }
            "\xc" { return ^l }
            "\xd" { return xd }
            "\xe" { return xe }
            "\xf" { return xf }
            "\x10" { return x10 }
            "\x11" { return x11 }
            "\x12" { return x12 }
            "\x13" { return x13 }
            "\x14" { return x14 }
            "\x15" { return x15 }
            "\x16" { return x16 }
            "\x17" { return x17 }
            "\x18" { return x18 }
            "\x19" { return x19 }
            "\x1a" { return x1a }
            "\x1b" { return ESC }
            "\x1c" { return x1c }
            "\x1d" { return x1d }
            "\x1e" { return x1e }
            "\x1f" { return x1f }
            "\x7f" { return BK }
            default { return $key }
        }
    }

    # to use tcl 8.3 implement string map
    proc string_map { map key } {
        #TODO detect keyboard layout
        set str ""
        for { set i 0 } { $i < [string length $key] } {incr i} {
            append str [map_one_key [string index $key $i]]
        }
        return $str
    }

    proc map_to_vkey { key } {
        variable ascii_map
        # if is normal key 0x1c - 0x7e (i.e A-Za-z 0-9, signs)
        # put it in line/edit buffer, move cursor
        # else do special process

        # print out key value for debugging.
        set vkey [string_map $ascii_map $key]
        # out put key for debugging 
        #send_user "<map:$vkey>"

        # save ESC sequence here
        set esc ""
        if { $vkey != "ESC" } {
            return $vkey
        }

        #capture ESC and special key sequences like ~[A
        send_user "d: ESC"
        # TODO: may need to change the timeout
        set k1 [readByte]
        switch $k1 {
            "\[" {
                set k2 [readByte]
                switch $k2 {
                    "F" { return END }
                    "H" { return HOME }
                    "A" { return UP }
                    "B" { return DOWN }
                    "C" { return RIGHT }
                    "D" { return LEFT }
                    2 { 
                        set k3 [readByte]; 
                        switch $k3 {
                            ~ { return INS }
                            0 { readByte; return F9 }
                            1 { readByte; return F10 }
                            3 { readByte; return F11 }
                            4 { readByte; return F12 }
                        }
                    }
                    5 { readByte; return PUP }
                    3 { readByte; return PDOWN }
                    6 { readByte; return DEL }
                    1 {
                        set k3 [readByte]; 
                        switch $k3 {                  
                            5 {  readByte; return F5 }
                            7 {  readByte; return F6 }
                            8 {  readByte; return F7 }
                            9 {  readByte; return F8 }
                            
                        }
                    }
                }
            }
            "O" {
                set k2 [readByte]
                switch $k2 {
                    "P" { return F1 }
                    "Q" { return F2 }
                    "R" { return F3 }
                    "S" { return F4 }               
                }
            }
        }
        # should be 2 , 3 or 4 keys after ESC
        
        return "<$vkey NotMapped>"
    }

    proc readByte { } {
        variable channel
        return [read $channel 1]
    }
} ; # end of namespace vtermKey
