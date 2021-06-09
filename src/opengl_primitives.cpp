#include "opengl_primitives.hpp"

namespace dmsc {

using OpenGLPrimitives::Mesh;

Mesh OpenGLPrimitives::createSphere(const float radius, const glm::vec3 center, const int accuracy) {
    int number_of_stacks = accuracy;
    int number_of_sectors = accuracy * 2;
    float stack_step = static_cast<float>(M_PI) / number_of_stacks;
    float sector_step = 2.f * static_cast<float>(M_PI) / number_of_sectors;

    Mesh model = Mesh();
    model.gl_draw_mode = GL_TRIANGLES;

    /**
     * 1.   Rasterize the sphere with the analytic equation for spheres.
     *      Stacks move along the z-axis. Sectors move around the z-axis.
     * 2.   All vertices are stored contiguous in an array where every 3 elements form one vertex. I.e. vertex
     * id "0" refers to the vertex ranged from index 0 to index 2 in the vericies array.
     * 3.   3 contiguous id's in the element array form a triangle (counterclockwise).
     * 4.   For every sector (x) on a stack we define two triangles on the right side. n is the number of
     * sectors in a stack:
     *
     *      stack i+1    x+n-------x+n+1
     *                    |       / |
     *                    |     /   |
     *                    |   /     |
     *                    | /       |
     *      stack i       x---------x+1
     *                sector x    sector x+1
     */

    // rasterize a sphere with the analytic equation for spheres
    for (int i = 0; i <= number_of_stacks; ++i) {      // stacks
        for (int j = 0; j <= number_of_sectors; ++j) { // sectors
            VertexData vertex = VertexData();

            // Vertices
            float stack_angle = static_cast<float>(M_PI) / 2.f - (i * stack_step);
            float sector_angle = (j * sector_step);

            vertex.x = center.y + radius * cos(stack_angle) * sin(sector_angle); // y
            vertex.y = -center.z - radius * sin(stack_angle);                    // z
            vertex.z = center.x + radius * cos(stack_angle) * cos(sector_angle); // x

            // texcoords
            vertex.tx = (float)j / number_of_sectors;
            vertex.ty = (float)i / number_of_stacks;

            // normal
            glm::vec3 position = glm::vec3(vertex.x, vertex.y, vertex.z) - center;
            glm::vec3 normal = glm::normalize(position);
            vertex.nx = normal.x;
            vertex.ny = normal.y;
            vertex.nz = normal.z;

            // volors
            vertex.setColor(0.0f, 0.0f, 0.0f);

            model.vertices.push_back(vertex);
        }
    }

    // define the triangles between the vertices
    for (int i = 0; i < number_of_stacks; ++i) {      // stacks
        for (int j = 0; j < number_of_sectors; ++j) { // sectors
            // calculate id's from the previously defined vertices
            int base_point = j + i * number_of_sectors + i; // x
            int right = (base_point + 1);                   // x+1
            int top = base_point + number_of_sectors + 1;   // x+n
            int top_right = (top + 1);                      // x+n+1

            // Triangle 1 ignoring last stack
            if (i != (number_of_stacks - 1)) {
                model.elements.push_back(base_point);
                model.elements.push_back(top_right);
                model.elements.push_back(top);
            }

            // Triangle 2 ignoring first stack
            if (i != 0) {
                model.elements.push_back(base_point);
                model.elements.push_back(right);
                model.elements.push_back(top_right);
            }
        }
    }

    return model;
}

// ------------------------------------------------------------------------------------------------

Mesh OpenGLPrimitives::createSatellite() {
    Mesh model = Mesh();
    model.gl_draw_mode = GL_TRIANGLES;
    const float cube_length = 0.005f;

    return createSphere(0.005f, glm::vec3(0.0f), 10);

    // cube
    for (int i = 0; i < 8; i++) {
        VertexData vertex = VertexData();
        float x = cube_length;
        x *= (i & 4) == 4 ? -1.0f : 1.0f;
        // x += position.x;
        vertex.x = x;
        i = i << 1;

        float y = cube_length;
        y *= ((i & 4) == 4) ? -1.0f : 1.0f;
        // y += position.y;
        vertex.y = y;
        i = i << 1;

        float z = cube_length;
        z *= (i & 4) == 4 ? -1.0f : 1.0f;
        // z += position.z;
        vertex.z = z;

        vertex.r = 1.0f;
        vertex.g = 1.0f;
        vertex.b = 1.0f;

        model.vertices.push_back(vertex);

        i = i >> 2;
    }

    std::vector<GLushort> cube_elements = {// front
                                           1, 5, 7, 1, 7, 3,
                                           // right
                                           5, 4, 6, 5, 6, 7,
                                           // back
                                           4, 0, 2, 4, 2, 6,
                                           // left
                                           1, 0, 2, 1, 2, 3,
                                           // bottom
                                           1, 0, 4, 1, 4, 5,
                                           // top
                                           3, 2, 6, 3, 6, 7};

    model.elements = cube_elements;

    return model;
}

// ------------------------------------------------------------------------------------------------

Mesh OpenGLPrimitives::createOrbit(const Satellite& orbit, const float scale, const glm::vec3 center) {
    int number_of_sides = 100;

    Mesh model = Mesh();
    model.gl_draw_mode = GL_LINE_STRIP;

    for (int i = 0; i <= number_of_sides; i++) {
        VertexData vertex = VertexData();
        float t = i * orbit.getPeriod() / number_of_sides;
        glm::vec3 cartesian_coords = orbit.cartesian_coordinates(t) / scale;
        // Vertices
        vertex.x = center.x + cartesian_coords.x;
        vertex.y = center.y + cartesian_coords.y;
        vertex.z = center.z + cartesian_coords.z;

        // Colors
        vertex.r = 0.5f;
        vertex.g = 0.5f;
        vertex.b = 0.5f;
        model.vertices.push_back(vertex);
    }

    return model;
}

// ------------------------------------------------------------------------------------------------

Mesh OpenGLPrimitives::createLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color, bool dashed) {
    Mesh m = Mesh();
    m.gl_draw_mode = GL_LINES;

    // for every colored segment we need a transparent counterpart - except the last segment
    int colored_segments = dashed ? 15 : 1;
    int segments = 2 * colored_segments - 1;
    glm::vec3 distance_vector = p2 - p1;

    for (float i = 0; i < colored_segments; i++) {
        VertexData v1;
        v1.setColor(color.r, color.g, color.b);
        v1.setPosition(p1 + distance_vector * (i * 2 / segments));
        m.vertices.push_back(v1);

        VertexData v2;
        v2.setColor(color.r, color.g, color.b);
        v2.setPosition(p1 + distance_vector * ((i * 2 + 1) / segments));
        m.vertices.push_back(v2);
    }

    return m;
}

} // namespace dmsc
