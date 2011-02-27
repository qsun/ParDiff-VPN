#!/usr/bin/perl

use strict;
use warnings;

use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

open CFH, "</proc/net/ip_conntrack" or die "can not open /proc/net/ip_conntrack";

my $count = 0;
while (<CFH>) {
	$count++;
}

my $sth = $dbh->prepare("INSERT INTO connection_logs (ts, connections) VALUES (NOW(), ?)");
$sth->execute($count);
