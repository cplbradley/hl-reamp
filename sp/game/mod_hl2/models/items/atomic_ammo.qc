// QC File generated by the Wall Worm Pro Version 5.1.21
// Exported From: Unsaved File
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1
$modelname	"items/atomic_ammo.mdl"
$body atomic_ammo "atomic_ammo.smd"
$collisionjoints "atomic_ammo_hull.smd" {
	$automass

}
$surfaceprop "metal" 
$cdmaterials "models/items"
$sequence idle "atomic_ammo_sequence_idle" fps 30.0 loop activity ACT_IDLE -1.0 autoplay
$keyvalues {
	"prop_data"
	{
		"base"		"Metal.Small"

	}

}

$include	"atomic_ammo_wwmt_custom.qci"
