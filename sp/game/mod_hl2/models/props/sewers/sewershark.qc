// QC File generated by the Wall Worm Pro Version 5.1.45
// Exported From: C:\Users\Computor Two\Documents\3dsMax\scenes\shurk.max
// Get the latest version and notes at:
// https://dev.wallworm.com 

$scale 1.5
$modelname	"props/sewers/sewershark.mdl"
$body sewershark "sewershark.smd"
// .qc block generated by HLMV begins.

// .qc block generated by HLMV begins.

// .qc block generated by HLMV begins.

$surfaceprop "default"

$attachment root "Bone001" 0 0 0 rotate 0

$hboxset "default"

$hbox 0 "Bone008"	     2.26  -22.50  -22.50    98.26   22.50   22.50
$hbox 0 "Bone009"	   -10.50  -20.00  -20.00    64.50   20.00   20.00
$hbox 0 "Bone010"	    15.00  -10.00  -10.00    75.00   10.00   10.00
$hbox 0 "Bone011"	    11.26  -13.61  -10.23    56.26   46.39    5.71
$hbox 0 "Bone002"	   -10.50  -32.00  -32.49    85.50   32.00   31.50
$hbox 0 "Bone006"	   -12.93   -9.66  -24.07    47.07   10.34   25.93
$hbox 0 "Bone004"	    -1.08  -24.93  -23.69    62.92    5.07   26.31

// .qc block generated by HLMV ends.







$animation "a_idle_pre_leap" "sewershark_sequence_leap.smd" frame 0 0
$animation "a_idle_post_leap" "sewershark_sequence_leap.smd" frame 40 40

$contents "solid"
$cdmaterials "models/props/sewers"
$attachment mouth "Bone002" 0 0 0 rotate 0
$sequence leap "sewershark_sequence_leap.smd" fps 30.0 fadein 0.2 fadeout 0.2 snap {
	{ event AE_HAMMER_EVENT1 18 }
	{ event AE_HAMMER_EVENT2 28 }
}
$sequence idle "sewershark_sequence_idle" fps 30.0 fadein 0.2 fadeout 0.2 snap
$sequence idle_pre_leap "a_idle_pre_leap" fps 30.0 fadein 0.2 fadeout 0.2 loop snap
$sequence idle_post_leap "a_idle_post_leap" fps 30.0 fadein 0.2 fadeout 0.2 loop snap

$include	"sewershark_wwmt_custom.qci"

$keyvalues
{
	particles
	{
		effect
		{
			name	"sewers_shark_droplets"
			attachment_type start_at_origin
			attachment_point "root"
		}
	}
}
	
