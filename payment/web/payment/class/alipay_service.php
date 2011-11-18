<?php
/**
 *类名：alipay_service
 *功能：支付宝外部服务接口控制
 *详细：该页面是请求参数核心处理文件，不需要修改
 *版本：3.0
 *修改日期：2010-05-24
 '说明：
 '以下代码只是为了方便商户测试而提供的样例代码，商户可以根据自己网站的需要，按照技术文档编写,并非一定要使用该代码。
 '该代码仅供学习和研究支付宝接口使用，只是提供一个参考。

*/

require_once("alipay_function.php");

class alipay_service {

    var $gateway;			//网关地址
    var $security_code;		//安全校验码
    var $mysign;			//加密结果（签名结果）
    var $sign_type;			//加密类型
    var $parameter;			//需要加密的参数数组
    var $_input_charset;    //字符编码格式

    /**构造函数
	*从配置文件及入口文件中初始化变量
	*$parameter 需要加密的参数数组
	*$security_code 安全校验码
	*$sign_type 加密类型
    */
    function alipay_service($parameter,$security_code,$sign_type) {
        $this->gateway	      = "https://www.alipay.com/cooperate/gateway.do?";
        $this->security_code  = $security_code;
        $this->sign_type      = $sign_type;
        $this->parameter      = para_filter($parameter);

        //设定_input_charset的值,为空值的情况下默认为GBK
        if($parameter['_input_charset'] == '')
            $this->parameter['_input_charset'] = 'GBK';

        $this->_input_charset   = $this->parameter['_input_charset'];

        //获得签名结果
        $sort_array   = arg_sort($this->parameter);    //得到从字母a到z排序后的加密参数数组
        $this->mysign = build_mysign($sort_array,$this->security_code,$this->sign_type);
    }

    /********************************************************************************/

    /**构造请求URL（GET方式请求）
	*return 请求url
     */
    function create_url() {
        $url         = $this->gateway;
        $sort_array  = array();
        $sort_array  = arg_sort($this->parameter);
        $arg         = create_linkstring($sort_array);	//把数组所有元素，按照“参数=参数值”的模式用“&”字符拼接成字符串
        
		//把网关地址、已经拼接好的参数数组字符串、签名结果、签名类型，拼接成最终完整请求url
        $url.= $arg."&sign=" .$this->mysign ."&sign_type=".$this->sign_type;
        return $url;
    }

    /********************************************************************************/

    /**构造Post表单提交HTML（POST方式请求）
	*return 表单提交HTML文本
     */
    function build_postform() {

        $sHtml = "<form id='alipaysubmit' name='alipaysubmit' action='".$this->gateway."_input_charset=".$this->parameter['_input_charset']."' method='post'>";

        while (list ($key, $val) = each ($this->parameter)) {
            $sHtml.= "<input type='hidden' name='".$key."' value='".$val."'/>";
        }

        $sHtml = $sHtml."<input type='hidden' name='sign' value='".$this->mysign."'/>";
        $sHtml = $sHtml."<input type='hidden' name='sign_type' value='".$this->sign_type."'/></form>";

        $sHtml = $sHtml."<input type='button' name='v_action' value='支付宝确认付款' onClick='document.forms[\"alipaysubmit\"].submit();'>";
        return $sHtml;
    }
    /********************************************************************************/

}
?>