#include "dialog_instance.h"
#include "dialog_add_edges.h"
#include "ui_dialog_instance.h"
#include "dmsc/satellite.hpp"
#include <QCheckBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <ctime>
#include <set>

namespace dmsc {

DialogInstance::DialogInstance(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Dialog()) {
    ui->setupUi(this);

    // connect button signals
    connect(ui->btn_add_single, SIGNAL(released()), this, SLOT(addSingleOrbit()));
    connect(ui->btn_remove_orbit, SIGNAL(released()), this, SLOT(removeOrbit()));
    connect(ui->bnt_add_linear, SIGNAL(released()), this, SLOT(addLinearOrbits()));
    connect(ui->bnt_delete_instance, SIGNAL(released()), this, SLOT(clear()));
    connect(ui->bnt_generate_random, SIGNAL(released()), this, SLOT(addRandomOrbits()));
    connect(ui->btn_remove_edge, SIGNAL(released()), this, SLOT(removeEdge()));
    connect(ui->btn_add_edge, SIGNAL(released()), this, SLOT(addEdgeDialog()));

    // connect "range"-checkbox signals
    connect(ui->chk_ecc, &QCheckBox::stateChanged, [this](int state) { ui->nbr_ecc_max->setEnabled(state == Qt::Checked); });
    connect(ui->chk_height, &QCheckBox::stateChanged,
            [this](int state) { ui->nbr_height_max->setEnabled(state == Qt::Checked); });
    connect(ui->chk_incl, &QCheckBox::stateChanged, [this](int state) { ui->nbr_incl_max->setEnabled(state == Qt::Checked); });
    connect(ui->chk_peri, &QCheckBox::stateChanged, [this](int state) { ui->nbr_peri_max->setEnabled(state == Qt::Checked); });
    connect(ui->chk_pos, &QCheckBox::stateChanged, [this](int state) { ui->nbr_pos_max->setEnabled(state == Qt::Checked); });
    connect(ui->chk_raan, &QCheckBox::stateChanged, [this](int state) { ui->nbr_raan_max->setEnabled(state == Qt::Checked); });

    // prepare tables
    QStringList edges_header = {"Orbit 1", "Orbit 2"};
    ui->table_edges->setColumnCount(2);
    ui->table_edges->setHorizontalHeaderLabels(edges_header);
    ui->table_edges->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QStringList orbits_header = {"hp", "e", "true anomaly", "inclination", "raan", "perigee", "rotation speed"};
    ui->table_orbits->setColumnCount(7);
    ui->table_orbits->setHorizontalHeaderLabels(orbits_header);
    ui->table_orbits->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void DialogInstance::accept() {
    Instance instance = Instance();

    // central mass
    CentralMass cm;
    cm.radius_central_mass = ui->nbr_radius_cm->value();
    cm.gravitational_parameter = ui->nbr_gravitation->value();
    instance.cm = cm;

    // 1. orbits
    for (int i = 0; i < ui->table_orbits->rowCount(); i++) {
        StateVector sv;
        sv.height_perigee = ((TableItem*)ui->table_orbits->item(i, 0))->data;
        sv.eccentricity = ((TableItem*)ui->table_orbits->item(i, 1))->data;
        sv.initial_true_anomaly = ((TableItem*)ui->table_orbits->item(i, 2))->data;
        sv.inclination = ((TableItem*)ui->table_orbits->item(i, 3))->data;
        sv.raan = ((TableItem*)ui->table_orbits->item(i, 4))->data;
        sv.argument_periapsis = ((TableItem*)ui->table_orbits->item(i, 5))->data;
        sv.rotation_speed = ((TableItem*)ui->table_orbits->item(i, 6))->data;
        instance.satellites.push_back(sv);
    }

    // 2. edges
    int nbr_edges = ui->table_edges->rowCount();
    for (int i = 0; i < nbr_edges; i++) {
        uint32_t o1 = ((TableItem*)ui->table_edges->item(i, 0))->data + 1;
        uint32_t o2 = ((TableItem*)ui->table_edges->item(i, 1))->data + 1;
        instance.edges.push_back(Edge(o1, o2));
    }

    // Combine the "raw" satellite data with the central mass.
    // TODO rework
    pyhsical_instance = PhysicalInstance(instance);
    pyhsical_instance.removeInvalidEdges();
    QDialog::accept();
}

void DialogInstance::addSingleOrbit() {
    prepareRandomGenerator();
    StateVector sv;
    sv.height_perigee = getValue(ui->nbr_height_min, ui->nbr_height_max, RANDOM, 0.0f, ui->chk_height->isChecked());
    sv.eccentricity = getValue(ui->nbr_ecc_min, ui->nbr_ecc_max, RANDOM, 0.0f, ui->chk_ecc->isChecked());
    sv.initial_true_anomaly = getValue(ui->nbr_pos_min, ui->nbr_pos_max, RANDOM, 0.0f, ui->chk_pos->isChecked());
    sv.raan = getValue(ui->nbr_raan_min, ui->nbr_raan_max, RANDOM, 0.0f, ui->chk_raan->isChecked());
    sv.argument_periapsis = getValue(ui->nbr_peri_min, ui->nbr_peri_max, RANDOM, 0.0f, ui->chk_peri->isChecked());
    sv.inclination = getValue(ui->nbr_incl_min, ui->nbr_incl_max, RANDOM, 0.0f, ui->chk_incl->isChecked());
    sv.rotation_speed = ui->nbr_rotation_speed->value();
    addOrbit(sv);
}

void DialogInstance::addLinearOrbits() {
    int n = ui->nbr_orbits->value();
    for (int i = 0; i < n; i++) {
        StateVector sv;
        float x = (float)i / (n - 1);
        sv.height_perigee = getValue(ui->nbr_height_min, ui->nbr_height_max, LINEAR, x, ui->chk_height->isChecked());
        sv.eccentricity = getValue(ui->nbr_ecc_min, ui->nbr_ecc_max, LINEAR, x, ui->chk_ecc->isChecked());
        sv.initial_true_anomaly = getValue(ui->nbr_pos_min, ui->nbr_pos_max, LINEAR, x, ui->chk_pos->isChecked());
        sv.raan = getValue(ui->nbr_raan_min, ui->nbr_raan_max, LINEAR, x, ui->chk_raan->isChecked());
        sv.argument_periapsis = getValue(ui->nbr_peri_min, ui->nbr_peri_max, LINEAR, x, ui->chk_peri->isChecked());
        sv.inclination = getValue(ui->nbr_incl_min, ui->nbr_incl_max, LINEAR, x, ui->chk_incl->isChecked());
        sv.rotation_speed = ui->nbr_rotation_speed->value();
        addOrbit(sv);
    }
}

void DialogInstance::addRandomOrbits() {
    prepareRandomGenerator();
    // generate orbits
    for (int i = 0; i < ui->nbr_orbits->value(); i++) {
        StateVector sv;
        float x = 0.0f;
        sv.height_perigee = getValue(ui->nbr_height_min, ui->nbr_height_max, RANDOM, x, ui->chk_height->isChecked());
        sv.eccentricity = getValue(ui->nbr_ecc_min, ui->nbr_ecc_max, RANDOM, x, ui->chk_ecc->isChecked());
        sv.initial_true_anomaly = getValue(ui->nbr_pos_min, ui->nbr_pos_max, RANDOM, x, ui->chk_pos->isChecked());
        sv.raan = getValue(ui->nbr_raan_min, ui->nbr_raan_max, RANDOM, x, ui->chk_raan->isChecked());
        sv.argument_periapsis = getValue(ui->nbr_peri_min, ui->nbr_peri_max, RANDOM, x, ui->chk_peri->isChecked());
        sv.inclination = getValue(ui->nbr_incl_min, ui->nbr_incl_max, RANDOM, x, ui->chk_incl->isChecked());
        sv.rotation_speed = ui->nbr_rotation_speed->value();
        addOrbit(sv);
    }
}

void DialogInstance::addEdgeDialog() {
    DialogAddEdges dialog = DialogAddEdges(this);
    if (dialog.exec() == QDialog::Accepted) {
        switch (dialog.getMode()) {
        case DialogAddEdges::CUSTOM: {
            int orbit1 = dialog.getCustom1() - 1; // id in ui begins with 1 not 0
            int orbit2 = dialog.getCustom2() - 1;
            // orbits out of range
            if (std::max(orbit1, orbit2) >= ui->table_orbits->rowCount()) {
                return;
            }
            addEdge(orbit1, orbit2);
            break;
        }
        case DialogAddEdges::ALL: {
            int n = ui->table_orbits->rowCount();
            for (int i = 0; i < n - 1; i++) {
                for (int j = i + 1; j < n; j++) {
                    addEdge(i, j);
                }
            }
        } break;
        case DialogAddEdges::RANDOM: {
            // not enough orbits
            if (ui->table_orbits->rowCount() < 2) {
                return;
            }

            int n = dialog.getNumberEdges();
            prepareRandomGenerator();
            std::set<std::pair<int, int>> used; // do not generate an edge twice
            std::uniform_int_distribution<int> distr(1, ui->table_orbits->rowCount());
            for (int i = 0; i < n; i++) {
                int orbit1 = distr(generator);
                int orbit2 = distr(generator);

                // ignore double elements
                auto it = used.find(std::make_pair(orbit1, orbit2));
                auto it2 = used.find(std::make_pair(orbit2, orbit1));
                if (it != used.end() || it2 != used.end()) {
                    continue;
                }

                // ignore loops
                if (orbit1 == orbit2) {
                    continue;
                }

                addEdge(orbit1, orbit2);
                used.emplace(orbit1, orbit2);
            }
        } break;
        default:
            break;
        }
    }
}

void DialogInstance::addOrbit(const StateVector& sv) {
    int row_index = ui->table_orbits->rowCount();
    ui->table_orbits->insertRow(row_index);
    ui->table_orbits->setItem(row_index, 0, new TableItem(QString::number(sv.height_perigee), sv.height_perigee));
    ui->table_orbits->setItem(row_index, 1, new TableItem(QString::number(sv.eccentricity), sv.eccentricity));
    ui->table_orbits->setItem(row_index, 2, new TableItem(QString::number(sv.initial_true_anomaly), sv.initial_true_anomaly));
    ui->table_orbits->setItem(row_index, 3, new TableItem(QString::number(sv.inclination), sv.inclination));
    ui->table_orbits->setItem(row_index, 4, new TableItem(QString::number(sv.raan), sv.raan));
    ui->table_orbits->setItem(row_index, 5, new TableItem(QString::number(sv.argument_periapsis), sv.argument_periapsis));
    ui->table_orbits->setItem(row_index, 6, new TableItem(QString::number(sv.rotation_speed), sv.rotation_speed));
}

void DialogInstance::addEdge(const int orbit1, const int orbit2) {
    int row_index = ui->table_edges->rowCount();
    ui->table_edges->insertRow(row_index);
    ui->table_edges->setItem(row_index, 0, new TableItem(QString::number(orbit1), orbit1 - 1));
    ui->table_edges->setItem(row_index, 1, new TableItem(QString::number(orbit2), orbit2 - 1));
}

void DialogInstance::removeOrbit() {
    auto range = ui->table_orbits->selectedRanges();
    if (range.size() != 1) {
        return;
    }

    for (const auto& selection : range) {
        // delete from bottom to top in order to prevent indices from changing each iteration
        for (int i = selection.bottomRow(); i >= selection.topRow(); i--) {
            ui->table_orbits->removeRow(i);

            // Update edges. Reverse order to avoid changing indices.
            for (int j = ui->table_edges->rowCount() - 1; j >= 0; j--) {
                TableItem* orbit1 = ((TableItem*)ui->table_edges->item(j, 0));
                TableItem* orbit2 = ((TableItem*)ui->table_edges->item(j, 1));

                // remove all edges that contains this orbit.
                if (orbit1->data == i || orbit2->data == i) {
                    ui->table_edges->removeRow(j);
                }

                // decrease indices for edges because one orbit is missing now
                if (orbit1->data > i) {
                    orbit1->data--;
                    orbit1->setText(QString::number(orbit1->data + 1));
                }
                if (orbit2->data > i) {
                    orbit2->data--;
                    orbit2->setText(QString::number(orbit2->data + 1));
                }
            }
        }
    }
}

void DialogInstance::removeEdge() {
    auto range = ui->table_edges->selectedRanges();
    if (range.size() != 1) {
        return;
    }

    for (const auto& selection : range) {
        // delete from bottom to top in order to prevent indices from changing each iteration
        for (int i = selection.bottomRow(); i >= selection.topRow(); i--) {
            ui->table_edges->removeRow(i);
        }
    }
}

void DialogInstance::clear() {
    ui->table_edges->clearContents();
    ui->table_edges->setRowCount(0);
    ui->table_orbits->clearContents();
    ui->table_orbits->setRowCount(0);
}

void DialogInstance::prepareRandomGenerator() {
    int seed = ui->nbr_seed->value();
    if (seed == -1) {
        generator = std::mt19937(std::time(nullptr));
    } else {
        generator = std::mt19937(seed);
    }
}

float DialogInstance::getValue(const QDoubleSpinBox* min_obj, const QDoubleSpinBox* max_obj, const int mode, const float x,
                               const bool is_ranged) {
    float value_min = min_obj->value();
    float value_max = max_obj->value();
    if (!is_ranged) {
        return value_min;
    }

    if (value_max <= value_min) {
        return value_min;
    }

    if (mode == RANDOM) { // random between min and max
        std::uniform_real_distribution<float> distr(value_min, value_max);
        return distr(generator);
    } else { // linear interpolated between min and max
        return value_min + x * (value_max - value_min);
    }
}

void DialogInstance::keyPressEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}

} // namespace dmsc