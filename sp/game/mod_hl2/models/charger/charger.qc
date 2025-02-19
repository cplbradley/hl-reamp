$modelname "charger.mdl"
$origin 0 0 0 90
$scale 1.3
$body "charger" "charger_test.smd"

$attachment "LeftFoot" "CATRigLLegAnkle" 0 0 0 rotate 0 0 0
$attachment "RightFoot" "CATRigRLegAnkle" 0 0 0 rotate 0 0 0
$attachment "camera" "CATRigHub002" 14.00 1.00 0.00 rotate 0 0 0

$surfaceprop "metal"

$collisionjoints "charger_phy.smd"
{
    $mass 50
}

$cdmaterials "models/charger/"

$sequence "surprise" {
	"charger_surprise.smd"
	activity "ACT_ANTLIONGUARD_SURPRISE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "charge_loop" {
	"charger_charge_loop.smd"
	activity "ACT_ANTLIONGUARD_CHARGE_RUN" 1
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 2 }
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 9 }
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 17 }
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 25 } 
	fadein 0.2
	fadeout 0.2
	fps 50
	loop
	walkframe 32 LX LY
}

$sequence "charge_start" {
	"charger_charge_start.smd"
	activity "ACT_ANTLIONGUARD_CHARGE_START" 1
	fadein 0.2
	fadeout 0.2
	fps 40
	walkframe 0 LX LY
	walkframe 20 LX LY
	walkframe 23 LX LY
	walkframe 25 LX LY
}
$sequence "charge_stop" {
	"charger_charge_cancel.smd"
	activity "ACT_ANTLIONGUARD_CHARGE_STOP" 1
	{ event AE_ANTLIONGUARD_VOICE_SQUEEZE 0 }
	{ event AE_CHARGER_SLIDE 0 }
	{ event AE_CHARGER_SLIDE_STOP 20 }
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 37 }
	fadein 0.2
	fadeout 0.2
	fps 40
	walkframe 0 LX LY
	walkframe 25 LX LY
}
$sequence "charge_canel" {
	"charger_charge_cancel.smd"
	activity "ACT_ANTLIONGUARD_CHARGE_CANCEL" 1
	{ event AE_ANTLIONGUARD_VOICE_SQUEEZE 0 }
	{ event AE_CHARGER_SLIDE 0 }
	{ event AE_CHARGER_SLIDE_STOP 20 }
	{ event AE_ANTLIONGUARD_FOOTSTEP_HEAVY 37 }
	fadein 0.2
	fadeout 0.2
	fps 40
	walkframe 0 LX LY
	walkframe 25 LX LY
}
$sequence "idle" {
	charger_idle.smd
	activity "ACT_IDLE" 1
	loop
	fps 10
}
$sequence "run" {
	charger_charge_loop.smd
	activity "ACT_WALK" 1
	activity "ACT_RUN" 1
	loop
	walkframe 32 LX LY
	fps 16
}
$sequence "crash" {
	charger_crash.smd
	activity "ACT_ANTLIONGUARD_CHARGE_CRASH" 1
	fps 27
	walkframe 0 LX LY
	walkframe 16 LX LY
}
$sequence "ragdoll" {
	charger_ragdoll.smd
	activity "ACT_DIERAGDOLL" 1
	activity "ACT_DIESIMPLE" 1
	loop
}