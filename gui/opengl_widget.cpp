#include "opengl_widget.h"
#include "../instance/vector3d.h"
#include "opengl_toolkit.h"
#include <QImage>
#include <QOpenGLTexture>
#include <QTimer>
#include <QWheelEvent>
#include <iostream>

using OpenGLPrimitives::Mesh;
using OpenGLPrimitives::Object;
using OpenGLPrimitives::Subscene;
using OpenGLPrimitives::VertexData;

OpenGLWidget::OpenGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMouseTracking(false); // track mouse only if mouse button is pressed
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(.08f, .08f, .08f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    std::cout << "Open GL 3.3 needed. Given: ";
    std::cout << glGetString(GL_VERSION) << std::endl;

    OpenGLToolkit toolkit = OpenGLToolkit();
    GLint link_ok = GL_FALSE;
    GLuint vertex_shader = toolkit.createShader("shaders/basic.vert", GL_VERTEX_SHADER);
    GLuint fragment_shader = toolkit.createShader("shaders/basic.frag", GL_FRAGMENT_SHADER);
    GLuint earth_frag_shader = toolkit.createShader("shaders/earth.frag", GL_FRAGMENT_SHADER);
    GLuint satellite_vert_shader = toolkit.createShader("shaders/satellite.vert", GL_VERTEX_SHADER);
    GLuint earth_vert_shader = toolkit.createShader("shaders/earth.vert", GL_VERTEX_SHADER);

    basic_program = toolkit.createProgram(vertex_shader, fragment_shader);
    satellite_prog = toolkit.createProgram(satellite_vert_shader, fragment_shader);
    earth_prog = toolkit.createProgram(earth_vert_shader, earth_frag_shader);

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
    const QImage earth_texture = QImage(QString("bin/earth_day.png")).mirrored();
    if (earth_texture.isNull()) {
        throw std::runtime_error("Could not load earth texture!");
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA, // internalformat
                 earth_texture.width(), earth_texture.height(), 0,
                 GL_BGRA, // target format
                 GL_UNSIGNED_INT_8_8_8_8_REV, earth_texture.constBits());

    // bind texture uniform
    const char* uniform_name = "texture"; // name of uniform in shader
    uniform_texture_location = glGetUniformLocation(earth_prog, uniform_name);
    if (uniform_texture_location == -1) {
        std::cout << "Could not bind uniform " << uniform_name << std::endl;
    }

    // start draw loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &OpenGLWidget::drawLoop);
    timer->start(10);
}

void OpenGLWidget::resizeGL(int w, int h) {
    // update projection matrix
    projection = glm::perspective(45.0f, 1.0f * w / h, 0.1f, 10.0f);
}

void OpenGLWidget::paintGL() {
    recalculate();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniform_texture_location, /*GL_TEXTURE*/ 0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw the scene:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (const Subscene& s : scene) {
        drawSubscene(s);
    }

    // FPS-Counter
    fps_counter++;
    std::chrono::duration<double> diff = std::chrono::system_clock::now() - last_fps_update;
    if (diff.count() >= 1.0) { // update every second
        emit fpsChanged(fps_counter);
        fps_counter = 0;
        last_fps_update = std::chrono::system_clock::now();
    }
}

