#!/usr/bin/perl

use strict;
use warnings;

use Net::OpenVPN::Manage;
use Data::Dumper;

use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

sub _log($)
{
    my $message = shift;

    return unless $message;

    my $sth = $dbh->prepare('INSERT INTO logs (ts, event) VALUES (NOW(), ?)');
    $sth->execute('[openvpn]' . $message);
}

my $vpn = Net::OpenVPN::Manage->new({
                                     host => 'localhost',
                                     port => '1195'});
unless ($vpn->connect()) {
    _log('Failed to connect openvpn server');
    exit 1;
}

my $client_list = $vpn->status_ref()->{'CLIENT_LIST'};

my @clients;
if ($client_list) {
    @clients = @$client_list;
}

_log('connected clients: ' . scalar(@clients));

my %connections; # current connections

foreach my $client (@clients) {
    my ($name, $client_addr, $internal_ip, $recv, $sent, $connect_since, $connect_since_number) = @$client;

    chomp $connect_since_number;
    my ($client_ip, $client_port) = split(/:/, $client_addr);

    my $hash_str = $name. ':' . $client_ip . ':' . $client_port . ':' . $connect_since_number;
    $connections{$hash_str} = 1;
    print $hash_str, "\n";

    # if exist then update
    my $sth = $dbh->prepare('SELECT count(*) AS c FROM openvpn_connections WHERE ip = ? AND port = ? AND username = ? AND since = ?');
    $sth->execute($client_ip, $client_port, $name, $connect_since_number);

    my $ref = $sth->fetchrow_hashref();
    if ($ref && $ref->{c} > 0) {
        # old connection
        $sth = $dbh->prepare('UPDATE openvpn_connections SET traffic = ? WHERE ip = ? AND port = ? AND username = ? AND since = ?');
        $sth->execute($recv + $sent, $client_ip, $client_port, $name, $connect_since_number);
    }
    else {
        _log('new connection: user ' . $name . ' ip: ' . $client_ip);
        #new connections
        $sth = $dbh->prepare('INSERT INTO openvpn_connections (username, ip, port, internalip, traffic, since) VALUES (?, ?, ?, ?, 0, ?)');
        $sth->execute($name, $client_ip, $client_port, $internal_ip, $connect_since_number);
    }
}

my @finished_connections;

# lets try to find the connections to be removed.
my $sth = $dbh->prepare('SELECT * FROM openvpn_connections');
$sth->execute();
while (my $ref = $sth->fetchrow_hashref()) {
    my $id = $ref->{id};
    my $username = $ref->{username};
    my $ip = $ref->{ip};
    my $port = $ref->{port};
    my $traffic = $ref->{traffic};
    my $since = $ref->{since};

    my $hash_str = $username . ':' . $ip . ':' . $port . ':' . $since;
    print $hash_str, "\n";

    next if ($connections{$hash_str} && $connections{$hash_str} > 0); #is the connection currently exist?
    _log('User ' . $username . ' terminated connection.');

    # now lets move the terminated connection back into our connections

    push @finished_connections, {id => $id, username => $username, traffic => $traffic, since => $since, ip => $ip};
}

# now lets find connections which have used up its exceed

$sth = $dbh->prepare('SELECT o.username AS username FROM (SELECT username, sum(traffic) AS traffic FROM openvpn_connections GROUP BY username) AS o JOIN login AS l ON o.username = l.username WHERE l.credit < o.traffic');
$sth->execute();
while (my $ref = $sth->fetchrow_hashref()) {
    my $commonname = $ref->{username};
    _log('User ' . $commonname . ' exceeds the limit.');
    $vpn->kill($commonname);
}

# now lets move the connections
foreach my $connection (@finished_connections) {
    print Dumper($connection);
    my $id = $connection->{id};
    my $username = $connection->{username};
    my $traffic = $connection->{traffic};
    my $since = $connection->{since};
    my $ip = $connection->{ip};

    # login info
    $sth = $dbh->prepare('INSERT INTO connections (ip, connection_traffic, start_time, end_time, username, interface, ppid) VALUES (?, ?, FROM_UNIXTIME(?), NOW(), ?, \'OPENVPN\', -1)');
    $sth->execute($ip, $traffic, $since, $username);

    # credit info
    $sth = $dbh->prepare('UPDATE login SET credit = credit - ? WHERE username = ?');
    $sth->execute($traffic, $username);

    # remove from openvpn connections
    $sth = $dbh->prepare('DELETE FROM openvpn_connections WHERE id = ?');
    $sth->execute($id);
}

    
