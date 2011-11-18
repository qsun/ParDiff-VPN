<?php
require_once('header.php');

require_once('db-config.php');
if (!$mysqli) {
    echo("Failed to connect database");
    exit(0);
}

require_login();

require_once('transactions.php');

$transactions = get_transactions($current_user);

if (is_array($transactions) && count($transactions) == 0) 
{
    print("您现在没有订单");
    exit(0);
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.4.2/jquery.min.js"></script>
<title>订单列表</title>
<script language="javascript">
function refresh_price() 
{
    amount = $("#order_amount").val();
    discount = get_discount(amount);
    price = get_price(amount);
    total = discount * price;
    save = price - total;
    
    $("#calculator_result").fadeIn();
    $("#before_discount .price").html(get_price(amount));
    $("#discount .price").html(discount);
    $("#after_discount .price").html(total);
    $("#save .price").html(save);
}

function get_price(amount)
{
    price = 10; // 1 GB for 10RMB
    special = 1;
    
    return amount * price * special;
}

function get_discount(amount)
{
    if (amount > 10) 
    {
        return 0.8;
    }
    else {
        return 1 - amount/50;
    }
}

function hide_result()
{
    $("#calculator_result").fadeOut();
}
    
</script>
</head>

<body>
<div id="container">
<div id="header">
订单列表
</div>
<div id="content">
<div id="order_list">
<?php

if (is_array($transactions) && count($transactions) > 0) {
    print("<table border=1>");
    
    print(Transaction::toHeader());
    
    foreach ($transactions as $transaction) {
        print($transaction->toHTMLList());
    }

    print("</table>");
}
else {
    print('<h1>目前没有订单</h1>');
}

?>
</div>
<div id="new_order">
    <h1>新订单</h1>
    <div id="calculator">
    <form action="new-order.php" method="post">
    流量 (GB) <input id="order_amount" type="text" name="traffic" value="0"/><br />
    <span style="border: 1px dotted black"  onClick="javascript:refresh_price()">计算价格</span><br />
    <div id="calculator_result" style="display:none">
    <div id="before_discount">
    价格(RMB):<span class="price"></span>
    </div>
    <div id="discount">
    折扣(RMB):<span class="price"></span>
    </div>
    <div id="after_discount">
    实际价格(RMB):<span class="price"></span>
    </div>
    <div id="save">
    您节约了(RMB):<span class="price"></span>
    </div>
    </div>
    <input type="submit" value="订购" /><input type="reset" onclick="javascript:hide_result()" />
    </form>
    </div>
</div>
</div>
<div id="footer">
<a href="http://www.pardiffvpn.com">ParDiffVPN</a> 2010
</div>
</div>
</body>
</html>
