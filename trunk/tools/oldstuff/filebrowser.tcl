#!/bin/sh
# the next line restarts using wish\
exec wish "$0" "$@" 

if {![info exists vTcl(sourcing)]} {

    # Provoke name search
    catch {package require bogus-package-name}
    set packageNames [package names]

    package require BWidget
    switch $tcl_platform(platform) {
	windows {
	}
	default {
	    option add *ScrolledWindow.size 14
	}
    }
    
    package require Tk
    switch $tcl_platform(platform) {
	windows {
            option add *Button.padY 0
	}
	default {
            option add *Scrollbar.width 10
            option add *Scrollbar.highlightThickness 0
            option add *Scrollbar.elementBorderWidth 2
            option add *Scrollbar.borderWidth 2
	}
    }
    
}

#############################################################################
# Visual Tcl v1.60 Project
#


#################################
# VTCL LIBRARY PROCEDURES
#

if {![info exists vTcl(sourcing)]} {
#############################################################################
## Library Procedure:  Window

proc ::Window {args} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    global vTcl
    foreach {cmd name newname} [lrange $args 0 2] {}
    set rest    [lrange $args 3 end]
    if {$name == "" || $cmd == ""} { return }
    if {$newname == ""} { set newname $name }
    if {$name == "."} { wm withdraw $name; return }
    set exists [winfo exists $newname]
    switch $cmd {
        show {
            if {$exists} {
                wm deiconify $newname
            } elseif {[info procs vTclWindow$name] != ""} {
                eval "vTclWindow$name $newname $rest"
            }
            if {[winfo exists $newname] && [wm state $newname] == "normal"} {
                vTcl:FireEvent $newname <<Show>>
            }
        }
        hide    {
            if {$exists} {
                wm withdraw $newname
                vTcl:FireEvent $newname <<Hide>>
                return}
        }
        iconify { if $exists {wm iconify $newname; return} }
        destroy { if $exists {destroy $newname; return} }
    }
}
#############################################################################
## Library Procedure:  vTcl:DefineAlias

proc ::vTcl:DefineAlias {target alias widgetProc top_or_alias cmdalias} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    global widget
    set widget($alias) $target
    set widget(rev,$target) $alias
    if {$cmdalias} {
        interp alias {} $alias {} $widgetProc $target
    }
    if {$top_or_alias != ""} {
        set widget($top_or_alias,$alias) $target
        if {$cmdalias} {
            interp alias {} $top_or_alias.$alias {} $widgetProc $target
        }
    }
}
#############################################################################
## Library Procedure:  vTcl:DoCmdOption

proc ::vTcl:DoCmdOption {target cmd} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    ## menus are considered toplevel windows
    set parent $target
    while {[winfo class $parent] == "Menu"} {
        set parent [winfo parent $parent]
    }

    regsub -all {\%widget} $cmd $target cmd
    regsub -all {\%top} $cmd [winfo toplevel $parent] cmd

    uplevel #0 [list eval $cmd]
}
#############################################################################
## Library Procedure:  vTcl:FireEvent

proc ::vTcl:FireEvent {target event {params {}}} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    ## The window may have disappeared
    if {![winfo exists $target]} return
    ## Process each binding tag, looking for the event
    foreach bindtag [bindtags $target] {
        set tag_events [bind $bindtag]
        set stop_processing 0
        foreach tag_event $tag_events {
            if {$tag_event == $event} {
                set bind_code [bind $bindtag $tag_event]
                foreach rep "\{%W $target\} $params" {
                    regsub -all [lindex $rep 0] $bind_code [lindex $rep 1] bind_code
                }
                set result [catch {uplevel #0 $bind_code} errortext]
                if {$result == 3} {
                    ## break exception, stop processing
                    set stop_processing 1
                } elseif {$result != 0} {
                    bgerror $errortext
                }
                break
            }
        }
        if {$stop_processing} {break}
    }
}
#############################################################################
## Library Procedure:  vTcl:Toplevel:WidgetProc

proc ::vTcl:Toplevel:WidgetProc {w args} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    if {[llength $args] == 0} {
        ## If no arguments, returns the path the alias points to
        return $w
    }
    set command [lindex $args 0]
    set args [lrange $args 1 end]
    switch -- [string tolower $command] {
        "setvar" {
            foreach {varname value} $args {}
            if {$value == ""} {
                return [set ::${w}::${varname}]
            } else {
                return [set ::${w}::${varname} $value]
            }
        }
        "hide" - "show" {
            Window [string tolower $command] $w
        }
        "showmodal" {
            ## modal dialog ends when window is destroyed
            Window show $w; raise $w
            grab $w; tkwait window $w; grab release $w
        }
        "startmodal" {
            ## ends when endmodal called
            Window show $w; raise $w
            set ::${w}::_modal 1
            grab $w; tkwait variable ::${w}::_modal; grab release $w
        }
        "endmodal" {
            ## ends modal dialog started with startmodal, argument is var name
            set ::${w}::_modal 0
            Window hide $w
        }
        default {
            uplevel $w $command $args
        }
    }
}
#############################################################################
## Library Procedure:  vTcl:WidgetProc

