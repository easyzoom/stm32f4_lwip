// JavaScript Document
function isIp(r, a) 
{
    var e, t, i = 0;
    if (r.length < 7 || 15 < r.length)
        return !1;
    for (var s = 0; s < r.length; s++) {
        if (e = r.charAt(s),
        -1 == ".0123456789".indexOf(e))
            return !1;
        if ("." == e) {
            if ("." == r.charAt(s + 1))
                return !1;
            i++
        }
    }
    if (3 != i)
        return !1;
    if (0 == r.indexOf(".") || r.lastIndexOf(".") == r.length - 1)
        return !1;
    for (szarray = [0, 0, 0, 0],
    s = 0; s < 3; s++) {
        i = r.indexOf(".");
        szarray[s] = r.substring(0, i),
        r = t = r.substring(i + 1)
    }
    for (szarray[3] = t,
    s = 0; s < 4; s++)
        if (szarray[s] < 0 || 255 < szarray[s])
            return !1;
    if (0 == a && (0 == szarray[0] || 223 < szarray[0] || 127 == szarray[0]))
        return !1;
    if (2 == a) {
        if (0 == szarray[0])
            return 0 == szarray[1] && 0 == szarray[2] && 0 == szarray[3];
        if (223 < szarray[0] || 127 == szarray[0])
            return !1
    }
    return !0
}
function ipVerify(r) {
    if (0 == r.length)
        return alert(js_ipVerify1 = "请输入IP地址！"),
        !1;
    if (!/^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$/.test(r))
        return alert(js_ipVerify2 = "IP地址无效，请重新输入！"),
        !1;
    var a, e, t = [0, 0, 0, 0];
    for (e = 0; e < 3; e++) {
        var i = r.indexOf(".");
        t[e] = r.substring(0, i),
        r = a = r.substring(i + 1)
    }
    for (t[3] = a,
    e = 0; e < 4; e++)
        if (t[e] < 0 || 255 < t[e])
            return alert(js_ipVerify3 = "IP地址无效，请重新输入！"),
            !1;
    return 127 == t[0] && 0 == t[1] && 0 == t[2] ? (alert(js_ipVerify4 = "IP地址无效（环回地址），请重新输入！"),
    !1) : !(224 <= t[0] && t[0] <= 239) || (alert(js_ipVerify5 = "IP地址无效（组播地址），请重新输入！"),
    !1)
}
function maskVerify(r) {
    if (0 == r.length)
        return alert(js_maskVerify1 = "请输入子网掩码（例如：255.255.255.0）！"),
        !1;
    if (!/^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$/.test(r))
        return alert(js_maskVerify2 = "子网掩码无效，请重新输入子网掩码（例如：255.255.255.0）！"),
        !1;
    var a, e, t, i = [0, 0, 0, 0];
    for (e = 0; e < 3; e++) {
        var s = r.indexOf(".");
        i[e] = r.substring(0, s),
        r = a = r.substring(s + 1)
    }
    for (i[3] = a,
    e = 0; e < 4; e++)
        if (i[e] < 0 || 255 < i[e])
            return alert(js_maskVerify2 = "子网掩码无效，请重新输入子网掩码（例如：255.255.255.0）！"),
            !1;
    var n = !1;
    if ((t = parseInt(parseInt(parseInt(i[0]) << 24) + parseInt(parseInt(i[1]) << 16) + parseInt(parseInt(i[2]) << 8) + parseInt(i[3]))) == parseInt(0))
        return alert(js_maskVerify2 = "子网掩码无效，请重新输入子网掩码（例如：255.255.255.0）！"),
        !1;
    if (t == parseInt(-1))
        return alert(js_maskVerify2 = "子网掩码无效，请重新输入子网掩码（例如：255.255.255.0）！"),
        !1;
    for (e = 0; e < 32; ++e) {
        if (parseInt(t & 1 << 31 - e) == parseInt(0))
            n = !0;
        else if (1 == n)
            return alert(js_maskVerify2 = "子网掩码无效，请重新输入子网掩码（例如：255.255.255.0）！"),
            !1
    }
    return !0
}
function macVerify(r) {
    if (0 == r.length)
        return alert(js_maskVerify1 = "请输入MAC地址（例如：70-B3-D5-0B-70-00）！"),
        !1;
    if (!/^([0-9a-fA-F]{2})(([/\s-][0-9a-fA-F]{2}){5})$/.test(r))
        return alert(js_maskVerify2 = "MAC地址无效，请重新输入（例如：70-B3-D5-0B-70-00）！"),
        !1;
    return !0
}
function loadValue() {
    $id("txt_addr").value = nf.ip;
    $id("txt_mask").value = nf.nm;
    $id("txt_gateway").value = nf.gw;
    $id("txt_mac").value = nf.mac;
}
function doSubmit() {
        if (0 == ipVerify($id("txt_addr").value))
            return $id("txt_addr").focus(),
            !1;
        if (0 == maskVerify($id("txt_mask").value))
            return $id("txt_mask").focus(),
            !1;
        var r = $id("txt_gateway").value;
        if ("" != r && 0 == isIp(r, 2))
            return alert(js_gateway_err = "默认网关非法，请重新输入。"),
            $id("txt_gateway").focus(),
            !1;
        if (0 == macVerify($id("txt_mac").value))
            return $id("txt_mac").focus(),
            !1;
        if (!confirm(js_ipParam = "确认提交？"))
            return !1
    return document.ip_setting.submit(),
    !0
}
function cf_dft()
{
    if (window.confirm("设备将恢复出厂设置。\n\n确认提交吗？")) 
    {
        return true
    }
    return false
}
function cf_rst()
{
    if (window.confirm("设备将重新启动。\n\n确认提交吗？")) 
    {
        return true
    }
    return false
}
