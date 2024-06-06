#ifndef RRCMP_H
#define RRCMP_H

#include "ui_rrcmp.h"
#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class RRcmp; }
QT_END_NAMESPACE

class RRcmp : public QMainWindow
{
    Q_OBJECT

public:
    RRcmp(QWidget *parent = nullptr);
    ~RRcmp();
    QMap<QString, QLineEdit*> idToLineEdit;
    QMap<QString, QCheckBox*> idToCheckBox;

public Q_SLOTS:
    void loadENEA();
    void loadFISE();
    void loadDLR();
    void loadNREL();
    void loadSANDIA();
    void loadData(int iPartner);
    void loadAll();
    void compare();
    void displayAP();
    void map0();
    void map1();
    void map2();
    void map3();
    void map4();
    void plotMap(int iPtn);
    void plotMapDiff(int iPtn0, int iPtn1, double deltaMin, double deltaMax,double normFact);
    void idealValue();
    void checkRange();
    void setErr();

private:
    Ui::RRcmp *ui;
};
#endif // RRCMP_H
