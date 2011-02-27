#!/usr/bin/perl

use strict;
use warnings;
use Email::Valid;
use DBI;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');

sub email_to_address($$$)
{
    my ($email, $subject, $content) = @_;

    if (Email::Valid->address($email)) {
        open MAIL_FH, "|mail -a 'From: support\@pardiff.com' -s '${subject}' ${email}";
        print MAIL_FH $content;
        close MAIL_FH;
    }
}

my $expire_sth = $dbh->prepare('SELECT * FROM login WHERE expire_at IS NOT NULL AND expire_at < NOW()');

$expire_sth->execute();

my @tobe_suspended;
while (my $ref = $expire_sth->fetchrow_hashref) {
    push @tobe_suspended, $ref->{id} . ' ' . $ref->{username} . ' ' . $ref->{email};

    email_to_address($ref->{email}, '[ParDiff VPN] 您的帐户即将失效/Account expire soon', '亲爱的用户' . $ref->{username} . '
您的帐户截至今日已经过期，将会在7天之后被冻结。

目前，您还有' . int($ref->{credit}/1024/1024) . 'MB未使用流量，您可以在7天内使用完毕。如果7天之后还没有用完，那么将会被冻结。

详细帐户信息请登陆 http://vpn.pardiff.com/cp

ParDiff VPN support
support@pardiff.com

Dear Customer ' . $ref->{username} . '

Based on our record, your VPN account is already expired, and will be closed after 7 days.

Until now, there is still ' . int($ref->{credit} / 1024 / 1024) . 'MB traffic credit remaining in your account, we decided to give you 7 days extension. After that, your account will be suspended and credit will freezed based on our agreement.

On the other hand, if you charge your account within 7 days, all the current remaining credit will be kept, and expire date will be delayed as well.

For detailed account information, please login from http://vpn.pardiff.com/cp

Regards,

ParDiff VPN Support
support@pardiff.com

');
}

my $content = "The following accounts are expired, and will actually be suspended in 7 days. \n";

foreach (@tobe_suspended) {
    $content .= $_ . "\n\r";
}

email_to_address('admin@pardiff.com', '[suspend] Accounts expired notification', $content);

my @suspended;
my $expire_do_sth = $dbh->prepare("UPDATE login SET account_state = 'DISABLED', expire_at = NULL WHERE id = ?");
$expire_sth = $dbh->prepare('SELECT * FROM login WHERE expire_at IS NOT NULL AND expire_at < date_sub(NOW(), interval 7 day)'); 
$expire_sth->execute();
while (my $ref = $expire_sth->fetchrow_hashref()) {
    $expire_do_sth->execute($ref->{id});
    push @suspended, $ref->{id} . ' ' . $ref->{username} . ' ' . $ref->{email};
}

$content = "The following accounts are expired for 7 days, and suspended\n";

foreach (@suspended) {
    $content .= $_ . "\n\r";
}


email_to_address('admin@pardiff.com', '[suspend] Accounts suspended notification', $content);

