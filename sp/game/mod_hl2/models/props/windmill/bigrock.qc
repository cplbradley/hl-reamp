// QC File generated by the Wall Worm Pro Version 5.4.1
// Exported From: C:\Users\Computor Two\Documents\3dsMax\scenes\bigrock.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.5
$modelname	"props/windmill/bigrock.mdl"
$staticprop
$body bigrock "bigrock.smd"
$collisionmodel "bigrock_hull.smd" {
	$concave
	$maxconvexpieces 44

}
$surfaceprop "rock" 
$contents "solid"
$cdmaterials "models/props/windmill"
$sequence idle "bigrock.smd" activity ACT_IDLE -1 fps 1

$include	"bigrock_wwmt_custom.qci"
