// JavaScript Document
function ShowTips(b, a) 
{
    $id(b).innerHTML = '<span class="TIP_CONTENT">' + a + "</span>"
}
function scrollDown(a) 
{
    var c = $id(a);
    var b = parseInt(c.style.top);
    if (b >= 2) {
        clearInterval(c.interval);
        return
    } else {
        b += 1;
        c.style.top = b + "px"
    }
}
function scrollUp(a) 
{
    var c = $id(a);
    var b = parseInt(c.style.top);
    if (b <= -24) {
        clearInterval(c.interval);
        return
    } else {
        b -= 1;
        c.style.top = b + "px"
    }
}
function startDownScroll(a) 
{
    var b = $id(a);
    b.style.top = "-22px";
    clearInterval(b.interval);
    b.interval = setInterval('scrollDown("' + a + '")', 30);
    setTimeout('startUpScroll("' + a + '")', 5000)
}
function startUpScroll(a) 
{
    var b = $id(a);
    clearInterval(b.interval);
    b.interval = setInterval('scrollUp("' + a + '")', 30)
}
function pbmToStr(f) {
    var e = "";
    var c = parseInt(f);
    var b = 0;
    var a = 1;
    for (var d = 1; d <= 32; ++d) {
        if (c & a) {
            e += d;
            b = d;
            do {
                a <<= 1;
                a >>>= 0
            } while ((c & a) && ++d);
            if (b != d) {
                e += "-" + d
            }
            e += ",";
            ++d
        }
        a <<= 1;
        a >>>= 0
    }
    if (e.charAt(e.length - 1) == ",") {
        e = e.substr(0, e.length - 1)
    }
    return e
}
function setColSpan(a,b,c)
{
    var x=document.getElementById(a).rows[b].cells;
    x[0].colSpan=""+c+1;
}
function dosubmitRefresh(u) 
{
    document.location.href = u
}
function sleep(milliSeconds) {
        var startTime = new Date().getTime();
        while (new Date().getTime() < startTime + milliSeconds) {
            console.log(new Date().getTime());
        }//暂停一段时间 1000=1S。
}
;