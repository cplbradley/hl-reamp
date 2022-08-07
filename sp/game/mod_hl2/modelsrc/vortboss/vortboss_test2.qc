
$modelname	"vortboss_test_weights.mdl"
$model ufo "new_ufo.smd"
$model cannon "new_cannon.smd"
$contents "solid"
$cdmaterials "models/vortboss"
$bodygroup "body"
{
	studio body_normal
	studio body_badly_damaged
}
$texturegroup skins 
{
	{ "vortigaunt_base" "vort_eye" "vortcannon_diffuse" "vortufo" }
	{ "vortigaunt_glow" "vort_eye" "vortcannon_glow" "vortufo_glow" }
	{ "vortigaunt_base" "vort_eye" "vortcannon_diffuse" "vortufo_damaged" }
}

$lod 75 
{
	replacemodel "body_badly_damaged" "body_badly_damaged_lod1"
	replacemodel "body_normal" "body_normal_lod1"

}
$lod 110
{
	replacemodel "body_badly_damaged" "body_badly_damaged_lod2"
	replacemodel "body_normal" "body_normal_lod2"

}
$lod 150
{
	replacemodel "body_badly_damaged" "body_badly_damaged_lod3"
	replacemodel "body_normal" "body_normal_lod3"
}


$attachment "eyeball" "ValveBiped.head" 18 0 0 rotate 180 180 0
$attachment "cannonmuzzle" "ValveBiped.hlp_wrist_R" 36 3.50 -6 rotate 6 7 -0

$hboxset "default"

$hbox 0 "ValveBiped.spine1"	   -10.31  -12.71  -17.42    23.38   18.88   19.24
$hbox 0 "ValveBiped.spine3"	    -9.88  -18.48  -24.80    36.30   21.71   24.18
$hbox 0 "ValveBiped.bone"	     0.00   -4.54   -8.66    19.49    9.11    7.04
$hbox 0 "ValveBiped.bone1"	    -2.76   -5.14   -7.62    12.24    5.82    6.30
$hbox 0 "ValveBiped.bone2"	    -3.01   -2.11   -4.57     5.85    3.71    4.77
$hbox 0 "ValveBiped.bone5"	     0.00   -0.87   -1.39     7.50    2.57    1.94
$hbox 0 "ValveBiped.bone6"	    -0.11   -2.45   -1.24     9.12    1.52    1.42
$hbox 0 "ValveBiped.bone3"	     0.00   -1.08   -2.79     7.38    2.19    0.58
$hbox 0 "ValveBiped.bone4"	    -0.31   -2.33   -3.00     8.66    1.41    0.00
$hbox 0 "ValveBiped.arm1_R"	    -9.83  -14.96  -14.05    47.85   11.18   14.83
$hbox 0 "ValveBiped.arm1_L"	    -6.48  -17.25  -12.38    47.15   15.06   13.85
$hbox 0 "ValveBiped.arm2_L"	    -7.63  -11.61   -7.16    30.59   10.24   12.48
$hbox 0 "ValveBiped.hand1_L"	     0.00   -6.45   -4.70     8.43    3.40    4.34
$hbox 0 "ValveBiped.neck1"	   -12.49  -17.89  -14.94    15.11   16.00   14.26
$hbox 0 "ValveBiped.head"	    -8.84   -8.74  -13.61    18.61   14.43   14.20

$surfaceprop "alienflesh"


$poseparameter "move_yaw" -180 180 loop 0
$poseparameter "aim_pitch" -80 40 loop 0
$poseparameter "body_pitch" -70 70 loop 0

