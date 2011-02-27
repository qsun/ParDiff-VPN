#!/usr/bin/perl

use strict;
use warnings;
use DBI;
use Email::Valid;

my $dsn = "DBI:mysql:database=ppp;host=127.0.0.1;port=3306";
my $dbh = DBI->connect($dsn, 'vpn', 'vpn');


sub email_to_address($$$)
{
    my ($email, $subject, $content) = @_;
    # print $content, "\n";
    # return;
    if (Email::Valid->address($email)) {
        open MAIL_FH, "|mail -a 'From: support\@pardiff.com' -s '${subject}' ${email}";
        print MAIL_FH $content;
        close MAIL_FH;
    }
}

my ($Second, $Minute, $Hour, $Day, $Month, $Year, $WeekDay, $DayOfYear, $IsDST) = localtime(time);

my $subject = '今日流量报告 - ' . $Month .'月' . $Day . '日';

my $sth = $dbh->prepare("SELECT DISTINCT(connections.username) AS username, login.email AS email FROM login JOIN connections ON connections.username = login.username WHERE report = 'DAILY' AND connections.start_time > DATE_SUB(NOW(), interval 1 day) ");
$sth->execute();

while (my $ref = $sth->fetchrow_hashref()) {
    my $user_sth = $dbh->prepare("SELECT * FROM connections WHERE username = ? AND start_time > DATE_SUB(NOW(), interval 1 day)");
    $user_sth->execute($ref->{username});

    my $content = $ref->{username} . ' 今日连接状况:' ."\n";

    my $traffic = 0;

    while (my $cref = $user_sth->fetchrow_hashref()) {
        $content .= "From " . $cref->{start_time} . " to " . $cref->{end_time} . " traffic: " . ($cref->{connection_traffic} > 1024 * 1024 ?  int($cref->{connection_traffic} / 1024 / 1024) . ' MB' : $cref->{connection_traffic} . ' B') . " IP: " . $cref->{ip} . "\n";
        $traffic += $cref->{connection_traffic};
    }

    $content .= '共计' . ($traffic > 1024 * 1024 ? int($traffic/1024/1024)  . ' MB' : $traffic . ' B') . "\n";

    email_to_address($ref->{email}, $subject, $content);
}