proc ::vTcl:WidgetProc {w args} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    if {[llength $args] == 0} {
        ## If no arguments, returns the path the alias points to
        return $w
    }
    ## The first argument is a switch, they must be doing a configure.
    if {[string index $args 0] == "-"} {
        set command configure
        ## There's only one argument, must be a cget.
        if {[llength $args] == 1} {
            set command cget
        }
    } else {
        set command [lindex $args 0]
        set args [lrange $args 1 end]
    }
    uplevel $w $command $args
}
#############################################################################
## Library Procedure:  vTcl:toplevel

proc ::vTcl:toplevel {args} {
    ## This procedure may be used free of restrictions.
    ##    Exception added by Christian Gavin on 08/08/02.
    ## Other packages and widget toolkits have different licensing requirements.
    ##    Please read their license agreements for details.

    uplevel #0 eval toplevel $args
    set target [lindex $args 0]
    namespace eval ::$target {set _modal 0}
}
}


if {[info exists vTcl(sourcing)]} {

proc vTcl:project:info {} {
    set base .top43
    namespace eval ::widgets::$base {
        set set,origin 1
        set set,size 1
        set runvisible 1
    }
    namespace eval ::widgets::$base.tre44 {
        array set save {-highlightcolor 1 -selectbackground 1 -selectforeground 1}
    }
    namespace eval ::widgets::$base.tex46 {
        array set save {-background 1 -insertbackground 1}
    }
    namespace eval ::widgets::$base.scr48 {
        array set save {-command 1}
    }
    namespace eval ::widgets::$base.tex47 {
        array set save {-background 1 -insertbackground 1}
    }
    namespace eval ::widgets::$base.scr49 {
        array set save {}
    }
    namespace eval ::widgets_bindings {
        set tagslist _TopLevel
    }
    namespace eval ::vTcl::modules::main {
        set procs {
            init
            main
            ls_dir
            open_node
            drop_node
            fill_subs
            select_node
            build_cp_cmd
            double_click_node
            scroll_tree
            sync_tree_scroll
            con_dir
            source_text_popup
        }
        set compounds {
        }
        set projectType single
    }
}
}

#################################
# USER DEFINED PROCEDURES
#
#############################################################################
## Procedure:  main

proc ::main {argc argv} {
#  global smart_expand_symbol
#  global smart_collapse_symbol
#  set smart_expand_symbol ">>>>"
#  set smart_collapse_symbol "<<<<"

       
    # add a y scroll bar for source_text
    # scr49 <-> tex47
    #    scrollbar .top43.sy -orient v -takefocus 0 -bd 1 
    #     -command [list source_text xview]                 
                     
    set w source_tree
    # set up first level directories
    set dirs [ls_dir ""]
    foreach d $dirs {
        #puts $d
        set uid [lindex $d 0]
        if { ![$w exists $uid] } {
            $w insert end root $uid -text [leave_last [lindex $d 1] 20 ]
            set sub [lindex $d 0]
            fill_subs $w $sub
        }
        
    }

    # configure source tree
    $w configure -takefocus 1 -dragenabled 1 -dropenabled 1 -opencmd {open_node source_tree}  -dropcmd drop_node -selectcommand {select_node} -yscrollcommand [list .top43.scr48 set]
    $w bindText <Double-Button-1> {double_click_node source_tree}
#    $w bindText <
#    $w bindText <Key-Page_Up> {source_tree yview scroll -1 page}
#    $w bindText <Key-Page_Down> {source_tree yview scroll 1 page}
    bind .top43.tre44 <Key-Page_Up> {source_tree yview scroll -1 page}
    bind .top43.tre44 <KeyPress> { puts "key press" }

    menu .popup_menu
    
    set w source_text
    $w configure -wrap word
#    $w configure -wrap word -xscrollcommand [list .top43.sx set]
#    grid $w - .top43.sx -sticky news

#    set w .top43
#    grid co


    # setting for command text window
    set w  command_text
    bind .top43.tex46 <Return> eval_cmd
    
}



