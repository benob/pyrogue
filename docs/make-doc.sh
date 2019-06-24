(
cat <<EOF
<html>
	<head>
		<meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
		<link rel="stylesheet" href="https://highlightjs.org/static/demo/styles/default.css">
		<script
			charset="UTF-8"
			src="https://highlightjs.org/static/highlight.site.pack.js"></script>
		<script>hljs.initHighlightingOnLoad();</script>
		<style>
		code {
			color: dimgray;
		}
		h3 code {
			color: black;
		}
		</style>
	</head>
	<body>
EOF
cmark-gfm --unsafe documentation.md | sed 's/\t/    /g'
cat <<EOF
	</body>
</html>
EOF
) > documentation.html
