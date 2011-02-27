#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;
use DBI;
use Email::Valid;
use HTML::Entities;
use HTML::Template;
use Digest::MD5 qw(md5 md5_hex);
#use encoding 'utf8';

my %config;

use Config::Simple;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

my $category = param('category');
my $title = param('title');
my $username = param('username');
my $email = param('email');
my $content = param('content');
my $type = param('type');

my $error = 0;


sub email_to_address($$$)
{
    my ($email, $subject, $content) = @_;

    if (Email::Valid->address($email)) {
        open MAIL_FH, "|mail -a 'From: support\@pardiff.com' -s '${subject}' ${email}";
        print MAIL_FH $content;
        close MAIL_FH;
    }
}

my $template = HTML::Template->new(filename => 'create.tmpl');

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
    $error = '请输入正确地验证码';
}

if ($type ne 'old' && $type ne 'new') {
    $error = '请从正确入口进入';
}

if ($error) {
    $template->param(PAGE_TITLE => $error);
    $template->param(ERROR => $error);
    print header(-charset => 'utf-8');

    print $template->output();
    exit(0);
}


if ($type eq 'new') {
    if (!Email::Valid->address($email)) {
        $error = '请输入合法的电子邮箱地址';
    }
    if ($error) {
        $template->param(PAGE_TITLE => $error);
        $template->param(ERROR => $error);
print header(-charset => 'utf-8');


        print $template->output();
        exit(0);
    }


    $category =~ tr/\<\>/  /;
    $title =~ tr/\<\>/  /;
    $username =~ tr/\<\>/  /;
    $email =~ tr/\<\>/  /;
    $content =~ tr/\<\>/  /;

    $template->param(PAGE_TITLE => $title . '已提交');
    $template->param(CATEGORY => $category);
    $template->param(TITLE => $title);
    $template->param(USERNAME => $username);
    $template->param(EMAIL => $email);
    $template->param(CONTENT => $content);
print header(-charset => 'utf-8');


    print $template->output();

    my $sth = $dbh->prepare("INSERT INTO ticket_threads VALUES (0, ?, ?, ?, ?, 'open')");
    $sth->execute($title, $username, $email, $category);
    my $ticket_id = $dbh->{'mysql_insertid'};
    my $ticket_token = md5_hex($ticket_id . md5_hex($ticket_id));

    $sth = $dbh->prepare("INSERT INTO tickets VALUES (0, ?, 'client', now(), '', ?)");
    $sth->execute($ticket_id, $content);

    email_to_address('support@pardiff.com', '[ticket] New ticket: ' . $title, $content);

    my $email_content = '尊敬的ParDiff客户，

您提交的支持消息(' . $title . ')已经确认，很快ParDiff的T1支持人员将会与您联系。

所有后续消息都将通过ParDiff支持系统(vpn.pardiff.com/ticket)完成，您会在此邮箱中收到后续信息提示。

点此浏览后续信息 http://vpn.pardiff.com/ticket/view.cgi?ticket=' . $ticket_id . '&token=' . $ticket_token .'

感谢您的合作。

ParDiff VPN Support
support@pardiff.com
';

    email_to_address($email, $title . '记录确认', $email_content);
}
elsif ($type eq 'old') {
    my $ticket_id = param('ticket') || '';
    my $ticket_token = param('token') || '';
    my $content = param('content') || '';
    $content =~ tr/\<\>/  /;

    if ($ticket_token ne md5_hex($ticket_id . md5_hex($ticket_id))) {
            $error = '请从正确入口进入';
    }

    my $sth = $dbh->prepare('SELECT * FROM ticket_threads WHERE id = ?');
    $sth->execute($ticket_id);
    my $ref = $sth->fetchrow_hashref();
    if (!$ref) {
        $error = '请从正确入口进入';
    }

    if ($error) {
        $template->param(PAGE_TITLE => $error);
        $template->param(ERROR => $error);
print header(-charset => 'utf-8');


        print $template->output();
        exit(0);
    }

    $sth = $dbh->prepare("UPDATE ticket_threads SET status = 'open' WHERE id = ?");
    $sth->execute($ticket_id);

    my $email = $ref->{email};
    my $title = $ref->{title};

    # $template->param(PAGE_TITLE => $title . '已添加一条补充');
    #  $template->param(CATEGORY => $ref->{category});
    #  $template->param(TITLE => $title);
    #  $template->param(EMAIL => $email);
    #  $template->param(CONTENT => $content);
    #  print $template->output();

    print redirect('view.cgi?ticket=' . $ticket_id . '&token=' . $ticket_token);

    $sth = $dbh->prepare("INSERT INTO tickets VALUES (0, ?, 'client', now(), '', ?)");
    $sth->execute($ticket_id, $content);
    email_to_address('support@pardiff.com', '[ticket] New ticket: ' . $title, $content);
    my $email_content = '尊敬的ParDiff客户，

您提交的支持消息(' . $title . ')已经确认，很快ParDiff的T1支持人员将会与您联系。

所有后续消息都将通过ParDiff支持系统(vpn.pardiff.com/ticket)完成，您会在此邮箱中收到后续信息提示。

点此浏览后续信息 http://vpn.pardiff.com/ticket/view.cgi?ticket=' . $ticket_id . '&token=' . $ticket_token .'

感谢您的合作。

ParDiff VPN Support
support@pardiff.com
';

    email_to_address($email, $title . '记录确认', $email_content);
    
}