#############################################################################
## Procedure:  ls_dir

proc ::ls_dir {dir_ {full_list 0}} {
    # only ls directory
    global widget

    global vTcl

    # file Iwidegt node, called by Iwidget hierachy querycommand
    # node uid is the input $dir_ == %n

    if {! [info exists vTcl(hierarchy,root)] } {
        # a kinda unix-centric view
        # creat top level directory
        # guess what the user intends to copy from and to
        # 1. from current directory    
        set vTcl(hierarchy,root) "nothing"
    }

    if {$dir_ == ""} {
        # construct the top level directory

        # load history
        set total [get_history dir]

        set r ""

        # assume recently used first
        set cnt 0
        foreach d $total {
            #TODO: regulate length of node text
            lappend r [list $d $d]
        }

# do we need to set user home diretory shortcut?
#        lappend r [list ~ ~ ]
        lappend  r [list "/" "/"]
        return $r

    } else {

        set dir $dir_
    }

#  list subdirectory continue
    if {! [file isdirectory $dir]} {
        return ""
    }

    # if permission denied return immediately
    # throw error to some error window
    if { [catch {cd $dir} error] } { return "" }


    global symbol
    set sym $symbol(expand)

    set returnList ""
    set i 0

    foreach file [lsort [glob -nocomplain *]] {
        #puts "ls: $file"
        if { [file isdirectory $file] } {
            incr i
            lappend returnList [list [file join $dir $file] $file]
            #puts "$i $dir $file"
            if { $i > 10  && !$full_list} {
                lappend returnList [list "$dir$sym" "$sym"]
#                puts $returnList
                return $returnList  
            }
        }; # 
    }; # endof foreach file

    return $returnList
}
#############################################################################
## Procedure:  open_node

proc ::open_node {w n} {
    global widget

    #puts "open_node: $w $n"

    foreach child [$w nodes $n] {
        #puts "open_node: $child"
        fill_subs $w $child
    }
    set xy   [$w yview ]
    Scrollbar1 set [lindex $xy 0] [lindex $xy 1] 
}
#############################################################################
## Procedure:  drop_node

proc ::drop_node {w sn tn op dt data} {
global widget

# w the pathname of the tree,
# sn: the pathname of the drag source, 
# tn: a list describing where the drop occurs.
# It can be:  {widget}, {node node} or {position node index}. 
# op: the current operation, 
# dt: the data type, 
# the data.
# 

puts "drop_node: $w $sn \{$tn\} $op $dt $data"
}
#############################################################################
## Procedure:  fill_subs

proc ::fill_subs {w n} {
global widget

# given a tree widget $w, 
# fill in its subnode of $n
# note, use abs filename as node uid (i.e node name)

# TODO: limit node text to last some text
        set subs [ls_dir $n]
        #puts "fill_subs: $subs"
        foreach s $subs {
            #if already filled in, do not fill again
            if { ![$w exists [lindex $s 0]] } {
                $w insert  end $n  [lindex $s 0] -text [leave_last [lindex $s 1] 20]
            }
        }
}
#############################################################################
## Procedure:  select_node

proc ::select_node {w n} {
# TODO: integrate tkcon into source text
    global widget

    #puts "select_node: $w $n"

    # special directory treat differently
    global symbol
    set sym $symbol(expand)

    set i [string last "$sym" $n]
    if { $i != -1 } { return }  
    set sym $symbol(collapse)
    set i [string last "$sym" $n]
    if { $i != -1 } { return }

    add_history dir $n

    command_text delete 0.0 end
    command_text insert insert "ls $n"

    cd $n 
    set t source_text
    $t delete 0.0 end
    $t insert insert "cd $n ; ls \n"

    # format output
    # todo: follow bash convention dir/ exe* coloring
    bind .top43.tex47 <Button-3> {source_text_popup %X %Y}
    foreach f [glob -nocomplain *]  {
        set first [$t index insert]
        if {![in_hide_list $f] } {
            $t insert insert "$f "
            $t tag add file $first [$t index insert-1c]
        }
    }
#    $t insert insert "\n$n > "
}
#############################################################################
## Procedure:  build_cp_cmd

