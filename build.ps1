$path = Get-Location
$args = @()
$items = Get-ChildItem -path $my_path -Recurse -Filter *.cpp 
foreach ($file in $items){
	$args += ($file).FullName;
}
$args += "-std=c++14"
$args += "-Ofast"
$exepath = "C:/MinGW/bin/g++.exe"
& $exepath $args
