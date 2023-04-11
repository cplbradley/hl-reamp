// Created by Crowbar 0.72

$modelname "weapons/V_hands.mdl"

$bodygroup "studio"
{
	studio "v_hands_mark6.smd"
}


$surfaceprop "default"

$contents "solid"

$cdmaterials "models\Weapons\V_hands\"

$sequence "admire" {
	"handpunch.smd"
	activity "ACT_VM_IDLE" 1 { event AE_CL_PLAYSOUND 0 "Hands.Inspect" }
	fadein 0.2
	fadeout 0.2
	fps 30
}
