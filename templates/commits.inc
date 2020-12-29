R"(<!DOCTYPE html>
<head>
<title>{repo_name}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="/css/style.css">
</head>
<body>
<div id="content">
{header_content}
<table cellspacing="0" cellpadding="2" id="commits">
<thead>
<tr class="toptr">
<th style="text-align: left;">Date</th>
<th style="text-align: left;">Message</th>
<th style="text-align: left;">Author</th>
<th style="text-align: left;">Files</th>
<th style="text-align: left;">+</th>
<th style="text-align: left;">-</th>
<tr>
</thead>
<tbody>
{commits_content}
</tbody>
</table>
</div>
</body>
</html>)"
