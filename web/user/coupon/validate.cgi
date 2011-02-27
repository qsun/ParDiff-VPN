#!/usr/bin/perl

use lib '/home/zhangsuheng/perlmods/share/perl/5.8.8/';

use strict;
use warnings;

use DBI;
use Data::Dumper;
use Digest::MD5 qw(md5 md5_hex);

use Captcha::reCAPTCHA;
# Your reCAPTCHA keys from
#   https://admin.recaptcha.net/recaptcha/createsite/
use constant PUBLIC_KEY       => '6LdHVwsAAAAAAJf562_qCaDHPtqEv2jOF--ml5Ao';
use constant PRIVATE_KEY      => '6LdHVwsAAAAAAKBvGCIJuX6vwvVA5fquW3kkt1Dv';

use CGI qw/:standard/;
use Config::Simple;
my %config;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

my $captcha = Captcha::reCAPTCHA->new;

my $card = param('card');

my $result = $captcha->check_answer(PRIVATE_KEY, $ENV{'REMOTE_ADDR'}, 
                                    param( 'recaptcha_challenge_field' ),
                                    param( 'recaptcha_response_field' ));

sub _fail()
{
    print header(-charset => 'utf-8');
    print start_html(-title => '无效充值卡!');
    print '<h1>验证失败!</h1><p>该卡号无效，请即刻与销售者联系!</p>';
    print end_html;
}

sub _success($$)
{
    my $credit = shift;
    my $expire = shift;
    print header(-charset => 'utf-8');
    print start_html(-title => '验证成功');
    print '<h1>该充值卡当前有效! 点数: ' . int($credit / 1024 / 1024) . 'MB</h1><p>我们建议您立刻充值。</p>';
    print '<p>该卡充值后，有效期为' . $expire . '天</p>';
    print '<form method="POST" action="coupon_credit.cgi"><input type="hidden" name="card" value="' . $card . '" />请输入将冲入的用户名<input type="text" name="username" /><br>验证码'. $captcha->get_html(PUBLIC_KEY) . '<input type="submit" value="充值" /></form>';
    print end_html;
}

#if (!$result->{is_valid}) {
#    print header(-charset => 'utf-8');
#    print start_html(-title => '验证码不正确!');
#    print '<h1>验证码不正确，你被判定为机器人!</h1>';
#    print end_html;
#    exit 0;
#}

if (!$card) {
    _fail();
    exit 0;
}

$card = '' . $card;

if (length($card) != 32) {
    _fail();
    exit 0;
}

my $part_token = substr($card, 0,16) . '%';
my $part_pass = '%' . substr($card, 16, 16);

my $sth = $dbh->prepare("SELECT * FROM coupons WHERE token LIKE ? AND pass LIKE ?");
$sth->execute($part_token, $part_pass);

my $ref = $sth->fetchrow_hashref();
if ($ref && $ref->{status} eq 'N') {
    _success($ref->{credit}, $ref->{expire});
}
else {
    _fail();
    exit 0;
}

