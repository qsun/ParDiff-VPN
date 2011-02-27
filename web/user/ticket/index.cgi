#!/usr/bin/perl

use strict;
use warnings;

use CGI qw/:standard/;
use HTML::Template;

my $username = param('username') || '请输入用户名';
my $email = param('email') || '请输入电子邮箱地址';


my $template = HTML::Template->new(filename => 'index.tmpl');
$template->param(USERNAME => $username);
$template->param(EMAIL => $email);

print header(-charset => 'utf-8');
print $template->output();
