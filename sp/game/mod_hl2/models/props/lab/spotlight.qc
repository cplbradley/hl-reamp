// QC File generated by the Wall Worm Pro Version 5.9.6
// Exported From: Unsaved File
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1
$modelname	"props/lab/spotlight.mdl"
$staticprop
$body spotlight "spotlight.smd"
$collisionmodel "spotlight_hull.smd" {
	$automass

}
$surfaceprop "metal" 
$cdmaterials "models/props/lab"
$sequence idle "spotlight.smd" activity ACT_IDLE -1 fps 1

$include	"spotlight_wwmt_custom.qci"
