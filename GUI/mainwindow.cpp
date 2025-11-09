#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsScene>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentFunction(0)
    , currentStep(0)
    , scene(nullptr)
    , minBound(-5.0)
    , maxBound(5.0)
{
    ui->setupUi(this);

    // Initialize graphics scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // Connect signals
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::runOptimization);
    connect(ui->strategyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStrategyChanged);
    connect(ui->functionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFunctionChanged);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::previousStep);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetVisualization);
}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}

double MainWindow::objectiveFunction(const std::vector<double>& x)
{
    double result = 0.0;

    switch(currentFunction)
    {
    case 0:
        for (double val : x)
        {
            result += val * val;
        }
        break;

    case 1:
        result = 10.0 * x.size();
        for (double val : x)
        {
            result += val * val - 10.0 * std::cos(2.0 * M_PI * val);
        }
        break;

    case 2:
        for (size_t i = 0; i < x.size() - 1; ++i)
        {
            double term1 = x[i+1] - x[i] * x[i];
            double term2 = 1.0 - x[i];
            result += 100.0 * term1 * term1 + term2 * term2;
        }
        break;

    default:
        result = 0.0;
    }

    return result;
}

void MainWindow::clearVisualization()
{
    scene->clear();
}

void MainWindow::visualizeHistory(int stepIndex)
{
    if (stepIndex < 0 || stepIndex >= history.size())
    {
        return;
    }

    clearVisualization();

    const auto& snapshot = history[stepIndex];
    const std::string& phaseName = snapshot.first;
    const std::vector<Visual_Country_Snapshot>& countries = snapshot.second;

    double sceneSize = 400.0;
    double range = maxBound - minBound;
    double scale = sceneSize / range;

    scene->setSceneRect(-sceneSize/2, -sceneSize/2, sceneSize, sceneSize);

    QPen gridPen(Qt::lightGray, 0.5);
    for (int i = -5; i <= 5; ++i)
    {
        double pos = i * (sceneSize / 10.0);
        scene->addLine(-sceneSize/2, pos, sceneSize/2, pos, gridPen);
        scene->addLine(pos, -sceneSize/2, pos, sceneSize/2, gridPen);
    }

    QPen axisPen(Qt::darkGray, 1.5);
    scene->addLine(-sceneSize/2, 0, sceneSize/2, 0, axisPen);
    scene->addLine(0, -sceneSize/2, 0, sceneSize/2, axisPen);

    for (const auto& country : countries)
    {
        if (country.position.size() < 2) continue;

        double x = (country.position[0] - minBound) * scale - sceneSize/2;
        double y = (country.position[1] - minBound) * scale - sceneSize/2;

        double radius = country.is_emperor ? 8.0 : 5.0;

        QColor color(
            static_cast<int>(country.colour[0] * 255),
            static_cast<int>(country.colour[1] * 255),
            static_cast<int>(country.colour[2] * 255)
            );

        QGraphicsEllipseItem* item = scene->addEllipse(
            x - radius, y - radius,
            radius * 2, radius * 2,
            QPen(Qt::black, country.is_emperor ? 2 : 1),
            QBrush(color)
            );


        if (country.is_emperor) {
            QPen starPen(Qt::yellow, 2);
            double starSize = 12.0;
            scene->addLine(x - starSize, y, x + starSize, y, starPen);
            scene->addLine(x, y - starSize, x, y + starSize, starPen);
        }
    }

    QString info = QString("Step %1/%2 - Phase: %3")
                       .arg(stepIndex + 1)
                       .arg(history.size())
                       .arg(QString::fromStdString(phaseName));
    ui->infoLabel->setText(info);


    ui->prevButton->setEnabled(stepIndex > 0);
    ui->nextButton->setEnabled(stepIndex < history.size() - 1);
}

void MainWindow::setupVisualization()
{
    // This is now handled in the UI file
}

