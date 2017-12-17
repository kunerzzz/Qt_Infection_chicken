#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QList>
#include <QTcpServer>
#include <player.h>
#include <game.h>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(int port=0);
private:
    QList<Game*> GameList;
    QList<Player*> waitingPlayerList;
protected:
    void incomingConnection(qintptr);
public slots:
    void receiveEnterReguest(qintptr,int);
    void playQuitWhileWaiting(qintptr);
    void slotGameDelete(int);
};

#endif // SERVER_H
