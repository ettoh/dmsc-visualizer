#ifndef OPENGL_WIDGET
#define OPENGL_WIDGET

#define _USE_MATH_DEFINES

#include "../instance/problem_instance.h"
#include "../instance/timetable.h"
#include "../solver/scan_cover.h"
#include "glm_include.h"
#include "opengl_primitives.h"
#include <QOpenGLWidget>
#include <QOpenglFunctions_3_3_core.h>
#include <chrono>
#include <math.h>

using SysTime_t = std::chrono::time_point<std::chrono::system_clock>;
constexpr auto PI2 = 2 * M_PI; // 2pi

enum VisualisationState { EMPTY, INSTANCE, SOLUTION };

/**
 * @brief todo
 */
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
  public:
    OpenGLWidget(QWidget* parent);
    ~OpenGLWidget();

    /**
     * @brief Convert a given instance with orbits and communications between the satellites into an opengl
     * scene with vertices and display it until the next call.
     */
    void visualizeInstance(const ProblemInstance& instance);

    /**
     * @brief Visualize a given solution for the current displayed instance.
     */
    void visualizeSolution(const ScanCover& scan_cover);

    /**
     * @brief Doubles or halves the speed.
     * @param up True: speed will increase; False: speed will decrease.
     * @return Time boost after update.
     */
    float changeSpeed(bool up);

    float getTimeBoost() const { return time_boost; }
    float getTime() const { return time; }
    void setTimeBoost(const float speed) { time_boost = speed; };
    void setTime(const float time) { this->time = time; }
    void togglePause() { paused = !paused; };

  signals:
    void fpsChanged(int fps);

  private:
    /**
     * @brief Push the scene to the GPU.
     */
    void pushSceneToGPU();

    /**
     * @brief Recalculate the dynamic part of the scene as well as the MVP's.
     */
    void recalculate();

    void recalculateOrbitPositions();
    void recalculateEdges();

    /**
     * @brief todo
     * @param subscene
     */
    void drawSubscene(const OpenGLPrimitives::Subscene& subscene);

    /**
     * @brief Free rescources linked to the current displayed scene.
     */
    void deleteInstance();

  private:
    const float real_world_scale = 7000.0f; 
    // handler
    GLuint basic_program = 0, satellite_prog = 0, earth_prog = 0; 
    GLuint vbo_uniforms = 0;
    GLuint texture_id = 0;
    GLint uniform_texture_location = 0;
    // frames per second measurement
    unsigned int fps_counter = 0; // count the frames within one second
    SysTime_t last_fps_update = std::chrono::system_clock::now();
    // view
    float zoom = 1.0f;
    glm::vec2 mouse_start_location = glm::vec2(0);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 camera_rotation = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f)); // (position, look at, up)
    // scene
    int state = EMPTY;
    ProblemInstance problem_instance = ProblemInstance();
    std::vector<OpenGLPrimitives::Subscene> scene;
    SysTime_t last_frame_time = std::chrono::system_clock::now();
    float time = .0f;
    float time_boost = 1.0f;                                  // 1sec realtime -> time_boos sec in simulation
    bool paused = false;                                      // if true -> the simulations is paused
    OpenGLPrimitives::Subscene* satellite_subscene = nullptr; 
    OpenGLPrimitives::Subscene* edge_subscene = nullptr;
    // solution
    ScanCover solution = ScanCover();
    int current_scan = 0;
    Timetable<Orbit, Orientation> satellite_orientations = Timetable<Orbit, Orientation>();

  private slots:
    void drawLoop();

  protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif