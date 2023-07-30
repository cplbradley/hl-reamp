// Created by Crowbar 0.74

$modelname "weapons/w_plasmarifle.mdl"

$bodygroup "studio"
{
	studio "w_plasmarifle_ref.smd"
}


$surfaceprop "weapon"
$contents "solid"
$cdmaterials "models/weapons/plasmarifle"
$texturegroup "skins"
{
	{ "w_plasmarifle" }
	{ "w_plasmarifle_enemy" }
}

$attachment "muzzle" "ValveBiped.Weapon_bone" 19.12 0.23 -3.78 rotate 9 0 -92
$attachment "1" "ValveBiped.Weapon_bone" 2.4 0.57 -1.96 rotate 1.92 89.98 -80.98

$definebone "ValveBiped" "" 0 0 0 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped" 0.267086 -2.58952 10.171795 78.841283 -102.773508 -9.262035 0 0 0 0 0 0
$definebone "ValveBiped.Weapon_bone" "ValveBiped.Bip01_R_Hand" 5 -1.5 -4.999996 0.000001 0 0 0 0 0 0 0 0


$sequence "idle" {
	"idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "attack1" {
	"attack1.smd"
	activity "ACT_RANGE_ATTACK_SMG1" 1
	{ event AE_NPC_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	{ event 3002 0 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$sequence "attackoffcenter" {
	"attackoffcenter.smd"
	{ event AE_NPC_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	{ event 3002 0 "lefthand" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$collisionmodel "w_plasmarifle_phys.smd"
{
	$mass 8
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 6

}
