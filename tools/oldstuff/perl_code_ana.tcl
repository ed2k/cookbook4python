proc IsCommentsLine { line } {
    # start with pound sign
    set idx [string first "\#" [string trim $line " \t"]]
    if {$idx == 0} { return true }
    return false
}
proc NoCurly { line } {
    return [regexp "\[\{\}]" $line] 
}

proc ReplaceMatchedCurly { line } {
    # simplify right now
    # count 
    set stack_level 0
    set len [string length $line]
    for {set i 0} {$i < $len} {incr i} {
        set char [string index $line $i]
        if {$char == "\{" }  {
#            set line [string replace $line $i $i "%lc"]
            incr stack_level
        } elseif {$char == "\}" } {
#            set line [string replace $line $i $i "%rc"]
            incr stack_level -1
        }                   
                   
    }
    set line [string map {"\{" "%lc" "\}" "%rc"} $line]
 
    if { $stack_level > 0 } {
        set line $line[string repeat "\{" $stack_level]
    } elseif { $stack_level < 0 } {
#        puts stack:$stack_level
        set i [expr -$stack_level]
#        puts "rc: $i"
        set line $line[string repeat "\}" $i]
    }
    return $line
}


proc forall {varName list body} {
    set $varName $list
    eval $body
    foreach child [lindex $list 2] {
        forall $varName $child $body
    }
}

#------------- MAIN -----------------
set fd_in [open [lindex $argv 0]]
set buffer [read $fd_in]

set stack_level 0

# take out curly bracket block
set idx_lc [string first "\{" $buffer]
set idx_rc [string first "\}" $buffer]

# one true left curly push block down one level, incr stack_level
# one true right curly pop block up on level, incr stack_level -1


set lines [split $buffer "\n"]

# try to map any source into [tag attributes childrean] xml like tree.
# tag == 
# attributes ==
# children == subblock

#regsub -all ";" $buffer "<end>" buffer
#set buffer [string map {; <end> "\n" " " "\t" " "  "\r" " "} $buffer ]

#puts $buffer

set idx 0
set prev_tag ""
set result ""



foreach b $lines {
#    puts line:$b

    set b [string map {"\\\{" "\\%lc"} $b]
    set b [string map {"\\\}" "\\%rc"} $b]
#    set b [string map {"eval" "%ev"} $b]
#    puts line:$b    

    if {[IsCommentsLine $b] == "true" } {

        set b [string map {"\{" "%lc"} $b]
        set b [string map {"\}" "%rc"} $b]

        if { $prev_tag != "comments" } {
 #           puts "new comments"
            append result "{blocks {} {{bb {} {{#text {$b}} } } } } "
#            append result "\{comments \{\} \{\{#text \{$b\} \} \} \} "
#            puts newcomm:$result
        } else {
            # back two curly bracket
            set i [string last "\}" $result]
            set i [string last "\}" $result [expr $i -1]]
            incr i -1
            set result "[string range $result 0 $i] "
            append result "{comments {} {{#text {$b}} } } \} \} "
        }
        set prev_tag comments
    } else {
      set prev_tag ""
      set b [ReplaceMatchedCurly $b]
      
      if { [regexp "\[\{\}]" $b] == 0   } {
        set prev_tag ""
        #set statements [split $b ";"]
#        puts nocurly:$b
        append result "\{statements \{\} \{\{#text \{$b\} \} \} \} "
      } else { 

        # just consider simply case for now, each line only have at most one curly
        set idx_lc [string first "\{" $b]
        if { $idx_lc > -1 } {
            set b [string replace $b $idx_lc $idx_lc ""]
            if { [regexp "\[\{\}]" $b] == 1   } { 
                # assume its array for simplicity now and preprocessed already
                error "E1 $idx multiple curly $b"
            } else {
                # leave 2 curly open
#                puts $b
                append result "\{blocks {} \{{bb {} {{#text {$b}} } } "
            }
        } else {
            set idx_rc [string first "\}" $b]
            set b [string replace $b $idx_rc $idx_rc ""]
            if { [regexp "\[\{\}]" $b] == 1   } {
#                error "E2 $idx multiple curly $b"
                set idx_rc [string first "\}" $b]
                set b [string replace $b $idx_rc $idx_rc ""]                
                append result "\} \} "; 
                if { [regexp "\[\{\}]" $b] == 1   } {
                    error "E2 $idx multiple curly $b"
                }
            }
            # close
            # assume it is statements not comments
            append result "{statements {} {{#text {$b}} } } \} \} "; 
        }
    }
   };# end of if comments
    
    if {[regexp "^=\[h|p]"  $b] } {
        # the end of perl code syntax
        #append result "\{htmldoc \{\} \{[lrange $lines $idx end] \} \} "
#        puts "perl html doc $b"
        break;
    }

    incr idx 
}; # end of for line



set result "perlfile {} \{$result \} "

#puts "-------- $result -----------------"

foreach child  [lindex $result 2] {
#    puts $child
    foreach {tag line children} $child {
        #    puts [lindex $b 0]
        if {$tag == "comments"} {
            foreach c $children {
#                puts "C: $c"
            }
        } elseif {$tag == "statements" } {
            foreach c $children {
#                puts "S: $c"
            }
        } else {
#            puts B:$children
        }

    }
}


 proc list2xml list {
     set res ""
    switch -- [llength $list] {
        1 { return ""; }
        2 { #1,2 both text case, 
#            puts 2----------$list
            set a [lindex $list 1]
            set a [string map {"%lc" "\{" "%rc" "\}" "&" "\&amp;" } $a]
            set a [string map {">" "\&gt;" "<" "\&lt;"} $a]
            # ctrl-V ctrl-[
            set a [string map { "" "%ctrlV[;"} $a]
            # ctrl-V ctrl-G
            set a [string map { "" "%ctrlVG;"} $a]
            return $a
        }
        3 {
#            puts 3--------$list
            foreach {tag attributes children} $list {
                append res <$tag
                foreach {name value} $attributes {
                    append res " $name=\"$value\""
                }
                if [llength $children] {
                    append res >
                    foreach child $children {
                        append res [list2xml $child]
                    }
                    append res "</$tag>\n"
                } else {append res />}
            }
        }
        default {
            puts "could not parse len:[llength $list] $list"
            puts "err: [lindex $list end]"
        }
    }
    return $res
 }

# output to xml format
puts "<?xml version=\"1.0\"?>\n<?xml-stylesheet type=\"text/xsl\" href=\"tree.xsl\"?>"
set a [list2xml $result]
puts $a

