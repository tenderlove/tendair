LAYER_HEIGHT = 0.2;

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

module BoltPosts() {
    translate([10, 0, 0])
        BoltPost(10, 12, 4, 3, 6, 5.4, 2.3, 1);

    translate([-10, 0, 0])
        rotate(180)
        BoltPost(10, 12, 4, 3, 6, 5.4, 2.3, 1);
}

BoltPosts();
