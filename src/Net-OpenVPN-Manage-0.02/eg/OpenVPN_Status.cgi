use strict;
use CGI;
use Net::OpenVPN::Manage;

my $cgi=CGI->new();
print $cgi->header();

my $vpn=Net::OpenVPN::Manage->new({host=>'10.250.8.2', port=>'6000', password=>'password', timeout=>'5'});
unless ($vpn->connect()){
	print $vpn->{error_msg}."\n\n";
	exit 0;
}

my $r=$vpn->status_ref();
print qq|<table border="1"><tr>|;
foreach my $heading ( @{$r->{HEADER}{CLIENT_LIST}} ){
	print qq|<th>$heading</th>|;
}
print qq|</tr>|;
foreach my $aref ( @{$r->{CLIENT_LIST}} ){
	print qq|<tr>|;
	foreach my $r ( @{$aref} ){
		print qq|<td>$r</td>|;
	}
	print qq|</tr>|;
}
print qq|</table>|;

