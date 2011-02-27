#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;
use DBI;
use Data::Dumper;
use Email::Valid;

use Config::Simple;

my %config;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

sub email_to_address($$$)
{
    my ($email, $subject, $content) = @_;

    if (Email::Valid->address($email)) {
        open MAIL_FH, "|mail -a 'From: support\@pardiff.com' -s '${subject}' ${email}";
        print MAIL_FH $content;
        close MAIL_FH;
    }
}    

sub num_to_ip($)
{
    my $num = shift;

    return int($num / (256 * 256 * 256)) . '.' . int($num / (256 * 256) % 256) . '.' . int($num / 256 %256 ) . '.' . ($num % 256);
}

sub ip_to_num($)
{
    my $ip = shift;
    if ($ip =~ /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/) {
        return ((($1 * 256) + $2) * 256 + $3) * 256 + $4;
    }
    return 0;
}

use Captcha::reCAPTCHA;
# Your reCAPTCHA keys from
#   https://admin.recaptcha.net/recaptcha/createsite/
use constant PUBLIC_KEY       => 'key';
use constant PRIVATE_KEY      => 'key';


my $captcha = Captcha::reCAPTCHA->new;
my $result = $captcha->check_answer(PRIVATE_KEY, $ENV{'REMOTE_ADDR'}, 
                                    param( 'recaptcha_challenge_field' ),
                                    param( 'recaptcha_response_field' ));

if (!$result->{is_valid}) {
    print header(-charset => 'utf-8');
    print start_html(-title => '验证码不正确!');
    print '<h1>验证码不正确，你被判定为机器人!</h1>';
    print end_html;
    exit 0;
}

sub get_client_ip()
{
    my $sth = $dbh->prepare("SELECT clientip FROM login ORDER BY id DESC LIMIT 1");
    $sth->execute();
    while (my $ref = $sth->fetchrow_hashref()) {
        return num_to_ip(ip_to_num($ref->{'clientip'}) + 1);
    }
    return '0.0.0.0';
}

my $password = param('password');
my $username = param('username');
my $password_again = param('password_again');
my $email = param('email');

sub _fail($)
{
    my $msg = shift;
    print header(-charset => 'utf-8');
    print start_html(-title => '建立帐号失败!');
    print $msg;
    print end_html;
}

if (!$password || !$username || !$password_again || !$email) {
    _fail('请填写完整。');
    exit(0);
}

if (!Email::Valid->address($email)) {
    _fail('电子邮件不正确。');
    exit(0);
}

if ($password ne $password_again) {
    _fail('两次密码输入不匹配');
    exit(0);
}

my $new_client_ip = get_client_ip();

my $sth = $dbh->prepare("INSERT INTO login (id, username, password, clientip, serverip, credit, email, expire_at) VALUES (0, ?, ?, ?, '10.1.1.1', ? * 1024 * 1024, ?, date_add(now(), interval 3 day))");
$sth->execute($username, $password, $new_client_ip, 0, $email);

if ($dbh->{mysql_insertid} > 0) {
    print header(-charset => 'utf-8'), start_html(-title=>'增加用户成功'), '<h1>成功增加用户' . $username . '</h1><p>稍后使用教程将会发送到您的注册邮箱，您现在即可购买点卡。</p><p>购买信息可以在<a href="http://vpn.pardiff.com">首页</a>找到</p>', end_html;

    $sth = $dbh->prepare('INSERT INTO logs (ts, event) VALUES (NOW(), ?)');
    $sth->execute('[user] Successfully created user ' . $username);

    email_to_address($email, "$username ParDiff VPN帐户已建立", "
尊敬的$username,

您的 ParDiff VPN 帐号($username)已建立。以下是各项信息

= Twitter =

我们为您提供了第三方高速Twitter网页客户端，登陆地址
http://twitter.pardiff.com , 用户名密码同VPN。


= VPN设置 =

图文并茂的ParDiff VPN设置可以在这里下载
 http://vpn.pardiff.com/tutorial

我们推荐您在可能的情况下使用 OpenVPN 连接服务器，更快更安全。

= 疑问解答 =

您可以在使用我们的客服系统与我们联系
  http://vpn.pardiff.com/ticket/
您也可以联系我们的销售人员

= 购买点卡 =

目前在淘宝我们有代理商，您可以在我们的首页找到他们的链接。

= 充值 =

您购买充值卡后，可以在如下地址充值
  http://vpn.pardiff.com/coupon/

= 查询帐户信息 =

您可以在如下地址修改密码、查询登陆记录、查询帐户余额
  http://vpn.pardiff.com/cp/

= 最后 =

欢迎您来到 ParDiff VPN
我们一直在努力为用户提供更好更快速的互联网体验。

如果您有Twitter账户，欢迎您Follow us, 我们是 \@pardiff.com

最后，再次感谢您注册 ParDiff VPN.
");

}
else {
    _fail('重复的用户名');
    exit(0);
}
