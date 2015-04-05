<?

// Written by etienne Laurin
// simple (and broken) php file browser

$base = "/mods/files.php"; //set relative or absolute location of this file
if($user=="lol"&&$password=="rotfl"){
	if(isset($dir)){
		if(substr($dir,strlen($dir)-1,1)!="/"){$dir=$dir."/";}
		if ($handle = opendir($dir)) {
			echo "<h1>$dir</h1>\n";
			echo "<form ENCTYPE=\"multipart/form-data\" method=\"POST\"";
			echo "action=\"$base?user=$user&password=$password&upload=$dir\">";
			echo "File to upload:<INPUT TYPE=\"FILE\" NAME=\"userfile\" SIZE=\"35\">";
			echo "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"1000000\">";
			echo "<input type=\"submit\" value=\"Upload it\"><br><br><Br>";

			while (false !== ($file = readdir($handle))) {
				$lfile = $dir.$file;
				if(is_dir($lfile)&&$file!="."){
					echo "<a href=\"$base?user=$user&password=$password&dir=$lfile\">&lt;$file&gt;</a>\n<br>";
				}elseif($file!="."){
					echo "$file - <a href=\"$base?user=$user&password=$password&file=$lfile\">view</a><bR>";
				}
			}
			closedir($handle);
		}
	}elseif(isset($file)){
		if(is_file($file)){
			echo "<h1>$file</h1><pre>";
			readfile($file);
			echo "</pre>";
		}else{
			echo "file $file doesnt exist";
		}
	}elseif(isset($upload)){
		if (is_uploaded_file($_FILES['userfile']['tmp_name'])) {
			$filename = $_FILES['userfile']['tmp_name'];
			$realname = $_FILES['userfile']['name'];
			copy($_FILES['userfile']['tmp_name'],$upload.$realname);
			echo "successfully uploaded $dir/$realname<br>";
		} else {
			echo "Error uploading file";
		}
	}
}else{
?>
<form type=get>
username:<input type=input name=user><br>
password:<input type=input name=password><br>
folder:<input type=input name=dir><br>
<input type=submit>
</form>
<?
}
?>

