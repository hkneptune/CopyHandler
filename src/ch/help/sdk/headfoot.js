function header(text)
{
	document.write("<table width=\"100%\">");
	document.write("<tr><td>");
	document.write("<img src=\"ch.gif\" alt=\"CH Img\" border=\"0\" align=\"left\">");
	document.write("<div class=\"headertext\">Copy Handler Software Development Kit Documentation</div>");
	document.write("<div class=\"doctitle\">");
	document.write( text );
	document.write("</div>");
	document.write("</td></tr>");
	document.write("<tr><td><hr></td></tr>");
	document.write("<tr><td>");
}

function footer()
{
	document.write("</td></tr>");
	document.write("<tr><td>");
	document.write("<hr>");
	document.write("<div class=\"footer\">Copyright © 2004 Ixen Gerthannes.</div>");
	document.write("<hr>");
	document.write("</td></tr>");
	document.write("</table>");
}