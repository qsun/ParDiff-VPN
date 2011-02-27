#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;
use DBI;
use Data::Dumper;
use Digest::MD5 qw(md5 md5_hex);
use Email::Valid;
use HTML::Template;
use Config::Simple;

my %config;
Config::Simple->import_from('config.ini', \%config);

my $dsn = $config{'mysql.dsn'};
my $dbh = DBI->connect($dsn, $config{'mysql.username'}, $config{'mysql.password'});

my $header_sent = 0;

my $op = param('op');

my $template = HTML::Template->new(filename => 'admin.tmpl');

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

sub get_client_ip()
{
    my $sth = $dbh->prepare("SELECT clientip FROM login ORDER BY id DESC LIMIT 1");
    $sth->execute();
    while (my $ref = $sth->fetchrow_hashref()) {
        return num_to_ip(ip_to_num($ref->{'clientip'}) + 1);
    }
    return '0.0.0.0';
}
    
sub exist_username($)
{
    my $username = shift;
    
    my $sth = $dbh->prepare("SELECT * FROM login WHERE username = ?");
    $sth->execute($username);
    if ($sth->rows <= 0) {
        return undef;
    }

    return 1;
}

sub auth
{
    my $q = new CGI;
    my $session = $q->cookie('session');
    my $username = $q->cookie('username');

    if (!$session or !$username) {
        return undef;
    }
    
    my $sth = $dbh->prepare('SELECT md5(concat(password, session)) AS session_db FROM admins WHERE username = ?');
    $sth->execute($username);
    my $ref = $sth->fetchrow_hashref();

    if ($ref && $ref->{'session_db'} eq $session) {
        return $username;
    }
    else {
        return undef;
    }
}


my $content = '';
my $title = '';

sub _index()
{
    $title = '请选择';

    my $sth = $dbh->prepare('select sum(connection_traffic) / 1024 / 1024 AS consumtion from connections where end_time > date_sub(now(), interval 1 day)');
    $sth->execute();
    $content .= '<div class="section">';
    $content .= '<h1>最近24小时流量信息</h1>';

    my $sumref = $sth->fetchrow_hashref();
    if ($sumref) {
        $content .= '<p>客户共消费: ' . $sumref->{consumtion} . ' MB 流量</p>';
    }

    $sth = $dbh->prepare("SELECT sum(connection_traffic) / 1024 / 1024 AS consumtion FROM connections WHERE start_time > date(now())");
    $sth->execute();
    $sumref = $sth->fetchrow_hashref();
    if ($sumref) {
        $content .= '<h1>今日流量信息</h1>';
        $content .= '<p>客户共消费: ' . $sumref->{consumtion} . ' MB 流量</p>';
    }

    $content .= '<h1>连接数量图</h1><img src="ccconnections.png" /><h1>流量图</h1><img src="trafficc.png" />';

    $sth = $dbh->prepare('SELECT username, (unix_timestamp(now()) - unix_timestamp(start_time))/60 AS duration,  connection_traffic AS traffic FROM connections WHERE start_time > end_time');
    $sth->execute();

    $content .= '</div><div class="section">';

    $content .= '<h1>当前在线客户</h1><table border=1><tr><td>Username</td><td>连接时间</td><td>流量</td></tr>';
    while (my $ref = $sth->fetchrow_hashref()) {
        $content .= '<tr>';
        $content .= '<td>' . $ref->{username} . '</td><td>' . int($ref->{duration}) . '分钟</td><td>' .  int($ref->{traffic}/1024/1024) .'MB</span></td></tr>';
    }
    $content .= '</table></div><div class="section">';

    # now lets display the recent activities
    $sth = $dbh->prepare('SELECT * FROM logs ORDER BY ts DESC LIMIT 20');
    $sth->execute();
    
    $content = $content . '<h1>近期活动</h1><ol>';

    while (my $ref = $sth->fetchrow_hashref()) {

        $content = $content . '<li><span class=ts>' . $ref->{ts} . '</span> ' . $ref->{event} . "</li>\n";
    }

    $content = $content . '</ol>';


    $sth = $dbh->prepare("SELECT * FROM connections ORDER BY start_time DESC LIMIT 10");
    $sth->execute();

    $content .= '<h1>最近登陆:</h1><table border=1>';

    while (my $ref = $sth->fetchrow_hashref) {
         $content = $content . '<tr><td><span class=ts>' . $ref->{start_time} .'</span></td><td><span>' . $ref->{end_time} . '</span></td>';
         $content = $content . '<td>'. $ref->{username} . '</td>';

         if ($ref->{connection_traffic} > 1024 * 1024) {
             $content .= '<td>' . int($ref->{connection_traffic}/1024/1024) . 'MB</td>';
         }
         else {
             $content .= '<td>' . $ref->{connection_traffic} . 'B</td>';
         }

         $content .= '<td>' . $ref->{interface} . '</td>';

         $content .= '<tr>';
    }
    
    $content .= '</table></div><form action="main.cgi" method="post">查询充值卡<input type="text" name="coupon" /><br /><input type="submit" value="查询"/><input type="hidden" name="op" value="check_coupon" /></form>';
    $template->param(CONTENT => $content);
};

