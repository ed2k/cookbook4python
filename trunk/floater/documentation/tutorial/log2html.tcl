proc includecmd {filename} {
    set f [open $filename r]
    while {[gets $f s] >= 0} {puts $s}
    close $f
}

proc c {name} {
    return "<tt><a href=ref.html#$name>$name</a></tt>"
}

proc include {filename} {
    if {[string index $filename 0] != "|"} {puts <pre>}
    set ext .txt
    if {[string index $filename 0] != "|"} {set filename gtext$filename$ext}
    puts "<!-- including $filename -->"
    includecmd $filename
    if {[string index $filename 0] != "|"} {puts </pre>}
}

proc command args {
    puts "<tt>$args</tt>"
}
