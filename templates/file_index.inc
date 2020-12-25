R"(<!DOCTYPE html>
<head>
<title>{reponame}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="{csspath}">
</head>
<body>
<div id="content">
{headercontent}
<table cellspacing="0" cellpadding="2" id="repotree">
<thead>
<tr class="toptr">
<th style="text-align: left;">Filename</th>
<th style="text-align: left;">Size</th>
<tr>
</thead>
<tbody>
{treecontent}
</tbody>
</table>
</div>
</body>
</html>)"
