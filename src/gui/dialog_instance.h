#ifndef DIALOG_INSTANCE_H
#define DIALOG_INSTANCE_H

#include "vdmsc/instance.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QTableWidgetItem>
#include <random>

namespace Ui {
    class Dialog;
}

class DialogInstance : public QDialog {
    Q_OBJECT
  public:
    DialogInstance(QWidget* parent = nullptr);
    ProblemInstance instance = ProblemInstance();

  private:
    Ui::Dialog* ui;
    std::mt19937 generator = std::mt19937(0);
    enum MODE { RANDOM, LINEAR };
    struct StateVector {
        float hp = 0.0f;             // [km]
        float true_anomaly = 0.0f;   // [deg]
        float eccentricity = 0.0f;   // [deg]
        float inclination = 0.0f;    // [deg]
        float perigee = 0.0f;        // [deg]
        float raan = 0.0f;           // [deg]
        float rotation_speed = 0.0f; // [rad/sec]
    };

  private:
    /**
     * @brief Set the seed for the random generator depending on the seed in the ui.
     */
    void prepareRandomGenerator();

    /**
     * @param mode If a range is given, which mode shall be used to find the value in between.
     * @param x in [0,1]. Determines the interpolation between min and max. Only used if interpolation mode is
     * used.
     * @return
     */
    float getValue(const QDoubleSpinBox* min_obj, const QDoubleSpinBox* max_obj, const int mode,
                   const float x, const bool is_ranged);

    /**
     * @brief Add an orbit to the table.
     */
    void addOrbit(const StateVector& s);

  private slots:
    void addSingleOrbit();
    void addLinearOrbits();
    void addRandomOrbits();
    void addEdgeDialog();
    void addEdge(const int orbit1, const int orbit2);
    void accept();

    /**
     * @brief Removes one or multiple adjacent orbits from the table.
     */
    void removeOrbit();

    /**
     * @brief Removes one or multiple adjacent edges from the table.
     */
    void removeEdge();

    /**
     * @brief Delete all orbits and edges.
     */
    void clear();

  protected:
    /**
     * @brief Prevent dialog from closing if enter is pressed.
     */
    void keyPressEvent(QKeyEvent* evt);
};

// The additional data will be used for creating the orbit later (=> no string conversion).
class TableItem : public QTableWidgetItem {
  public:
    TableItem(const QString& text, const float data) : QTableWidgetItem(text), data(data){};
    float data = 0.0f;
};

#endif // !DIALOG_INSTANCE_H
