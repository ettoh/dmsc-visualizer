#ifndef DIALOG_ADD_EDGES_H
#define DIALOG_ADD_EDGES_H

#include <QDialog>

namespace dmsc {

namespace Ui {
class DialogEdges;
}

class DialogAddEdges : public QDialog {
    Q_OBJECT
  public:
    DialogAddEdges(QWidget* parent = nullptr);
    int getMode();
    int getCustom1() const;
    int getCustom2() const;
    int getNumberEdges() const;

    enum MODE { CUSTOM, RANDOM, ALL, ERROR };

  private:
    Ui::DialogEdges* ui;
};

} // namespace dmsc

#endif