$sequence "ragdoll" {
	"anims\vortboss_ragdoll.smd"
	activity "ACT_DIERAGDOLL" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionjoints "vortboss_phy.smd"
{
	$mass 260
	$inertia 10
	$damping 0.01
	$rotdamping 1
	$rootbone "ValveBiped.hips"


	$jointconstrain "ValveBiped.spine1" x limit -30 30 0.2
	$jointconstrain "ValveBiped.spine1" y limit -23 23 0.2
	$jointconstrain "ValveBiped.spine1" z limit -30 30 0.2

	$jointconstrain "ValveBiped.spine4" x limit -41 41 0.2
	$jointconstrain "ValveBiped.spine4" y limit -41 41 0.2
	$jointconstrain "ValveBiped.spine4" z limit -44 48 0.2

	$jointconstrain "ValveBiped.arm1_L" x limit -19 39 0.2
	$jointconstrain "ValveBiped.arm1_L" y limit -15 21 0.2
	$jointconstrain "ValveBiped.arm1_L" z limit -113 117 0.2

	$jointconstrain "ValveBiped.hand1_L" x limit 0 0 0
	$jointconstrain "ValveBiped.hand1_L" y limit 0 0 0
	$jointconstrain "ValveBiped.hand1_L" z limit -129 15 0.2

	$jointconstrain "ValveBiped.arm1_R" x limit -55 21 0.2
	$jointconstrain "ValveBiped.arm1_R" y limit -17 12 0.2
	$jointconstrain "ValveBiped.arm1_R" z limit -129 95 0.2

	$jointconstrain "ValveBiped.hand1_R" x limit 0 0 0
	$jointconstrain "ValveBiped.hand1_R" y limit 0 0 0
	$jointconstrain "ValveBiped.hand1_R" z limit -135 8 0.2

	$jointconstrain "ValveBiped.head" x limit -39 39 0.2
	$jointconstrain "ValveBiped.head" y limit -57 57 0.2
	$jointconstrain "ValveBiped.head" z limit -32 46 0.2
}

$weightlist cannon_weight {
	"ValveBiped.hips"	0.0
	"ValveBiped.spine1"	0.4
	"ValveBiped.spine2"	0.8
	"ValveBiped.spine3" 1.0
}
$weightlist move_weight {
	"ValveBiped.hips"	1.0
	"ValveBiped.spine1" 0.6
	"ValveBiped.spine2" 0.2
	"ValveBiped.spine3" 0.0
}
$sequence "idle01" {
	"anims/vortboss_idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 20
	loop
}


$animation "move_N" "anims/vortboss_moveN.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_NW" "anims/vortboss_moveNW.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_W" "anims/vortboss_moveW.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_SW" "anims/vortboss_moveSW.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_S" "anims/vortboss_moveS.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_SE" "anims/vortboss_moveSE.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_E" "anims/vortboss_moveE.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "move_NE" "anims/vortboss_moveNE.smd" {
	fps 40
	loop
	walkframe 39 LX LY
}
$animation "aim_center" "anims/vortboss_aim_center.smd" {
	fps 30
	loop
}
$animation "aim_up" "anims/vortboss_aim_up.smd" {
	fps 30
	loop
}
$animation "aim_down" "anims/vortboss_aim_down.smd" {
	fps 30
	loop
}
$animation "body_center" "anims/vortboss_body_center.smd" {
	fps 30
	loop
}
$animation "body_up" "anims/vortboss_body_up.smd" {
	fps 30
	loop
}
$animation "body_down" "anims/vortboss_body_down.smd" {
	fps 30
	loop
}
$animation "fire_up" "anims/vortboss_firecannon_up.smd" {
	fps 50
}
$animation "fire_center" "anims/vortboss_firecannon.smd" {
	fps 50
}
$animation "fire_down" "anims/vortboss_firecannon_down.smd" {
	fps 50
}
$sequence "move" {
	"move_S"
	"move_SE"
	"move_E"
	"move_NE"
	"move_N"
	"move_NW"
	"move_W"
	"move_SW"
	"move_S"
	blend "move_yaw" -180 180
	blendwidth 9
	fadein 0.2
	fadeout 0.2
	node "running"
	activity "ACT_RUN" 1
}
$sequence "move2" {
	"move_S"
	"move_SE"
	"move_E"
	"move_NE"
	"move_N"
	"move_NW"
	"move_W"
	"move_SW"
	"move_S"
	blend "move_yaw" -180 180
	blendwidth 9
	fadein 0.2
	fadeout 0.2
	node "walking"
	activity "ACT_WALK" 1
}

