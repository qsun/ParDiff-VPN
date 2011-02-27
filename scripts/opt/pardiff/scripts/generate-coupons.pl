#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use Data::Dumper;
use Digest::MD5 qw(md5 md5_hex);

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

if (scalar(@ARGV) != 5) {
    die 'generate-coupons.pl credit(in MB) amount expire_days filename notes';
}

my ($credit, $amount, $expire_days, $filename, $notes) = @ARGV;
$credit = $credit * 1024 * 1024;

open COUPON_FH, ">${filename}" or die "Can not open ${filename} for write\n";

sub _generate_salt()
{
    my $salt = '';
    foreach (1..5) {
        $salt .= md5_hex(rand(100000));
    }

    return $salt;
}

my $sth = $dbh->prepare("INSERT INTO coupons (status, credit, generator, token, pass, salt, ts, expire, notes ) VALUES ('N', ?, 'console', ?, ?, ?, NOW(), ?, ?)");
foreach (1..$amount) {
    my $salt = _generate_salt();
    my $token = md5_hex($salt);
    my $pass = md5_hex($token . $salt);

    $sth->execute($credit, $token, $pass, $salt, $expire_days, $notes);

    print COUPON_FH substr($token, 0, 16), substr($pass, 16, 16), "\n";
}

close COUPON_FH;
