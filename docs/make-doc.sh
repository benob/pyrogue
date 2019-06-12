(
cat <<EOF
<html>
	<head>
		<link rel="stylesheet" href="https://highlightjs.org/static/demo/styles/default.css">
		<script
			charset="UTF-8"
			src="https://highlightjs.org/static/highlight.site.pack.js"></script>
		<script>hljs.initHighlightingOnLoad();</script>
	</head>
	<body>
EOF
cmark-gfm --unsafe documentation.md
cat <<EOF
	</body>
</html>
EOF
) > documentation.html