if (!$op or $op eq "login") {
    if (!auth()) {

        use Captcha::reCAPTCHA;
        # Your reCAPTCHA keys from
        #   https://admin.recaptcha.net/recaptcha/createsite/
        use constant PUBLIC_KEY       => 'xxx';
        use constant PRIVATE_KEY      => 'xxx';

        my $captcha = Captcha::reCAPTCHA->new;
        my $result = $captcha->check_answer(PRIVATE_KEY, $ENV{'REMOTE_ADDR'}, 
                                            param( 'recaptcha_challenge_field' ),
                                            param( 'recaptcha_response_field' ));

        if (!$result->{is_valid}) {
            print header(-charset => 'utf-8');
            print start_html(-title => '验证码不正确!');
            print '<h1>验证码不正确，你被判定为机器人!</h1><a href="/admin">登陆</a>';
            print end_html;
            exit 0;
        }

        my $username = param('username');
        my $password = param('password');

        my $sth = $dbh->prepare("SELECT id, username, password FROM admins WHERE username = ? AND password = md5(?)");
        $sth->execute($username, $password);
        if ($sth->rows <= 0) {
            #login failed
            print redirect('index.html');
        } else {
            my $salt = md5_hex(rand(20));
            my $session = md5_hex(md5_hex($password) . $salt);

            $sth = $dbh->prepare("UPDATE admins SET session = ? WHERE username = ?");
            $sth->execute($salt, $username);

            my $username_cookie = cookie(-name => 'username', -value => $username);
            my $session_cookie = cookie(-name => 'session', -value => $session);
            print header(-charset => 'utf-8',
                         -cookie => [$username_cookie, $session_cookie]);

            $header_sent = 1;

            _index();
        }
    } else {
        _index();
    }
}
elsif ($op eq "add_user_form") {
    if (auth()) {
        $title = '增加用户';
        $content = '<form action="main.cgi" method="post"><input type="hidden" name="op" value="add_user" />Username: <input type="text" name="username" /><br>Password: <input type="text" name="password" /><br>Password again: <input type="text" name="password-2"/><br>Email:<input type="text" name="email"/><br>流量(in MB):<input type="text" name="credit" value=0><br><input type="submit" value="建立用户"/>';
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq "add_user") {
    my $admin_username = auth();
    if ($admin_username) {
        my $new_username = param('username');
        my $new_password = param('password');
        my $new_password_2 = param('password-2');
        my $new_email = param('email');
        my $new_credit = param('credit');

        my $new_client_ip = get_client_ip();

        if ($new_username && $new_password && $new_password_2 && $new_email && ($new_credit >= 0) && $new_password eq $new_password_2 && !exist_username($new_username)) {
            my $sth = $dbh->prepare("INSERT INTO login (id, username, password, clientip, serverip, credit, email) VALUES (0, ?, ?, ?, '10.1.1.1', ? * 1024 * 1024, ?)");
            $sth->execute($new_username, $new_password, $new_client_ip, $new_credit, $new_email);

            $title = '增加用户成功';
            $content = '<p>成功增加用户: ' . $new_username . '</p>';

            my $sql_statement = "INSERT INTO transactions_records (credit, operator, username, ts, note) VALUES (?, ?, ?, NOW(), 'Admin人工')";
            $sth = $dbh->prepare($sql_statement);
            $sth->execute($new_credit * 1024 * 1024, $admin_username, $new_username);
        }
        else {
            print redirect('main.cgi?op=add_user_form');
            exit(0);
        }
    } else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'change_user_state') {
    if (auth()) {
        if (param('action') && (param('action') eq 'active' || param('action') eq 'freeze') && param('userid')) {
            my $sth;
            if (param('action') eq 'active') {
                $sth = $dbh->prepare("UPDATE login SET account_state = 'ACTIVE' WHERE id = ?");
            } elsif (param('action') eq 'freeze') {
                $sth = $dbh->prepare("UPDATE login SET account_state = 'DISABLED' WHERE id = ?");
            }
        
            $sth->execute(param('userid'));
            
            $sth = $dbh->prepare('INSERT INTO logs (event) VALUES (?)');
            $sth->execute(param('action') . ' on userid: ' . param('userid'));
        }

        print redirect('main.cgi?op=list');
        exit(0);
    } else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'change_note') {
    if (auth()) {
        my $userid = param('userid');
        my $notes = param('notes');
        my $sth = $dbh->prepare("UPDATE login SET notes = ? where id = ? LIMIT 1");
        $sth->execute($notes, $userid);
        
        $title = "修改备注";
        $content = '备注修改成功。';
    }
}
elsif ($op eq 'list') {
    if (auth()) {
        my $sth = $dbh->prepare("SELECT * FROM login");
        $sth->execute();
        
        $title = '用户列表';
        $content = '<table id="users_list_table" border=1><tr><td>id</td><td>username</td><td>credit</td><td>email</td><td>状态</td><td>备注</td></tr>';

        while (my $ref = $sth->fetchrow_hashref()) {
            $content = $content . '<tr><td>' . $ref->{id}. '</td><td><a href="main.cgi?op=list_user&username='.  $ref->{username} .  '"><!--<img src="http://www.gravatar.com/avatar/' . md5_hex($ref->{email}) . '"></img>-->' . $ref->{username} . '</a></td><td>' . (int($ref->{credit}/1024/1024) > 0 ? '<span>' : '<span class="attention">') .int($ref->{credit} / 1024 / 1024) . 'MB</td><td><a href="main.cgi?op=sendmail_form&to=' .  $ref->{email} . '">' .  $ref->{email} .  '</a></td><td>' . ($ref->{account_state} eq "ACTIVE" ? ('活动 (<a href="main.cgi?op=change_user_state&action=freeze&userid=' . $ref->{id} . '">冻结用户</a>)') : ('<span class="attention">冻结</span> (<a href="main.cgi?op=change_user_state&action=active&userid=' . $ref->{id} . '">激活用户</a> )')) . '</td><td>' . 
                '<form action="main.cgi" method="post"><input type="hidden" name="op" value="change_note" /><input size="5" name="notes" value="' . ($ref->{notes} ? $ref->{notes} : 'N/A') . '" /><input type="submit" value="change" /><input type="hidden" name="userid" value="'.$ref->{id} . '" /></form>'
                    . '</td></tr>';
        }

        $content .= '</table>';
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'list_user') {
    if (auth()) {
        my $username = param('username');

        $title = $username . '详细信息';

        $content = '<form action="main.cgi" method="post"><input type="hidden" name="op" value="change_user_info" /><table border=1><input type="hidden" name="username" value="' . $username . '"/>';
        my $sth = $dbh->prepare("SELECT * FROM login WHERE username = ? LIMIT 1;");
        $sth->execute($username);
        while (my $ref = $sth->fetchrow_hashref()) {
            $content .= '<tr><td>ID</td><td>' . $ref->{id} . '</td></tr>' . "\n";
            $content .= '<tr><td>Username</td><td>' . $ref->{username} . '</td></tr>' . "\n";
            $content .= '<tr><td>Password</td><td><input type="text" name="password" value="new password here!"/></td></tr>' . "\n";
            $content .= '<tr><td>Credit</td><td><a href="main.cgi?op=credit_list&username=' . $username . '">' . int($ref->{credit}/1024/1024) . 'MB</a></td></tr>' . "\n";
            $content .= '<tr><td>Email: </td><td><input type="text" name="email" value="' . $ref->{email} . '"/></td></tr>' . "\n";
	    $content .= '<tr><td>过期: </td><td><pre>' . ($ref->{expire_at} ? $ref->{expire_at} : '不过期') . '</pre>延长<input type="text" name=extend value=0 />天</td></tr>' . "\n";
        }

        $content .= '</table><br><input type="submit" value="Submit"/></form><br><a href="main.cgi?op=list">退回</a>';
        $content .= '<h1>登录信息</h1><br>';

        $sth = $dbh->prepare("SELECT * FROM connections WHERE username = ? ORDER BY id DESC LIMIT 100");
        $sth->execute($username);
        
        $content .= '<table border=1>';
        $content .= "<tr><td>id</td><td>ip</td><td>from</td><td>to</td><td>Traffic</td><td>Type</td></tr>\n";

        while (my $ref = $sth->fetchrow_hashref()) {
            $content .= '<tr><td>'.  $ref->{id}. '</td><td>'. $ref->{ip}. '</td><td>'. $ref->{start_time}. '</td><td>'. $ref->{end_time}. '</td><td>'. 
                ($ref->{connection_traffic} > 1024 * 1024 ? int($ref->{connection_traffic} / 1024 / 1024) . 'MB' : $ref->{connection_traffic} . 'B') . 
                '</td><td>' . $ref->{interface} . '</td><tr>';
        }
        
        $content .=  '</table>';

        $content .= '<hr></table><h2>充值历史</h2><table border=1><tr><td>时间</td><td>流量</td><td>类型(点卡/人工)</td><td>备注</td>';

        {
            my $sth = $dbh->prepare('SELECT credit, operator, username, ts, note FROM transactions_records WHERE username = ? ORDER BY ts DESC');
            $sth->execute($username);
            while (my $ref = $sth->fetchrow_hashref()) {
                $content .= '<tr><td>' . $ref->{ts}. '</td><td>'. int($ref->{credit} / 1024 / 1024). 'MB</td><td>' . ($ref->{operator} eq 'coupon' ? '点卡' : '人工') . '</td><td>'.  ($ref->{note} ? $ref->{note} : '') . '</td></tr>'; 
            }
        }

        $content .= '</table>';

    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'change_user_info') {
    if (auth()) {
        my $username = param('username');
        my $new_password = param('password');
        my $new_email = param('email');
        my $extend = param('extend');
        my $sth;

        if ($new_password ne 'new password here!') {
            $sth = $dbh->prepare('UPDATE login SET password = ? WHERE username = ? LIMIT 1');
            $sth->execute($new_password, $username);
        }

        $sth = $dbh->prepare('UPDATE login SET email = ? WHERE username = ? LIMIT 1');
        $sth->execute($new_email, $username);

        $sth = $dbh->prepare("UPDATE login SET expire_at = IF(expire_at IS NOT NULL, date_add(expire_at, interval ? day), date_add(now(), interval ? day)) WHERE username = ? limit 1");
        $sth->execute($extend, $extend, $username);

        print redirect('main.cgi?op=list_user&username=' . $username);
        exit(0);
    } else {
        print redirect('index.html');
        exit(0);
    }
} elsif ($op eq 'credit_list') {
    if (auth()) {
        my $new_username = param('username');
        my $sth = $dbh->prepare("SELECT * FROM login WHERE username = ?");
        $sth->execute($new_username);
        my $ref = $sth->fetchrow_hashref();

        if ($ref) {
            $title .= '余额操作';
            $content .= '<h1>Change ' . $new_username . ' credit</h1>Current credit: ' . int($ref->{credit}/1024/1024) . 'MB(' . $ref->{credit} . ')<br>Delta credit: <form action="main.cgi" method="post"><input type="hidden" value="onchangecredit" name="op"/><input type="text" name="delta" />MB<br><input type="hidden" name="username" value="' . $ref->{username} . '"/><input type="submit" value="Submit!" /></form>';
        }
        else {
            print redirect('main.cgi?op=list');
            exit(0);
        }
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'onchangecredit') {
    my $admin_username = auth();
    if ($admin_username) {
        my $change_username = param('username');
        my $delta = param('delta') * 1024 * 1024;

        my $sth = $dbh->prepare("UPDATE login SET account_state = 'ACTIVE', credit = credit + ? WHERE username = ? LIMIT 1");
        $sth->execute($delta, $change_username);

        # record
        my $sql_statement = "INSERT INTO transactions_records (credit, operator, username, ts, note) VALUES (?, ?, ?, NOW(), 'Admin人工')";
        $sth = $dbh->prepare($sql_statement);
        $sth->execute($delta, $admin_username, $change_username);
        
        print redirect('main.cgi?op=list_user&username=' . $change_username);
        exit(0);
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'change_admin_password_form') {
    if (auth()) {
        $title = '修改管理员密码';
        $content = '<form action="main.cgi" method="post"><input type="hidden" name="op" value="change_admin_password"/>new password:<input type="text" name="password_1" /><br>retype password:<input type="text" name="password_2" /><br><input type="submit" value="修改"/></form>';
    } else {
        print redirect('index.html');
        exit(0);
    }
} elsif ($op eq 'change_admin_password') {
    my $username = auth();

    if ($username) {
        my $password_1 = param('password_1');
        my $password_2 = param('password_2');
        
        if ($password_1 && $password_2 && $password_1 eq $password_2) {
            my $sth = $dbh->prepare('UPDATE admins SET password = md5(?) WHERE username = ?');
            $sth->execute($password_1, $username);
            print redirect('main.cgi');
            exit(0);
        }
        else {
            print redirect('main.cgi?op=change_admin_password_form');
            exit(0);
        }
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'sendmail_form') {
    if (auth()) {
        my $to = param('to');
        if (Email::Valid->address($to)) {
            $title = '发送邮件';
            $content .= '<h1>发送邮件给' . $to . '</h1>';
            $content .= '<form action="main.cgi" method="post"><input type=hidden name="op" value="sendmail" />标题:<input type=text name="subject" /><br>正文<textarea name="content"></textarea><br><input type=submit value="submit"/><input type=hidden name="to" value="' . $to . '"/></form>';
        }
        else {
            $title = '发送邮件给所有用户';
            $content = '<h1>发送邮件给所有用户</h1><form action="main.cgi" method="post">';
            $content .= '<p>请选择收件人:</p><ul id="email_recipient_selection">';
            $content .= '<li>新用户<input type="radio" name="to" value="new" /></li>';
            $content .= '<li>已冻结用户<input type="radio" name="to" value="frozen" /></li>';
            $content .= '<li>活动用户<input type="radio" name="to" value="active" /></li>';
            $content .= '<li>所有用户<input type="radio" name="to" value="all" /></li>';
            $content .= '</ul></p>';

            $content .= '<input type=hidden value="sendmail" name="op"/>标题:<br /><input class="email_input_field" type=text name="subject" /><br />正文<br /><textarea class="email_input_field email_input_content_field" name="content"></textarea><br><input type=submit value="submit"/></form>';
        }
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'sendmail') {
    if (auth()) {
        my $to = param('to');
        my $subject = param('subject');
        my $email_content = param('content');
        $content .= "\n";
        my @recipients;

        if ($to eq 'all' ||
            $to eq 'new' ||
            $to eq 'frozen' ||
            $to eq 'active')
        {
            my $sth;

            if ($to eq 'all') {
                $sth = $dbh->prepare('SELECT username, email FROM login');
            } elsif ($to eq 'new') {
                $sth = $dbh->prepare('SELECT username, email FROM login WHERE credit = 0');
            } elsif ($to eq 'frozen') {
                $sth = $dbh->prepare("SELECT username, email FROM login WHERE account_state = 'DISABLED'");
            } elsif ($to eq 'active') {
                $sth = $dbh->prepare("SELECT username, email FROM login WHERE credit >= 0 AND account_state = 'ACTIVE'");
            }

            $sth->execute();
            while (my $ref = $sth->fetchrow_hashref()) {
                next if (!Email::Valid->address($ref->{email}));
                my $email = $ref->{email};

                push @recipients, $email;
                email_to_address($email, $subject, $email_content);
            }
            $title = '已发送到';
            $content = 'delivered to <br><table border=1>';
            foreach (@recipients) {
                $content .= '<tr><td>' . $_ . '</td></tr>';
            }
            $content .= '</table>';
        }
        elsif (Email::Valid->address($to)) {
            email_to_address($to, $subject, $email_content);

            push @recipients, $to;
            $title = '已发送到';
            $content = 'delivered to <br><table border=1>';
            foreach (@recipients) {
                $content .= '<tr><td>' . $_ . '</td></tr>';
            }
            $content .= '</table>';
        }
        else {
            print redirect('main.cgi?op=list');
            exit(0);
        }
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'list_tickets') {
    if (auth()) {
        my $sth = $dbh->prepare("SELECT tt.status, tt.id, tt.title, tt.username, tt.email, tt.category, t.ts as last_ts, t.username as last_username FROM ticket_threads tt JOIN (SELECT max(id) as ticket_id, thread_id FROM tickets GROUP BY thread_id) AS x ON x.thread_id = tt.id JOIN tickets t WHERE t.id = ticket_id");
        $sth->execute();

        $title = '列出所有tickets';

        $content .= '<table border=1 width="90%">';

        $content .= '<tr><td>状态</td><td>标题</td><td>用户名</td><td>用户邮箱</td><td>类别</td><td>最后修改时间</td><td>最后回复人</td></tr>';
        
        while (my $ref = $sth->fetchrow_hashref()) {
            warn(Dumper($ref));
            my $status = ($ref->{status} eq 'closed') ? 'closed' : '<span class="attention">open</span>';

            $content .= '<tr><td>' . $status . '</td><td><a href="main.cgi?op=view_ticket&ticket_id=' . $ref->{id} . '">' . $ref->{title} . '</a></td><td>' . $ref->{username} . '</td><td>' . $ref->{email} . '</td><td>' . $ref->{category} . '</td><td>' . $ref->{last_ts} . '</td><td>' . $ref->{last_username} . '</td></tr>';
        }

        $content .= '</table>';

    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'check_coupon') {
    if (auth()) {
        my $coupon = '%' . param('coupon') . '%';
        
        my $sth = $dbh->prepare("SELECT * FROM transactions_records WHERE note LIKE ? ORDER BY ts DESC LIMIT 20");
        $sth->execute($coupon);
        
        $title = "Coupon $coupon  查询";

        $content .= '<table border="1" width="90%"><tr><td>流量</td><td>操作人</td><td>用户</td><td>时间</td><td>记录</td></tr>';
        while (my $ref = $sth->fetchrow_hashref()) {
            $content .= "<tr><td>" . $ref->{credit}/1024/1024 . "MB</td><td>" . $ref->{operator} . "</td><td>" . $ref->{username} . "</td><td>" . $ref->{ts} . "</td><td>" . $ref->{note} . "</td></tr>";
        }
        
        $content .= "</table>";
    }
}
elsif ($op eq 'view_ticket') {
    my $admin_name = auth();
    if ($admin_name) {

        my $ticket_id = param('ticket_id');
        $title = 'Ticket ' . $ticket_id;

        my $sth = $dbh->prepare("SELECT * FROM tickets WHERE thread_id = ? ORDER BY id DESC");
        $sth->execute($ticket_id);

        $content .= '<table border="1" width="90%">';

        while (my $ref = $sth->fetchrow_hashref()) {
            my $role = $ref->{role};
            my $ts = $ref->{ts};
            my $username = $ref->{username};
            my $ticket_content = $ref->{content};

            $content .= '<tr><td><p>日期: ' . $ts . '</p>' . ($role ne 'client'? "<p>回复人: ${username}" : '') . '<pre>' . $ticket_content . '</pre></td></tr>';
        }

        $content .= '</table>';

        $content .= '<form action="main.cgi" method="post">
<input type="hidden" name="op" value="mod_ticket" />
<input type="hidden" name="ticket_id" value="' . $ticket_id . '" />
<textarea name="ticket_content" cols="80" rows="10">
请在此输入
</textarea>
<br />
<select name="status">
<option value="closed">关闭</option>
<option value="open" selected>开启</option>
</select>
<br />
<input type="submit" value="提交" />
</form>';

    } else {
        print redirect('index.html');
        exit(0);
    }
}
elsif ($op eq 'mod_ticket') {
    my $admin_name = auth();
    if ($admin_name) {
        my $ticket_id = param('ticket_id');
        my $ticket_token = md5_hex($ticket_id . md5_hex($ticket_id));
        my $ticket_status = param('status');
        my $ticket_content = param('ticket_content');

        my $sth = $dbh->prepare("INSERT INTO tickets VALUES (0, ?, 'pardiff', now(), ?, ?)");
        $sth->execute($ticket_id, $admin_name, $ticket_content);

        $sth = $dbh->prepare("UPDATE ticket_threads SET status = ? WHERE id = ?");
        $sth->execute($ticket_status, $ticket_id);

        $title = "ticket修改成功";
        $content = '<p>' . $ticket_content . '</p>';

        email_to_address('support@pardiffvpn.com', 'ticket replied - ' . $ticket_id . ' by ' . $admin_name, 
                         $ticket_content . "\n\n==========\n\n status: ${ticket_status} \n\n");

        $sth = $dbh->prepare("SELECT title, status, email FROM ticket_threads WHERE id = ?");
        $sth->execute($ticket_id);
        my $ref = $sth->fetchrow_hashref();
        if ($ref && $ref->{email}) {
            email_to_address($ref->{email}, '技术支持状态修改: TicketID-' . $ticket_id, '尊敬的ParDiff客户，

您提交的支持消息(' . $ref->{title} . ')已经得到ParDiff技术支持响应:
'
. $ticket_content .
'
目前状态为' . ($ref->{status} eq 'open' ? '打开' : '关闭') . 
'
所有后续消息都将通过ParDiff支持系统(vpn.pardiff.com/ticket)完成，您会在此邮箱中收到后续信息提示。

点此浏览后续信息 http://vpn.pardiff.com/ticket/view.cgi?ticket=' . $ticket_id . '&token=' . $ticket_token .'

感谢您的合作。

ParDiff VPN Support
support@pardiff.com
');
    
        }
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}
else {
    if (auth()) {
        $title = '未知操作';
        $content .= '<h1>未知操作</h1>';
    }
    else {
        print redirect('index.html');
        exit(0);
    }
}


if (!$header_sent) {
    print header(-charset => 'utf-8');
}

$template->param(CONTENT => $content);
$template->param(TITLE => $title);
print $template->output();
