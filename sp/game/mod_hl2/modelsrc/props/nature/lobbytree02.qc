// QC File generated by the Wall Worm Pro Version 6.1.5
// Exported From: C:\Users\Computor Two\Documents\3dsMax\scenes\trees.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.0
$modelname	"props/nature/lobbytree02.mdl"
$staticprop
$body lobbytree02 "lobbytree02.smd"
$collisionmodel "lobbytree02_hull.smd" {
	$concave

}
$surfaceprop "Wood" 
$contents "solid"
$cdmaterials "models/props/nature/trees"
$sequence idle "lobbytree02.smd" activity ACT_IDLE -1 fps 1

$include	"lobbytree02_wwmt_custom.qci"
