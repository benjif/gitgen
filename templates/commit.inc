R"(<!DOCTYPE html>
<head>
<title>{reponame}</title>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="/css/style.css">
</head>
<body>
<div id="content">
{headercontent}
<div id="commitmessage">{message}</div>
<span id="commitdate">{date}</span> - <span id="commitauthor">{author} &lt;<a href="mailto:{email}">{email}</a>&gt;</span>
</div>
</body>
</html>)"
