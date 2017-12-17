#ifndef GAME_H
#define GAME_H
#include <QTcpSocket>
#include <player.h>

class Game : public QObject
{
    Q_OBJECT
public:
    Game();
    ~Game();
    int roomNumber;
    Player *black,*white;
    int gridInfo[7][7];
    int currColor;
    bool inGaming;
    bool block;
    void changeColor();
    void ReadyToStart();
signals:
    void gameDelete(int);
public slots:
    void slotPlayerQuit(int color);
    void procStep(int color, int beginX, int beginY, int endX, int endY);

};

#endif // GAME_H
