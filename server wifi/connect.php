<?php

	function Connection(){
		$server="mysql.hostinger.it";
		$user="u278094231_ardu";
		$pass="arduino16";
		$db="u278094231_ardu";

		$connection = mysql_connect($server, $user, $pass);

		if (!$connection) {
	    	die('MySQL ERROR: ' . mysql_error());
		}

		mysql_select_db($db) or die( 'MySQL ERROR: '. mysql_error() );

		return $connection;
	}
?>
