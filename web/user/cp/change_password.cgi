#!/usr/bin/perl

use strict;
use warnings;

use CGI qw/:standard/;
use DBI;
use Data::Dumper;

my %config;

use Config::Simple;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

use Captcha::reCAPTCHA;
# Your reCAPTCHA keys from
#   https://admin.recaptcha.net/recaptcha/createsite/
use constant PUBLIC_KEY       => '6LdHVwsAAAAAAJf562_qCaDHPtqEv2jOF--ml5Ao';
use constant PRIVATE_KEY      => '6LdHVwsAAAAAAKBvGCIJuX6vwvVA5fquW3kkt1Dv';


my $captcha = Captcha::reCAPTCHA->new;
my $result = $captcha->check_answer(PRIVATE_KEY, $ENV{'REMOTE_ADDR'}, 
                                    param( 'recaptcha_challenge_field' ),
                                    param( 'recaptcha_response_field' ));

#if (!$result->{is_valid}) {
#    print header(-charset => 'utf-8');
#    print start_html(-title => '验证码不正确!');
#    print '<h1>验证码不正确，你被判定为机器人!</h1>';
#    print end_html;
#    exit 0;
#}

my $password = param('password');
my $password_1 = param('password_1');
my $password_2 = param('password_2');
my $username = param('username');

if ($password && $username) {
    my $sth = $dbh->prepare("SELECT * FROM login WHERE username = ? AND password =?");
    $sth->execute($username, $password);

    my $ref = $sth->fetchrow_hashref();
    if ($ref) {
        if ($password_1 && $password_2 && $password_1 eq $password_2) {
            $sth = $dbh->prepare("UPDATE login SET password = ? WHERE username = ? LIMIT 1");
            $sth->execute($password_2, $username);
        }
        else {
            print header(-charset=>'utf-8'), start_html('两次密码输入不符');
            print '<h1>两次密码输入不符</h1>', end_html;
            exit 0;
        }
    }
}

print redirect('index.html');

