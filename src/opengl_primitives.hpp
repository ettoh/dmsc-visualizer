#ifndef DMSC_OPENGL_PRIMITIVES
#define DMSC_OPENGL_PRIMITIVES

#define _USE_MATH_DEFINES

#include "dmsc/glm_include.hpp"
#include "dmsc/satellite.hpp"
#include <glad/glad.h>
#include <math.h>
#include <string>
#include <vector>

namespace dmsc {
namespace OpenGLPrimitives {

/**
 * @brief Store all the data to describe a vertex. Must not contain additional data.
 */
struct VertexData {
    glm::vec3 position = glm::vec3(0.f);
    glm::vec3 color = glm::vec3(0.f);
    glm::vec2 texture = glm::vec2(0.f);
    glm::vec3 normal = glm::vec3(0.f);

    VertexData() = default;
    VertexData(const float x, const float y, const float z) { position = glm::vec3(x, y, z); }
    VertexData(const glm::vec3& v) { position = v; }
};

// ------------------------------------------------------------------------------------------------

/**
 * Contains the mesh of an object.
 * The data within a container must be written contiguously!
 */
struct Object {
    std::vector<VertexData> vertices;
    std::vector<unsigned short> elements;
    std::vector<glm::mat4> object_transformations; // one for each object instance
    GLenum gl_draw_mode = GL_TRIANGLES;
    GLint gl_program = 0; // which programm is used to shade this object
    std::string name = "default";

    size_t elementCount() const { return elements.size(); }
    size_t vertexCount() const { return vertices.size(); }
    size_t instanceCount() const { return object_transformations.size(); }

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

// ------------------------------------------------------------------------------------------------

/**
 * @brief Store all neccessary data to draw the objects later without the object data itself.
 */
struct ObjectInfo {
    size_t number_vertices = 0;
    size_t number_elements = 0;
    size_t number_instances = 1;
    size_t base_index = 0;
    size_t base_instance = 0;
    GLenum gl_draw_mode = GL_TRIANGLES;
    size_t offset = 0;
    std::string name = "default";
    bool enabled = true;

    ObjectInfo(size_t vert, size_t elem, GLenum draw_mode)
        : number_vertices(vert)
        , number_elements(elem)
        , gl_draw_mode(draw_mode) {}

    ObjectInfo(const Object& object) {
        gl_draw_mode = object.gl_draw_mode;
        number_elements = object.elementCount();
        number_vertices = object.vertexCount();
        number_instances = object.instanceCount();
        name = object.name;
    }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Group of drawable objects that shares a program and can be described in the same way.
 */
struct Subscene {
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo_static = 0, vbo_dynamic = 0;
    GLuint ibo_static = 0;

  private:
    std::vector<Object> objects;
    std::vector<ObjectInfo> object_info;

    // size of subscene
    size_t total_element_size = 0;
    size_t total_vertex_size = 0;
    size_t vertex_count = 0;
    size_t element_count = 0;

  public:
    void add(Object obj) {
        objects.push_back(obj);
        object_info.push_back(ObjectInfo(obj.vertexCount(), obj.elementCount(), obj.gl_draw_mode));

        // update size
        total_vertex_size += obj.totalVertexSize();
        total_element_size += obj.totalElementSize();
        vertex_count += obj.vertexCount();
        element_count += obj.elementCount();
    }

    void setEnabled(const size_t i, const bool enabled) {
        if (object_info.size() > i) {
            object_info[i].enabled = enabled;
        }
    }

    void setAllEnabled(const bool enabled) {
        for (auto& obj_info : object_info) {
            obj_info.enabled = enabled;
        }
    }

    void clearObjectData() {
        objects.clear();
        objects.shrink_to_fit();
    }

    // GETTER
    size_t totalVertexSize() const { return total_vertex_size; }
    size_t totalElementSize() const { return total_element_size; }
    size_t vertexCount() const { return vertex_count; }
    size_t elementCount() const { return element_count; }
    size_t objectCount() const { return object_info.size(); }
    const std::vector<Object>& getObjects() const { return objects; }
    const std::vector<ObjectInfo>& getObjectInfo() const { return object_info; }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Create a list of elements that form a sphere in gl.
 * @param radius WIP
 * @param accuracy number of stacks; 1/2 number of sectors
 */
Object createSphere(const float radius, const glm::vec3 center, const unsigned short accuracy,
                    const glm::vec3 color = glm::vec3(1.f, 1.f, 1.f));

/**
 * @brief Create a list of elements that form a satellite (cube) in gl.
 * @param position Current position of the satellite (in real world coordinates).
 */
Object createSatellite();

/**
 * @brief Create list of vertices that form a line.
 */
Object createLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color, bool dashed = false);

/**
 * @brief Create a list of vertices and colors that form an orbit in gl.
 */
Object createOrbit(const Satellite& orbit, const float scale, const glm::vec3 center);

/**
 * @brief Create a list of elements that form a pipe like shape (around y-axis). Centered at (0,0,0).
 */
Object createPipe(const float radius, const float height, const glm::vec3 color, const unsigned int sector_count = 15u);

/**
 * @brief Create a list of elements that form a cone (around y-axis). Centered at (0,0,0); top points to (0, h/2, 0).
 */
Object createCone(const float base_radius, const float height, const glm::vec3 color,
                  const unsigned short sector_count = 15u);

} // namespace OpenGLPrimitives
} // namespace dmsc

#endif
