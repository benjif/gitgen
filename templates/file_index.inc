R"(<!DOCTYPE html>
<head>
<title>{repo_name}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="/css/style.css">
</head>
<body>
<div id="content">
{header_content}
<div id="treepath">{tree_path}</div>
<table cellspacing="0" cellpadding="2" id="repotree">
<thead>
<tr class="toptr">
<th style="text-align: left;">Filename</th>
<th style="text-align: left;">Size</th>
<th style="text-align: left;"><!-- Unit --></th>
<tr>
</thead>
<tbody>
{tree_content}
</tbody>
</table>
<div id="readmeview">
{readme_content}
</div>
</div>
</body>
</html>)"
