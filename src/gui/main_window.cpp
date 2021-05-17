#include "main_window.h"
#include "../solver/greedy_next.h"
#include "../solver/iterated_local_search.h"
#include "../solver/simulated_annealing.h"
#include "../solver/solver.h"
#include "dialog_instance.h"
#include "ui_main_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <chrono>
#include <fstream>

namespace dmsc {

MainWindow::MainWindow() : ui(new Ui::MainWindow) {
    ui->setupUi(this);
    Solver::solver_abort = false;

    // add OpenGL Widget
    opengl_widget = new OpenGLWidget(this);
    delete (ui->openGL_container);
    ui->gridLayout->addWidget(opengl_widget, 0, 0, 2, 1);

    // add combo box for solver
    ui->combo_solver->addItem(QString("GreedyNext"), ID_GREEDY_NEXT);
    ui->combo_solver->addItem(QString("ILS"), ID_ILS);
    ui->combo_solver->addItem(QString("Simulated Annealing"), ID_SIM_ANNEALING);

    // init gui elements
    ui->nbr_speed->setValue(opengl_widget->getTimeBoost());

    // connect signals
    connect(opengl_widget, SIGNAL(fpsChanged(int)), this, SLOT(updateFPS(int)));
    connect(ui->btn_new_instance, SIGNAL(released()), this, SLOT(visualizeInstance()));
    connect(ui->btn_speed_down, SIGNAL(released()), this, SLOT(decrSimulationSpeed()));
    connect(ui->btn_speed_up, SIGNAL(released()), this, SLOT(incrSimulationSpeed()));
    connect(ui->btn_solve, SIGNAL(released()), this, SLOT(solve()));
    connect(ui->btn_restart, SIGNAL(released()), this, SLOT(restart_simulation()));
    connect(ui->nbr_speed, SIGNAL(valueChanged(double)), this, SLOT(setSimulationSpeed(double)));
    connect(ui->action_save_instance, SIGNAL(triggered(bool)), this, SLOT(saveInstance()));
    connect(ui->action_load_instance, SIGNAL(triggered(bool)), this, SLOT(loadInstance()));
    connect(ui->bnt_play_pause, &QPushButton::released, [this]() { opengl_widget->togglePause(); });
}

void MainWindow::saveInstance() {
    QString file = QFileDialog::getSaveFileName(this, "Save instance", QDir::currentPath(), tr("Instance (*.csv *.instance*)"));
    if (file.size() == 0) {
        return;
    }

    if (!problem_instance.save(file.toStdString())) {
        QMessageBox msg;
        msg.setText("An error occured!");
        msg.exec();
    }
}

void MainWindow::loadInstance() {
    QString file = QFileDialog::getOpenFileName(this, "Load instance", QDir::currentPath(), tr("Instance (*.csv *.instance*)"));
    if (file.size() == 0) {
        return;
    }

    problem_instance = Instance(file.toStdString());
    opengl_widget->visualizeInstance(problem_instance);
}

void MainWindow::visualizeInstance() {
    DialogInstance dialog = DialogInstance(this);
    if (dialog.exec() == QDialog::Accepted) {
        problem_instance = dialog.instance;
        opengl_widget->visualizeInstance(problem_instance);
    }
}

void MainWindow::solverCallback(const float progress) { this->progress = progress; }

void MainWindow::solve() {
    progress = 0.0f;
    // which solver is selected?
    QVariant data = ui->combo_solver->itemData(ui->combo_solver->currentIndex());
    int solver_id = data.toInt();

    // progress bar; multithreading; abort
    delete progress_diag;
    progress_diag = new QProgressDialog(QString("Solving ..."), QString("Cancel"), 0, 100, this);
    progress_diag->setAutoClose(true);
    connect(progress_diag, &QProgressDialog::canceled, this, [this]() {
        timer_solution->stop();
        Solver::solver_abort = true;
    });
    progress_diag->open();

    // solve in another thread
    Solver::solver_abort = false;
    solution = std::async(std::launch::async, [solver_id, this]() {
        using namespace std::placeholders;
        Solver* solver;
        switch (solver_id) {
        case ID_ILS: {
            try {
                solver = new ILS(problem_instance, std::bind(&MainWindow::solverCallback, this, _1));
            } catch (const std::exception&) {
                return ScanCover();
            }
            break;
        }
        case ID_SIM_ANNEALING: {
            try {
                solver = new SimulatedAnnealing(problem_instance, std::bind(&MainWindow::solverCallback, this, _1));
            } catch (const std::exception&) {
                return ScanCover();
            }
            break;
        }
        case ID_GREEDY_NEXT:
        default:
            try {
                solver = new GreedyNext(problem_instance, std::bind(&MainWindow::solverCallback, this, _1));
            } catch (const std::exception&) {
                return ScanCover();
            }
            break;
        }
        ScanCover solution = solver->solve();
        delete solver;
        return solution;
    });

    // get solution
    delete timer_solution;
    timer_solution = new QTimer(this);
    connect(timer_solution, &QTimer::timeout, this, [this]() {
        using namespace std::chrono; // for ms
        progress_diag->setValue(progress * 100);
        auto status = solution.wait_for(0ms);
        // task finished?
        if (status != std::future_status::timeout) {
            progress_diag->setValue(100);
            progress_diag->close();
            timer_solution->stop();
            ScanCover result = solution.get();
            ui->lbl_makespan->setText(QString::number(result.getScanTime()));
            ui->lbl_computation_time->setText(QString::number(result.getComputationTime()));
            ui->lbl_lower_bound->setText(QString::number(result.getLowerBound()));
            ui->lbl_nbr_edges->setText(QString::number(result.size()));
            opengl_widget->visualizeSolution(result);
        }
    });
    timer_solution->start(20);
}

void MainWindow::updateFPS(const int fps) {
    QString s = "FPS: " + QString::number(fps);

    // time in simulation
    float time = opengl_widget->getTime(); // [sec]
    int minutes = (int)time / 60;
    int seconds = fmod(time, 60);
    s.append(" | Time: " + QString::number(minutes) + "min ");
    s.append(QString::number(seconds) + "sec");

    ui->statusbar->showMessage(s);
}

void MainWindow::incrSimulationSpeed() {
    float time_boost = opengl_widget->changeSpeed(true);
    ui->nbr_speed->setValue(time_boost);
}

void MainWindow::decrSimulationSpeed() {
    float time_boost = opengl_widget->changeSpeed(false);
    ui->nbr_speed->setValue(time_boost);
}

void MainWindow::setSimulationSpeed(const double speed) { opengl_widget->setTimeBoost((float)speed); }

void MainWindow::restart_simulation() { opengl_widget->setTime(0.0f); }

} // namespace dmsc
