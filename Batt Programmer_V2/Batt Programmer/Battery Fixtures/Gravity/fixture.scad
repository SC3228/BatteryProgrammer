// Gravity battery fixture
// .4mm nozzle, .3mm layers, 3 shells, 20% rectilinear infill
// Drill out pogo pin holes with #60 - .040 bit in a pinvise
// Push in pins so wide part of pin is flush with bottom of pogo plate

Show = 0;  // True to show assembly

ContactSpacing = 3;  // Spacing of battery contacts
PinDia = 1.5;  // Pogo pin holw diameter

if (Show)
{
	doBase();
	translate([0,0,6.5])
		doSocket();
	translate([10.3,73.3,3.5])
		doPogos();
}
else
{
	translate([30,0,0])
		doBase();
//	translate([-50,0,0])
//		doSocket();
//	doPogos();
}

module doPogos()
{
	difference()
	{
		union()  // Plate to hold pogos
		{
			cube([24,10,1.5]);
			translate([4.75,7.5/2,0])
				cube([14.5,2.5,3.5]);
		}
		for(cnt = [0:4])  // Loop thru pins
			translate([(24-4*ContactSpacing)/2+ContactSpacing*cnt,5,-1])
				cylinder(h=10,d=PinDia,$fn=16);
	}
}

module doBase()
{
	difference()
	{
		cube([44.4,104,6]);  // Main base
		translate([7.2,70,-1])  // Hole for pogo plate
			cube([30,25,12]);
		translate([19.2,86,-1])  // Slot for wires
			cube([6,30,12]);
	}
}

module doSocket()
{
	union()
	{
		translate([3,3,0])
			doSocketRaw();

		cube([4,104,13.1]);
		translate([40.4,0,0])
			cube([4,104,13.1]);
		cube([44.4,3.1,13.1]);
		translate([0,101,0])
			cube([44,3,17.5]);
		cube([44.4,40,2]);
	}
}

module doSocketRaw()
{
	difference()
	{
		translate([39.4,1.3,-52])
			rotate([30,0,0])
				import("gravity_4b_top-cover_front_to-Frank_0612.stl");
		// Square it up
		translate([-12,0,0])
			cube([12,120,20]);
		translate([-10.6,-8.7,0])
			cube([50,8.7,20]);
		translate([37.5,0,0])
			cube([10,120,20]);
		translate([0,98,0])
			cube([50,10,20]);
		// Flatten bottom
		translate([0,0,-2])
			cube([50,120,2]);
		// lose the "lip"
		translate([-1,-1,12.9])
			cube([40,94,10]);
		translate([-1,93,17.5])
			cube([40,20,10]);
		// Losen up the fit
		translate([2.7,29,3.15])
			rotate([0,-1.5,0])
				cube([2,2,12]);
		translate([33.9,29,3.19])
			rotate([0,1.5,0])
				cube([2,2,12]);
		translate([2.75,76.7,1.39])
			rotate([0,-1.5,0])
				cube([2,2.5,12]);
		translate([33.85,76.7,1.43])
			rotate([0,1.5,0])
				cube([2,2.5,12]);
	}
}
