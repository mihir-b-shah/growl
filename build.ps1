$path = Get-Location
$args = @()
$items = Get-ChildItem -path $my_path -Recurse -Filter *.cpp 
foreach ($file in $items){
	$args += ($file).FullName;
}
$args += "-Ofast"
$args += "-Iinclude"
$exepath = "C:/MinGW/bin/g++.exe"
& $exepath $args