void OpenGLWidget::visualizeInstance(const ProblemInstance& instance) {
    makeCurrent();
    deleteInstance();
    state = INSTANCE;
    // copy so visualization does not depend on original instance
    problem_instance = ProblemInstance(instance);
    last_frame_time = std::chrono::system_clock::now();
    time = 0;

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
    Mesh sphere = OpenGLPrimitives::createSphere(problem_instance.getRadiusCentralMass() / real_world_scale,
                                                 glm::vec3(0.0f), 35);
    earth_subscene.add(sphere);
    for (const Orbit& o : problem_instance.orbits) {
        // Orbit
        Mesh orbit = OpenGLPrimitives::createOrbit(o, real_world_scale, glm::vec3(0.0f));
        static_subscene.add(orbit);

        // Satellite
        Mesh satellite = OpenGLPrimitives::createSatellite();
        satellite_subscene.add(satellite); // position is later set by shader
    }

    // 1.2 Edges & orientations
    // 1 line per edge + 1 line for each; 2 vertices per line
    size_t size = 2 * (problem_instance.edges.size() + problem_instance.orbits.size());
    for (int i = 0; i < size; i += 2) {
        Mesh edge;
        edge.gl_draw_mode = GL_LINES;
        edge.elements.push_back(i);
        edge.elements.push_back(i + 1);
        edge_subscene.add(edge);
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

    doneCurrent();
}

void OpenGLWidget::visualizeSolution(const ScanCover& scan_cover) {
    solution = ScanCover(scan_cover);
    if (solution.size() == 0)
        return;

    state = SOLUTION;
    last_frame_time = std::chrono::system_clock::now();
    time = .0f;
    current_scan = 0;
    if (edge_subscene != nullptr)
        edge_subscene->enableAll();

    // build timetable for satellite orientations
    for (auto const& s : solution) {
        const Edge& e = problem_instance.edges.at(s.edge_index);
        satellite_orientations.add(&e.getV1(), s.edge_orientation.sat1);
        satellite_orientations.add(&e.getV2(), s.edge_orientation.sat2);
    }
}

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
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset_elements, object_element_size,
                                &model.elements[0]);
                offset_elements += object_element_size;
            }
        }
    }
}

