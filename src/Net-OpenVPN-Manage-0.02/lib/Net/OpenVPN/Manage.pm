package Net::OpenVPN::Manage;

use strict;
use Net::Telnet;
use vars qw( $VERSION );

$VERSION = '0.02';

# $vpn = Net::OpenVPN::Manage->new({ 
#                    host => 'hostname.domain.com', 
#                    port => '7000' 
#                    password => 'password',
#                    timeout => 20
#                 });
sub new {
  my $class = shift;
  my $self  = shift;

  # Check if required arguments were passed.
  if (! ( $self->{'host'} && $self->{'port'} )){
  	return 0;
  }
  
  bless  ($self, $class);
  return ($self);
}

# $vpn->auth_retry($arg);
sub auth_retry {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'auth-retry '.$arg,  Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# $result = $vpn->echo($arg);
sub echo {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'echo '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n|END.*\n)/');
  unless ($telnet->last_prompt =~ /(SUCCESS:.*|END.*\n)/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  if ($telnet->last_prompt =~ /END.*\n/){
    return join('', @output);
  } else {
    return $telnet->last_prompt();
  }
}

# $help_text = $vpn->help();
sub help() {
  my $self = shift;
  my $telnet = $self->{objects}{_telnet_};  
  my @output = $telnet->cmd(String => 'help', Prompt => '/(END.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /END.*\n/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return join('', @output);
}

# $result = $vpn->hold($arg);
sub hold {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'hold '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# $result = $vpn->kill($arg);
sub kill($) {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'kill '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# $result = $vpn->log($arg);
sub log($) {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'log '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n|END.*\n)/');
  unless ($telnet->last_prompt =~ /(SUCCESS:.*|END.*\n)/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  if ($telnet->last_prompt =~ /END.*\n/){
    return \@output;
  } else {
    return $telnet->last_prompt();
  }
}

# $result = $vpn->mute();
sub mute {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'mute '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# Not implemented
sub net {
  my $self = shift;
  $self->{error_msg} = "net: command not implemented.\n";
  return 0;
}

# Not implemented
sub password {
  my $self = shift;
  $self->{error_msg} = "password: command not implemented.\n";
  return 0;
}

# $result = $vpn->signal('SIGHUP');
sub signal($) {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'signal '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# $array_ref = $vpn->state('all');
sub state {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'state '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n|END.*\n)/');
  unless ($telnet->last_prompt =~ /(SUCCESS:.*|END.*\n)/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  if ($telnet->last_prompt =~ /END.*\n/){
    return \@output;
  } else {
    return $telnet->last_prompt();
  }
}

# $array_ref = $vpn->status(2);
sub status {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'status '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n|END.*\n)/');
  unless ($telnet->last_prompt =~ /(SUCCESS:.*|END.*\n)/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  if ($telnet->last_prompt =~ /END.*\n/){
    return \@output;
  } else {
    return $telnet->last_prompt();
  }
}

# $hash_ref = $vpn->status_ref();
sub status_ref() {
  my $ref;
  my $self = shift;
  my $arg  = 2;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'status '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n|END.*\n)/');
  unless ($telnet->last_prompt =~ /(SUCCESS:.*|END.*\n)/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  unless ($telnet->last_prompt =~ /END.*\n/){
    return $telnet->last_prompt();
  }
  foreach my $ln ( @output ){
    if (( $ln eq '' ) || ( $ln =~ /^\s*$/ )){ 
      next;
    } elsif (( $ln =~ s/^(ROUTING_TABLE),// ) || ( $ln =~ s/^(CLIENT_LIST),// )){
      my @a = split ',', $ln;
      push @{$ref->{$1}}, \@a;
    } elsif ( $ln =~ s/^(HEADER),([\w\_]+),// ){
      push @{$ref->{$1}{$2}}, split(',', $ln);
    } elsif ( $ln =~ s/^(TITLE),// ){
      $ref->{$1}=$ln;
    } elsif ( $ln =~ s/^(TIME),// ){
      $ref->{$1}=$ln;
    } elsif ( $ln =~ s/^(GLOBAL_STATS),// ){
      $ref->{$1}=$ln;
    } 
  }
  return $ref;
}

# Not implemented
sub test {
  my $self = shift;
  $self->{error_msg} = "test: command not implemented.\n";
  return 0;
}

# Not implemented
sub username {
  my $self = shift;
  $self->{error_msg} = "username: command not implemented.\n";
  return 0;
}

# $result = $vpn->verb();
sub verb {
  my $self = shift;
  my $arg  = shift;
  my $telnet = $self->{objects}{_telnet_};
  $telnet->cmd(String => 'verb '.$arg, Prompt => '/(SUCCESS:.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /SUCCESS:.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return $telnet->last_prompt();
}

# $version = $vpn->version();
sub version() {
  my $self = shift;
  my $telnet = $self->{objects}{_telnet_};
  my @output = $telnet->cmd(String => 'version', Prompt => '/(END.*\n|ERROR:.*\n)/');
  unless ($telnet->last_prompt =~ /END.*/){
    $self->{error_msg} = $telnet->last_prompt();
    return 0;
  }
  return join('', @output);
}

# $boolean = $vpn->connect();
sub connect() {
  my $self = shift;
  my $telnet = Net::Telnet->new(Telnetmode => 0);
  
  # Set non-default timeout if one was specified when the object was created.
  if ( $self->{timeout} ){ $telnet->timeout($self->{timeout}); }
  
  # Set errormode to return so any timeout events don't die our script.
  $telnet->errmode('return');
  
  # Verify successful connection else error.
  unless ($telnet->open(Host => $self->{host}, Port => $self->{port})){
    $self->{error_msg} = 'Connection failed, verify host name/port and connectivity.';
    return 0;
  }
  
  # If a password was given, login to interface.
  if ( $self->{password} ){
    print $telnet->cmd(String => $self->{password}, Prompt => '/ENTER PASSWORD:/');
    unless ($telnet->last_prompt() =~ /ENTER PASSWORD/){
      $self->{error_msg} = 'Login failed, verify password and that management interface is not in use.';
      return 0;
    } else {
      # Remove extra lines returned from login that we don't want in later output.
      $telnet->getline();
      $telnet->getline();
    }
  }
  
  # If no password used verify connection using command.
  else {
    # Test if valid session by issuing the 'verb' command and checking response.
    # not a great way to validate, but it will work.
    $telnet->cmd(String => 'verb', Prompt => '/(SUCCESS:.*\n|ENTER PASSWORD:)/');
    unless ($telnet->last_prompt =~ /SUCCESS/){
      $self->{error_msg} = 'Invalid response from host, is a password expected, or the management interface in use?';
      return 0;
    }
  }
  
  $self->{objects}{_telnet_} = $telnet;
  return 1;
}

1;

__END__

=head1 NAME

Net::OpenVPN::Manage - Manage an OpenVPN process via it's management port

Version 0.02

=head1 DESCRIPTION

This module connects to the OpenVPN management interface, executes 
commands on the interface and returns the results or errors that result.

=head1 CHANGES

 0.02: 
   1: Added the 'status_ref' method so you don't have to parse out the 
      values from the scalar data returned from the 'status' method 
      yourself.
   2: Changed the 'status', 'log' and 'state' methods to return array 
      references instead of multi-line scalars when multi-line data 
      is returned.
   3: Made several changes to the POD text to show examples and data 
      types returned.

=head1 SYNOPSIS

  use Net::OpenVPN::Manage;

  my $vpn = Net::OpenVPN::Manage->new({ 
  		host=>'127.0.0.1', 
  		port=>'1195', 
  		password=>'password',
  		timeout=>10
  });
  
  # Error if unable to connect.
  unless($vpn->connect()){
    print $vpn->{error_msg}."\n";
    exit 0;
  }
  
  # Get the current status table in version 2 format from the process.
  my $status = $vpn->status(2);
  
  # If method returned false, print error message.
  # Otherwise print table to STDOUT.
  if ( ! $status ) {
    print $vpn->{error_msg};
    exit 0;
  } else {
    print $status."\n";
  }

=head1 USING THE MODULE

All the methods in this module will return 0, or boolean false if there is any error.
In most cases an error message detailing the problem will be returned in $obj->{error_msg}.

=head1 METHODS

=over 4

=item $vpn = Net::OpenVPN::Manage->new({ host=>'', port=>'', password=>'', timeout=>20 });

Constructs a new Net::OpenVPN::Manage object to connect to the specified process's management interface.
The anonymous hash that is passed specifies the target hostname or IP address, TCP port, and an optional password.
If no password is configured for your OpenVPN process, just omit the password reference. Optionally, you can
change the network timeout value as well.

=item $vpn->connect();

The connect method has no arguments passed to it. This method opens the connection to the remote host at the port specified, 
in the event that the host or port provided to the object are incorrect; or if there is already another network
session to this port (OpenVPN only supports a single management session at a time) this command will timeout.

For more extensive information on the use of the OpenVPN management commands referenced by these methods, see the
OpenVPN documentation (http://www.openvpn.net) or at least the management help screen (print $vpn->help();).


=item $vpn->auth_retry( $arg );

Changes the Auth failure retry mode. Arguments are: none, interact, or nointeract.

	# Sets auth-retry mode to 'none'
	$vpn->auth_retry('none'); 


=item $vpn->echo( $arg );

Returns messages from the echo buffer or changes echo state. Arguments are: on, off, all, or a integer designating number of lines to be returned.
The on and off arguments are really of no use here since it changes the state of the realtime management console echo messages and our session only connected for a brief time.

	# Returns entire echo buffer
	$vpn->echo('all'); 

=item $vpn->help();

Returns the help screen for the management command usage as a multiline string.

	# Prints the help screen to STDOUT
	print $vpn->help(); 

=item $vpn->hold( $arg );

If called without an argument it returns the current hold state; if called with an argument of: on, off, or release it changes the current hold state.

	# Releases the hold state on the OpenVPN process.
	$vpn->hold('release'); 
	
	# Prints current hold state.
	print $vpn->hold();

=item $vpn->kill( $arg );

Kills the VPN connection referenced. The argument may be either the common name of the connection or the real client IP:Port address.

	# kills the connection with the common name of 'jsmith'
	$vpn->kill('jsmith');

	# kills the connection where the client is connecting from: '63.73.83.93:17023'
	$vpn->kill('63.73.83.93:17023'); 

=item $vpn->log( $arg );

Returns messages from the log buffer or changes realtime log state. Arguments are: on, off, all, or an integer designating number of lines to be returned.
The on and off arguments are really of no use here since it changes the state of the realtime management console log messages and our session only connected for a brief time.
If logged records are requested, they are returned as an array reference - otherwise a scalar value is returned.

	# prints the entire log buffer.
	print @{$vpn->log('all')}; 
	
	# turns off logging.
	my $result = $vpn->log('off');

=item $vpn->mute( $arg );

If no argument is given it will show the log mute level for recurring log messages; if called with an argument it will change the log mute level to the value given.

	# Sets the log mute level to 10.
	$vpn->mute(10); 

=item $vpn->net();

This method has not been implemented. Only applicable on the Windows platform.

=item $vpn->password();

This method has not been implemented. Only of use when the management session is continuous - ours is not.

=item $vpn->signal( $arg );

Sends a signal to the OpenVPN daemon process. Arugments are: SIGHUP, SIGTERM, SIGUSR1, or SIGUSR2.
If the daemon is running under a non root or Administrator|System account it will not be able to restart
itself after a reset since it won't have the priveledges required to reopen the network interfaces.
See the OpenVPN HOWTO and the OpenVPN Management Interface documentation.

	# Sends SIGHUP signal to the process.
	$vpn->signal('SIGHUP'); 

=item $vpn->state();

Either turns on or off real time state notification if called with arguments of 'on' or 'off'; or returns current connection state information as an array reference if called without an argument, 'all', or some integer value that requests some X number of entries.

	# Print all connection states
	print @{$vpn->state()};

=item $vpn->status( $arg );

Returns the active connections status information as a multiline scalar where the optional argument (either 1 or 2 at this time) specifies the output format version. 
If called without an argument, it will return the data in the format defaulted by the daemon.

	# Print the connection status page using the version 2 format.
	print $vpn->status(2); 

=item $vpn->status_ref( );

Returns the active connections status information as a reference to a hash of arrays.

C<< my $r = $vpn->status_ref(); >>

C<< $r->{TITLE} >> is a scalar showing the status TITLE.

C<< $r->{TIME} >> is a scalar showing the status TIME.

C<< $r->{GLOBAL_STATS} >> is a scalar showing the GLOBAL_STATS line.

C<< $r->{HEADER}{CLIENT_LIST} >> is an array of the CLIENT_LIST column headers.

C<< $r->{HEADER}{ROUTING_TABLE} >> is an array of the ROUTING_TABLE column headers.

C<< $r->{CLIENT_LIST} >> is an array of arrays containing connection data for each active connection.

 [ 
  ["John Doe", "1.2.3.4:5678", "4.3.2.1", "67264", "87264", "Fri Jul  7 14:20:51 2006", "1152300051"], >
  ["Jane Doe", "2.3.4.5:6789", "5.4.3.2", "12347101", "19043721", "Tue Jul  3 12:10:05 2006", "1150000050"] >
  ...
 ]
 
C<< $r->{ROUTING_TABLE} >> is an array of arrays containing ROUTING_TABLE data for each active connection.
This structure is identical to that in $r->{CLIENT_LIST}, with an array containing an array of values for each connection.

 [
  ["4.3.2.1", "John Doe" ,"1.2.3.4:5678", "Fri Jul  7 14:41:35 2006", "1152301295"],
  ["5.4.3.2", "Jane Doe" ,"2.3.4.5:6789", "Tue Jul  3 12:10:05 2006", "1150000050"]
  ...
 ]
	
	 my $r = $vpn->status_ref(); 
	 foreach my $array_ref ( @{$r->{CLIENT_LIST}} ){ 
		print "Common Name: $$array_ref[0], bytes sent: $$array_ref[3], bytes recvd: $$array_ref[4]\n"; 
	 } 

	-- or to use the column heading ref --

	 my $r = $vpn->status_ref(); 
	 foreach my $array_ref ( @{$r->{CLIENT_LIST}} ){ 
	 	print "$r->{HEADER}{CLIENT_LIST}[0]: $$array_ref[0], $r->{HEADER}{CLIENT_LIST}[3]: $$array_ref[3], $r->{HEADER}{CLIENT_LIST}[4]: $$array_ref[4]"; 
	 } 

=item $vpn->test();

This method is not implemented. No real need to test management console.

=item $vpn->username();

This method has not been implemented. Only of use when the management session is continuous - ours is not.

=item $vpn->verb( $arg );

If called without an argument it returns the log verbosity level; if called with an argument (any valid log level) it changes the verbosity setting to the given value.

	# Change verbosity level to 1.
	$vpn->verb('1'); 

=item $vpn->version();

Returns a string showing the processes version information as well as the management interface's version.

	# Prints the version information to STDOUT.
	print $vpn->version(); 

=back

=head1 COOKBOOK

Example of CGI script that shows connected clients. Rough and definitely not pretty, just an example...
Keep in mind that to extend this, you could have hyperlinks that callback and can disconnect sessions, print log entries, etc.

	use strict;
	use CGI;
	use Net::OpenVPN::Manage;

	my $cgi=CGI->new();
	print $cgi->header();

	my $vpn=Net::OpenVPN::Manage->new({host=>'openvpn.domain.com', port=>'1195', password=>'password', timeout=>'5'});
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

=head1 AUTHOR

Copyright (c) 2006 Aaron Meyer / MeyerTech.net

This module is free software; you can redistribute it or modify it under
the same terms as Perl itself.

=cut
