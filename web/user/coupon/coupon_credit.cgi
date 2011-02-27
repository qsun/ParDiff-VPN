#!/usr/bin/perl

use lib '/home/zhangsuheng/perlmods/share/perl/5.8.8/';

use strict;
use warnings;

use DBI;
use Data::Dumper;
use Digest::MD5 qw(md5 md5_hex);

use CGI qw/:standard/;

use Captcha::reCAPTCHA;
# Your reCAPTCHA keys from
#   https://admin.recaptcha.net/recaptcha/createsite/
use constant PUBLIC_KEY       => '6LdHVwsAAAAAAJf562_qCaDHPtqEv2jOF--ml5Ao';
use constant PRIVATE_KEY      => '6LdHVwsAAAAAAKBvGCIJuX6vwvVA5fquW3kkt1Dv';


my $captcha = Captcha::reCAPTCHA->new;
my $result = $captcha->check_answer(PRIVATE_KEY, $ENV{'REMOTE_ADDR'}, 
                                    param( 'recaptcha_challenge_field' ),
                                    param( 'recaptcha_response_field' ));

use Config::Simple;
my %config;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

#if (!$result->{is_valid}) {
#    print header(-charset => 'utf-8');
#    print start_html(-title => '验证码不正确!');
#    print '<h1>验证码不正确，你被判定为机器人!</h1>';
#    print end_html;
#    exit 0;
#}

my $card = param('card');
my $username = param('username');

sub _fail($)
{
    my $msg = shift;
    print header(-charset => 'utf-8');
    print start_html(-title => '充值失败!');
    print $msg;
    print end_html;
}

my $fail_msg = '<h1>验证失败!</h1><p>该卡号无效，请即刻与销售者联系!</p>';

if (!$card) {
    _fail($fail_msg);
    exit 0;
}

$card = '' . $card;

if (length($card) != 32) {
    _fail($fail_msg);
    exit 0;
}

my $sth = $dbh->prepare('SELECT count(*) AS c FROM login WHERE username = ?');
$sth->execute($username);
my $ref = $sth->fetchrow_hashref();
my $c = $ref->{c};

if ($c <= 0) {
    _fail('<h1>充值失败! 没有找到用户' . $username . '</h1>');
    exit 0;
}

my $part_token = substr($card, 0,16) . '%';
my $part_pass = '%' . substr($card, 16, 16);

$sth = $dbh->prepare("select count(*) as c FROM coupons WHERE status = 'N' AND token LIKE ? AND pass LIKE ? LIMIT 1");
$sth->execute($part_token, $part_pass);
my $ref_c = $sth->fetchrow_hashref();
if ($ref_c->{c} <= 0) {
    _fail('充值失败，请核实卡号密码。');
    exit(0);
}

$sth = $dbh->prepare("UPDATE coupons SET status = 'U' WHERE status = 'N' AND token LIKE ? AND pass LIKE ? LIMIT 1");
my $coupon_found = $sth->execute($part_token, $part_pass);

if (!$coupon_found) {
    _fail($fail_msg);
    exit 0;
}

$sth = $dbh->prepare("SELECT credit, expire FROM coupons WHERE token LIKE ? AND pass LIKE ?");
$sth->execute($part_token, $part_pass);
$ref = $sth->fetchrow_hashref();
if ($ref->{credit}) {
    $sth = $dbh->prepare("UPDATE login SET account_state = 'ACTIVE', credit = credit + ?, expire_at = IF(expire_at, date_add(expire_at, interval ? day), date_add(now(), interval ?  day)) WHERE username = ?");
    $sth->execute($ref->{credit}, $ref->{expire}, $ref->{expire}, $username);
    
    my $sql_statement = "INSERT INTO transactions_records (credit, operator, username, ts, note) VALUES (?, ?, ?, NOW(), ?)";
    warn($sql_statement);
    $sth = $dbh->prepare($sql_statement);
    $sth->execute($ref->{credit}, 'coupon', $username, $card . '++' . $ref->{expire});

    print header(-charset => 'utf-8');
    print start_html(-title => '充值成功');
    print '<h1>成功充值，请登陆。</h1>';
    print end_html;

}
else {
    _fail($fail_msg);
    exit 0;
}
