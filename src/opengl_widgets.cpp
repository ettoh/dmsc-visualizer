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

using OpenGLPrimitives::Object;
using OpenGLPrimitives::ObjectInfo;
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

        // ignore every input except left mouse button
        if (button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        // if clicked inside the visualization (except UI), begin camera movement - if mouse button released (no matter
        // where), stop it
        switch (action) {
        case GLFW_PRESS:
            if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
                !ImGui::IsAnyItemHovered()) {
                p->is_mouse_pressed = true;
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                p->mouse_start_location = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
            }
            break;
        case GLFW_RELEASE:
            p->camera_rotation_angle += p->camera_rotation_angle_offset;
            p->camera_rotation_angle_offset = glm::vec2(0.f);
            p->camera_rotation_angle.x = std::fmod(p->camera_rotation_angle.x, static_cast<float>(M_PI) * 2.f);
            p->is_mouse_pressed = false;
            break;
        default:
            break;
        }
    });

    // Mouse move event
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        OpenGLWidget* p = (OpenGLWidget*)glfwGetWindowUserPointer(window);

        if (p->is_mouse_pressed) {
            glm::vec2 diff = glm::vec2(xpos, ypos) - p->mouse_start_location;
            int screen_width, screen_height;
            glfwGetWindowSize(window, &screen_width, &screen_height);
            // moving the mouse along half the screen size rotates the scene by 90 degrees
            glm::vec2 angle = (static_cast<float>(M_PI) / 2.f) * diff / glm::vec2(screen_width / 2, -screen_height / 2);
            // as long as the mouse button is not released, the changed rotation is applied as an offset
            p->camera_rotation_angle_offset = angle;
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
    glEnable(GL_PRIMITIVE_RESTART);
    glLineWidth(1.5f);
    glPrimitiveRestartIndex(MAX_ELEMENT_ID);

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
    loadTextures("earth_day", uniform_texture_location[0], "textures/earth_day.jpg", texture_id[0]);
    loadTextures("earth_water", uniform_texture_location[1], "textures/earth_water.jpg", texture_id[1]);
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::loadTextures(const char* uniform_name, GLint& tex_location, const char* file, GLuint& id) {
    int w, h;
    int channels;
    unsigned char* image;
    stbi_set_flip_vertically_on_load(1);
    image = stbi_load(file, &w, &h, &channels, STBI_rgb);

    if (image == nullptr) {
        printf("Failed to load image %s\n", file);
        assert(false);
        exit(EXIT_FAILURE);
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGB, // internalformat
                 w, h, 0,
                 GL_RGB, // target format
                 GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);

    // bind texture uniform
    tex_location = glGetUniformLocation(earth_prog, uniform_name);
    if (tex_location == -1) {
        std::cout << "Could not bind uniform " << uniform_name << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::show(const PhysicalInstance& instance, const float t0) {
    prepareInstanceScene(instance);
    sim_time = t0;
    openWindow();
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::show(const PhysicalInstance& instance, const Solution& solution) {
    prepareSolutionScene(instance, solution);
    openWindow();
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::openWindow() {
    glm::vec3 clear_color = glm::vec3(0.03f);

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

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniform_texture_location[0], /*GL_TEXTURE*/ 0);
    glBindTexture(GL_TEXTURE_2D, texture_id[0]);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(uniform_texture_location[1], /*GL_TEXTURE*/ 1);
    glBindTexture(GL_TEXTURE_2D, texture_id[1]);

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
    if (!paused)
        sim_time += diff * sim_speed;

    // sun rotation
    float sun_angle = sim_time * 0.000290f; // 6h -> one turn around the earth
    glm::mat4 sun_rotation = glm::rotate(glm::mat4(1.f), sun_angle, glm::vec3(0.f, 1.f, 0.f));

    /* Camera circumnavigating around the central mass.
     * Instead of performing two rotations for the camera, the rotation around the y-axis is done by rotation the entire
     * world itself. */
    glm::vec2 delta = camera_rotation_angle + camera_rotation_angle_offset;
    // the earth can not be circumnavigated along the poles.
    float max_angle_y = static_cast<float>(M_PI) / 2.f - 0.1f;
    delta.y = glm::clamp(delta.y, -max_angle_y, max_angle_y);
    camera_rotation_angle.y = glm::clamp(camera_rotation_angle.y, -max_angle_y, max_angle_y);

    // calc view matrix
    glm::mat4 camera_rotation = glm::rotate(glm::mat4(1.f), delta.y, glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 world_rotation = glm::rotate(glm::mat4(1), delta.x, glm::vec3(0.f, 1.f, 0.f));
    glm::vec4 camera_position = camera_rotation * glm::vec4(camera_init_position, 0.f);
    view = glm::lookAt(glm::vec3(camera_position), glm::vec3(0.0f, 0.0f, 0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f)); // (position, look at, up)

    // Calc MVP
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    projection = glm::perspective(45.0f, 1.0f * viewport[2] / viewport[3], 0.1f, 10.0f);
    glm::mat4 scale = glm::scale(glm::vec3(zoom));
    glm::mat4 model = world_rotation;
    // glm::mat4 normal_proj = glm::transpose(glm::inverse(modelview));

    // push mvp to VBO
    glBindBuffer(GL_UNIFORM_BUFFER, vbo_uniforms);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(glm::mat4), 0, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(scale));
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(sun_rotation));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // dynamic part of scene
    recalculateOrbitPositions();
    recalculateEdges();
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::recalculateOrbitPositions() {
    // move satellites
    std::vector<glm::mat4> transformations;
    transformations.reserve(problem_instance.getSatellites().size());

    for (const Satellite& o : problem_instance.getSatellites()) {
        glm::vec3 position = o.cartesian_coordinates(sim_time) / real_world_scale;
        glm::mat4 translation = glm::translate(position);
        glm::mat4 scale = glm::inverse(glm::scale(glm::vec3(-zoom))); // ignore zoom for satellites
        transformations.push_back(translation * scale);
    }

    // push data to VBO
    if (transformations.size() != 0) {
        size_t size_transformation = sizeof(transformations[0]) * transformations.size();
        glBindBuffer(GL_ARRAY_BUFFER, satellite_subscene->vbo_dynamic);                          // set active
        glBufferData(GL_ARRAY_BUFFER, size_transformation, &transformations[0], GL_STREAM_DRAW); // push data
    }
}

// ------------------------------------------------------------------------------------------------

std::vector<Object> OpenGLWidget::createLines() {
    std::vector<Object> all_lines;

    // build ISL network
    for (uint32_t i = 0; i < problem_instance.islCount(); i++) {
        const InterSatelliteLink& edge = problem_instance.getISLs().at(i);
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

        Object edge_line = OpenGLPrimitives::createLine(sat1, sat2, color);
        all_lines.push_back(edge_line);
    }

    // build schedules communications
    for (const auto& c : problem_instance.scheduled_communications) {
        glm::vec3 sat1 = problem_instance.getSatellites()[c.first].cartesian_coordinates(sim_time) / real_world_scale;
        glm::vec3 sat2 = problem_instance.getSatellites()[c.second].cartesian_coordinates(sim_time) / real_world_scale;
        Object communication_line = OpenGLPrimitives::createLine(sat1, sat2, glm::vec3(.55f, .1f, 1.f), true);
        all_lines.push_back(communication_line);
    }
    edgescene_com_start = problem_instance.islCount();
    edgescene_com_end = edgescene_com_start + problem_instance.scheduled_communications.size() - 1;

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
    std::vector<Object> line_meshes = createLines();

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
        for (uint32_t i = 0; i < problem_instance.getISLs().size(); i++) {
            auto range = scan_cover.equal_range(i);
            if (range.first == scan_cover.end()) {
                edge_subscene->setEnabled(i, false); // edge is not part of the scan cover -> hide it
            } else {
                float latest_use = 0.f;
                for (auto it = range.first; it != range.second; ++it) {
                    latest_use = std::max(latest_use, it->second);
                }

                // edge will not be part of a communication anymore
                if (latest_use < sim_time) {
                    edge_subscene->setEnabled(i, false); // actually: diables the i-th object in subscene
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

void OpenGLWidget::drawSubscene(const Subscene& subscene) {
    glUseProgram(subscene.program);
    glBindVertexArray(subscene.vao);

    if (&subscene == satellite_subscene) {
        if (!subscene.getObjectInfo()[0].enabled) {
            return;
        }

        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLint>(subscene.elementCount()), GL_UNSIGNED_SHORT,
                                (void*)(0), problem_instance.getSatellites().size());
        glBindVertexArray(0);
        return;
    }

    // draw each object individually
    size_t offset = 0;
    size_t offset_elements = 0;
    for (const ObjectInfo& o : subscene.getObjectInfo()) {
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

void OpenGLWidget::prepareInstanceScene(const PhysicalInstance& instance) {
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
    this->earth_subscene = &earth_subscene;
    this->static_subscene = &static_subscene;

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
    Object sphere =
        OpenGLPrimitives::createSphere(problem_instance.getRadiusCentralMass() / real_world_scale, glm::vec3(0.0f), 35);
    earth_subscene.add(sphere);
    Object all_orbits;
    all_orbits.gl_draw_mode = GL_LINE_LOOP;
    for (const Satellite& o : problem_instance.getSatellites()) {
        // Orbit
        Object orbit = OpenGLPrimitives::createOrbit(o, real_world_scale, glm::vec3(0.0f));

        // the number of vertices is limited by the range of GL_unsigned_short (type of indices)
        if (all_orbits.vertices.size() + orbit.vertices.size() >= MAX_ELEMENT_ID) {
            static_subscene.add(all_orbits);
            all_orbits = Object();
            all_orbits.gl_draw_mode = GL_LINE_LOOP;
        }

        size_t offset = all_orbits.vertices.size(); // index 0 in orbit object has to refer the correct vertex
        all_orbits.vertices.insert(all_orbits.vertices.end(), orbit.vertices.begin(), orbit.vertices.end());
        all_orbits.elements.reserve(all_orbits.elements.capacity() + orbit.elements.size());
        all_orbits.elements.push_back(MAX_ELEMENT_ID); // restart GL_LINE_LOOP

        for (const auto& i : orbit.elements) {
            all_orbits.elements.push_back(i + static_cast<GLushort>(offset));
        }
    }
    static_subscene.add(all_orbits);

    // Satellite
    // one sphere is enough - it will be reused (instanced rendering)
    Object satellite = OpenGLPrimitives::createSatellite();
    satellite_subscene.add(satellite); // position is later set by shader

    // 1.2 Edges & orientations
    std::vector<Object> line_meshes = createLines();
    for (const auto& mesh : line_meshes) {
        edge_subscene.add(mesh);
    }

    pushSceneToGPU();

    // delete vertex data (it is now stored in the gpu memory)
    earth_subscene.clearObjectData();
    static_subscene.clearObjectData();
    satellite_subscene.clearObjectData();
    edge_subscene.clearObjectData();

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
    glBindBuffer(GL_ARRAY_BUFFER, satellite_subscene.vbo_dynamic); // satellite transformation
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    // maximum size for vertexAttr is 4. So we split the 4x4matrix into 4x vec4
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)(0));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)(sizeof(float) * 4));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)(sizeof(float) * 8));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)(sizeof(float) * 12));
    glVertexAttribDivisor(1, 1); // update transformation attr. every 1 instance instead of every vertex
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
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

void OpenGLWidget::prepareSolutionScene(const PhysicalInstance& instance, const Solution& solution) {
    prepareInstanceScene(instance);
    scan_cover = solution.scan_cover;
    satellite_orientations.clear();
    edge_order.clear();
    state = SOLUTION;
    sim_time = .0f;

    // build timeline for satellite orientations and edge order
    for (const auto& scan : solution.scan_cover) {
        if (scan.first >= problem_instance.getISLs().size()) {
            printf("Solution and instance does not match!\n");
            assert(false);
            exit(EXIT_FAILURE);
        }

        float t = scan.second; // time when edge is scheduled
        const InterSatelliteLink& isl = problem_instance.getISLs().at(scan.first);
        EdgeOrientation needed_orientation = isl.getOrientation(t);

        // add corresponding events for both satellites where they have to face in the needed direction in order to
        // perform the scan
        TimelineEvent<glm::vec3> orientation_sat1 = TimelineEvent<glm::vec3>(t, t, needed_orientation.first);
        TimelineEvent<glm::vec3> orientation_sat2 = TimelineEvent<glm::vec3>(t, t, needed_orientation.second);
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
        for (const auto& model : subscene.getObjects()) {
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

        ImGui::SetWindowSize(ImVec2(350, 300));

        ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
        const char* btn_text = paused ? "Play" : "Pause";
        if (ImGui::Button(btn_text)) {
            paused = !paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            sim_time = 0.f;
            sim_speed = 1;
            edge_subscene->setAllEnabled(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset camera")) {
            camera_rotation_angle = glm::vec2(0.f);
            zoom = 1.f;
        }

        ImGui::InputInt("Speed", &sim_speed);

        int t = (int)sim_time;
        ImGui::Text("t = %+id %ih %imin %isec", (t / 86400), (t / 3600) % 24, (t / 60) % 60, t % 60);

        if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_None)) {
            static bool hide_satellites = false;
            if (ImGui::Checkbox("Hide satellites", &hide_satellites)) {
                satellite_subscene->setAllEnabled(!hide_satellites);
            }

            static bool hide_earth = false;
            if (ImGui::Checkbox("Hide earth", &hide_earth)) {
                earth_subscene->setAllEnabled(!hide_earth);
            }

            static bool hide_orbits = false;
            if (ImGui::Checkbox("Hide orbits", &hide_orbits)) {
                static_subscene->setAllEnabled(!hide_orbits);
            }

            static bool hide_isl = false;
            if (ImGui::Checkbox("Hide ISL-network", &hide_isl)) {
                for (size_t i = 0; i < edgescene_com_start; i++) {
                    edge_subscene->setEnabled(i, !hide_isl);
                }
            }

            static bool hide_comms = false;
            if (ImGui::Checkbox("Hide scheduled communications", &hide_comms)) {
                for (size_t i = edgescene_com_start; i <= edgescene_com_end; i++) {
                    edge_subscene->setEnabled(i, !hide_comms);
                }
            }

            static bool hide_orientations = false;
            if (ImGui::Checkbox("Hide satellite orientations", &hide_orientations)) {
                for (size_t i = edgescene_com_end + 1; i < edge_subscene->objectCount(); i++) {
                    edge_subscene->setEnabled(i, !hide_orientations);
                }
            }
        }

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
    glDeleteTextures(1, &texture_id[0]);
    glDeleteTextures(1, &texture_id[1]);
    glDeleteTextures(1, &texture_id[2]);

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
    earth_subscene = nullptr;
    static_subscene = nullptr;

    edgescene_com_start = ~0u;
    edgescene_com_end = ~0u;

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
        printf("Error in glLinkProgram\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    return program;
}

} // namespace dmsc
