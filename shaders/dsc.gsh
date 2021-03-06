#version 150
layout(triangles) in;
in int vert_status[];
layout(triangle_strip, max_vertices=3) out;

out vec3 color;
noperspective out vec3 dist;

uniform vec3 STATUS_COLS[4] = {
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0)
};

uniform vec2 screen_dimensions;
uniform bool per_vertex_coloring;
uniform bool display_outside;
uniform bool display_inside;
uniform bool display_interface;
uniform bool display_error;

// This is copied from tetmesh.cpp
int get_face_status() {
    bool is_inside = vert_status[0] == 0 || vert_status[1] == 0 || vert_status[2] == 0;
    bool is_outside = vert_status[0] == 1 || vert_status[1] == 1 || vert_status[2] == 1;
    if (is_inside && is_outside) {
        // error
        return 4;
    } else if (is_inside) {
        return 0;
    } else if (is_outside) {
        return 1;
    } else {
        return 2;
    }
}

void main() {
    int status = get_face_status();
    if ((status == 0 && display_inside) ||
        (status == 1 && display_outside) ||
        (status == 2 && display_interface) ||
        (status == 4 && display_error)) {
        color = STATUS_COLS[status];

        vec2 ab = vec2(gl_in[0].gl_Position - gl_in[1].gl_Position);
        vec2 ac = vec2(gl_in[0].gl_Position - gl_in[2].gl_Position);
        vec2 ba = vec2(gl_in[1].gl_Position - gl_in[0].gl_Position);
        vec2 bc = vec2(gl_in[1].gl_Position - gl_in[2].gl_Position);
        ab *= screen_dimensions;
        ac *= screen_dimensions;
        ba *= screen_dimensions;
        bc *= screen_dimensions;

        float ab_len = length(bc);
        float ac_len = length(ac);
        float bc_len = length(bc);

        dist = vec3(length(cross(vec3(ab, 0), vec3(ac, 0))) / bc_len, 0, 0);
        gl_Position = gl_in[0].gl_Position;
        if (per_vertex_coloring) {
            color = STATUS_COLS[vert_status[0]];
        }
        EmitVertex();

        dist = vec3(0, length(cross(vec3(ba, 0), vec3(bc, 0))) / ac_len, 0);
        gl_Position = gl_in[1].gl_Position;
        if (per_vertex_coloring) {
            color = STATUS_COLS[vert_status[1]];
        }
        EmitVertex();

        dist = vec3(0, 0, length(cross(vec3(ac, 0), vec3(bc, 0))) / ab_len);
        gl_Position = gl_in[2].gl_Position;
        if (per_vertex_coloring) {
            color = STATUS_COLS[vert_status[2]];
        }
        EmitVertex();
    }
}