#!/usr/bin/perl

use strict;
use warnings;

use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

my $sth = $dbh->prepare("SELECT sum(connection_traffic) AS traffic FROM connections");
$sth->execute;

my $ref = $sth->fetchrow_hashref();
my $traffic = $ref->{traffic};

$sth = $dbh->prepare("INSERT INTO traffic_logs (ts, traffic) VALUES (NOW(), ?)");
$sth->execute($traffic);
