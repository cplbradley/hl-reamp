// QC File generated by the Wall Worm Pro Version 5.10.1
// Exported From: C:\Users\Computor Two\Documents\3dsMax\scenes\robodude_gibs.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.2
$modelname	"gibs/robot/robothigh_r.mdl"
$staticprop
$body robothigh_r "robothigh_r.smd"
$collisionmodel "robothigh_r_hull.smd" {
	$automass

}
$surfaceprop "metal" 
$contents "solid"
$cdmaterials "models/robodude"
$texturegroup "robothigh_r_skins"
{

 {  "robodude" }
 {  "robodude_black" }
 {  "robodude_white" }

}
$sequence idle "robothigh_r.smd" activity ACT_IDLE -1 fps 1
$keyvalues {
	"prop_data"
	{
		"base"		"Metal.Large"
		"health"		"0"
		"physicsmode"		"0"
		"dmg.bullets"		"0.0"
		"dmg.club"		"0.0"
		"dmg.explosive"		"0.0"
		"explosive_radius"		"0"
		"explosive_damage"		"0.0"

	}

}

$include	"robothigh_r_wwmt_custom.qci"
