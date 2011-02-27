#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;
use DBI;
use Data::Dumper;

use Captcha::reCAPTCHA;
# Your reCAPTCHA keys from
#   https://admin.recaptcha.net/recaptcha/createsite/
use constant PUBLIC_KEY       => 'key';
use constant PRIVATE_KEY      => 'key';

my %config;

use Config::Simple;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

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
my $username = param('username');
my $email = '';

my $state = '';
if ($password && $username) {
    my $sth = $dbh->prepare("SELECT * FROM login WHERE username = ? AND password =?");
    $sth->execute($username, $password);

    my $ref = $sth->fetchrow_hashref();
    if ($ref) {
        $email = $ref->{email};
        my $credit = $ref->{'credit'};
        
        $state = $ref->{account_state};
        
        print header(-charset => 'utf-8');
        print '<html><head><title>余额查询</title></head><body><p>您的余额为</p><pre>';
        print int($ref->{'credit'} / 1024 / 1024);
        print 'MB</pre><p>您的余额过期时间为: ';
	if ($ref->{expire_at}) {
		print $ref->{expire_at};
	}
	else {
		print '不过期';
	}
	print '</p><br><p>请注意:如果您在连接VPN状态下查询剩余流量，结果可能会没有包括您此次会话所消耗流量。</p>';

        print '<h1>登录信息</h1><br>';
        print '<p>您的账户状态是: . ' . $state . '</p>';
        $sth = $dbh->prepare("SELECT * FROM connections WHERE username = ? ORDER BY id DESC LIMIT 100");
        $sth->execute($username);
        
        print '<table border=1>';
        print "<tr><td>id</td><td>ip</td><td>from</td><td>to</td><td>Traffic</td></tr>\n";

        while (my $ref = $sth->fetchrow_hashref()) {
            print '<tr><td>', $ref->{id}, '</td><td>', $ref->{ip}, '</td><td>', $ref->{start_time}, '</td><td>', $ref->{end_time}, '</td><td>', $ref->{connection_traffic}, '</td><tr>';
        }
        
        print '</table><h2>充值历史</h2><table border=1><tr><td>时间</td><td>流量</td><td>类型(点卡/人工)</td><td>备注</td>';

        {
            my $sth = $dbh->prepare('SELECT credit, operator, username, ts, note FROM transactions_records WHERE username = ? ORDER BY ts DESC');
            $sth->execute($username);
            while (my $ref = $sth->fetchrow_hashref()) {
                print '<tr><td>', $ref->{ts}, '</td><td>', int($ref->{credit} / 1024 / 1024), 'MB</td><td>', $ref->{operator} eq 'coupon' ? '点卡' : '人工', '</td><td>',  $ref->{note} ? $ref->{note} : '', '</td></tr>'; 
            }
        }

        print '</table>';

        print '<h1>帐单有疑问?配置有困难?　请<a href="/ticket/?username=' . $username . '&email=' . $email . '">点此</a>与客服人员联系。</h1>';

        print '<hr><br>修改密码<br>';
        print '<form action="change_password.cgi"><!--请输入验证码' . $captcha->get_html(PUBLIC_KEY) . '--><input type="hidden" value="' . $username . '" name="username"/>旧密码/Old password: <input type="text" name="password"/><br>新密码/New password:<input type="text" name="password_1" /><br>Retype new password: <input type="text" name="password_2"/><input type="submit" value="修改"/></form>';
        print '<hr><h1><a href="/coupon/">充值点此</a></h1>';
        print '</body></html>';

        exit 0;
    }
}

print header(-charset => 'utf-9');
print '<html><head><title> 密码或用户名错误。</title></head><body>密码或用户名错误</body></html>';
