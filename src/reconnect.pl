#!/usr/bin/perl -w
#
# This tests more than 1024 reconnects to the server
#
use Socket;
for ($i=1;$i<61100;$i++) {
    print $i;
    socket (SH, PF_INET, SOCK_STREAM, getprotobyname('tcp')) || die $!;
    $dest= sockaddr_in($i, inet_aton('172.16.17.1'));

    connect(SH, $dest);
    print $!,"\n";
}
