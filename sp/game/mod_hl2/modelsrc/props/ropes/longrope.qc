// QC File generated by the Wall Worm Pro Version 5.0.93
// Exported From: C:\Users\Computor\Documents\3dsMax\autoback\tongs_recover.max
// Get the latest version and notes at:
// http://dev.wallworm.com 

$scale 1
$modelname	"props/ropes/longrope.mdl"
$body longrope "longrope.smd"
$cdmaterials "models/props/ropes"
$sequence idle "longrope.smd"  activity ACT_IDLE -1 loop fps 30
$sequence grab "longrope_sequence_grab" fps 30.0 fadein 0.2 fadeout 0.2 

$include	"longrope_wwmt_custom.qci"