$sequence "firecannon" {
	"fire_up"
	"fire_center"
	"fire_down"
	activity "ACT_VORTBOSS_CANNON" 1
	{ event AE_VORTBOSS_FIRECANNON 36 "" }
	{ event AE_VORTBOSS_FIRECANNON 41 "" }
	{ event AE_VORTBOSS_FIRECANNON 46 "" }
	{ event AE_VORTBOSS_FIRECANNON 51 "" }
	{ event AE_VORTBOSS_FIRECANNON 56 "" }
	{ event AE_VORTBOSS_FIRECANNON 61 "" }
	blend "aim_pitch" -80 40
	blendwidth 3
	node "aim"
	fadein 0.2
	fadeout 0.2
	fps 50
	
}
$sequence "jump" {
	"anims/vortboss_jump.smd"
	activity ACT_JUMP 1
	fadein 0.2
	fadeout 0.2
	fps 35
}
$sequence "glide" {
	"anims/vortboss_glide.smd"
	activity ACT_GLIDE 1
	fadein 0.2
	fadeout 0.2
	fps 35
	loop
}
$sequence "land" {
	"anims/vortboss_land.smd"
	activity ACT_LAND 1
	fadein 0.2
	fadeout 0.2
	fps 35
}
$sequence "eyeblast" {
	"anims/vortboss_eyeblast.smd"
	activity "ACT_VORTBOSS_EYEBLAST" 1
	{ event AE_VORTBOSS_START_TARGETBEAM 18 ""}
	{ event AE_VORTBOSS_FIRE_ATTACKBEAM 75 ""}
	{ event AE_VORTBOSS_CLEAR_BEAM 90 ""}
	fadein 0.2
	fadeout 0.2
	fps 40
	
}
$sequence "spinattack" {
	"anims/vortboss_spinattack.smd"
	activity "ACT_VORTBOSS_SPINATTACK" 1
	{ event AE_VORTBOSS_START_SPINATTACK 35 "" }
	{ event AE_VORTBOSS_END_SPINATTACK 155 "" }
	fadein 0
	fadeout 0
	fps 35
	
}
$sequence "groundattack" {
	"anims/vortboss_groundattack.smd"
	activity "ACT_VORTBOSS_GROUND_ATTACK" 1
	{ event AE_VORTBOSS_GROUNDATTACK 17 "" }
	fadein 0.2
	fadeout 0.2
	fps 45
	
}
$sequence "roar" {
	"anims/vortboss_roar.smd"
	{ event AE_CL_PLAYSOUND 50 "VortBoss.Roar" }
	fadein 0.2
	fadeout 0.2
	fps 30
}
$sequence "chargestart" {
	"anims/vortboss_charge_start.smd"
	activity "ACT_VORTBOSS_CHARGE_START" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	walkframe 40 LX
}
$sequence "chargeloop" {
	"anims/vortboss_charge_loop.smd"
	activity "ACT_VORTBOSS_CHARGE_LOOP" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
	walkframe 21 LX
}
$sequence "chargestop" {
	"anims/vortboss_charge_stop.smd"
	activity "ACT_VORTBOSS_CHARGE_STOP" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	walkframe 40 LX
}
$sequence "chargecrash" {
	"anims/vortboss_crash2.smd"
	activity "ACT_VORTBOSS_CRASH" 1
	{ event AE_CL_PLAYSOUND 1 "VortBoss.Crash" }
	fadein 0.0
	fadeout 0.2
	fps 30
	walkframe 56 LX
}
$sequence "dodgeleft" {
	"anims/vortboss_dodge_left.smd"
	activity ACT_VORTBOSS_DODGE_LEFT 1
	fadein 0.0
	fadeout 0.2
	fps 30
	walkframe 20 LY
}
$sequence "dodgeright" {
	"anims/vortboss_dodge_right.smd"
	activity ACT_VORTBOSS_DODGE_RIGHT 1
	fadein 0.0
	fadeout 0.2
	fps 30
	walkframe 20 LY
}
$sequence "rocketbarage" {
	"anims/vortboss_rocket_barage.smd"
	activity ACT_VORTBOSS_BARAGE 1
	{ event AE_VORTBOSS_ROCKET_LAUNCH 16 "" }
	{ event AE_VORTBOSS_ROCKET_LAUNCH 20 "" }
	{ event AE_VORTBOSS_ROCKET_LAUNCH 24 "" }
	{ event AE_VORTBOSS_ROCKET_LAUNCH 28 "" }
	{ event AE_VORTBOSS_ROCKET_LAUNCH 32 "" }
	{ event AE_VORTBOSS_ROCKET_LAUNCH 36 "" }
	fadein 0.2
	fadeout 0.2
	fps 30
}
$sequence "swing" {
	"anims/vortboss_swing.smd"
	activity ACT_VORTBOSS_SWING 1
	{ event AE_VORTBOSS_SWING_IMPACT 7 "" }
	fadein 0.2
	fadeout 0.2
	fps 30
	walkframe 21 LX LY
}