<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Web Log</title>
	<link rel="stylesheet" href="design.css">
	
</head>

<body>
	<div class="wrapper">
		
		<header class="header0">
			<h1>Web Log</h1>
			<h1>Read mode</h1>
		</header>
		
		<nav class="nav0">
			<p><a href="index.php">Read</a></p><br><br><br>
			<p><a href="admin.php">Admin</a></p>
		</nav>
		
		<!-- <article class="article0"> -->
		<article class="article1">
		
			<h1>Read previous logs</h1>
			<p>
			<textarea readonly rows="25" cols="75"><?php
				$file = fopen("text.txt","r");
				while(! feof($file)) {
					/* echo(fgets($file). "<br>"); */
					echo(fgets($file));
				}
				fclose($file);
			?></textarea>
			</p>
			
		</article>
		
		
		<footer class="footer0">
			<p>&copy; Web Log App</p>
			<p>This website is used for educational purpose only.</p>
			<p>Any abuse will be reported.</p>
		</footer>
		
	</div>
</body>
