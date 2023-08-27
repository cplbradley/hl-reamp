$scale 1
$modelname	"weapons/v_chaingun.mdl"
$origin 0 0 0	
$body v_chaingun "v_chaingun_ref.smd"
$attachment "muzzle" "chaingun_barrel" 0.00 -30.00 0.00 rotate -0 180 0
$cdmaterials "models/weapons/chaingun"
$sequence draw "chaingun_draw" fps 30.0 activity ACT_VM_DRAW -1.0 fadein 0.2 fadeout 0.2 snap 
$sequence idle "chaingun_idle" fps 5.0 loop  activity ACT_VM_IDLE -1.0 fadein 0.2 fadeout 0.2 
$sequence fire1 "chaingun_fire1" fps 30.0 activity ACT_VM_PRIMARYATTACK -1.0 fadein 0.2 fadeout 0.2 snap 
$sequence fire2 "chaingun_fire2" fps 30.0 activity ACT_VM_SECONDARYATTACK -1.0 fadein 0.2 fadeout 0.2 snap

