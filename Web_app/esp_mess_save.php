<?php
	date_default_timezone_set("CET");
	$handle1 = fopen("admin.txt", "r");
	$passFile = fgets($handle1);
	fclose($handle1);
	
	if(strcmp(hash('sha3-512', $_GET["pass"]), $passFile) == 0 ) {
		$handle = fopen("text.txt", "a");
		fwrite($handle, date("Y-m-d H:i:s")." ".$_GET["esp_mess"].PHP_EOL);
		fclose($handle);
		echo ("1\n\n");
	} else {
		echo ("0\n\n");
	}
?>
