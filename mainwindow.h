/**
 * @brief Drone_demo project
 * @author B.Piranda ---STUDENTS-ZAHRAHMAN Bilal & ABIONA Boluwatife
 * @date dec. 2024
 **/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <drone.h>
#include <QListWidget>
#include <QMap>
#include <QTimer>
#include <QElapsedTimer>

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
    void on_actionQuit_triggered();
    void update();

private:
    Ui::MainWindow *ui;
    QMap<QString,Drone*> mapDrones;
    QTimer *timer;
    QElapsedTimer elapsedTimer;
     void refreshDronesUI();
};
#endif // MAINWINDOW_H
