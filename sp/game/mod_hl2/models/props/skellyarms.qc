// QC File generated by the Wall Worm Pro Version 5.0.93
// Exported From: C:\Users\Computor\Documents\3dsMax\scenes\skellyarms01.max
// Get the latest version and notes at:
// http://dev.wallworm.com 

$scale 1.0
$modelname	"props/skellyarms.mdl"
$body skellyarms "skellyarms.smd"
$collisionjoints "skellyarms_hull.smd" {
	$concaveperjoint

}
$surfaceprop "concrete" 
$contents "solid"
$cdmaterials "models/props"
$sequence idle "skellyarms.smd"  activity ACT_IDLE -1 loop fps 30
$sequence sink "skellyarms_sequence_sink" fps 5.0 fadein 0.2 fadeout 0.2 

$include	"skellyarms_wwmt_custom.qci"
