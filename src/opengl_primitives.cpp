#include "opengl_primitives.hpp"

namespace dmsc {

using OpenGLPrimitives::Object;

Object OpenGLPrimitives::createSphere(const float radius, const glm::vec3 center, const unsigned short accuracy) {
    unsigned short number_of_stacks = accuracy;
    unsigned short number_of_sectors = accuracy * 2;
    float stack_step = static_cast<float>(M_PI) / number_of_stacks;
    float sector_step = 2.f * static_cast<float>(M_PI) / number_of_sectors;

    Object model = Object();
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

            vertex.position.x = center.y + radius * cos(stack_angle) * sin(sector_angle); // y
            vertex.position.y = -center.z - radius * sin(stack_angle);                    // z
            vertex.position.z = center.x + radius * cos(stack_angle) * cos(sector_angle); // x

            // texcoords
            vertex.texture.x = (float)j / number_of_sectors;
            vertex.texture.y = (float)i / number_of_stacks;

            // normal
            glm::vec3 position = vertex.position - center;
            vertex.normal = glm::normalize(position);

            // colors
            vertex.color = glm::vec3(0.f);

            model.vertices.push_back(vertex);
        }
    }

    // define the triangles between the vertices
    for (unsigned short i = 0; i < number_of_stacks; ++i) {      // stacks
        for (unsigned short j = 0; j < number_of_sectors; ++j) { // sectors
            // calculate id's from the previously defined vertices
            unsigned short base_point = j + i * number_of_sectors + i; // x
            unsigned short right = (base_point + 1);                   // x+1
            unsigned short top = base_point + number_of_sectors + 1;   // x+n
            unsigned short top_right = (top + 1);                      // x+n+1

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

Object OpenGLPrimitives::createSatellite() {
    Object model = Object();
    model.gl_draw_mode = GL_TRIANGLES;

    return createSphere(0.005f, glm::vec3(0.0f), 10);
}

// ------------------------------------------------------------------------------------------------

Object OpenGLPrimitives::createOrbit(const Satellite& orbit, const float scale, const glm::vec3 center) {
    int number_of_sides = 100;

    Object model = Object();
    model.gl_draw_mode = GL_LINE_STRIP;

    for (int i = 0; i <= number_of_sides; i++) {
        VertexData vertex = VertexData();
        float t = i * orbit.getPeriod() / number_of_sides;
        glm::vec3 cartesian_coords = orbit.cartesian_coordinates(t) / scale;
        // Vertices
        vertex.position = center + cartesian_coords;

        // Colors
        vertex.color = glm::vec3(.5f);
        model.vertices.push_back(vertex);
    }

    return model;
}

// ------------------------------------------------------------------------------------------------

Object OpenGLPrimitives::createLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color, bool dashed) {
    Object m = Object();
    m.gl_draw_mode = GL_LINES;

    // for every colored segment we need a transparent counterpart - except the last segment
    int colored_segments = dashed ? 15 : 1;
    int segments = 2 * colored_segments - 1;
    glm::vec3 distance_vector = p2 - p1;

    for (float i = 0; i < colored_segments; i++) {
        VertexData v1;
        v1.color = color;
        v1.position = p1 + distance_vector * (i * 2 / segments);
        m.vertices.push_back(v1);

        VertexData v2;
        v2.color = color;
        v2.position = p1 + distance_vector * ((i * 2 + 1) / segments);
        m.vertices.push_back(v2);
    }

    return m;
}

} // namespace dmsc
