R"(<!DOCTYPE html>
<head>
<title>{reponame}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="/css/style.css">
</head>
<body>
<div id="content">
{headercontent}
<b>Commit:</b><br> <div id="commit"><a href="/{reponame}/commits/{commit}.html">{commit}</a></div><br>
<b>Parent:</b><br> <div id="commitparent"><a href="/{reponame}/commits/{parent}.html">{parent}</a></div><br>
<b>Date:</b> <div id="commitdate">{date}</div><br>
<b>Author: </b><div id="commitauthor">{author} &lt;<a href="mailto:{email}">{email}</a>&gt;</div><br>
<div id="commitmessage">{message}</div>
</div>
</body>
</html>)"
