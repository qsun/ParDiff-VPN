#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;
use DBI;
use Email::Valid;
use HTML::Entities;
use HTML::Template;
use Digest::MD5 qw(md5 md5_hex);

my $template = HTML::Template->new(filename => 'view.tmpl');

my %config;

use Config::Simple;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

my $sth;

my $error = '';
my $title = '';
my $content = '';
my $client_username;

my $ticket_id = param('ticket') || '';
my $ticket_token = param('token') || '';
if ($ticket_token ne md5_hex($ticket_id . md5_hex($ticket_id))) {
    $title = 'id与token不匹配。';
    $error = '<h1>请点击确认函中的浏览地址。</h1>';
}

$sth = $dbh->prepare("SELECT * FROM ticket_threads WHERE id = ? LIMIT 1");
$sth->execute($ticket_id);
my $ref = $sth->fetchrow_hashref();
if (!$ref || !$ref->{id}) {
    $title = 'ticket未找到。';
    $error = '<h1>您所要求的支持编号未找到。请点击确认函中的浏览地址。</h1>';
    exit 0;
}
else {
    $title = $ref->{title};
    $content .= '<h1>' . $ref->{title} . '</h1>';
    $content .= '<p>用户名: ' . $ref->{username} . '</p>';
    $client_username = $ref->{username};
    $content .= '<p>状态:' . ($ref->{status} eq 'closed' ? '关闭' : '开启') . "</p>"
}


$sth = $dbh->prepare("SELECT * FROM tickets WHERE thread_id = ? ORDER BY id DESC");
$sth->execute($ticket_id);

$content .= '<table border="1" width="90%">';

while ($ref = $sth->fetchrow_hashref()) {
    my $role = $ref->{role};
    my $ts = $ref->{ts};
    my $username = $ref->{username};
    my $ticket_content = $ref->{content};

    $content .= '<tr><td><p>日期: ' . $ts . '</p>' . ($role ne 'client'? "<p>回复人: ${username}" : '') . '<pre>' . $ticket_content . '</pre></td></tr>';
}

$content .= '</table>';

print header(-charset => 'utf-8');
$template->param(PAGE_TITLE => $title);
$template->param(ERROR => $error);
$template->param(CONTENT => $content);
$template->param(TOKEN => $ticket_token);
$template->param(TICKET => $ticket_id);
print $template->output();
