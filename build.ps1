$path = Get-Location
$args = @()
$items = Get-ChildItem -path $my_path -Recurse -Filter *.cpp 
foreach ($file in $items){
	$args += ($file).FullName;
}
$args += "-Ofast"
$args += "-g"
$exepath = "C:/MinGW/bin/g++.exe"
& $exepath $args