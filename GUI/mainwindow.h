#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include "../src/visual_ica.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void runOptimization();
    void onStrategyChanged(int index);
    void onFunctionChanged(int index);
    void nextStep();
    void previousStep();
    void resetVisualization();
private:
    Ui::MainWindow *ui;

    double objectiveFunction(const std::vector<double>& x);
    void visualizeHistory(int stepIndex);
    void setupVisualization();
    void clearVisualization();

    int currentFunction;
    int currentStep;
    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> history;
    QGraphicsScene* scene;

    double minBound;
    double maxBound;
};
#endif // MAINWINDOW_H
