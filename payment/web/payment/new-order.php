<?php
require_once('header.php');

require_once('db-config.php');
if (!$mysqli) {
    echo("Failed to connect database");
    exit(0);
}

require_login();

require_once('transactions.php');

$amount = $_POST['traffic'];
if (is_numeric($amount)) {
    /* put it into database and generate link for alipay */

    /*

      insert into payment_transactions (subject, body, total, traffic, username, status, `type`) VALUES ('ParDiff VPN traffic {$amount} GB - ￥{$total}', 'ParDiff VPN traffic {$amount} GB - ￥{$total}, http://www.pardiffvpn.com', {$total}, {$amount}, {$current_user}, 'UNPAID', 'VPN')

     */
    $total = get_price($amount) * get_discount($amount);
    if ($total < 1) {
        $total = 1;
    }
    $total = floor($total);
    
    $subject = "ParDiff VPN traffic {$amount} GB - ￥{$total}";
    $body = "ParDiff VPN traffic {$amount} GB - ￥{$total}, http://www.pardiffvpn.com";

    if ($mysqli->query("insert into payment_transactions (subject, body, total, traffic, username, status, `type`) VALUES ('{$subject}', '{$body}', {$total}, {$amount}, '{$current_user}', 'UNPAID', 'VPN')")) {
        require_once("alipay_config.php");
        require_once("class/alipay_service.php");
        
        $out_trade_no = $mysqli->insert_id;
        $extra_common_param = $current_user;
        
        $parameter = array(
            "service"         => "create_direct_pay_by_user",	//接口名称，不需要修改
            "payment_type"    => "1",               			//交易类型，不需要修改
            //获取配置文件(alipay_config.php)中的值
            "partner"         => $partner,
            "seller_email"    => $seller_email,
            "return_url"      => $return_url,
            "notify_url"      => $notify_url,
            "_input_charset"  => $_input_charset,
            "show_url"        => $show_url,
            
            //从订单数据中动态获取到的必填参数
            "out_trade_no"    => $out_trade_no,
            "subject"         => $subject,
            "body"            => $body,
            "total_fee"       => $total,

            //扩展功能参数——网银提前
            "paymethod"	      => $paymethod,
            "defaultbank"	      => $defaultbank,
            
            //扩展功能参数——防钓鱼
            "anti_phishing_key"   => $encrypt_key,

            //扩展功能参数——分润(若要使用，请取消下面两行注释)
            //$royalty_type   => "10",	  //提成类型，不需要修改
            //$royalty_parameters => "111@126.com^0.01^分润备注一|222@126.com^0.01^分润备注二",
            /*提成信息集，与需要结合商户网站自身情况动态获取每笔交易的各分润收款账号、各分润金额、各分润说明。最多只能设置10条
              提成信息集格式为：收款方Email_1^金额1^备注1|收款方Email_2^金额2^备注2
            */
            
            //扩展功能参数——自定义超时(若要使用，请取消下面一行注释)。该功能默认不开通，需联系客户经理咨询
            //$it_b_pay	      => "2h",	  //超时时间，不填默认是15天。八个值可选：1h(1小时),2h(2小时),3h(3小时),1d(1天),3d(3天),7d(7天),15d(15天),1c(当天)
            
            //扩展功能参数——自定义参数
            "extra_common_param" => $extra_common_param
            );
        $alipay = new alipay_service($parameter,$security_code,$sign_type);

        //若改成GET方式传递
        $url = $alipay->create_url();
    }
    else {
        print($mysqli->error);
        
        print("error happened. ");
        
        exit(0);
    }
}
else {
    print("malicious guy");
    var_dump($amount);
    
    exit(0);
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.4.2/jquery.min.js"></script>
<title>新订单</title>
</head>
<body>
    <div id="content">
    <h1>新订单</h1>
    <p>流量：<span><?php print($amount); ?></span> GB</p>
    <p>总价(￥)：<span><?php print($total); ?></span></p>
    <a href="<?php print($url);?>"><img border="0" src="img/alipay.gif" /></a>
</body>