#include "dmsc/instance.hpp"
#include "dmsc/solver.hpp" // solution data type
#include "opengl_primitives.hpp"
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace dmsc {

constexpr auto PI2 = 2 * M_PI; // 2pi

class OpenGLWidget {
  public:
    OpenGLWidget();
    ~OpenGLWidget();
    OpenGLWidget(const OpenGLWidget&) = delete;
    OpenGLWidget& operator=(const OpenGLWidget&) = delete;
    void show(const PhysicalInstance& instance, const float t0 = 0.f);
    void show(const PhysicalInstance& instance, const Solution& solution);

    enum VisualisationState { EMPTY, INSTANCE, SOLUTION };

  private:
    void init();
    void destroy();
    void buildGUI();
    void renderScene();
    void drawSubscene(const OpenGLPrimitives::Subscene& subscene);
    void recalculate();
    void recalculateOrbitPositions();
    std::vector<OpenGLPrimitives::Object> createLines();
    void recalculateEdges();
    void deleteInstance();
    void pushSceneToGPU();

    /**
     * @brief Convert a given instance with orbits and communications between the satellites into an opengl
     * scene with vertices and display it until the next call.
     */
    void visualizeInstance(const PhysicalInstance& instance);

    /**
     * @brief Visualize a given solution for the current displayed instance.
     */
    void visualizeSolution(const PhysicalInstance& instance, const Solution& solution);

    /**
     * @brief Read source code for a shader from a local file.
     * @param file_name Relative path to the file.
     * @return String that contains the shader source code.
     */
    std::string readShader(const std::string& file_name);

    /**
     * @brief Compile given source code into a shader.
     * @param file_name Relative path to the file that contains the source code.
     * @param shader_type GL constant that determines the shader type.
     * @return A compiled shader object.
     */
    GLuint createShader(const std::string& file_name, GLenum shader_type);

    /**
     * @brief Link two shader to an opengl program.
     * @param vertex_shader
     * @param fragment_shader
     * @return Linked opengl program.
     */
    GLuint createProgram(const GLuint vertex_shader, const GLuint fragment_shader);

    static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

  private:
    GLFWwindow* window = nullptr;
    const float real_world_scale = 7000.0f;
    // Handler
    GLuint basic_program = 0, satellite_prog = 0, earth_prog = 0;
    GLuint vbo_uniforms = 0;
    GLuint texture_id = 0;
    GLint uniform_texture_location = 0;
    // view and camera
    float zoom = 1.0f;
    glm::vec2 camera_rotation_angle_offset = glm::vec2(.0f, .0f);
    glm::vec2 camera_rotation_angle = glm::vec2(.0f, .0f);
    glm::vec2 mouse_start_location = glm::vec2(0.0f);
    glm::vec3 camera_init_position = glm::vec3(0.f, 0.f, 2.f);
    bool is_mouse_pressed = false;
    glm::mat4 projection = glm::perspective(45.0f, 1.0f * 1280 / 720, 0.1f, 10.0f);
    glm::mat4 view = glm::mat4(1.f);
    // scene
    int state = VisualisationState::EMPTY;
    PhysicalInstance problem_instance = PhysicalInstance();
    std::vector<OpenGLPrimitives::Subscene> scene;
    float sim_time = 0.0f;
    int sim_speed = 1;
    bool paused = false; // if true, the simulations is paused
    OpenGLPrimitives::Subscene* satellite_subscene = nullptr;
    OpenGLPrimitives::Subscene* edge_subscene = nullptr;
    OpenGLPrimitives::Subscene* earth_subscene = nullptr;
    OpenGLPrimitives::Subscene* static_subscene = nullptr;
    size_t edgescene_com_start = ~0u; // object ID where the edges for scheduled communications begin
    size_t edgescene_com_end = ~0u;   // object ID where the edges for scheduled communications end
    // solution
    std::map<const Satellite*, Timeline<glm::vec3>> satellite_orientations;
    ScanCover scan_cover;
    Timeline<uint32_t> edge_order;
};

} // namespace dmsc
