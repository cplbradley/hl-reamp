// QC File generated by the Wall Worm Pro Version 5.9.6
// Exported From: C:\Users\Computor Two\Documents\3dsMax\scenes\windmillroof.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.0
$modelname	"props/windmill/windmill_roof.mdl"
$staticprop
$body windmill_roof "windmill_roof.smd"
$collisionmodel "windmill_roof_hull.smd" {

}
$surfaceprop "Wood" 
$contents "solid"
$cdmaterials "models/props/windmill"
$sequence idle "windmill_roof.smd" activity ACT_IDLE -1 fps 1

$include	"windmill_roof_wwmt_custom.qci"
