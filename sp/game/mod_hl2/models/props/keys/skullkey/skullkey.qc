// QC File generated by the Wall Worm Pro Version 5.0.61
// Exported From: C:\Users\Computor\Documents\3dsMax\scenes\skullkeys.max
// Get the latest version and notes at:
// http://dev.wallworm.com 

$scale 1
$modelname	"props/keys/skullkey/skullkey.mdl"
$body skullkey "skullkey.smd"
$collisionmodel "skullkey_hull.smd" {
	$automass

}
$cdmaterials "models/props/keys"
$texturegroup "skullkey_skins"
{

 {  "skullkey_sheetred" }
 {  "skullkey_sheetblue" }
 {  "skullkey_sheetpurple" }

}
$sequence idle "skullkey.smd"  activity ACT_IDLE -1 loop fps 30
$attachment "righteye" "skullkey" -2.15444 1.24516 0.488541 rotate 0 0 0
$attachment "lefteye" "skullkey" 1.7681 1.24314 0.489152 rotate 0 0 0
$keyvalues {
	"prop_data"
	{
		"base"		"Item.Small"

	}

}

$include	"skullkey_wwmt_custom.qci"
