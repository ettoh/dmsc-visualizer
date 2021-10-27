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

template <typename T>
struct GLBuffer {
    GLuint buffer_idx = 0u; // opengl index of this buffer
    GLenum usage;           // opengl usage of this buffer
    std::vector<T> values;

    GLBuffer(const GLenum usage = GL_STATIC_DRAW)
        : usage(usage) {}
    ~GLBuffer() { glDeleteBuffers(1, &buffer_idx); }

    size_t size() const { return values.size(); }
    size_t byte_size() const { return sizeof(T) * values.size(); }
    void gen() { glGenBuffers(1, &buffer_idx); }
    void pushToGPU() const {
        glBindBuffer(GL_ARRAY_BUFFER, buffer_idx);
        glBufferData(GL_ARRAY_BUFFER, byte_size(), &values[0], usage);
    }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Store all the data to describe a vertex. Must not contain additional data.
 */
struct VertexData {
    glm::vec3 position = glm::vec3(0.f);
    glm::vec4 color = glm::vec4(0.f);
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
    std::vector<GLuint> elements;
    size_t instance_count = 1;
    GLenum gl_draw_mode = GL_TRIANGLES;
    GLenum gl_element_type = GL_UNSIGNED_INT;
    GLint gl_program = 0; // which programm is used to shade this object
    GLint gl_vao = 0;
    std::string name = "";
    bool drawInstanced = false;

    size_t elementCount() const { return elements.size(); }
    size_t vertexCount() const { return vertices.size(); }
    bool isElementObject() const { return elements.size() != 0; }

    size_t totalVertexSize() const;            // total size in bytes
    size_t totalElementSize() const;           // total size in bytes
    std::vector<GLushort> elements_16() const; // converts the element vector to uint16
    std::vector<GLubyte> elements_8() const;   // converts the element vector to uint8

    /**
     * @brief Adds the vertices and elements of the given object to this object. All other properties (e.g. VAO,
     * name) of the given object are ignored. Elements are added with an offset corresponding to the current number
     * of vertices. So ensure that the data type for the elements offers enough space!
     */
    void add(const Object& obj) {
        GLuint offset = static_cast<GLuint>(vertices.size());
        vertices.insert(vertices.end(), obj.vertices.begin(), obj.vertices.end());
        for (const auto& e : obj.elements) {
            elements.push_back(e + offset);
        }
    }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Store all neccessary data to draw the objects later without the object data itself.
 */
struct ObjectInfo {
    size_t number_vertices = 0;
    size_t number_elements = 0;
    size_t number_instances = 0;
    size_t base_index = 0;
    size_t base_instance = 0;
    GLenum gl_draw_mode = GL_TRIANGLES;
    GLenum gl_element_type = GL_UNSIGNED_SHORT;
    GLuint gl_program = 0;
    GLuint gl_vao = 0;
    size_t offset_elements = 0;
    size_t offset_vertices = 0;
    std::string name = "";
    bool drawInstanced = false;
    bool enabled = true;

    ObjectInfo(size_t vert, size_t elem, GLenum draw_mode)
        : number_vertices(vert)
        , number_elements(elem)
        , gl_draw_mode(draw_mode) {}

    ObjectInfo(const Object& object) {
        gl_draw_mode = object.gl_draw_mode;
        number_elements = object.elementCount();
        number_vertices = object.vertexCount();
        number_instances = object.instance_count;
        name = object.name;
        drawInstanced = object.drawInstanced;
        gl_element_type = object.gl_element_type;
        gl_program = object.gl_program;
        gl_vao = object.gl_vao;
    }

    /**
     * @brief Objects can be drawn with different glsl-programs and different VAO's. In order to draw an object, we
     * have to tell GL which VAO/program shall be used. In case this VAO/program is different to currently used one,
     * GL has to perform state changes. We want to reduce these state changes by drawing all objects with the same
     * VAO/program successively. This is why we sort the Objects by their used VAO/program. We also prefer reduced
     * program swaps over VAO.
     */
    friend bool operator<(const ObjectInfo& l, const ObjectInfo& r) {
        if (l.gl_program != r.gl_program) {
            return l.gl_program < r.gl_program;
        } else {
            return l.gl_vao < r.gl_vao;
        }
    }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Creates a list of elements that form a sphere in gl.
 * @param accuracy number of stacks; 1/2 number of sectors
 */
Object createSphere(const float radius, const glm::vec3 center, const unsigned short accuracy,
                    const glm::vec4 color = glm::vec4(1.f));

/**
 * @brief Creates a list of elements that form a satellite (cube) in gl.
 * @param position Current position of the satellite (in real world coordinates).
 */
Object createSatellite();

/**
 * @brief Creates list of vertices that form a line.
 */
Object createLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color, bool dashed = false);

/**
 * @brief Creates a list of vertices and colors that form an orbit in gl.
 */
Object createOrbit(const Satellite& orbit, const float scale, const glm::vec3 center);

/**
 * @brief Creates a list of elements that form a pipe like shape (around y-axis). Centered at (0,0,0).
 */
Object createPipe(const float radius, const float height, const glm::vec4 color, const unsigned int sector_count = 15u);

/**
 * @brief Creates a list of elements that form a cone (around y-axis). Centered at (0,0,0); top points to (0, h/2,
 * 0).
 */
Object createCone(const float base_radius, const float height, const glm::vec4 color,
                  const unsigned short sector_count = 15u, const bool centered = false);

} // namespace OpenGLPrimitives
} // namespace dmsc

#endif
