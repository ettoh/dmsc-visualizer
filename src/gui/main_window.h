#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "../solver/scan_cover.h"
#include "opengl_widget.h"
#include "vdmsc/instance.h"
#include <QMainWindow>
#include <QProgressDialog>
#include <QSurfaceFormat>
#include <QTimer>
#include <future>

namespace dmsc {

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MainWindow();
    void solverCallback(const float progress);

  private:
    Ui::MainWindow* ui;
    OpenGLWidget* opengl_widget;
    Instance problem_instance = Instance();
    // solving
    float progress = 0.0f; // percentage [0, 1]
    std::future<ScanCover> solution;
    QTimer* timer_solution = nullptr; // checks if the solution is ready
    QProgressDialog* progress_diag = nullptr;

  private slots:
    void updateFPS(const int fps);
    void visualizeInstance();
    void incrSimulationSpeed();
    void decrSimulationSpeed();
    void solve();
    void restart_simulation();
    void setSimulationSpeed(const double speed);
    void saveInstance();
    void loadInstance();

  private:
    enum SolverID { ID_GREEDY_NEXT, ID_ILS, ID_SIM_ANNEALING, ID_GENETIC };
};

} // namespace dmsc

#endif