proc ::build_cp_cmd {src dst} {
global widget

command_text insert "cp $src $dst"
}
#############################################################################
## Procedure:  double_click_node

proc ::double_click_node {w n} {
    global widget
    global symbol
    #puts "dblclick_node: $n"
    
    # find if it is a special diretory
    set sym $symbol(expand)
    set i [string last "$sym" $n]
    if { $i == -1 } { return }
    
    # special directory, focus on it
    # list it subdir in full
    set len [string length $n]
    set dir [string replace $n $i $len]
    
    puts "dblclick_node: special dir $dir"
    
    # force list all available contents
    set dirs [ls_dir $dir 1]
    foreach d $dirs {
        #        puts $d
        set uid [lindex $d 0]
        if { ![$w exists $uid] } {
            #insert before expand_dir
           set idx [$w index $n]
           $w insert $idx  $dir $uid -text [lindex $d 1]
           fill_subs $w $uid
       }     
        
    }
    
    #    change this special node to collapse
    #    function, such <<<
    global symbol
    set sym $symbol(collapse)
    set idx [$w index $n]
    $w insert $idx  $dir "$dir$sym" -text "$sym"
    $w delete $n
}
#############################################################################
## Procedure:  scroll_tree

proc ::scroll_tree {op n {ut {}}} {
    global widget
# this function is useless now
# op = scroll n ut = unit/page
# op = moveto n = pos relative to first button pressed
#    puts "scroll_tree $op $n $ut"
    
    if { $op == "scroll" } {
        source_tree yview $op $n $ut
        set dif 0
    } else {
        global mouse_pos
        set dif [expr $n - $mouse_pos ]
        set mouse_pos $n
    }
        set xy [source_tree yview]
        set x [expr [lindex $xy 0] + $dif]
        set y [expr [lindex $xy 1] + $dif]

#        puts "$x , $y  dif $dif $n"
        if { $x == 0 && $dif < 0 } { 
            return 
        }  elseif { $y == 1 && $dif > 0 } {
            return 
        }
        
        set x [expr $x + $dif]
        set y [expr $y + $dif]
        source_tree yview moveto $x 
        Scrollbar1 set   $x  $y


# d: direction
# per: percetage
}
#############################################################################
## Procedure:  sync_tree_scroll

proc ::sync_tree_scroll {} {
global widget

puts "sync_tree:"
}
#############################################################################
## Procedure:  con_dir

proc ::con_dir {args} {
global widget

puts "con_dir:"
# copy from tkcon.tcl
## dir - directory list
# ARGS:	args	- names/glob patterns of directories to list
# OPTS:	-all	- list hidden files as well (Unix dot files)
#	-long	- list in full format "permissions size date filename"
#	-full	- displays / after directories and link paths for links
# Returns:	a directory listing
## 



}
#############################################################################
## Procedure:  source_text_popup

proc ::source_text_popup {x y} {
    # copy and modified from PopupMenu in tkcon.tcl
    global widget
    set m .popup_menu
    $m delete 0 end


#    set w source_text how to get a window pathname by its alias???
    set w .top43.tex47
    set relX [expr {$x-[winfo rootx $w]} ]
    set relY [expr {$y-[winfo rooty $w]} ]
    if { [llength [set tags [$w tag names @$relX,$relY] ]] } {
        if { [lsearch -exact $tags "file"] >= 0} {
            foreach {first last} [$w tag prevrange file @$relX,$relY] {
                set word [$w get $first $last]; break;
            }
            # set menu item depend on each file extension
            # assume file is in current directory
            
            # has to use list to evaluate $word otherwise unknown variable
            set f [file join [pwd] $word]
            $m add command -label "View  " -command [list open_file $f]
            $m add command -label "Run   " -command [list exec_file $f]
            $m add command -label "Delete" -command [list delete_file $f]
            $m add separator

            $m add command -label "$word" -state disabled

        }
    } else {
        # no tags found popup defult one
        $m add command -label "open ..." -command [list open_file ""]
        $m add command -label "run ... " -command [list exec_file ""]
    }

    
    tk_popup $m $x $y

}

#############################################################################
## Initialization Procedure:  init

proc ::init {argc argv} {

}

init $argc $argv

#################################
# VTCL GENERATED GUI PROCEDURES
#

