// JavaScript Document
function docW(a) 
{
    document.write(a)
}
function incCss(a) {
    docW("<link rel=stylesheet type='text/css' href='" + a + "'/>")
}
function incJs(a) {
    docW("<script type='text/javascript' src='" + a + "'><\/script>")
}
function incMeta() {
    docW('<meta http-equiv="x-ua-compatible" content="edge" />');
}
function $id(a) {
    return document.getElementById(a)
}
function $name(a) {
    return document.getElementsByTagName(a)
};