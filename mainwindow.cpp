#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidgetItem>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* preset initial positions of the drones */
    const QVector<Vector2D> tabPos={{60,80},{400,700},{50,250},{800,800},{700,50}};

    int n=0;
    for (auto &pos:tabPos) {
        QListWidgetItem *LWitems=new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitems);
        QString name="Drone"+QString::number(++n);
        //mapDrones[name]=new Drone(name);
        mapDrones[name] = new Drone(name, ui->widget);

        mapDrones[name]->setInitialPosition(pos);
        ui->listDronesInfo->setItemWidget(LWitems,mapDrones[name]);
    }

    ui->widget->setMap(&mapDrones);
    // Connect the "Load" button to loadJsonData
    connect(ui->actionLoad, &QAction::triggered, [this]() {
        ui->widget->loadJsonData(); // Call the Canvas method to load JSON
         refreshDronesUI();
    });


    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start();

    elapsedTimer.start();
}


MainWindow::~MainWindow() {
    delete ui;
    delete timer;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::update() {
    static int last=elapsedTimer.elapsed();
    static int steps=10;
    int current=elapsedTimer.elapsed();
    double dt=(current-last)/(1000.0*steps);
    for (int step=0; step<steps; step++) {
        // update positions of drones
        for (auto &drone:mapDrones) {
            ui->widget->updateDroneTarget(drone);// Update the drone's target based on server connections

            // detect collisions between drone and other flying drones
            if (drone->getStatus()!=Drone::landed) {
                drone->initCollision();
                for (auto &obs:mapDrones) {
                    if (obs->getStatus()!=Drone::landed && obs->getName()!=drone->getName()) {
                        Vector2D B=obs->getPosition();
                        drone->addCollision(B,ui->widget->droneCollisionDistance);
                    }
                }
            }
            drone->update(dt);
        }
    }
    int d = elapsedTimer.elapsed()-current;
    ui->statusbar->showMessage("duree:"+QString::number(d)+" steps="+QString::number(steps));
    if (d>90) {
        steps/=2;
    } else {

        if (steps<10) steps++;
    }
    last=current;
    ui->widget->repaint();
}

void MainWindow::refreshDronesUI() {
    // Clear the current drone list in the UI
    ui->listDronesInfo->clear();

    // Rebuild the drone list with updated drones from the map
    for (auto &drone : mapDrones) {
        QListWidgetItem *LWitem = new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitem);
        ui->listDronesInfo->setItemWidget(LWitem, drone);
    }

    // Update the canvas to reflect changes
    ui->widget->update();
}