proc vTclWindow. {base} {
    if {$base == ""} {
        set base .
    }
    ###################
    # CREATING WIDGETS
    ###################
    wm focusmodel $top passive
    wm geometry $top 1x1+0+0; update
    wm maxsize $top 1009 738
    wm minsize $top 1 1
    wm overrideredirect $top 0
    wm resizable $top 1 1
    wm withdraw $top
    wm title $top "vtcl.tcl"
    bindtags $top "$top Vtcl.tcl all"
    vTcl:FireEvent $top <<Create>>
    wm protocol $top WM_DELETE_WINDOW "vTcl:FireEvent $top <<DeleteWindow>>"

    ###################
    # SETTING GEOMETRY
    ###################

    vTcl:FireEvent $base <<Ready>>
}

proc vTclWindow.top43 {base} {
    if {$base == ""} {
        set base .top43
    }
    if {[winfo exists $base]} {
        wm deiconify $base; return
    }
    set top $base
    ###################
    # CREATING WIDGETS
    ###################
    vTcl:toplevel $top -class Toplevel \
        -highlightcolor black 
    wm focusmodel $top passive
    wm geometry $top 609x422+312+280; update
    wm maxsize $top 1137 870
    wm minsize $top 1 1
    wm overrideredirect $top 0
    wm resizable $top 1 1
    wm deiconify $top
    wm title $top "New Toplevel 1"
    vTcl:DefineAlias "$top" "Toplevel1" vTcl:Toplevel:WidgetProc "" 1
    bindtags $top "$top Toplevel all _TopLevel"
    vTcl:FireEvent $top <<Create>>
    wm protocol $top WM_DELETE_WINDOW "vTcl:FireEvent $top <<DeleteWindow>>"

    Tree $top.tre44 \
        -highlightcolor black -selectbackground #c1c1c1 \
        -selectforeground black 
    vTcl:DefineAlias "$top.tre44" "source_tree" vTcl:WidgetProc "Toplevel1" 1
    bind $top.tre44 <Configure> {
        Tree::_update_scrollregion %W
    }
    bind $top.tre44 <Destroy> {
        Tree::_destroy %W
    }
    bind $top.tre44 <FocusIn> {
        after idle {BWidget::refocus %W %W.c}
    }
    text $top.tex46 \
        -background white -insertbackground black 
    vTcl:DefineAlias "$top.tex46" "command_text" vTcl:WidgetProc "Toplevel1" 1
    scrollbar $top.scr48 -takefocus 0 -bd 1\
        -command [list $top.tre44 yview ]
    vTcl:DefineAlias "$top.scr48" "Scrollbar1" vTcl:WidgetProc "Toplevel1" 1
    text $top.tex47 -yscrollcommand [list $top.scr49 set]\
        -background white -insertbackground black 
    vTcl:DefineAlias "$top.tex47" "source_text" vTcl:WidgetProc "Toplevel1" 1
    scrollbar $top.scr49 -command [list $top.tex47 yview]  -takefocus 0 -bd 1
    vTcl:DefineAlias "$top.scr49" "Scrollbar2" vTcl:WidgetProc "Toplevel1" 1
    ###################
    # SETTING GEOMETRY
    ###################
    place $top.tre44 \
        -x 20 -y 15 -width 216 -height 316 -anchor nw -bordermode ignore 
    place $top.tex46 \
        -x 15 -y 345 -width 568 -height 60 -anchor nw -bordermode ignore 
    place $top.scr48 \
        -x 235 -y 15 -width 16 -height 317 -anchor nw -bordermode ignore 
    place $top.tex47 \
        -x 255 -y 15 -width 338 -height 320 -anchor nw -bordermode ignore 
    place $top.scr49 \
        -x 589 -y 16 -width 16 -height 317 -anchor nw -bordermode ignore 

    vTcl:FireEvent $base <<Ready>>
}

#############################################################################
## Binding tag:  _TopLevel

bind "_TopLevel" <<Create>> {
    if {![info exists _topcount]} {set _topcount 0}; incr _topcount
}
bind "_TopLevel" <<DeleteWindow>> {
    if {[set ::%W::_modal]} {
                vTcl:Toplevel:WidgetProc %W endmodal
            } else {
                destroy %W; if {$_topcount == 0} {exit}
            }
}
bind "_TopLevel" <Destroy> {
    if {[winfo toplevel %W] == "%W"} {incr _topcount -1}
}

Window show .
Window show .top43

main $argc $argv
