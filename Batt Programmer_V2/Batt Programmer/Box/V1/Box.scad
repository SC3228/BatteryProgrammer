// Box for Mega & shield based EEPROM programmer
// Print at .3mm layer height, 30% infill, 1 extra shell.

Show = 0;  // Set to true to show the assembly

Num2HoleDia = 2.6; // #2 screw hole diameter
Num2ClearHoleDia = 3.5; // #2 screw clearance hole
Hole6_32Tap = 2.8; // 6-32 tap hole
Hole6_32Clear = 3.75; // 6-32 clearance hole

// Box dimensions
WallThk = 1.8;
BaseThk = 1.5;
BoxX = 110;
BoxY = 61;
BoxThk = 28;
Rnd = 4;  // Diameter of rounding on corners

// Lid dimensions
LidThk = 1.5;
LipThk = 7;
LipInset = .2;
LidScrewDown = 3.5;
LidScrewIn = 9;

// Arduino MEGA mounting stud dimensions
MegaStudX = [14,15.3,66.1,66.1,90.2,96.8];
MegaStudY = [2.5,50.7,7.6,35.5,50.7,2.5];
MegaStudDia = 3;
MegaStudHgt = 2;
MegaStudBaseDia = 4.5;
MegaStudBaseHgt = 3;
MegaMountX = 5;
MegaMountY = 4;
MegaBoardThk = 1.8;

// Board stack dimensions
MegaStackX = 0;
MegaStackY = 0;
MegaStackWid = 54;
MegaStackThkFrnt = 15.5;
MegaStackThkBack = 17.5;
MegaStackLen = 102;

// USB hole dimensions
USBwid = 13.5;
USBhgt = 14;
USBy = 30.5;
USBz = 1.1;

// SD card hole dimensions
SDwid = 32.5;
SDtop = 4.9;
SDhgt = 10;
SDy = 11.5;
SDz = 14;


	// Do the box
	rotate([0,0,90])
		DrawBox();

	// Do the lid
	if (Show)
	{

		translate([-BoxY,0,BoxThk+LidThk+10])
			rotate([0,180,-90])
				color("RED")
					DrawLid();

		// Show Mega and shield boards
		rotate([0,0,90])
			translate([MegaMountX,MegaMountX,MegaStudBaseHgt+BaseThk])
				color("GREEN")
				union()
				{
					cube([MegaStackLen,MegaStackWid,MegaStackThkFrnt]);
					translate([MegaStackLen-10,0,0])
						cube([10,MegaStackWid,MegaStackThkBack]);
				}
	}
	else
		translate([-BoxY-2,0,0])
			rotate([0,0,90])
				DrawLid();

module DrawLid()
{
	// Draw box lid
	difference()
	{
		union()
		{
			RndCube(BoxX,BoxY,LidThk);  // Lid
			difference()  // Do lip
			{
				translate([WallThk+LipInset,WallThk+LipInset,0])
					RndCube(BoxX-(2*WallThk)-(2*LipInset),BoxY-(2*WallThk)-(2*LipInset),LipThk);
				translate([WallThk*2+LipInset,WallThk*2+LipInset,LidThk])
					RndCube(BoxX-(4*WallThk)-(2*LipInset),BoxY-(4*WallThk)-(2*LipInset),LipThk); // Sub out inside of lid
				translate([WallThk-1,BoxY-(SDy+MegaMountY)-(SDwid),LidThk+SDtop])  // Opening for SD card
					cube([WallThk+2,SDwid,LipThk]);
			}
			translate([0,BoxY-(SDy+MegaMountY)-(SDwid-.5),LidThk])  // Top of opening for SD card
				cube([WallThk+2,SDwid-1,SDtop]);

			// Back board hold down
			translate([BoxX-WallThk*2-10-LipInset,WallThk+LipInset+1.5,LidThk])
				difference()
				{
					cube([10,BoxY-2*WallThk-2*LipInset-3,BoxThk-(MegaStudBaseHgt+BaseThk+MegaStackThkBack)]);
					translate([-1,13,-2])
						cube([10,25,12]);
				}

			// Front board hold downs
			translate([WallThk*2,WallThk+LipInset+1.5,LidThk])
				difference()
				{
					cube([8,BoxY-2*WallThk-2*LipInset-3,BoxThk-(MegaStudBaseHgt+BaseThk+MegaStackThkFrnt)]);
					translate([-1,BoxY-(SDy+MegaMountY)-(SDwid-.5)-(WallThk+LipInset+2),-2])
						cube([10,SDwid,12]);
				}
		}

		// Do mounting holes for terminal strip
		translate([84,BoxY/2-8,0])
			cylinder(h=7,d=Hole6_32Clear,center=true,$fn=16);
		translate([84,BoxY/2+8,0])  // Must be 8mm*2 from other hole
			cylinder(h=7,d=Hole6_32Clear,center=true,$fn=16);

		// Do hole for wires
		translate([80,BoxY/2,0])
			cylinder(h=7,d=4,d=2.5,center=true,$fn=16);

		// Holes for lid screws
		translate([0,LidScrewIn,LidScrewDown+LidThk])
			rotate([0,90,0])
				cylinder(h=30,d=Num2HoleDia,$fn=8,center=true);
		translate([0,BoxY-LidScrewIn,LidScrewDown+LidThk])
			rotate([0,90,0])
				cylinder(h=30,d=Num2HoleDia,$fn=8,center=true);
		translate([BoxX,LidScrewIn,LidScrewDown+LidThk])
			rotate([0,90,0])
				cylinder(h=30,d=Num2HoleDia,$fn=8,center=true);
		translate([BoxX,BoxY-LidScrewIn,LidScrewDown+LidThk])
			rotate([0,90,0])
				cylinder(h=30,d=Num2HoleDia,$fn=8,center=true);

	// Sub out lightning holes
//	translate([14,5.5,-1])
//		cube([69,50,LidThk+2]);
	}
}

