// QC File generated by the Wall Worm Pro Version 5.1.27
// Exported From: F:\Users\Computor\Documents\3dsMax\w_rpg.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.0
$modelname	"weapons/w_rocketlauncher.mdl"
$body w_rocketlauncher "w_rocketlauncher.smd"
$collisionmodel "w_rocketlauncher_hull.smd" {
	$concave
	$masscenter -0.0418952 -4.89558 9.23917 
	$automass

}
$contents "solid"
$cdmaterials "models/weapons/rocketlauncher"
$sequence idle "w_rocketlauncher.smd" activity ACT_IDLE -1 fps 1

$include	"w_rocketlauncher_wwmt_custom.qci"
