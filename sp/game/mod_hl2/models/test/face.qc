// QC File generated by the Wall Worm Pro Version 5.0.85
// Exported From: C:\Users\Computor\Documents\3dsMax\scenes\face.max
// Get the latest version and notes at:
// http://dev.wallworm.com 

$scale 1.0
$modelname	"test/face.mdl"
$body face "face.smd"
$contents "solid"
$cdmaterials "models/test"
$sequence break "face_sequence_break" fps 30.0 fadein 0.0 fadeout 0.0  {
	{ event AE_CL_PLAYSOUND 0 "MetalVehicle.ScrapeRough" }
	{ event AE_CL_PLAYSOUND 4 "Metal_Barrel.ImpactHard" }
	{ event AE_CL_PLAYSOUND 46 "Canals.d1_canals_01a_metal_large_debris2" }

}
$sequence idle_broke "face_sequence_idle_broke" fps 30.0 loop  fadein 0.0 fadeout 0.0 
$sequence idle "face_sequence_idle" fps 30.0 loop  fadein 0.0 fadeout 0.0 

$include	"face_wwmt_custom.qci"
