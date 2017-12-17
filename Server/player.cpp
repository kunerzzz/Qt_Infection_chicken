#include "player.h"


Player::Player(QObject *parent)
{
    roomNumber=0;
    color=0;
    printf("A player created!\n");
    connect(this,SIGNAL(readyRead()),this,SLOT(messageReceived()));
    printf("First connection\n");
    connect(this,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
    printf("Second connection\n");
}

void Player::slotDisconnected()
{
    if(!roomNumber)
        emit playerQuitWhileWaiting(this->socketDescriptor());
    else emit playerQuitWhilePlaying(this->color);
}

void Player::messageReceived()
{
    if(roomNumber&&color)
    {
        int beginX,beginY,endX,endY;
        char msg[4];
        read(msg,4);
        beginX=msg[0]-20;
        beginY=msg[1]-20;
        endX=msg[2]-20;
        endY=msg[3]-20;
        emit instructionReceived(color,beginX,beginY,endX,endY);
        return;
    }
    else
    {
        char *msg;
        msg=new char;
        read(msg,1);
        printf("receive data:%d\n",*msg);
        emit receiveEnterReguest(this->socketDescriptor(),(int)*msg);
        return;
    }
}
