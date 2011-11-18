<?php
/*
 *功能：设置商品有关信息（入口页）
 *详细：该页面是接口入口页面，生成支付时的URL
 *版本：3.0
 *修改日期：2010-05-24
 '说明：
 '以下代码只是为了方便商户测试而提供的样例代码，商户可以根据自己网站的需要，按照技术文档编写,并非一定要使用该代码。
 '该代码仅供学习和研究支付宝接口使用，只是提供一个参考。

*/

////////////////////注意/////////////////////////
//该页面测试时出现“调试错误”请参考：http://club.alipay.com/read-htm-tid-8681712.html
//要传递的参数要么不允许为空，要么就不要出现在数组与隐藏控件或URL链接里。
/////////////////////////////////////////////////

require_once("alipay_config.php");
require_once("class/alipay_service.php");

/*以下参数是需要通过下单时的订单数据传入进来获得*/
//必填参数
$out_trade_no = date(Ymdhms);	//请与贵网站订单系统中的唯一订单号匹配
$subject      = "订单名称";		//订单名称，显示在支付宝收银台里的“商品名称”里，显示在支付宝的交易管理的“商品名称”的列表里。
$body         = "订单描述备注";	//订单描述、订单详细、订单备注，显示在支付宝收银台里的“商品描述”里
$total_fee    = "0.01";			//订单总金额，显示在支付宝收银台里的“应付总额”里

//扩展功能参数——网银提前
$paymethod    = "bankPay";		//默认支付方式，四个值可选：bankPay(网银); cartoon(卡通); directPay(余额); CASH(网点支付)
$defaultbank  = "CMB";			//默认网银代号，代号列表见http://club.alipay.com/read.php?tid=8681379

//扩展功能参数——防钓鱼
$encrypt_key  = '';				//默认关闭，用于防止模拟提交、防钓鱼等功能
if($antiphishing == 1){
    $encrypt_key = query_timestamp($partner);
}

//扩展功能参数——其他
$extra_common_param = "会员号：2010001";  //自定义参数，可存放任何内容（除=、&等特殊字符外），不会显示在页面上

/////////////////////////////////////////////////

//构造要请求的参数数组
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
        "total_fee"       => $total_fee,

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
        //$it_b_pay	      => "1c",	  //超时时间，不填默认是15天。八个值可选：1h(1小时),2h(2小时),3h(3小时),1d(1天),3d(3天),7d(7天),15d(15天),1c(当天)

		//扩展功能参数——自定义参数
        "extra_common_param" => $extra_common_param
);

//构造请求函数
$alipay = new alipay_service($parameter,$security_code,$sign_type);

//若改成GET方式传递
$url = $alipay->create_url();
$sHtmlText = "<a href=".$url."><img border='0' src='img/alipay.gif' /></a>";

//POST方式传递，得到加密结果字符串，请取消下面一行的注释
//$sHtmlText = $alipay->build_postform();

?>
<html>
    <head>
        <title>支付宝即时支付</title>
        <style type="text/css">
            .font_content{
                font-family:"宋体";
                font-size:14px;
                color:#FF6600;
            }
            .font_title{
                font-family:"宋体";
                font-size:16px;
                color:#FF0000;
                font-weight:bold;
            }
            table{
                border: 1px solid #CCCCCC;
            }
        </style>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"></head>
    <body>

        <table align="center" width="350" cellpadding="5" cellspacing="0">
            <tr>
                <td align="center" class="font_title" colspan="2">订单确认</td>
            </tr>
            <tr>
                <td class="font_content" align="right">订单号：</td>
                <td class="font_content" align="left"><?php echo $out_trade_no; ?></td>
            </tr>
            <tr>
                <td class="font_content" align="right">付款总金额：</td>
                <td class="font_content" align="left"><?php echo $total_fee; ?></td>
            </tr>
            <tr>
                <td align="center" colspan="2"><?php echo $sHtmlText; ?></td>
            </tr>
        </table>
    </body>
</html>
