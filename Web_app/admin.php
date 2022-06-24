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
			
			<form action="admin_pass.php" method="GET">
				<h1>Change admin password</h1>
				<label for="pass">Old password (admin):</label> <br>
				<input type="password" id="pass" name="pass" minlength="4"> <br><br>
				<label for="pass1">New password (admin):</label> <br>
				<input type="password" id="pass1" name="pass1" minlength="4"> <br>
				<label for="pass2">Repeat password (admin):</label> <br>
				<input type="password" id="pass2" name="pass2" minlength="4"> <br><br>
				
				<input type="submit" value="Change" /><br><br><br><br>
			</form>
			
			
			<form action="admin_mess.php" method="GET">
				<h1>Write your message</h1>
				<label for="pass">Admin password:</label> <br>
				<input type="password" id="pass" name="pass" minlength="4"> <br><br>
				
				<input type="submit" value="Load" /><br><br><br><br>
			</form>
			
			<form action="admin_ip.php" method="GET">
				<h1>Other</h1>
				<label for="pass">Admin password:</label> <br>
				<input type="password" id="pass" name="pass" minlength="4"> <br><br>
				
				<input type="submit" value="Load" /><br><br><br><br>
			</form>
			
		</article>
		
		<footer class="footer0">
			<p>&copy; Web Log App</p>
			<p>This website is used for educational purpose only.</p>
			<p>Any abuse will be reported.</p>
		</footer>
		
	</div>
</body>
