# little XML parser
# http://wiki.tcl.tk/3919

 proc xml2list xml {
     regsub -all {>\s*<} [string trim $xml " \n\t<>"] "\} \{" xml
     set xml [string map {> "\} \{#text \{" < "\}\} \{"}  $xml]

     set res ""   ;# string to collect the result
     set stack {} ;# track open tags
     set rest {}
     foreach item "{$xml}" {
         switch -regexp -- $item {
            ^# {append res "{[lrange $item 0 end]} " ; #text item}
            ^/ {
                regexp {/(.+)} $item -> tagname ;# end tag
                set expected [lindex $stack end]
                if {$tagname!=$expected} {error "$item != $expected"}
                set stack [lrange $stack 0 end-1]
                append res "\}\} "
          }
            /$ { # singleton - start and end in one <> group
               regexp {([^ ]+)( (.+))?/$} $item -> tagname - rest
               set rest [lrange [string map {= " "} $rest] 0 end]
               append res "{$tagname [list $rest] {}} "
            }
            default {
               set tagname [lindex $item 0] ;# start tag
               set rest [lrange [string map {= " "} $item] 1 end]
               lappend stack $tagname
               append res "\{$tagname [list $rest] \{"
            }
         }
         if {[llength $rest]%2} {error "att's not paired: $rest"}
     }
     if [llength $stack] {error "unresolved: $stack"}
     string map {"\} \}" "\}\}"} [lindex $res 0]
 }


#---- Now that this went so well, I'll throw in the converse:

 proc list2xml list {
    set res ""
    switch -- [llength $list] {
        2 {lindex $list 1}
        3 {
            foreach {tag attributes children} $list break
            set res <$tag
            foreach {name value} $attributes {
                append res " $name=\"$value\""
            }
            if [llength $children] {
                append res >
                foreach child $children {
                    append res [list2xml $child]
                }
                append res </$tag>
            } else {append res />}
        }
        default {error "could not parse $list"}
    }
    return $res
 }



#-------------------------------------------- now testing:
proc generate_xml_example { } {
 return {<foo a="b">bar and<grill x:c="d" e="f g"><baz x="y"/></grill><room/></foo>}
}

proc tdomlist x {[[dom parse $x] documentElement root] asList} ;# reference
proc lequal {a b} {
    if {[llength $a] != [llength $b]} {return 0}
    if {[lindex $a 0] == $a} {return [string equal $a $b]}
    foreach i $a j $b {if {![lequal $i $j]} {return 0}}
    return 1
}

proc try x {
    puts [set a [tdomlist $x]]
    puts [set b [xml2list $x]]
    puts list:[lequal $a $b],string:[string equal $a $b]
}


#puts [set res [xml2list [generate_xml_example]]]



proc forall {varName list body} {
    set $varName $list
    eval $body
    foreach child [lindex $list 2] {
        forall $varName $child $body
    }
} ;

set file_name [lindex $argv 0]

set fd [open $file_name r]
set a [read  $fd] 
close $fd


# remove doc type
#<?xml-stylesheet type="text/xsl" href="MeasDataCollection.xsl"?>
#<!DOCTYPE mdc SYSTEM "MeasDataCollection.dtd">

regsub -all {<\?[^\?]*\?>} $a "" a
regsub -all "<!\[^>]*>" $a "" a

#puts $a
set a [xml2list $a]

#bypass mdc, the root
set a [lindex $a 2]


#--------------------------------------------
proc extract_txt {xml_list} {
    return [lindex [lindex [lindex $xml_list 2] 0] 1]
}

proc extract_row_value {elem_list} {
    set each_row {}
    foreach row $elem_list {
        switch -- [lindex $row 0] {
            moid {
                # cdmachannel is a special case
                set mo_name  [extract_txt $row]
                regsub -all "\[^,]*," $mo_name "" row_name
                set d "(\[0-9]+)"
                if {[regexp "Site=$d,Cell=$d,CDMAChannel=$d" $mo_name dummy site cell ch]} { 
                    lappend each_row "CDMAChannel=$site:$cell:$ch"
                } else {
                    lappend each_row $row_name                        
                }
            }
            r {
                lappend each_row [extract_txt $row]
            }
        }; # end of swith
    } ; #end of foreach
    return $each_row
}



#-------------------------
# proc RpTNInterface_ccpd {fields f_value } {
#     return $f_value
# }

proc comma_sum { v } {
    set sum 0
    foreach value [split $v "," ] {
        set sum [expr $sum + $value]
    }
    return $sum
}

proc list_comma_sum { row idx } {
    foreach i $idx {
        set v [lindex $row $i]
        set v [comma_sum $v]
        lset row $i $i:$v
    }
    return $row
}

