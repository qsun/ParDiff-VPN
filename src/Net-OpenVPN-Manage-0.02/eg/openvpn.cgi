#!/usr/local/bin/perl
use strict;
use CGI;
use Net::OpenVPN::Manage;

#network traffic graph image created by mrtg or another network monitoring tool.
#If you set this to a null or false value the image section of the webpage
#won't be displayed.
our $vpn_traffic_image = '/images/vpn-day.png';
our $vpn_traffic_link  = 'http://10.250.8.2:8888/mrtg/vpn.html';

#create CGI object
our $cgi = CGI->new();
print $cgi->header();

#create management object and connect.
our $vpn = Net::OpenVPN::Manage->new({
        host => '10.250.8.2',
        port => '1195',
        password => 'password'
});
$vpn->connect();

# Retrieve status string.
my $status = $vpn->status();
unless($status){
  print $vpn->{error_msg}."\n";
  exit 0;
}

# Parse and htmlize the returned status string.
html_main(parse($status));

# Use regexes to parse the string into a hash.
sub parse($){
  my($s) = @_;
  my %o;

  $s =~ /TITLE,(.*)?\n/;
  $o{title} = $1;

  $s =~ /TIME,(.*)?\n/;
  $o{time} = $1;

  $s =~ /GLOBAL_STATS,(.*)?\n/;
  $o{global_stats} = $1;

  foreach my $l ( $s =~ /HEADER,(.*)?\n/g ){
   my @h = split ',', $l;
   $o{header}{shift(@h)} = \@h;
  }

  foreach my $k (keys %{$o{header}}){
    foreach my $l ( $s =~ /^$k,(.*)?\n/gm ){
      my @h = split ',', $l;
      my $r = $h[0];
      foreach my $i ( @{$o{header}{$k}} ){
        $o{$k}{$r}{$i} = shift(@h);
      }
    }
  }
  return %o;
}


# Output the tabular status data in html.
sub html_main(%){
my (%s) = @_;
print qq|
<html>
<body>
<head>
<title>OpenVPN Status</title>
<style>
        a { font: normal 10pt arial;
            text-decoration: none;
            color: black;
          }
        a:hover {
            text-decoration: underline;
          }
        table { align:  center;
            border: 1pt solid black;
            color:  white;
            background-color: #EEEEEE;
          }
        table.none {
            border: none;
            background-color: #FFFFFF;
            padding: 20px;
          }
        th, td.heading    {
            text-align: left;
            background-color: #EEEEEE;
            color: black;
            border-bottom: 1pt solid black;
            font: bold 12px arial;
            padding-right: 10px;
            padding-left: 4px;
            white-space: nowrap;
          }
        td    { text-align: left;
            color: black;
                background-color: white;
            border: none;
            font: normal 10pt arial;
            padding-right: 10px;
            padding-left: 4px;
            white-space: nowrap;
          }
        td.heading {
                border: none;
                border-right: 1pt solid grey;
                   }
        h1     {
            color: black;
            background-color: white;
            font: bold italic 14pt arial;
            text-align: center;
            border: 1pt solid black;
            padding: 3px;
          }
        p     {
        color: black;
            font: bold italic 10pt arial;
            text-align: center;
          }
        img       { border: none; }
</style>
<script language="Javascript">
</script>
</head>
<body>

<table class="none" align="center" cellpadding="0" cellspacing="0">
<tr><td>
        <h1 class="title">OpenVPN Status Page</h1>
|;

# only show this part of the page if we have been given an image file.
if ( $vpn_traffic_image ){
print qq|
        <p style="padding-bottom: 10pt;">
                <a href="$vpn_traffic_link" title="Click to see interface's MRTG page."><img src="$vpn_traffic_image"</a>
                <br>VPN Trafic Graph: Daily
        </p>|;
}

print qq|
        <table cellpadding="0" cellspacing="0" style="border-bottom:none;"><tr><td class="heading" style="font-size: 90%;">Process Information</td></tr></table>
        <table cellpadding="0" cellspacing="0">
                <tr><td class="heading">Build Version:</td><td>$s{title}</td></tr>
                <tr><td class="heading">Last update:</td><td>$s{time}</td></tr>
                <tr><td class="heading">Global Stats</td><td>$s{global_stats}</td></tr>
        </table>

        <br>

        <table cellpadding="0" cellspacing="0" style="border-bottom:none;"><tr><td class="heading" style="font-size: 90%;">Client List</td></tr></table>
        <table cellpadding="0" cellspacing="0">
                <tr>|;


foreach my $a ( @{$s{header}{CLIENT_LIST}} ){
  print qq|<th>$a</th>|;
}
print '</tr>';

foreach my $r (sort keys %{$s{CLIENT_LIST}} ){
  print '<tr>';
  foreach my $f ( @{$s{header}{CLIENT_LIST}} ){
    if ($f =~ /(Common Name|Real Address)/){
      print qq|<td><a href="#" title="Click to disconnect">$s{CLIENT_LIST}{$r}{$f}</a></td>|;
    }else{
      print qq|<td>$s{CLIENT_LIST}{$r}{$f}</td>|;
    }
  }
  print '</tr>';
}
print qq|
        </table>
        <br>
        <table cellpadding="0" cellspacing="0" style="border-bottom:none;"><tr><td class="heading" style="font-size: 90%;">Routing Table</td></tr></table>
        <table cellpadding="0" cellspacing="0">
        <tr>|;
foreach my $a ( @{$s{header}{ROUTING_TABLE}} ){
  print qq|<th>$a</th>|;
}
print '</tr>';

foreach my $r (sort keys %{$s{ROUTING_TABLE}} ){
  print '<tr>';
  foreach my $f ( @{$s{header}{ROUTING_TABLE}} ){
    if ($f =~ /(Common Name|Real Address)/){
      print qq|<td><a href="#" title="Click to disconnect">$s{ROUTING_TABLE}{$r}{$f}</a></td>|;
    }else{
      print qq|<td>$s{ROUTING_TABLE}{$r}{$f}</td>|;
    }
  }
  print '</tr>';
}

print qq|
        </table>
</td></tr></table>
</body>
</html>|;
}

# Output an html error message
sub error($){
my ($msg) = @_;
print qq|
<table height="100%" width="100%"><tbody>
  <tr height="50%" width="100%"><td colspan="3"></td></tr>
  <tr width="100%">
    <td width="33%"></td>
    <td width="34%" style="border:1pt solid black;background-color:red;color:black;text-align:center;">
      $msg
    </td>
    <td width="33%"></td></tr>
  <tr height="50%" width="100%"><td colspan="3"></td></tr>
</tbody></table>|;
exit 0;
}
