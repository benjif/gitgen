R"(<!DOCTYPE html>
<head>
<title>{repo_name}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="/css/style.css">
</head>
<body>
<div id="content">
{header_content}
<b>Commit:</b><br>
<div id="commit"><a href="/{repo_name}/commits/{commit}.html">{commit}</a></div><br>
<b>Parent:</b><br>
<div id="commitparent"><a href="/{repo_name}/commits/{parent}.html">{parent}</a></div><br>
<b>Date:</b>
<div id="commitdate">{date}</div><br>
<b>Author: </b>
<div id="commitauthor">{author} &lt;<a href="mailto:{email}">{email}</a>&gt;</div><br>
<b>Message: </b>
<div id="commitmessage">{message}</div><br>
<!--<br>-->
<b>Diff: </b>
<div id="diff">
{diff_content}
</div>
</div>
</body>
</html>)"
