LAYER_HEIGHT = 0.2;
PCB_X = 55;
PCB_Y = 1.8;
PARTS_Y = 5;
PCB_Z = 26;
WALL_THICKNESS = 1.6;

PCB_BUFFER_Z = 4.5;

GAP = 20; // Gap between PCB and sensor

SEN66_X = 55; // Hopefully
SEN66_Y = 22; // Hopefully
SEN66_Z = 25.5; // Hopefully

inner_x = PCB_X;
inner_y = PCB_Y + PARTS_Y + SEN66_Y + GAP;
inner_z = PCB_Z + (PCB_BUFFER_Z * 2);

SEN66_BUFFER_Z = (inner_z - SEN66_Z) / 2;

BOLT_DIAMETER = 3.5;
BOLT_HEAD_DIAMETER = 6;
BOLT_HEAD_HEIGHT = 3;
NUT_FLAT_TO_FLAT = 5.4;
NUT_LENGTH = 3;
FROM_BOLT_BOTTOM = 20;
POST_SIZE = 10;

USB_X = WALL_THICKNESS + 1;
USB_Y = 4.2;
USB_Z = 9;

module BoltNegative(post_size, nominal_length, head_length) {
    flat_to_flat = NUT_FLAT_TO_FLAT;
    bolt_diameter = BOLT_DIAMETER;
    head_diameter = BOLT_HEAD_DIAMETER;
    from_bolt_bottom = FROM_BOLT_BOTTOM;
    nut_length = NUT_LENGTH;

    corner_to_corner = flat_to_flat * 1.1547;

    union() {
        // Bridges for nut slot
        translate([0, 0, from_bolt_bottom + nut_length + LAYER_HEIGHT])
            linear_extrude(LAYER_HEIGHT)
            square(size = [bolt_diameter, bolt_diameter], center = true);

        // Bridges for nut slot
        translate([0, 0, from_bolt_bottom + nut_length - 0.1])
            linear_extrude(LAYER_HEIGHT + 0.1)
            square(size = [bolt_diameter, flat_to_flat], center = true);

        // Slot for nut
        color("pink")
            translate([-((((post_size - corner_to_corner) / 2) / 2) + 0.1), 0, from_bolt_bottom])
            linear_extrude(nut_length)
            square(size = [corner_to_corner + ((post_size - corner_to_corner) / 2) + 0.1, flat_to_flat], center = true);

        // Bolt head sink
        color("blue")
            translate([0, 0, nominal_length])
            linear_extrude(head_length + 1)
            circle(d = head_diameter, $fn = 50);

        // Bridges for head
        translate([0, 0, nominal_length - LAYER_HEIGHT])
            linear_extrude(LAYER_HEIGHT)
            square(size = [bolt_diameter, head_diameter], center = true);

        // Bridges for head
        translate([0, 0, nominal_length - (LAYER_HEIGHT * 2)])
            linear_extrude(LAYER_HEIGHT)
            square(size = [bolt_diameter, bolt_diameter], center = true);

        // Bolt shaft
        color("green")
            translate([0, 0, 0])
            linear_extrude(nominal_length + 1)
            circle(d = bolt_diameter, $fn = 50);
    }
}

module BoltPositive(post_size, nominal_length, head_length) {
    difference() {
        linear_extrude(nominal_length + head_length)
            square(size = post_size, center = true);
    }
}

module BoltPostsPositive(width, height) {
    post_size = POST_SIZE;
    post_height = height;

    move = (width / 2) - (post_size / 2);

    translate([move, 0, 0])
        BoltPositive(post_size, post_height - BOLT_HEAD_HEIGHT, BOLT_HEAD_HEIGHT);

    translate([-move, 0, 0])
        rotate(180)
        BoltPositive(post_size, post_height - BOLT_HEAD_HEIGHT, BOLT_HEAD_HEIGHT);
}

module BoltPostsNegative(width, height) {
    post_size = POST_SIZE;
    post_height = height;

    move = (width / 2) - (post_size / 2);

    for (i = [0, 180]) {
        for (j = [1, -1]) {
            translate([j * move, 0, 0])
                rotate(i)
                BoltNegative(post_size, post_height - BOLT_HEAD_HEIGHT, BOLT_HEAD_HEIGHT);
        }
    }
}

module Main() {
    difference() {
        // Exterior
        translate([0, 0, -WALL_THICKNESS])
            linear_extrude(inner_z + (WALL_THICKNESS * 2))
            square(size = [inner_x + (WALL_THICKNESS * 2), inner_y + (WALL_THICKNESS * 2)], center = true);

        // Interior
        linear_extrude(inner_z)
            square(size = [inner_x, inner_y], center = true);
    }
}

