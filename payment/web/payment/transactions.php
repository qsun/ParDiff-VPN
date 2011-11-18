<?php
/* util library for dealing with transactions */

class Transaction 
{
    var $transaction;
    
    function Transaction($id, $subject, $body, $total, $traffic, $status, $type) 
    {
        $this->transaction[id] = $id;
        $this->transaction[subject] = $subject;
        $this->transaction[body] = $body;
        $this->transaction[total] = $total;
        $this->transaction[traffic] = $traffic;
        $this->transaction[status] = $status;
        $this->transaction[type] = $type;
    }

    static function toHeader($fields = false) 
    {
        if ($fields == false) {
            $fields = array('id', '商品名称', '说明', '总价格(元)', '流量 (G)', '状态', '类型');
        }

        $ret_str = "<tr>";
        
        foreach ($fields as $field) {
            $ret_str .= "<td>" . $field . "</td>";
        }

        $ret_str .= "</tr>";
        
        return $ret_str;
    }

    function getOwner() 
    {
        return $this->transaction[username];
    }
    
    function toHTMLList()
    {
        if ($fields == false) {
            $fields = array('subject', 'body', 'total', 'traffic', 'status', 'type');
        }

        $count = 1;
        $ret_str = '<tr><td>' . $count . '</td>';
        
        foreach ($fields as $field) {
            if ($field == 'status' && $this->transaction[$field] == 'UNPAID') {
                $ret_str .= '<td class="cls-' . $field . '"><a href="#">' . $this->transaction[$field] . '</a></td>';
            }
            else {
                $ret_str .= '<td class="cls-' . $field . '">' . $this->transaction[$field] . '</td>';
            }
            $count++;
        }

        $ret_str .= "</tr>";
        
        return $ret_str;
    }
}

function charge($id)
{
    global $mysqli;

    if ($stmt = $mysqli->prepare('SELECT traffic, username, alipay_trade_no, alipay_buyer_email FROM payment_transactions WHERE id = ?')) {
        $stmt->bind_param('i', $id);
        $stmt->execute();
        
        $stmt->bind_result($traffic, $username, $trade_no, $email);
        $trans = array();
        
        while ($stmt->fetch()) {
            $trans[] = array('username' => $username, 
                             'traffic' => $traffic,
                             'trade_no' => $trade_no,
                             'email' => $email);
        }
        
        if (is_array($trans) && count($trans) > 0) {
            foreach ($trans as $tran) {
                $username = $tran['username'];
                $traffic = $tran['traffic'] * 1024 * 1024 * 1024;
                $trade_no = $tran['trade_no'];
                $email = $tran['email'];
            
                $stmt_insert = $mysqli->prepare("INSERT INTO transactions_records (credit, operator, username, ts, note) VALUES (?, 'ALIPAY_DIRECT', ?, now(), CONCAT('online pay: ', ?, ' - ', ?))");
                $stmt_insert->bind_param('isss', $traffic, $username, $trade_no, $email);
                $stmt_insert->execute();

                $stmt_update = $mysqli->prepare('UPDATE login SET credit = credit + ? WHERE username = ?');
                $stmt_update->bind_param('is', $traffic, $username);
                $stmt_update->execute();
            }
        }
    }
    
}

function get_transaction_by_id($id) 
{
    global $mysqli;
    if ($stmt = $mysqli->prepare("SELECT status FROM payment_transactions WHERE id = ?")) {
        $stmt->bind_param('i', $id);
        $stmt->execute();
        $stmt->bind_result($status);
        while ($stmt->fetch()) {
            return $status;
        }
    }
    
    return false;
}

function pay_transaction($id, $tn, $email) 
{
    global $mysqli;
    if ($stmt = $mysqli->prepare("UPDATE payment_transactions SET status = 'PAID', alipay_trade_no = ?, alipay_buyer_email = ? WHERE id = ? AND status = 'UNPAID'")) {
        $stmt->bind_param('ssi', $tn, $email, $id);
        $stmt->execute();
        if ($stmt->affected_rows == 1) {
            return 1;
        }
    }

    return false;
}

function get_transactions($current_user) 
{
    global $mysqli;
    global $current_user;

    $transactions = array();

    if ($stmt = $mysqli->prepare("SELECT id, subject, body, total, traffic, status, `type` FROM payment_transactions WHERE username = ?")) {
        
        $stmt->bind_param('s', $current_user);
        $stmt->execute();
        $stmt->bind_result($id, $subject, $body, $total, $traffic, $status, $type);
        while ($stmt->fetch()) {
            $transactions[] = new Transaction($id, $subject, $body, $total, $traffic, $status, $type);
        }
        //var_dump("Transactions: ");
        
        // var_dump($transactions);
    }

    return $transactions;
}

function get_price($amount) 
{
    $price = 10;
    $special = 1;
    
    return $amount * $price * $special;
}

function get_discount($amount) 
{
    if ($amount > 10) {
        return 0.8;
    }
    else {
        return 1- $amount / 50;
    }
}

?>
