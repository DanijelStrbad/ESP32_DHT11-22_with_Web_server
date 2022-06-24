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
				$handle1 = fopen("admin.txt", "r");
				$passFile = fgets($handle1);
				fclose($handle1);
				
				if(strcmp(hash('sha3-512', $_GET["pass"]), $passFile) == 0 ) {
					echo ("<h1>Write your message</h1>");
					
					echo ("<form action=\"admin_mess_save.php\" method=\"GET\">");
					echo ("<input type=\"hidden\" id=\"pass\" name=\"pass\" value=\"".$_GET["pass"]."\">");
					echo ("<label for=\"mes1\">Message:</label><br>");
					echo ("<textarea class=\"textarea0\" id=\"mes1\" name=\"mes1\" rows=\"25\" cols=\"75\">");
					
					$file = fopen("text.txt","r");
					while(! feof($file)) {
						echo(fgets($file));
					}
					fclose($file);
					
					echo ("</textarea> <br><br>");
					echo ("<input type=\"submit\" value=\"Save\" /><br><br><br><br>");
					echo ("</form>");
				} else {
					echo ("<h1>Wrong password!</h1>");
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
