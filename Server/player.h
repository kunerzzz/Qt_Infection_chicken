#ifndef PLAYER_H
#define PLAYER_H
#include <QTcpSocket>
#include <QObject>

class Player : public QTcpSocket
{
    Q_OBJECT
public:
    Player(QObject *parent=0);
    void messageUpdate(QByteArray);
    int roomNumber;
    int color;
signals:
    void playerQuitWhileWaiting(qintptr);
    void playerQuitWhilePlaying(int);
    void instructionReceived(int,int,int,int,int);
    void receiveEnterReguest(qintptr,int);
private slots:
    void messageReceived();
    void slotDisconnected();
};

#endif // PLAYER_H
