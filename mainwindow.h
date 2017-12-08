#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <QPushButton>
#include <QTimer>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    QLabel *point[7][7],*blackImage,*whiteImage,*blackNum,*whiteNum,*GameInfo1,*GameInfo2;
    QImage *empty,*emptyGreen,*emptyYellow,*black,*white,*blackSelected,*whiteSelected;
    QImage *blackInfo,*whiteInfo,*AITarget,*AIBlack,*AIWhite;
    QPushButton *PvPStart,*PvEStart,*Help;
    QTimer *tm;
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void GridDisplay();
    void GameCore();
    void AIturn();
    void RecordLoad();
    void RecordSave();
private slots:
    void PvP();
    void PvE();
    void tmTimeout();
    void GameHelp();
};

#endif // MAINWINDOW_H
