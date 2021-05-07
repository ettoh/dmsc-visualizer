#include "dialog_add_edges.h"
#include "ui_dialog_add_edges.h"
#include <QSpinBox>

DialogAddEdges::DialogAddEdges(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEdges()) {
    ui->setupUi(this);

    // if spin box changed -> activate radio-check-box
    connect(ui->nbr_orbit1, QOverload<int>::of(&QSpinBox::valueChanged),
            [this]() { ui->radio_custom->setChecked(true); });
    connect(ui->nbr_orbit2, QOverload<int>::of(&QSpinBox::valueChanged),
            [this]() { ui->radio_custom->setChecked(true); });
    connect(ui->nbr_n, QOverload<int>::of(&QSpinBox::valueChanged),
            [this]() { ui->radio_random->setChecked(true); });
}

int DialogAddEdges::getMode() {
    if (ui->radio_custom->isChecked()) {
        return CUSTOM;
    }

    if (ui->radio_random->isChecked()) {
        return RANDOM;
    }

    if (ui->radio_all->isChecked()) {
        return ALL;
    }

    return ERROR;
}

int DialogAddEdges::getCustom1() const { return ui->nbr_orbit1->value(); }
int DialogAddEdges::getCustom2() const { return ui->nbr_orbit2->value(); }
int DialogAddEdges::getNumberEdges() const { return ui->nbr_n->value(); }