#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use Data::Dumper;
use Digest::MD5 qw(md5 md5_hex);

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

if (scalar(@ARGV) != 1) {
    die 'check-status-coupon.pl coupon';
}

my ($coupon) = @ARGV;

my $part_token = substr($coupon, 0,16) . '%';
my $part_pass = '%' . substr($coupon, 16, 16);

my $sth = $dbh->prepare("SELECT * FROM coupons WHERE token LIKE ? AND pass LIKE ?");
$sth->execute($part_token, $part_pass);

my $ref = $sth->fetchrow_hashref();
if ($ref && $ref->{status} eq 'N') {
    print "OK\n";
}
else {
    print "wrong!\n";
}

