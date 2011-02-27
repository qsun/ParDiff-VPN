#!/usr/bin/perl

use strict;
use warnings;

use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');


open TRAFFIC_FH,  ">/tmp/traffic-stat" or die 'Can not open /tmp/traffic-stat for writing';

my $sth = $dbh->prepare('SELECT * FROM traffic_logs WHERE ts > DATE_SUB(NOW(), interval 2 day) ORDER BY id DESC');
$sth->execute();
while (my $ref = $sth->fetchrow_hashref()) 
{
	print TRAFFIC_FH "$ref->{id} $ref->{traffic}\n";
}
close TRAFFIC_FH;

open CONN_FH, ">/tmp/conn-stat" or die 'Can not open /tmp/conn-stat for writing.';
$sth = $dbh->prepare('SELECT * FROM connection_logs WHERE ts > DATE_SUB(NOW(), interval 2 day) ORDER BY id DESC');
$sth->execute();
while (my $ref = $sth->fetchrow_hashref())
{
	print CONN_FH "$ref->{id} $ref->{connections}\n";
}
close CONN_FH;


