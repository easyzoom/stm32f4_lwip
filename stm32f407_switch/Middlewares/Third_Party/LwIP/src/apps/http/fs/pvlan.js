// JavaScript Document
function pvlanEn() 
{
    if (pvState) 
    {
        if ($id("pvl_e").checked == true) 
        {
            return true
        } 
        else 
        {
            if (confirm("端口VLAN会被禁止。\n\n确认提交吗？")) 
            {
                return true
            }
        }
    } 
    else 
    {
        if ($id("pvl_e").checked == true)
        {
            if (confirm("端口VLAN将会被使能。\n\n确认提交吗？")) 
            {
                return true
            }
        } 
        else 
        {
            return true
        }
    }
    return false
}
function pvlanStateInit() 
{
    if (pvState) {
        $id("pvl_e").checked = true
    } else {
        $id("pvl_d").checked = true;
        t_num = 0
    }
}
function selMbrPortsByPbm(b) {
    for (var c = 1, a = 1; c <= pmax; c++) {
        $id("port_" + c).checked = (a & b) ? true : false;
        a <<= 1
    }
}
function selMbrPorts() {
    var b = $id("SL_PORT_LIST").value - "0";
    $id("psl_id").value = $id("SL_PORT_LIST").value;
    if (1 <= b && b <= pmax) {
        for (var a = 0; a < pvIds.length; a++) {
            if (b == pvIds[a]) {
                selMbrPortsByPbm(pvMbrs[a]);
                return
            }
        }
    }
    for (var a = 1; a <= pmax; a++) {
        $id("port_" + a).checked = false
    }
}
function chkSelCnt() {
    var c = 0;
    var a = 0;
    for (var b = 1; b <= pmax; b++) {
        if ($id("port_" + b).checked == true) {
            a |= (1 << (b - 1));
            c++
        }
    }
    if (c == 0) {
        return -1
    } else {
        return 0
    }
}
function pvlAdd() {
    var a = 0;
    if (pvState) {
        a = chkSelCnt();
        if (0 != a) {
            alert("请至少选择一个端口。")
        } else {
            return true
        }
    } else {
        alert("设置端口组之前应该先使能端口VLAN")
    }
    return false
}
function VlanShowSet() {
    var b;
    if (pvState) {
        $id("vlan_1").disabled = true
        for (b = 1; b <= pmax; b++) {
        $id("port_" + b).disabled = false
        }
    }
    else
    {
        for (b = 1; b <= pmax; b++) {
        $id("port_" + b).disabled = true
        }
    }
}
function selAllVlan() {
    var c = 0;
    var b;
    var a;
    for (b = 1; b < t_num; b++) {
        if ($id("vlan_" + pvIds[b]).checked == true) {
            c++;
            break
        }
    }
    a = (c) ? false : true;
    for (b = 1; b < t_num; b++) {
        $id("vlan_" + pvIds[b]).checked = a
    }
    t_num && ($id("vlan_1").checked = false);
    return true
}
function pvlanDel() {
    var b = 0;
    for (var a = 1; a < t_num; a++) {
        if ($id("vlan_" + pvIds[a]).checked == true) {
            b++;
            break
        }
    }
    if (b) {
        return true
    } else {
        return false
    }
}
