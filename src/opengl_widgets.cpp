#include "opengl_widgets.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "dmsc/glm_include.hpp"
#include <fstream>
#include <iostream>

namespace dmsc {

using OpenGLPrimitives::Mesh;
using OpenGLPrimitives::Object;
using OpenGLPrimitives::Subscene;
using OpenGLPrimitives::VertexData;

// ------------------------------------------------------------------------------------------------

OpenGLWidget::OpenGLWidget() { init(); }

// ------------------------------------------------------------------------------------------------

OpenGLWidget::~OpenGLWidget() { destroy(); }

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::init() {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    window = glfwCreateWindow(1280, 720, "Dynamic Minimum Scan Cover - Visualizer", NULL, NULL);
    if (window == NULL)
        return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Resize event
    glfwSetWindowSizeCallback(window,
                              [](GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); });
    // Mouse wheel event
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        OpenGLWidget* p = (OpenGLWidget*)glfwGetWindowUserPointer(window);
        float zoom_per_deg = .03f * p->zoom * 2.5f; // depends on current zoom value to avoid endless scrolling for
                                                    // high- and too big jumps for small zoom values
        p->zoom += static_cast<float>(yoffset) * zoom_per_deg;
    });

    // Mouse click event
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        OpenGLWidget* p = (OpenGLWidget*)glfwGetWindowUserPointer(window);
        if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
            !ImGui::IsAnyItemHovered()) {

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                p->mouse_start_location = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
            } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                p->view *= p->camera_rotation;        // save rotation
                p->camera_rotation = glm::mat4(1.0f); // reset to avoid applying it twice
            }
        }
    });

    // Mouse move event
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        OpenGLWidget* p = (OpenGLWidget*)glfwGetWindowUserPointer(window);

        if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
            !ImGui::IsAnyItemHovered()) {

            int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (state == GLFW_PRESS) {
                glm::vec2 m1 = p->mouse_start_location;
                glm::vec2 m2 = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));

                int width, height;
                glfwGetWindowSize(window, &width, &height);

                // map coords to camera coords
                int h = height;
                int w = width;
                m1.x = -2 * m1.x / w + 1;
                m1.y = 2 * m1.y / h - 1;
                m2.x = -2 * m2.x / w + 1;
                m2.y = 2 * m2.y / h - 1;

                // bound coords if mouse moves out of widget
                m1 = glm::clamp(m1, -1.0f, 1.0f);
                m2 = glm::clamp(m2, -1.0f, 1.0f);

                // map screen coords to arcball
                glm::vec3 a1 = glm::vec3(m1.x, m1.y, sqrt(1 - (m1.x * m1.x)));
                glm::vec3 a2 = glm::vec3(m2.x, m2.y, sqrt(1 - (m2.x * m2.x)));
                a1 = glm::normalize(a1);
                a2 = glm::normalize(a2);

                // calculate rotation
                float rotation_angle = acos(std::min(1.0f, glm::dot(a1, a2)));
                glm::vec3 rotation_axis = glm::inverse(glm::mat3(p->view)) * glm::cross(a2, a1);
                p->camera_rotation = glm::rotate(glm::degrees(rotation_angle) * 0.025f, rotation_axis);
            }
        }
    });

    bool err = gladLoadGL() == 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // OpenGL-Version
    std::cout << "Open GL 3.3 needed. Given: ";
    std::cout << glGetString(GL_VERSION) << std::endl;

    // Read shader programs from files
    GLuint vertex_shader = createShader("shader/basic.vert", GL_VERTEX_SHADER);
    GLuint fragment_shader = createShader("shader/basic.frag", GL_FRAGMENT_SHADER);
    GLuint earth_frag_shader = createShader("shader/earth.frag", GL_FRAGMENT_SHADER);
    GLuint satellite_vert_shader = createShader("shader/satellite.vert", GL_VERTEX_SHADER);
    GLuint earth_vert_shader = createShader("shader/earth.vert", GL_VERTEX_SHADER);
    basic_program = createProgram(vertex_shader, fragment_shader);
    satellite_prog = createProgram(satellite_vert_shader, fragment_shader);
    earth_prog = createProgram(earth_vert_shader, earth_frag_shader);

    // create uniform buffer
    glGenBuffers(1, &vbo_uniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, vbo_uniforms);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // bind uniform vbo to programs
    GLuint index = glGetUniformBlockIndex(basic_program, "Global");
    glUniformBlockBinding(basic_program, index, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, vbo_uniforms);
    index = glGetUniformBlockIndex(satellite_prog, "Global");
    glUniformBlockBinding(satellite_prog, index, 1);
    index = glGetUniformBlockIndex(earth_prog, "Global");
    glUniformBlockBinding(earth_prog, index, 1);

    // load textures
    int w, h;
    int channels;
    unsigned char* image;
    const char* filename = "textures/earth_day.jpg";
    stbi_set_flip_vertically_on_load(1);
    image = stbi_load(filename, &w, &h, &channels, STBI_rgb);

    if (image == nullptr) {
        printf("Failed to load image %s\n", filename);
        assert(false);
        exit(EXIT_FAILURE);
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGB, // internalformat
                 w, h, 0,
                 GL_RGB, // target format
                 GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);

    // bind texture uniform
    const char* uniform_name = "texture"; // name of uniform in shader
    uniform_texture_location = glGetUniformLocation(earth_prog, uniform_name);
    if (uniform_texture_location == -1) {
        std::cout << "Could not bind uniform " << uniform_name << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::show(const PhysicalInstance& instance, const float t0) {
    visualizeInstance(instance);
    sim_time = t0;
    glm::vec3 clear_color = glm::vec3(0.15f, 0.15f, 0.15f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        buildGUI(); // Update gui

        // Prepare rendering
        ImGui::Render(); // State change
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render new frame
        renderScene();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap
        glfwSwapBuffers(window);
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::show(const PhysicalInstance& instance, const Solution& solution) {
    visualizeSolution(instance, solution);
    glm::vec3 clear_color = glm::vec3(0.15f, 0.15f, 0.15f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        buildGUI(); // Update gui

        // Prepare rendering
        ImGui::Render(); // State change
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render new frame
        renderScene();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap
        glfwSwapBuffers(window);
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::renderScene() {
    recalculate();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniform_texture_location, /*GL_TEXTURE*/ 0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw the scene:
    for (const Subscene& s : scene) {
        drawSubscene(s);
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::recalculate() {
    if (state == EMPTY) {
        return;
    }

    // relative time
    float diff = ImGui::GetIO().DeltaTime;
    // std::chrono::duration<double> diff = std::chrono::system_clock::now() - last_frame_time;
    // last_frame_time = std::chrono::system_clock::now();
    if (!paused)
        sim_time += diff * sim_speed;

    // Calc MVP
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    projection = glm::perspective(45.0f, 1.0f * vp[2] / vp[3], 0.1f, 10.0f);
    glm::mat4 scale = glm::scale(glm::vec3(1.0f) * zoom);
    glm::mat4 modelview = view * camera_rotation * scale;
    glm::mat4 projection = this->projection;
    glm::mat4 normal_proj = glm::transpose(glm::inverse(modelview));

    // push mvp to VBO
    glBindBuffer(GL_UNIFORM_BUFFER, vbo_uniforms);
    glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), 0, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(modelview));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(normal_proj));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // dynamic part of scene
    recalculateOrbitPositions();
    recalculateEdges();
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::recalculateOrbitPositions() {
    // move satellites
    std::vector<GLfloat> positions;
    size_t nmbr_vertecies = 0;
    if (problem_instance.getSatellites().size() != 0) // size() will fail if no orbits in instance
        // assume that all satellites have the same amount of vertices
        nmbr_vertecies = satellite_subscene->getObjects().at(0).number_vertices;
    for (const Satellite& o : problem_instance.getSatellites()) {
        glm::vec3 offset = o.cartesian_coordinates(sim_time) / real_world_scale;
        for (int i = 0; i < nmbr_vertecies; i++) {
            positions.push_back(offset.x);
            positions.push_back(offset.y);
            positions.push_back(offset.z);
        }
    }

    // push data to VBO
    if (positions.size() != 0) {
        size_t size_satellite_pos = sizeof(positions[0]) * positions.size();
        glBindBuffer(GL_ARRAY_BUFFER, satellite_subscene->vbo_dynamic);                   // set active
        glBufferData(GL_ARRAY_BUFFER, size_satellite_pos, &positions[0], GL_STREAM_DRAW); // push data
    }
}

// ------------------------------------------------------------------------------------------------

std::vector<Mesh> OpenGLWidget::createLines() {
    std::vector<Mesh> all_lines;

    // build edges
    for (uint32_t i = 0; i < problem_instance.getEdges().size(); i++) {
        Mesh edge_line;
        edge_line.gl_draw_mode = GL_LINES;
        const InterSatelliteLink& edge = problem_instance.getEdges().at(i);
        glm::vec3 sat1 = edge.getV1().cartesian_coordinates(sim_time) / real_world_scale;
        glm::vec3 sat2 = edge.getV2().cartesian_coordinates(sim_time) / real_world_scale;
        glm::vec3 color = glm::vec3(1.f);

        if (edge.isBlocked(sim_time)) { // edge can not be scanned
            color = glm::vec3(1.0f, 0.0f, 0.0f);
        } else { // edge can be scanned
            color = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (state == SOLUTION) {
            uint32_t next_edge = edge_order.prevailingEvent(sim_time).data;
            if (next_edge == i) { // edge is scanned next
                color = glm::vec3(1.0f, .75f, 0.0f);
            } else if (edge.isBlocked(sim_time) ||
                       !edge.canAlign(satellite_orientations[&edge.getV1()].previousEvent(sim_time),
                                      satellite_orientations[&edge.getV2()].previousEvent(sim_time), sim_time)) {

                color = glm::vec3(1.0f, 0.0f, 0.0f);
            } else {
                color = glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }

        edge_line = OpenGLPrimitives::createLine(sat1, sat2, color, edge.isOptional());
        all_lines.push_back(edge_line);
    }

    // build satellite orientations
    for (auto const& satellite : problem_instance.getSatellites()) {
        glm::vec3 position = satellite.cartesian_coordinates(sim_time) / real_world_scale;
        TimelineEvent<glm::vec3> last_orientation = satellite_orientations[&satellite].previousEvent(sim_time, false);
        TimelineEvent<glm::vec3> next_orientation = satellite_orientations[&satellite].prevailingEvent(sim_time, false);

        if (!last_orientation.isValid()) {
            last_orientation.t_begin = 0.f;
            last_orientation.data = glm::vec3(0.f);
        }

        if (!next_orientation.isValid()) {
            next_orientation.t_begin = 0.f;
            next_orientation.data = glm::vec3(0.f);
        }

        float angle = std::acos(glm::dot(last_orientation.data, next_orientation.data)); // [rad]
        float dt = sim_time - last_orientation.t_begin;

        glm::vec3 direction_vector = last_orientation.data;
        direction_vector = glm::rotate(direction_vector, std::min(angle, dt * satellite.getRotationSpeed()),
                                       glm::cross(direction_vector, next_orientation.data));

        all_lines.push_back(
            OpenGLPrimitives::createLine(position, position + direction_vector * 0.025f, glm::vec3(1.0f)));
    }

    return all_lines;
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::recalculateEdges() {
    std::vector<Mesh> line_meshes = createLines();

    // push data to gpu
    size_t offset_vertices = 0;
    glBindBuffer(GL_ARRAY_BUFFER, edge_subscene->vbo_dynamic);
    glBufferData(GL_ARRAY_BUFFER, edge_subscene->totalVertexSize(), 0, GL_STREAM_DRAW); // allocate memory
    for (const auto& model : line_meshes) {
        size_t object_size = model.totalVertexSize(); // size of all vertices in byte

        if (object_size != 0) {
            glBufferSubData(GL_ARRAY_BUFFER, offset_vertices, object_size, &model.vertices[0]);
            offset_vertices += object_size;
        }
    }
    // iterate through scan cover
    if (state == SOLUTION) {
        // hide all edges that have already been scanned
        for (uint32_t i = 0; i < problem_instance.getEdges().size(); i++) {
            auto range = scan_cover.equal_range(i);
            if (range.first == scan_cover.end()) {
                edge_subscene->disable(i); // edge is not part of the scan cover -> hide it
            } else {
                float latest_use = 0.f;
                for (auto it = range.first; it != range.second; ++it) {
                    latest_use = std::max(latest_use, it->second);
                }

                // edge will not be part of a communication anymore
                if (latest_use < sim_time) {
                    edge_subscene->disable(i); // actually: diables the i-th object in subscene
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::drawSubscene(const Subscene& subscene) {
    glUseProgram(subscene.program);
    glBindVertexArray(subscene.vao);

    // draw each object individually
    size_t offset = 0;
    size_t offset_elements = 0;
    for (const Object& o : subscene.getObjects()) {
        if (!o.enabled) {
            offset_elements += o.number_elements;
            offset += o.number_vertices;
            continue;
        }

        if (o.number_elements != 0) { // object is drawn by elements instead of vertices
            assert(offset <= INT_MAX || o.number_elements <= INT_MAX);
            glDrawElementsBaseVertex(o.gl_draw_mode, static_cast<GLint>(o.number_elements), GL_UNSIGNED_SHORT,
                                     (void*)(offset_elements * sizeof(GLushort)), static_cast<GLint>(offset));

            // skip elements and vertices of this object in the next draw call
            offset_elements += o.number_elements;
            offset += o.number_vertices;
        } else { // object is drawn by vertices
            assert(offset <= INT_MAX || o.number_vertices <= INT_MAX);
            glDrawArrays(o.gl_draw_mode, static_cast<GLint>(offset), static_cast<GLsizei>(o.number_vertices));
            offset += o.number_vertices;
        }
    }

    glBindVertexArray(0);
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::visualizeInstance(const PhysicalInstance& instance) {
    deleteInstance();
    state = INSTANCE;
    // copy so visualization does not depend on original instance
    problem_instance = instance;
    sim_time = 0.f;

    // 0. init subscenes
    scene.push_back(Subscene()); // todo ugly!
    scene.push_back(Subscene());
    scene.push_back(Subscene());
    scene.push_back(Subscene());
    Subscene& static_subscene = scene[0];
    Subscene& satellite_subscene = scene[1];
    Subscene& edge_subscene = scene[2];
    Subscene& earth_subscene = scene[3];
    static_subscene.program = basic_program;
    satellite_subscene.program = satellite_prog;
    edge_subscene.program = basic_program;
    earth_subscene.program = earth_prog;
    this->satellite_subscene = &satellite_subscene;
    this->edge_subscene = &edge_subscene;

    // todo generalize
    glGenBuffers(1, &static_subscene.vbo_static);
    glGenBuffers(1, &static_subscene.ibo_static);

    glGenBuffers(1, &earth_subscene.vbo_static);
    glGenBuffers(1, &earth_subscene.ibo_static);

    glGenBuffers(1, &satellite_subscene.vbo_static);
    glGenBuffers(1, &satellite_subscene.ibo_static);
    glGenBuffers(1, &satellite_subscene.vbo_dynamic);

    glGenBuffers(1, &edge_subscene.vbo_dynamic);
    glGenBuffers(1, &edge_subscene.ibo_static);

    // 1. build meshes and push them to the gpu
    Mesh sphere =
        OpenGLPrimitives::createSphere(problem_instance.getRadiusCentralMass() / real_world_scale, glm::vec3(0.0f), 35);
    earth_subscene.add(sphere);
    for (const Satellite& o : problem_instance.getSatellites()) {
        // Orbit
        Mesh orbit = OpenGLPrimitives::createOrbit(o, real_world_scale, glm::vec3(0.0f));
        static_subscene.add(orbit);

        // Satellite
        Mesh satellite = OpenGLPrimitives::createSatellite();
        satellite_subscene.add(satellite); // position is later set by shader
    }

    // 1.2 Edges & orientations
    std::vector<Mesh> line_meshes = createLines();
    for (const auto& mesh : line_meshes) {
        edge_subscene.add(mesh);
    }

    pushSceneToGPU();

    // 2. define format for data
    /* Create VAO that handle the vbo's and it's format.
     * All following VBO's and attribPointer will belong to this VAO (until a different VAO is bound). */
    glGenVertexArrays(1, &static_subscene.vao);
    glBindVertexArray(static_subscene.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_subscene.ibo_static);
    glBindBuffer(GL_ARRAY_BUFFER, static_subscene.vbo_static);
    glEnableVertexAttribArray(0); // vertices
    // (attribute, values per vertex, type, normalize?, stride (in byte), offset for the 1st element)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
    glEnableVertexAttribArray(1); // colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(GL_FLOAT) * 3));
    glBindVertexArray(0);

    glGenVertexArrays(1, &earth_subscene.vao);
    glBindVertexArray(earth_subscene.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, earth_subscene.ibo_static);
    glBindBuffer(GL_ARRAY_BUFFER, earth_subscene.vbo_static);
    glEnableVertexAttribArray(0); // vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
    glEnableVertexAttribArray(1); // texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(GL_FLOAT) * 6));
    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(GL_FLOAT) * 8));
    glBindVertexArray(0);

    glGenVertexArrays(1, &satellite_subscene.vao);
    glBindVertexArray(satellite_subscene.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, satellite_subscene.ibo_static);
    glBindBuffer(GL_ARRAY_BUFFER, satellite_subscene.vbo_static);
    glEnableVertexAttribArray(0); // vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
    glBindBuffer(GL_ARRAY_BUFFER, satellite_subscene.vbo_dynamic); // satellite positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &edge_subscene.vao);
    glBindVertexArray(edge_subscene.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_subscene.ibo_static);
    glBindBuffer(GL_ARRAY_BUFFER, edge_subscene.vbo_dynamic);
    glEnableVertexAttribArray(0); // vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
    glEnableVertexAttribArray(1); // colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(GL_FLOAT) * 3));
    glBindVertexArray(0);
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::visualizeSolution(const PhysicalInstance& instance, const Solution& solution) {
    visualizeInstance(instance);
    scan_cover = solution.scan_cover;
    satellite_orientations.clear();
    edge_order.clear();
    state = SOLUTION;
    sim_time = .0f;

    // build timeline for satellite orientations and edge order
    for (const auto& scan : solution.scan_cover) {
        if (scan.first >= problem_instance.getEdges().size()) {
            printf("Solution and instance does not match!\n");
            assert(false);
            exit(EXIT_FAILURE);
        }

        float t = scan.second; // time when edge is scheduled
        const InterSatelliteLink& isl = problem_instance.getEdges().at(scan.first);
        EdgeOrientation needed_orientation = isl.getOrientation(t);

        // add corresponding events for both satellites where they have to face in the needed direction in order to
        // perform the scan
        TimelineEvent<glm::vec3> orientation_sat1 = TimelineEvent<glm::vec3>(t, t, needed_orientation.sat1.direction);
        TimelineEvent<glm::vec3> orientation_sat2 = TimelineEvent<glm::vec3>(t, t, needed_orientation.sat2.direction);
        bool res_1 = satellite_orientations[&isl.getV1()].insert(orientation_sat1);
        bool res_2 = satellite_orientations[&isl.getV2()].insert(orientation_sat2);

        if (!res_1 || !res_2) {
            printf("The needed orientation for satellites can not be applied at t=%f!\n", t);
        }

        // fill edge order
        if (!edge_order.insert(TimelineEvent<uint32_t>(t, t, scan.first))) {
            printf("The edge with index %u could not be insterted into the edge order!\n", scan.first);
        }
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::pushSceneToGPU() {
    for (const Subscene& subscene : scene) {
        // allocate memory on gpu
        glBindBuffer(GL_ARRAY_BUFFER, subscene.vbo_static);                           // set active
        glBufferData(GL_ARRAY_BUFFER, subscene.totalVertexSize(), 0, GL_STATIC_DRAW); // allocate memory
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subscene.ibo_static);                   // set active
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, subscene.totalElementSize(), 0,
                     GL_STATIC_DRAW); // allocate memory

        // push data to gpu
        size_t offset_vertices = 0;
        size_t offset_elements = 0;
        for (const auto& model : subscene.getModels()) {
            size_t object_size = model.totalVertexSize(); // size of all vertices in byte

            if (object_size != 0) {
                // all static vertex data
                glBindBuffer(GL_ARRAY_BUFFER, subscene.vbo_static);
                glBufferSubData(GL_ARRAY_BUFFER, offset_vertices, object_size, &model.vertices[0]);
                offset_vertices += object_size;
            }

            // if object is defined by vertex id's, because multiple triangles uses the same vertices
            if (model.isElementObject()) {
                size_t object_element_size = model.totalElementSize(); // size of all vertex id's in byte
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subscene.ibo_static);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset_elements, object_element_size, &model.elements[0]);
                offset_elements += object_element_size;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::buildGUI() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Simulation control panel
    {
        ImGui::Begin("Simulation control panel"); // Create a window and append into it.

        ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
        if (ImGui::Button("Pause")) {
            paused = !paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            sim_time = 0.f;
            sim_speed = 1;
            edge_subscene->enableAll();
        }

        ImGui::InputInt("Speed", &sim_speed);

        int t = (int)sim_time;
        ImGui::Text("t = %+id %ih %imin %isec", (t / 86400), (t / 3600) % 24, (t / 60) % 60, t % 60);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::destroy() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    deleteInstance();
    glDeleteProgram(basic_program);
    glDeleteProgram(satellite_prog);
    glDeleteTextures(1, &texture_id);

    glfwDestroyWindow(window);
    glfwTerminate();
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::deleteInstance() {
    state = EMPTY;
    // free rescources on GPU
    for (const auto& subscene : scene) {
        glDeleteVertexArrays(1, &subscene.vao);
        glDeleteBuffers(1, &subscene.ibo_static);
        glDeleteBuffers(1, &subscene.vbo_dynamic);
        glDeleteBuffers(1, &subscene.vbo_static);
    }
    satellite_subscene = nullptr;
    edge_subscene = nullptr;
    scene.clear();
}

// ------------------------------------------------------------------------------------------------

std::string OpenGLWidget::readShader(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file) {
        printf("Failed to load shader %s\n", file_name.c_str());
        assert(false);
        exit(EXIT_FAILURE);
    }

    std::string shader = "";
    std::string buffer;
    while (getline(file, buffer)) {
        shader.append(buffer + "\n");
    }

    return shader;
}

// ------------------------------------------------------------------------------------------------

GLuint OpenGLWidget::createShader(const std::string& file_name, GLenum shader_type) {
    GLint compile_flag = GL_FALSE;
    GLuint shader = glCreateShader(shader_type);

    // Read source code
    std::string sourcecode = readShader(file_name);
    const GLchar* source = sourcecode.c_str();
    glShaderSource(shader, 1, &source, NULL); // read an array of source files

    // Compile shader
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS,
                  &compile_flag); // get infos about the shader object and store it into compile_ok
    if (!compile_flag) {
        std::cout << "Error in compiling shader: '" + file_name + "'!" << std::endl;
    }

    return shader;
}

// ------------------------------------------------------------------------------------------------

GLuint OpenGLWidget::createProgram(const GLuint vertex_shader, const GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    GLint link_ok = GL_FALSE;
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        // todo unified error handling
        throw std::runtime_error("Error in glLinkProgram");
    }

    return program;
}

} // namespace dmsc
