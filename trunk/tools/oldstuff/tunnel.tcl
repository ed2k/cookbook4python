proc Connect {sockid host} {
    fileevent $sockid writable {}
    set msg "CONNECT $host HTTP/1.1\r\nHOST: $host\r\n\r\n"

    puts -nonewline $sockid $msg
    puts -nonewline "tunnel: $msg"    
}

proc TunnelRead {sock1 sock2} {
    fileevent $sock1 readable {}

    set data [read $sock1]
    set len [string bytelength $data]
#    puts "len = $len"
    if {$len == 0} {
        close $sock1;
        close $sock2;
        return;
    }
    
    puts "$sock1 > $sock2 : $len bytes"
    puts -nonewline $sock2 $data


    fileevent $sock1 readable "TunnelRead $sock1 $sock2"
}

proc ProxyRead {sock1 sock2} {
    fileevent $sock2 readable {}

    set data [read $sock2]
    set len [string bytelength $data]
#    puts "len = $len"
    if {$len == 0} {
        close $sock1;
        close $sock2;
        return;
    }
    
    # expect HTTP/1.0 200 Connection established
    puts -nonewline "proxy: $data"
#    puts "$sock1 > $sock2 : $len bytes"
#    puts -nonewline $sock2 $data

    fileevent $sock1 readable "TunnelRead $sock1 $sock2"
    fileevent $sock2 readable "TunnelRead $sock2 $sock1"
}


# accept connection
proc AcceptConnection {sockid hostaddr hostport} {
    puts "Got one from $sockid $hostaddr $hostport\n"

    set ipaddr www-proxy.lml.ericeric.com
    set port 80
    set ipaddr messenger.hotmail.com
    set port 1863

    set sock2 [socket $ipaddr $port]
    puts "connect to $sock2 $ipaddr $port\n"
    set vncip sunyin.3322.org
    set vncport 5901


    fconfigure $sockid -blocking 0 -buffering none -translation {binary binary}
    fconfigure $sock2 -blocking 0 -buffering none -translation {binary binary}

    set host "$vncip:$vncport"
    fileevent $sock2 writable "Connect $sock2 $host"
#    fileevent $sockid readable "TunnelRead $sockid $sock2"
    fileevent $sock2  readable "ProxyRead $sockid $sock2"
}


# listen at some port 
if {$argc < 3 } { 
    puts "tunnel <listenPort> <connetToIP> <connectToPort>"
    return
}

#set port 5902
set port 11863
set sockid [socket -server "AcceptConnection" $port]
  
vwait stop 