module DrawBox()
{
	// Draw project box
	difference()
	{
		// Do base
		union()
		{
			difference()
			{
				RndCube(BoxX,BoxY,BoxThk);
				translate([WallThk,WallThk,BaseThk])
					RndCube(BoxX-2*WallThk,BoxY-2*WallThk,BoxThk,Rnd-WallThk); // Sub out inside of box

	// Sub out lightning holes
//	translate([MegaMountX+17,MegaMountY+3,-1])
//		cube([46,45,BaseThk+2]);
//	translate([MegaMountX+69,MegaMountY+3,-1])
//		cube([25,45,BaseThk+2]);
//	translate([MegaMountX,MegaMountY+3,-1])
//		cube([11,45,BaseThk+2]);

				// USB hole
				translate([-1,USBy+MegaMountY,MegaStudBaseHgt+BaseThk+USBz])
					cube([WallThk+2,USBwid,USBhgt]);

				// SD card hole
				translate([-1,SDy+MegaMountY,MegaStudBaseHgt+BaseThk+SDz])
					cube([WallThk+2,SDwid,SDhgt]);

				// Holes for lid screws
				translate([0,LidScrewIn,BoxThk-LidScrewDown])
					rotate([0,90,0])
						cylinder(h=30,d=Num2ClearHoleDia,$fn=18,center=true);
				translate([0,BoxY-LidScrewIn,BoxThk-LidScrewDown])
					rotate([0,90,0])
						cylinder(h=30,d=Num2ClearHoleDia,$fn=18,center=true);
				translate([BoxX,LidScrewIn,BoxThk-LidScrewDown])
					rotate([0,90,0])
						cylinder(h=30,d=Num2ClearHoleDia,$fn=18,center=true);
				translate([BoxX,BoxY-LidScrewIn,BoxThk-LidScrewDown])
					rotate([0,90,0])
						cylinder(h=30,d=Num2ClearHoleDia,$fn=18,center=true);

	// Sub out the sides
//	translate([10,-5,8])
//		cube([85,10,35]);
//	translate([10,BoxY-5,8])
//		cube([85,10,35]);
			}

			translate([MegaMountX,MegaMountY,0]) // MEGA Mount
				DoMegaMount();
		}
	}
}
// Routine to do mounting studs for an Arduino MEGA board
module DoMegaMount()
{
	// Do studs
	for (cnt = [0:5])
		translate([MegaStudX[cnt],MegaStudY[cnt],0])
			union()
			{
				cylinder(d=MegaStudBaseDia,h=MegaStudBaseHgt+BaseThk,$fn=8);
				cylinder(d=MegaStudDia,h=MegaStudHgt+MegaStudBaseHgt+BaseThk,$fn=8);
			}
}

// Routine to make a rounded cube
module RndCube(Wid,Hgt,Thk,RndDia = Rnd)
{
	difference()
	{
		cube([Wid,Hgt,Thk]);
		translate([Wid-RndDia,Hgt-RndDia,0])
			RndTool(RndDia,Thk);
		translate([RndDia,RndDia,0])
			rotate([0,0,180])
				RndTool(RndDia,Thk);
		translate([Wid-RndDia,RndDia,0])
			rotate([0,0,270])
				RndTool(RndDia,Thk);
		translate([RndDia,Hgt-RndDia,0])
			rotate([0,0,90])
				RndTool(RndDia,Thk);
	}
}

// Routine to make the tool for rounding a vertical corner
module RndTool(Rad, Thk)
{
	translate([0,0,-1])
		difference()
		{
			cube([2*Rad,2*Rad,Thk+2]);
			translate([0,0,-1])
				cylinder(Thk+4,Rad,Rad,$fn=36);
		}
}
