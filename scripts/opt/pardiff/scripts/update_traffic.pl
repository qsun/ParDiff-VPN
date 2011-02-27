#!/usr/bin/perl

use strict;
use warnings;
use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

open IF_FH, "/sbin/ifconfig |";

while (<IF_FH>) {
    if ($_ =~ /^ppp(\d+)\s+/) {
        my $interface = 'ppp' . $1;
        open TRAFFIC_IF_FH, "/sbin/ifconfig ${interface} |";
        while (<TRAFFIC_IF_FH>) {
            if (/RX bytes:\s?(\d+)\s.*TX bytes:\s?(\d+)\s/) {
                my $traffic = $1 + $2;

                my $sql_statement = "UPDATE connections SET connection_traffic = ? WHERE interface = ? AND (end_time IS NULL OR end_time < start_time)";
                my $sth = $dbh->prepare($sql_statement);
                $sth->execute($traffic, $interface);
            }
        }
        close TRAFFIC_IF_FH;
    }
}

my $cmd;

my @ended_connections;
# stop all connections where traffic is nearly 3.5G
{
    my $sth = $dbh->prepare('SELECT * FROM connections WHERE end_time < start_time AND connection_traffic IS NOT NULL AND connection_traffic > 3 * 1024 * 1024 * 1024');
    $sth->execute;
    while (my $ref = $sth->fetchrow_hashref) {
        $cmd = 'kill ' . $ref->{ppid};
        `$cmd`;
        push @ended_connections, $ref->{id};
    }
}

# find all connections which use all traffic credit
{
    my $sth = $dbh->prepare('SELECT c.id AS cid, c.ppid FROM login AS u JOIN (SELECT * FROM connections WHERE end_time < start_time) AS c ON c.username = u.username WHERE c.connection_traffic > u.credit');
    $sth->execute;
    while (my $ref = $sth->fetchrow_hashref) {
        $cmd = 'kill ' . $ref->{ppid};
        `$cmd`;
        push @ended_connections, $ref->{cid};
    }
}

# update information
{
    my $sth = $dbh->prepare('UPDATE connections SET end_time = now() WHERE id = ?');
    foreach (@ended_connections) {
        $sth->execute($_);
    }
}

{
    my $sth = $dbh->prepare("INSERT INTO logs (ts, event) VALUES (NOW(), ?)");
    $sth->execute("[update] Kill pid: " . join(',', @ended_connections));
}