void OpenGLWidget::drawSubscene(const Subscene& subscene) {
    glUseProgram(subscene.program);
    glBindVertexArray(subscene.vao);

    // draw each object individually
    size_t offset = 0;
    size_t offset_elements = 0;
    for (const Object& o : subscene.getObjects()) {
        if (o.number_elements != 0) { // object is drawn by elements instead of vertices
            if (o.enabled) {
                assert(offset <= INT_MAX || o.number_elements <= INT_MAX);
                glDrawElementsBaseVertex(o.gl_draw_mode, static_cast<GLint>(o.number_elements), GL_UNSIGNED_SHORT,
                                         (void*)(offset_elements * sizeof(GLushort)), static_cast<GLint>(offset));
            }

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

void OpenGLWidget::drawLoop() {
    update(); // QT widget function
}

void OpenGLWidget::recalculate() {
    if (state == EMPTY) {
        return;
    }

    // relative time
    std::chrono::duration<double> diff = std::chrono::system_clock::now() - last_frame_time;
    last_frame_time = std::chrono::system_clock::now();
    if (!paused)
        time += diff.count() * time_boost; // [sec]

    // Calc MVP
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

void OpenGLWidget::recalculateOrbitPositions() {
    // move satellites
    std::vector<GLfloat> positions;
    size_t nmbr_vertecies = 0;
    if (problem_instance.orbits.size() != 0) // size() will fail if no orbits in instance
        // assume that all satellites have the same amount of vertices
        nmbr_vertecies = satellite_subscene->getObjects().at(0).number_vertices;
    for (const Orbit& o : problem_instance.orbits) {
        Vector3D offset = o.cartesian_coordinates(time) / real_world_scale;
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

void OpenGLWidget::recalculateEdges() {
    // build edges
    Mesh all_lines;
    for (int i = 0; i < problem_instance.edges.size(); i++) {
        const Edge& edge = problem_instance.edges.at(i);
        Vector3D sat1 = edge.getV1().cartesian_coordinates(time) / real_world_scale;
        Vector3D sat2 = edge.getV2().cartesian_coordinates(time) / real_world_scale;
        VertexData vertex_1(sat1.x, sat1.y, sat1.z);
        VertexData vertex_2(sat2.x, sat2.y, sat2.z);

        if (edge.isBlocked(time)) { // edge can not be scanned
            vertex_1.setColor(1.0f, 0.0f, 0.0f);
            vertex_2.setColor(1.0f, 0.0f, 0.0f);
        } else { // edge can be scanned
            vertex_1.setColor(0.0f, 1.0f, 0.0f);
            vertex_2.setColor(0.0f, 1.0f, 0.0f);
        }

        if (state == SOLUTION) {
            if (solution.at(current_scan).edge_index == i) { // edge is scanned next
                vertex_1.setColor(1.0f, .75f, 0.0f);
                vertex_2.setColor(1.0f, .75f, 0.0f);
            } else if (edge.isBlocked(time) ||
                       !edge.canAlign(satellite_orientations.previous(&edge.getV1(), time),
                                      satellite_orientations.previous(&edge.getV2(), time), time)) {
                vertex_1.setColor(1.0f, 0.0f, 0.0f);
                vertex_2.setColor(1.0f, 0.0f, 0.0f);
            } else {
                vertex_1.setColor(0.0f, 1.0f, 0.0f);
                vertex_2.setColor(0.0f, 1.0f, 0.0f);
            }
        }

        all_lines.vertices.push_back(vertex_1);
        all_lines.vertices.push_back(vertex_2);
    }

    // build satellite orientations
    for (auto const& orbit : problem_instance.orbits) {
        Vector3D position = orbit.cartesian_coordinates(time) / real_world_scale;
        Orientation last_orientation = satellite_orientations.previous(&orbit, time);
        Orientation next_orientation = satellite_orientations.next(&orbit, time);
        float angle = std::acos(dot_product(last_orientation.direction, next_orientation.direction)); // [rad]
        float dt = time - last_orientation.start;

        // todo ugh
        glm::vec3 direction_vector = glm::vec3(last_orientation.direction.x, last_orientation.direction.y,
                                               last_orientation.direction.z);
        glm::vec3 tmp = glm::vec3(next_orientation.direction.x, next_orientation.direction.y,
                                  next_orientation.direction.z);
        direction_vector = glm::rotate(direction_vector, std::min(angle, dt * orbit.getMeanRotationSpeed()),
                                       glm::cross(direction_vector, tmp));

        VertexData origin(position);
        VertexData direction(position +
                             Vector3D(direction_vector.x, direction_vector.y, direction_vector.z) * 0.025f);

        origin.setColor(1.0f, 1.0f, 1.0f);
        direction.setColor(1.0f, 1.0f, 1.0f);

        all_lines.vertices.push_back(origin);
        all_lines.vertices.push_back(direction);
    }

    // push data to VBO
    if (all_lines.vertexCount() != 0) {
        size_t edges_size = all_lines.totalVertexSize();
        glBindBuffer(GL_ARRAY_BUFFER, edge_subscene->vbo_dynamic); // set active
        glBufferData(GL_ARRAY_BUFFER, edges_size, &all_lines.vertices[0],
                     GL_STREAM_DRAW); // push data
    }

    // iterate through scan cover
    if (state == SOLUTION) {
        // todo multiple edges at the same time?
        if (solution.at(current_scan).time < time) {
            const Edge& e = problem_instance.edges[solution.at(current_scan).edge_index];
            edge_subscene->disable(solution.at(current_scan).edge_index);
            current_scan++;
            std::cout << ".";
        }

        // end of solution
        if (current_scan >= solution.size()) {
            state = INSTANCE;
            edge_subscene->enableAll();
            satellite_orientations.clear();
        }
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent* event) {
    float zoom_per_deg = .001f * zoom * 2.5f; // depends on current zoom value to avoid endless scrolling for
                                              // high- and too big jumps for small zoom values
    int turned_deg = event->angleDelta().y() / 8; // negative if zoomed out
    zoom += turned_deg * zoom_per_deg;            // if the zoom value increase -> zoom out
}

void OpenGLWidget::mousePressEvent(QMouseEvent* event) {
    mouse_start_location = glm::vec2(event->pos().x(), event->pos().y());
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event) {
    view *= camera_rotation;           // save rotation
    camera_rotation = glm::mat4(1.0f); // reset to avoid applying it twice
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event) {
    glm::vec2 m1 = mouse_start_location;
    glm::vec2 m2 = glm::vec2(event->pos().x(), event->pos().y());

    // map coords to camera coords
    int h = this->size().height();
    int w = this->size().width();
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
    glm::vec3 rotation_axis = glm::inverse(glm::mat3(view)) * glm::cross(a2, a1);
    camera_rotation = glm::rotate(glm::degrees(rotation_angle) * 0.025f, rotation_axis);
}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    deleteInstance();
    glDeleteProgram(basic_program);
    glDeleteProgram(satellite_prog);
    glDeleteTextures(1, &texture_id);
    doneCurrent();
}

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

float OpenGLWidget::changeSpeed(bool up) {
    time_boost *= up ? 2.0f : 0.5f;
    return time_boost;
}