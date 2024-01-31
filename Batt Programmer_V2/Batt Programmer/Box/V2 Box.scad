// Box for Feather & swing based EEPROM programmer
// Print at .3mm layer height, 30% rectilinear infill, 2 extra shells.

Show = 0;  // Set to true to show the assembly

Num4HoleDia = 2.8; // #4 screw hole diameter
Num4ClearHoleDia = 3.4; // #4 screw clearance hole
WireHoleDia = 4; // Wire feed thru hole

// Featherwing dimensions
WingWid = 38.5;
WingLen = 89.8;
FeatherWid = 24;
USBoverhang = 2.5;
WingMntHgt = 2.5;
USBconHgt = 9;  // USB connector hole size/posistion
USBconWid = 13;
USBconIn = 19;
USBconUp = 15.7;
LEDin = 16.6;  // LED in from side of box
LEDback = 18.9;  // LED back from USB end of box

// Box dimensions
WallThk = 1.6;
BoxWid = WingWid+2*WallThk+2;
BoxLen = WingLen+USBoverhang+2*WallThk+1;
BoxHgt = 25;
LipHgt = 7;
ScrewIn = 6;
ScrewDown = 4;

	doBox();

	if (Show)
		translate([0,BoxWid,BoxHgt+WallThk])
			rotate([180,0,0])
				doLid();
	else
		translate([0,-BoxWid-2,0])
			doLid();

module doLid()
{
	difference()
	{
		union()
		{
			// Main lid
			RndCube(BoxLen,BoxWid,WallThk,3);

			// Screw mounts
			translate([2*WallThk,2*WallThk+.25,WallThk])
				cube([9,ScrewIn,LipHgt]);
			translate([BoxLen-9-2*WallThk,2*WallThk+.25,WallThk])
				cube([9,ScrewIn,LipHgt]);
			translate([2*WallThk,BoxWid-ScrewIn-2*WallThk-.25,WallThk])
				cube([9,ScrewIn,LipHgt]);
			translate([BoxLen-9-2*WallThk,BoxWid-ScrewIn-2*WallThk-.25,WallThk])
				cube([9,ScrewIn,LipHgt]);

			// LED "light pipe"
			translate([LEDback,BoxWid-LEDin,0])
				cylinder(h=4+WallThk,d=8.5,$fn=36);

			// Board hold downs
			translate([2*WallThk,8.5,0])
				cube([5,5,6+WallThk]);
			translate([2*WallThk,30.5,0])
				cube([5,5,6+WallThk]);
			translate([50,9.5,0])
				cube([5,5,6+WallThk]);
			translate([50,29.5,0])
				cube([5,5,6+WallThk]);

			difference()
			{
				// Do lip
				translate([WallThk+.25,WallThk+.25,WallThk])
					RndCube(BoxLen-2*WallThk-.5,BoxWid-2*WallThk-.5,LipHgt,4);

				// Hollow out
				translate([2*WallThk+.25,2*WallThk+.25,WallThk])
					RndCube(BoxLen-4*WallThk-.5,BoxWid-4*WallThk-.5,LipHgt+1,3);

				// Hole for USB connector
				translate([-1,BoxWid-USBconWid-((BoxWid-WingWid)/2+USBconIn-USBconWid/2)-2.1,BoxHgt-USBconHgt-(USBconUp-USBconHgt/2+WingMntHgt)])
					cube([10,USBconWid+4,USBconHgt]);
			}

			// Fill in for USB connector opening
			translate([0,BoxWid-USBconWid-((BoxWid-WingWid)/2+USBconIn-USBconWid/2)+.5,0])
				cube([WallThk+1,USBconWid-1,BoxHgt-USBconHgt-(USBconUp-USBconHgt/2+WingMntHgt)]);

		}

		// Screw holes
		translate([0,ScrewIn,ScrewDown+WallThk])
			rotate([0,90,0])
				cylinder(h=25,d=Num4HoleDia,$fn=12,center=true);
		translate([0,BoxWid-ScrewIn,ScrewDown+WallThk])
			rotate([0,90,0])
				cylinder(h=25,d=Num4HoleDia,$fn=12,center=true);
		translate([BoxLen,ScrewIn,ScrewDown+WallThk])
			rotate([0,90,0])
				cylinder(h=25,d=Num4HoleDia,$fn=12,center=true);
		translate([BoxLen,BoxWid-ScrewIn,ScrewDown+WallThk])
			rotate([0,90,0])
				cylinder(h=25,d=Num4HoleDia,$fn=12,center=true);

		// Terminal strip holes
		translate([BoxLen-9,(BoxWid-24)/2+8,0])
			cylinder(h=10,d=Num4HoleDia,$fn=12,center=true);
		translate([BoxLen-9,BoxWid-(BoxWid-24)/2-8,0])
			cylinder(h=10,d=Num4HoleDia,$fn=12,center=true);

		// Wire holes
		translate([BoxLen-16,(BoxWid-24)/2+8,0])
			cylinder(h=10,d=WireHoleDia,$fn=12,center=true);
		translate([BoxLen-16,BoxWid-(BoxWid-24)/2-8,0])
			cylinder(h=10,d=WireHoleDia,$fn=12,center=true);
		translate([BoxLen-16,(BoxWid-24)/2,0])
			cylinder(h=10,d=WireHoleDia,$fn=12,center=true);
		translate([BoxLen-16,BoxWid-(BoxWid-24)/2,0])
			cylinder(h=10,d=WireHoleDia,$fn=12,center=true);

		// LED hole
		translate([LEDback,BoxWid-LEDin,-.1])
			cylinder(h=10,d=6,$fn=36);

		// Notch in lip for board
		translate([0,10.5,6+WallThk])
			cube([15,FeatherWid,10]);
	}
}

module doBox()
{
	difference()
	{
		// Main box
		RndCube(BoxLen,BoxWid,BoxHgt,3);

		// Hole for bottom of wing
		translate([WallThk+USBoverhang+1,(BoxWid-WingWid+2)/2,WallThk])
			cube([WingLen-2,WingWid-2,BoxHgt]);

		// Hole for wing
		translate([WallThk+USBoverhang,(BoxWid-WingWid)/2,WingMntHgt+WallThk])
			cube([WingLen,WingWid,BoxHgt]);

		//  Cut to walls
		translate([WallThk,WallThk,WingMntHgt+WallThk+3])
			RndCube(BoxLen-2*WallThk,BoxWid-2*WallThk,BoxHgt,1.2);

		// Hole for USB connector
		translate([-1,(BoxWid-WingWid)/2+USBconIn-USBconWid/2,USBconUp-USBconHgt/2+WingMntHgt+WallThk])
			cube([10,USBconWid,USBconHgt+5]);

		// Holes for lid screws
		translate([0,ScrewIn,BoxHgt-ScrewDown])
			rotate([0,90,0])
				cylinder(d=Num4ClearHoleDia,h=10,$fn=36,center=true);
		translate([0,BoxWid-ScrewIn,BoxHgt-ScrewDown])
			rotate([0,90,0])
				cylinder(d=Num4ClearHoleDia,h=10,$fn=36,center=true);
		translate([BoxLen,ScrewIn,BoxHgt-ScrewDown])
			rotate([0,90,0])
				cylinder(d=Num4ClearHoleDia,h=10,$fn=36,center=true);
		translate([BoxLen,BoxWid-ScrewIn,BoxHgt-ScrewDown])
			rotate([0,90,0])
				cylinder(d=Num4ClearHoleDia,h=10,$fn=36,center=true);
	}
}


// Routine to make a rounded cube
module RndCube(Wid,Hgt,Thk,RndDia = 1)
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


