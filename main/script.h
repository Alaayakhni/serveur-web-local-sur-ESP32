
#ifndef MAIN_SCRIPT_H_
#define MAIN_SCRIPT_H_

const char refresh[] = "<script>"
"setInterval(function() {"
  "getData();"
"}, 1000);"
"function getData() {"
  "var xtemp = new XMLHttpRequest();"
  "xtemp.onreadystatechange = function() {"
    "if (this.readyState == 4 && this.status == 200) {"
      "document.getElementById(\"temp_value\").innerHTML ="
      "this.responseText;"
    "}"
  "};"
  "xtemp.open(\"GET\", \"temp\", true);"
  "xtemp.send();"

"}"
"</script>";

const char style[] =
	    "<style>"
		".TempCard{"
			"width: 50%;"
			"max-width: 650px;"
		    "background: #19A4AE;"
			"padding: 30px;"
			"color: #FFFFFF;"
			"margin:20px;"
			"box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);"
			"display: table-cell;"
			"text-align: center;"
			"font-size: 50px;"
			"font-family: fantasy;"

		"}"


		"@media (max-width: 800px) {"
			".TempCard{"
				"width: 85%;"
				"display: inherit;"
			"}"

		"}"
		"</style>";
#endif /* MAIN_SCRIPT_H_ */
