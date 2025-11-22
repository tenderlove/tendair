LAYER_HEIGHT = 0.2;
PCB_X = 55;
PCB_Y = 1.8;
PARTS_Y = 4;
PCB_Z = 26;
WALL_THICKNESS = 1.6;

GAP = 20; // Gap between PCB and sensor

SEN66_X = 55; // Hopefully
SEN66_Y = 21.3; // Hopefully
SEN66_Z = 26; // Hopefully

module BoltPost(post_size, nominal_length, bolt_diameter, head_length, head_diameter, flat_to_flat, nut_length, from_bolt_bottom) {
    corner_to_corner = flat_to_flat * 1.1547;

    difference() {
        linear_extrude(nominal_length + head_length)
            square(size = post_size, center = true);

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

            // Bolt shaft
            color("green")
                translate([0, 0, -1])
                linear_extrude(nominal_length + 2)
                circle(d = bolt_diameter, $fn = 50);
        }
    }
}

module ABolt(post_size) {
    BoltPost(post_size, 12, 3.5, 3, 6, 5.4, 3, 1);
}

module BoltPosts(width) {
    post_size = 10;

    move = (width / 2) - (post_size / 2);

    translate([move, 0, 0])
        ABolt(post_size);

    translate([-move, 0, 0])
        rotate(180)
        ABolt(post_size);
}

//BoltPosts(55);

inner_y = PCB_Y + PARTS_Y + SEN66_Y + GAP;
inner_z = max(PCB_Z, SEN66_Z);
inner_x = max(PCB_X, SEN66_X);

#difference() {
    // Exterior
    translate([0, 0, -WALL_THICKNESS])
        linear_extrude(inner_z + (WALL_THICKNESS * 2))
        square(size = [inner_x + (WALL_THICKNESS * 2), inner_y + (WALL_THICKNESS * 2)], center = true);

    // Interior
    linear_extrude(inner_z)
        square(size = [inner_x, inner_y], center = true);
}

tab_z = 3;
tab_y = 2;
tab_x = 5;