void MainWindow::runOptimization()
{
    try
    {
        // Disable button during optimization
        ui->runButton->setEnabled(false);

        // Parse parameters
        int popSize = ui->popSizeEdit->text().toInt();
        int dim = ui->dimEdit->text().toInt();
        int maxIter = ui->maxIterEdit->text().toInt();
        double beta = ui->betaEdit->text().toDouble();
        double gamma = ui->gammaEdit->text().toDouble();
        double eta = ui->etaEdit->text().toDouble();
        double lb = ui->lbEdit->text().toDouble();
        double ub = ui->ubEdit->text().toDouble();

        // Validate parameters
        if (popSize <= 0 || dim <= 0 || maxIter <= 0)
        {
            QMessageBox::warning(this, "Invalid Parameters",
                                 "Population size, dimensions, and max iterations must be positive!");
            ui->runButton->setEnabled(true);
            return;
        }

        if (lb >= ub)
        {
            QMessageBox::warning(this, "Invalid Parameters",
                                 "Lower bound must be less than upper bound!");
            ui->runButton->setEnabled(true);
            return;
        }

        if (dim < 2)
        {
            QMessageBox::warning(this, "Invalid Parameters",
                                 "Dimensions must be at least 2 for visualization!");
            ui->runButton->setEnabled(true);
            return;
        }

        minBound = lb;
        maxBound = ub;

        auto objFunc = [this](const std::vector<double>& x) -> double
        {
            return this->objectiveFunction(x);
        };

        history.clear();
        currentStep = 0;
        clearVisualization();

        ui->infoLabel->setText("Running optimization... Please wait.");
        QApplication::processEvents();

        Visual_ICA ica(popSize, dim, maxIter, beta, gamma, eta, lb, ub, objFunc);
        ica.setup();
        ica.run();

        std::vector<double> bestSolution = ica.get_best_solution();
        double bestFitness = ica.get_fitness();

        history = ica.get_history();

        QString resultMsg = QString("Optimization completed!\n\n"
                                    "Best Fitness: %1\n"
                                    "Best Solution: [%2, %3, ...]\n"
                                    "Total Steps: %4\n\n"
                                    "Use Next/Previous buttons to explore the algorithm steps.")
                                .arg(bestFitness, 0, 'g', 10)
                                .arg(bestSolution[0], 0, 'g', 6)
                                .arg(bestSolution[1], 0, 'g', 6)
                                .arg(history.size());

        QMessageBox::information(this, "Success", resultMsg);

        ui->nextButton->setEnabled(!history.empty());
        ui->resetButton->setEnabled(!history.empty());

        if (!history.empty())
        {
            visualizeHistory(0);
        }

    } catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Error",
                              QString("An error occurred: %1").arg(e.what()));
        ui->infoLabel->setText(QString("Error: %1").arg(e.what()));
    }

    ui->runButton->setEnabled(true);
}

void MainWindow::nextStep()
{
    if (currentStep < history.size() - 1)
    {
        currentStep++;
        visualizeHistory(currentStep);
    }
}

void MainWindow::previousStep()
{
    if (currentStep > 0)
    {
        currentStep--;
        visualizeHistory(currentStep);
    }
}

void MainWindow::resetVisualization()
{
    currentStep = 0;
    if (!history.empty())
    {
        visualizeHistory(currentStep);
    }
}

void MainWindow::onStrategyChanged(int index)
{
    ui->infoLabel->setText(QString("Strategy: %1").arg(ui->strategyCombo->currentText()));
}

void MainWindow::onFunctionChanged(int index)
{
    currentFunction = index;
    ui->infoLabel->setText(QString("Objective function: %1").arg(ui->functionCombo->currentText()));

    switch(index)
    {
    case 0:
        ui->lbEdit->setText("-5.0");
        ui->ubEdit->setText("5.0");
        break;
    case 1:
        ui->lbEdit->setText("-5.12");
        ui->ubEdit->setText("5.12");
        break;
    case 2:
        ui->lbEdit->setText("-2.0");
        ui->ubEdit->setText("2.0");
        break;
    }
}
