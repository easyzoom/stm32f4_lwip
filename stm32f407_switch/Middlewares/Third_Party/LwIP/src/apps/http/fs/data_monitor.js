// JavaScript Document
function loadXMLDoc(url,cfunc)
{
  if (window.XMLHttpRequest)
  {
     xmlhttp=new XMLHttpRequest();
  }
  else
  {
     xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
  xmlhttp.onreadystatechange=cfunc;
  xmlhttp.open("GET",url,true);
  xmlhttp.send();
}

function update()
{
    loadXMLDoc("/ajax_minitor.cgi",analysis);
}
function analysis()
{
    var pge,pge_pmax,pge_sws,pge_lks,pge_lkset,pge_txs,pge_rxs,pge_ibad,str;
    var temp;
    if (xmlhttp.readyState==4 && xmlhttp.status==200)
    {
        pge = xmlhttp.responseText;
        pge = pge.split(";");

        str = parseInt(pge[1]);
        pge_pmax = parseInt(str);
        pge_sws = pge[2].split(",");
        pge_lks = pge[3].split(",");
        pge_lkset = pge[4].split(",");
        pge_txs = pge[5].split(",");
        pge_rxs = pge[6].split(",");
        pge_ibad = pge[7].split(",");
        for(var index = 0; index < pge_pmax; index++)
        {
            $id("pstate"+index).innerHTML=s_nf[pge_sws[index]];
            $id("plink_set"+index).innerHTML=lk_nf[pge_lkset[index]];
            $id("plink_status"+index).innerHTML=lk_nf[pge_lks[index]];
            if(parseInt(pge_txs[index*2+1]) == 2)
            {
                $id("ptxs"+index).innerHTML=pge_txs[index*2]+"Mbps";
            }
            else if(parseInt(pge_txs[index*2+1]) == 1)
            {
                $id("ptxs"+index).innerHTML=pge_txs[index*2]+"Kbps";
            }
            else
            {
                $id("ptxs"+index).innerHTML=pge_txs[index*2]+"bps";
            }

            if(parseInt(pge_rxs[index*2+1]) == 2)
            {
                $id("prxs"+index).innerHTML=pge_rxs[index*2]+"Mbps";
            }
            else if(parseInt(pge_rxs[index*2+1]) == 1)
            {
                $id("prxs"+index).innerHTML=pge_rxs[index*2]+"Kbps";
            }
            else
            {
                $id("prxs"+index).innerHTML=pge_rxs[index*2]+"bps";
            }
            
            if(parseInt(pge_ibad[index*2+1]) == 2)
            {
                $id("prx_bad"+index).innerHTML=pge_ibad[index*2]+"MByte";
            }
            else if(parseInt(pge_ibad[index*2+1]) == 1)
            {
                $id("prx_bad"+index).innerHTML=pge_ibad[index*2]+"KByte";
            }
            else
            {
                $id("prx_bad"+index).innerHTML=pge_ibad[index*2]+"Byte";
            }
            
        }
    }
}
function init()
{
 setInterval(update,500);  
}