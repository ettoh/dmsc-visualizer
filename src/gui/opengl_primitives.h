#ifndef OPENGL_PRIMITIVES
#define OPENGL_PRIMITIVES

#define _USE_MATH_DEFINES

#include "../instance/orbit.h"
#include "vdmsc/glm_include.h"
#include <QOpenGLFunctions_3_3_Core>
#include <math.h>
#include <vector>

namespace OpenGLPrimitives {
/**
 * @brief Store all the data to describe a vertex. Must not contain additional data.
 */
struct VertexData {
    float x = 0.0f, y = 0.0f, z = 0.0f;    // position
    float r = 1.0f, g = 1.0f, b = 1.0f;    // color
    float tx = 0.0f, ty = 0.0f;            // texture coord
    float nx = 0.0f, ny = 0.0f, nz = 0.0f; // normal

    VertexData() = default;
    VertexData(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    VertexData(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
    void setColor(const float r, const float g, const float b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

/**
 * Contains the mesh of an object.
 * The data within a container must be written contiguously!
 */
struct Mesh {
    std::vector<VertexData> vertices;
    std::vector<unsigned short> elements;
    float gl_draw_mode = GL_TRIANGLES;

    size_t elementCount() const { return elements.size(); }
    size_t vertexCount() const { return vertices.size(); }
    size_t totalVertexSize() const { // total size in bytes
        if (vertices.size() != 0)
            return sizeof(vertices[0]) * vertices.size();
        return 0;
    }
    size_t totalElementSize() const { // total size in bytes
        if (elements.size() != 0) {
            return sizeof(elements[0]) * elements.size();
        }
        return 0;
    }
    bool isElementObject() const { return elements.size() != 0; }
};

/**
 * @brief Store all neccessary data to draw the objects later without the meshes itself.
 */
struct Object {
    size_t number_vertices = 0;
    size_t number_elements = 0;
    float gl_draw_mode = GL_TRIANGLES;
    bool enabled = true;

    Object(size_t vert, size_t elem, float draw_mode) : number_vertices(vert), number_elements(elem), gl_draw_mode(draw_mode) {}
};

/**
 * @brief Group of drawable objects that shares a program and can be described in the same way.
 */
struct Subscene {
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo_static = 0, vbo_dynamic = 0;
    GLuint ibo_static = 0;

  private:
    std::vector<Mesh> models;
    std::vector<Object> objects;
    // size of subscene
    size_t total_element_size = 0;
    size_t total_vertex_size = 0;
    size_t vertex_count = 0;
    size_t element_count = 0;

  public:
    void add(Mesh m) {
        models.push_back(m);
        objects.push_back(Object(m.vertexCount(), m.elementCount(), m.gl_draw_mode));

        // update size
        total_vertex_size += m.totalVertexSize();
        total_element_size += m.totalElementSize();
        vertex_count += m.vertexCount();
        element_count += m.elementCount();
    }

    void disable(const int i) {
        if (objects.size() > i) {
            objects[i].enabled = false;
        }
    }

    void enableAll() {
        for (auto& o : objects) {
            o.enabled = true;
        }
    }

    size_t totalVertexSize() const { return total_vertex_size; }
    size_t totalElementSize() const { return total_element_size; }
    size_t vertexCount() const { return vertex_count; }
    size_t elementCount() const { return element_count; }
    const std::vector<Mesh>& getModels() const { return models; }
    const std::vector<Object>& getObjects() const { return objects; }
};

/**
 * @brief Create a list of elements that forms a sphere in gl.
 * @param radius WIP
 * @param accuracy number of stacks; 1/2 number of sectors
 */
Mesh createSphere(const float radius, const glm::vec3 center, const int accuracy);

/**
 * @brief Create a list of elements that forms a satellite (cube) in gl.
 * @param position Current position of the satellite (in real world coordinates).
 */
Mesh createSatellite();

/**
 * @brief Create a list of vertices and colors that forms an orbit in gl.
 */
Mesh createOrbit(const Orbit& orbit, const float scale, const glm::vec3 center);

} // namespace OpenGLPrimitives

#endif