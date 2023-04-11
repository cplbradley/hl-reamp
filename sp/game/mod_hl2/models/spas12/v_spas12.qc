// Created by Crowbar 0.62

$modelname "weapons/v_spas.mdl"

$bodygroup "studio"
{
	studio "spas_ref.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition -4.919 -28.541 -16.577

$cdmaterials "models\Weapons\V_hand\"
$cdmaterials "models\Weapons\spas\"
$cdmaterials "models\weapons\shell\"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "1" "muzzle" -45 -2 2 rotate -90 0 0

$cbox 0 0 0 0 0 0

$bbox 5.199 -12.665 -29.433 51.883 2.828 -3.721


$sequence "idle01" {
	"spas_idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}
$sequence "idle_classic" {
	"spas_idle_classic.smd"
	activity "ACT_VM_IDLE_SPECIAL" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$sequence "fire01" {
	"spas_fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_classic" {
	"spas_fire_classic.smd"
	activity "ACT_VM_PRIMARYATTACK_SPECIAL" 1
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "altfire_classic" {
	"spas_fire_classic.smd"
	activity "ACT_VM_PRIMARYATTACK_SPECIAL" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire02" {
	"spas_fire.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "draw" {
	"spas_draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	snap
	fps 30
}
$sequence "draw_classic" {
	"spas_draw_classic.smd"
	activity "ACT_VM_DRAW_SPECIAL" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	snap
	fps 30
}

$sequence "holster" {
	"spas_idle.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "reload1" {
	"spas_idle.smd"
	activity "ACT_SHOTGUN_RELOAD_START" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "reload2" {
	"spas_idle.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "reload3" {
	"spas_idle.smd"
	activity "ACT_SHOTGUN_RELOAD_FINISH" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "pump" {
	"spas_pump.smd"
	activity "ACT_SHOTGUN_PUMP" 1
	{ event 6001 5 "2" }
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}
$sequence "pump_classic" {
	"spas_pump_classic.smd"
	activity "ACT_VM_PULLBACK_SPECIAL" 1
	{ event 6001 5 "2" }
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "dryfire" {
	"spas_idle.smd"
	activity "ACT_VM_DRYFIRE" 1
	fadein 0.2
	fadeout 0.2
	node "2"
	fps 30
}

$sequence "lowered" {
	"spas_idle.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "1"
	fps 30
	loop
}

$sequence "lowered_to_idle" {
	"spas_idle.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "1" "0"
	fps 30
}

$sequence "idle_to_lowered" {
	"spas_idle.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "0" "1"
	fps 30
}
