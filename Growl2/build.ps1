$path = Get-Location
$args = @()
$items = Get-ChildItem -path $my_path -Filter *.cpp 
foreach ($file in $items){
	$args += ($file.BaseName + $file.Extension)
}
$args += "-Ofast"
$exepath = "C:/MinGW/bin/g++.exe"
& $exepath $args