module BottomPCBTabs(tab_x, tab_y, tab_z) {
    for (i = [1, -1]) {
        translate([i * ((PCB_X / 2) - (tab_x / 2)), 0, -PCB_BUFFER_Z])
            linear_extrude(PCB_BUFFER_Z)
            square(size = [tab_x, tab_y * 2 + PCB_Y], center = true);
        for (j = [1, -1]) {
            translate([i * ((PCB_X / 2) - (tab_x / 2)), j * (((tab_y / 2) + (PCB_Y / 2))), 0])
                linear_extrude(tab_z)
                square(size = [tab_x, tab_y], center = true);
        }
    }
}

module PCBTabs() {
    tab_z = 3;
    tab_y = 2;
    tab_x = 5;

    translate([0, 0, (PCB_Z / 2) + PCB_BUFFER_Z]) {
        rotate([180, 0, 0])
            translate([0, 0, -PCB_Z / 2])
            BottomPCBTabs(tab_x, tab_y, tab_z);
        translate([0, 0, -PCB_Z / 2])
            BottomPCBTabs(tab_x, tab_y, tab_z);
    }
}

module FullBox() {
    difference() {
        union() {
            Main();
            translate([0, (-POST_SIZE / 2) + (inner_z - POST_SIZE - SEN66_Y), 0])
                BoltPostsPositive(inner_x, inner_z + WALL_THICKNESS);

            translate([0, (-(inner_y / 2)) + (PCB_Y / 2) + PARTS_Y, 0])
                PCBTabs();
            translate([0, (inner_y / 2) - SEN66_Y, 0])
                SEN66Tabs();
        }

        // Bolt cutouts
        translate([0, (-POST_SIZE / 2) + (inner_z - POST_SIZE - SEN66_Y), 0])
            BoltPostsNegative(inner_x, inner_z + WALL_THICKNESS);

        SensorWindow();

        translate([0, (-(inner_y / 2)) + PARTS_Y - (USB_Y / 2), 0])
            USBWindow();

        Vents();
    }
}

module BottomSlice() {
    difference() {
        FullBox();

        zshift = (PCB_BUFFER_Z + (PCB_Z / 2) + USB_Z / 2) + 5;

        translate([0, 0, zshift])
            linear_extrude(60)
            square(size = 60, center = true);
    }
}

module TopSlice() {
    intersection() {
        FullBox();

        zshift = (PCB_BUFFER_Z + (PCB_Z / 2) + USB_Z / 2) + 5;

        translate([0, 0, zshift])
            linear_extrude(60)
            square(size = 60, center = true);
    }
}

module SEN66Tabs() {
    rail_x = 3;
    translate([0, SEN66_Y / 2, (SEN66_Z / 2) + SEN66_BUFFER_Z]) {
        //color("purple")
        //    cube([SEN66_X, SEN66_Y, SEN66_Z], center = true);

        for (j = [1, -1]) {
            for (i = [1, -1]) {
                xoffset = i * ((SEN66_X / 2) - (rail_x / 2));
                zoffset = j * ((SEN66_Z / 2) + (SEN66_BUFFER_Z / 2));
                translate([xoffset, 0, zoffset])
                    cube(size = [rail_x, SEN66_Y, SEN66_BUFFER_Z], center = true);
            }
        }
    }
}

module SensorWindow() {
    translate([0, (inner_y / 2) + (WALL_THICKNESS / 2), SEN66_BUFFER_Z + 2])
        linear_extrude(SEN66_Z - 4)
        square(size = [SEN66_X - 4, WALL_THICKNESS + 2], center = true);
}

module USBWindow() {
    usb_move_z = (PCB_Z / 2) - (USB_Z / 2) + PCB_BUFFER_Z;
    translate([-((PCB_X / 2) + (USB_X / 2)) + 0.5, 0, usb_move_z])
        linear_extrude(USB_Z)
        square(size = [USB_X, USB_Y], center = true);
}

module Vents() {
    z = PCB_Z - 7;
    xoffset = (inner_x / 2) + WALL_THICKNESS - (WALL_THICKNESS / 2);
    zoffset = (inner_z / 2) - (z / 2);
    yoffset = -(inner_y / 2) + 1;

    // Top Vent
    translate([xoffset, yoffset, zoffset])
        linear_extrude(z)
        square(size = [WALL_THICKNESS + 0.2, 2], center = true);

    total_z = inner_z + (WALL_THICKNESS * 2);
    translate([0, -(inner_y / 2) + 1.6, (total_z / 2) - WALL_THICKNESS])
        for(j = [15, -15]) {
            for(i = [1, -1]) {
                translate([j, 0, i * (total_z / 2)])
                    cube([20, 2, WALL_THICKNESS+ 5], center = true);
            }
        }
}

rendering = "full";

if (rendering == "top") {
    translate([0, 60, 35])
        rotate([180, 0, 0])
        TopSlice();
}

if (rendering == "bottom") {
    BottomSlice();
}

if (rendering == "full") {
    FullBox();
}
