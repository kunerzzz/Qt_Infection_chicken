#include "server.h"
#include <QHostAddress>

Server::Server(int port) : QTcpServer()
{
    if(listen(QHostAddress::Any,port))
        printf("Server created! port:%d\n",port);
    else printf("Failed!\n");
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    Player *player=new Player(this);
    printf("A player enter\n");
    player->setSocketDescriptor(socketDescriptor);
    waitingPlayerList.append(player);
    connect(player,SIGNAL(receiveEnterReguest(qintptr,int)),this,SLOT(receiveEnterReguest(qintptr,int)));
    connect(player,SIGNAL(playerQuitWhileWaiting(qintptr)),this,SLOT(playQuitWhileWaiting(qintptr)));
    printf("WaitingPlayer count:%d\n",waitingPlayerList.count());
}

void Server::playQuitWhileWaiting(qintptr socketDescriptor)
{
    for(int i=0;i<waitingPlayerList.count();i++)
    {
        Player *item=waitingPlayerList.at(i);
        if(item->socketDescriptor()==socketDescriptor)
        {
            waitingPlayerList.removeAt(i);
            //delete item;
            break;
        }
    }
    printf("WaitingPlayer count:%d\n",waitingPlayerList.count());
}

void Server::receiveEnterReguest(qintptr socketDescriptor,int roomNumber)
{
    Player *item=NULL;
    printf("A Client try to enter Room %d\n",roomNumber);
    for(int i=0;i<waitingPlayerList.count();i++)
    {
        item=waitingPlayerList.at(i);
        if(item->socketDescriptor()==socketDescriptor)
        {
            waitingPlayerList.removeAt(i);
            break;
        }
    }
    bool exist=false;
    for(int i=0;i<GameList.count();i++)
    {
        Game *gameItem=GameList.at(i);
        if(gameItem->roomNumber==roomNumber)
        {
            exist=true;
            if(!gameItem->block)
            {
                gameItem->white=item;
                gameItem->block=true; //Block the room after two players entered
                item->color=-1;
                item->roomNumber=roomNumber;
                char msg[2]={1,0};
                item->write(msg,1);
                printf("send:%d\n",msg[0]);
                connect(item,SIGNAL(playerQuitWhilePlaying(int)),gameItem,SLOT(slotPlayerQuit(int)));
                connect(item,SIGNAL(instructionReceived(int,int,int,int,int)),gameItem,SLOT(procStep(int,int,int,int,int)));
                gameItem->ReadyToStart();
                return;
            }
            else
            {
                char msg[2]={2,0};
                //room is full
                waitingPlayerList.append(item);
                item->write(msg,1);
                printf("send:%d\n",msg[0]);
                printf("WaitingPlayer count:%d\n",waitingPlayerList.count());
                return;
            }
        }
    }
    if(!exist)
    {
        Game *gameItem = new Game();
        gameItem->roomNumber=roomNumber;
        gameItem->black=item;
        GameList.append(gameItem);
        item->color=1;
        item->roomNumber=roomNumber;
        connect(item,SIGNAL(playerQuitWhilePlaying(int)),gameItem,SLOT(slotPlayerQuit(int)));
        connect(item,SIGNAL(instructionReceived(int,int,int,int,int)),gameItem,SLOT(procStep(int,int,int,int,int)));
        connect(gameItem,SIGNAL(gameDelete(int)),this,SLOT(slotGameDelete(int)));
        char msg[2]={0,0};
        if((item->write(msg,1))==1)
            printf("send:%d\n",msg[0]);
        return;
    }
}

void Server::slotGameDelete(int roomNumber)
{
    Game* gameItem;
    for(int i=0;i<GameList.count();i++)
    {
        gameItem=GameList.at(i);
        if(gameItem->roomNumber==roomNumber)
        {
            GameList.removeAt(i);
            delete gameItem;
        }
    }
}
