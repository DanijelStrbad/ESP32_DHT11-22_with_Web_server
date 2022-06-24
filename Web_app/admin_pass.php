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
			<h1>Admin mode</h1>
		</header>
		
		<nav class="nav0">
			<p><a href="index.php">Read</a></p><br><br><br>
			<p><a href="admin.php">Admin</a></p>
		</nav>
		
		<article class="article1">
			
			<?php
				$handle = fopen("admin.txt", "r");
				$passFile = fgets($handle);
				fclose($handle);
				if( strcmp(hash('sha3-512', $_GET["pass"]), $passFile) == 0
							&& strcmp($_GET["pass1"], $_GET["pass2"]) == 0 ) {
					$handle1 = fopen("admin.txt", "w");
					fwrite($handle1, hash('sha3-512', $_GET["pass1"]));
					fclose($handle1);
					
					echo ("<h1>Admin password changed!</h1>");
				} else {
					echo ("<h1>Wrong input!</h1>");
				}
			?>
			
		</article>
		
		<footer class="footer0">
			<p>&copy; Web Log App</p>
			<p>This website is used for educational purpose only.</p>
			<p>Any abuse will be reported.</p>
		</footer>
		
	</div>
</body>
