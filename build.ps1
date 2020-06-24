$path = Get-Location
$myargs = @()
$items = Get-ChildItem -path $my_path -Recurse -Filter *.cpp 
foreach ($file in $items){
	$myargs += ($file).FullName;
}
if($args[0] -eq "debug"){
    $myargs += "-g"
}
$myargs += "-Ofast"
$myargs += "-Iinclude"
$exepath = "C:/MinGW/bin/g++.exe"
& $exepath $myargs