#12, 6, 28, 4, 20, 39 cpu load (hub, dev1-5)
proc myis2kbss_row { row } {
    array set dev_name {
        12 HSR
        6 DSR1
        28 DSR2
        4 DSR3
        20 DSR4
        39 DSR5
    }
    foreach i {8 14 16 31 35 40 43 45 50} {
        set v [lindex $row $i]
        set v [comma_sum $v]
        lset row $i $i:$v
    }
    foreach i {12 6 28 4 20 39} {
        set v [lindex $row $i]
        set sum [comma_sum $v]
        if {$sum != 0} {            
            set cpu_list [split $v ","]
            set slot 0; set cpu_idx [expr $slot *11];
            set ave_load 0; set sum_print "";
         #   puts $cpu_list
            while { [lindex $cpu_list $cpu_idx] != "0" } {
                set bin_list [lrange $cpu_list [expr $cpu_idx +1] [expr $cpu_idx +10]]
#                puts $bin_list
                set weight 1;
                foreach bin $bin_list {
#                    puts "bin: $bin"
                    set ave_load [expr $ave_load + $weight * $bin]
                    incr weight
                }
                # 1+2+3+4+5+6+7+8+9+10=55
                set test_load [expr $ave_load /900]
                if {$test_load > 2} {
                    set ave_load [expr $ave_load*10/900]
                    set sum_print "${sum_print}_${slot}=$ave_load"
                }
                incr slot; set cpu_idx [expr $slot *11];
            }
            
            lset row $i "${i}:$dev_name($i)$sum_print"
        } else {
            lset row $i "$i:no_$dev_name($i)"
        }
    }

    return $row
}

proc PCF_ccpd {fields f_value } {
    set result {}
    foreach r $f_value {
        lappend result [list_comma_sum $r 3]
    }
    return $result
}

proc RpTNInterface_ccpd {fields f_value } {
    set result {}
    foreach r $f_value {
        set row [lindex $r 0]
        foreach vol [lrange $r 1 end] {
            set vol [expr $vol / 900.0 / 1000]
            lappend row "${vol}K/sec"
        }
        
        lappend result $row
    }

    return [lsort $result]
}

proc GeneralProcessorUnit_ccpd {fields f_value } {
    set result {}
    foreach r $f_value {
        set row [lindex $r 0]
        foreach vol [lrange $r 1 end] {
            set vol [expr $vol / 1000000]
            lappend row ${vol}M
        }
        
        lappend result $row
    }

    return [lsort $result]
}
#12, 6, 28, 4, 20, 39 cpu load (hub, dev1-5)
proc MyIS2000BSS_ccpd {fields f_value } {
    set result {}
    foreach r $f_value {
        lappend result [myis2kbss_row $r]
    }
    return $result
}

proc is_list_zero { v } {
#    puts $v
    foreach value $v {
        regsub "\[^:]+:" $value "" number
        if {$number != 0} { return 0}
    }
    return 1
}
proc CDMAChannel_ccpd {fields f_value } {
    set result {}
    foreach r $f_value {
        set v [list_comma_sum $r {7 }]
        if {[is_list_zero [lrange $v 1 end]] } { continue } 

        set row [lindex $v 0]
        set idx 1
        foreach value [lrange $v 1 end] {
            # take out counters which are zero
            if {$value != 0 } { lappend row $idx:$value }
            incr idx
        }
        lappend result $row
    }

    return [lsort $result ]
}

# proc SignalingLink_ccpd {fields f_value } {
#     return $f_value
# }

# proc Mtp2TpChina_ccpd {fields f_value } {
#     return $f_value
# }

proc SidehaulSpan_ccpd {fields f_value } {
    return [lsort $f_value]
}

proc BackhaulSpan_ccpd {fields f_value } {
    return [lsort $f_value]
}

# proc RpLink_ccpd {fields f_value } {
#     return $f_value
# }

# proc ManagedElementData_ccpd {fields f_value } {
#     return $f_value
# }

# proc Mtp3bSlChina_ccpd {fields f_value } {
#     return $f_value
# }


proc process_ccpd {fields f_value} {
    set mo_name [lindex [lindex $f_value 0] 0]
#    puts $mo_name
    regexp "(\[^=]+)" $mo_name dummy mo_type
#    puts $mo_type
    if {[lsearch [info procs] ${mo_type}_ccpd] != -1} {
        return [eval {${mo_type}_ccpd $fields $f_value}]
    } else {
        return $f_value
    }
}

foreach n $a {
    if {[lindex $n 0] != "md" } { continue }

    foreach n2 [lindex $n 2] {
        if {[lindex $n2 0] != "mi"} { continue }

        set fields {}
        set f_value {}
        foreach elem [lindex $n2 2] {
            switch -- [lindex $elem 0] {
                mt { lappend fields [extract_txt $elem] }
                mv { lappend f_value [extract_row_value [lindex $elem 2]] }
            };# end of switch elem tag
        }; # end of foreach elem n2
        set idx 1; set output ""
        foreach f $fields { set output "$output $idx:$f"; incr idx}
        puts $output
#        puts $f_value
        set f_value [process_ccpd $fields $f_value]        
        foreach r $f_value { puts $r }
        puts "----------------------"
    }    
} 