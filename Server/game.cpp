#include "game.h"
#include <cstring>
#include <cmath>
Game::Game()
{
    printf("Game Created! RoomNumber:%d\n",roomNumber);
    memset(gridInfo,0,sizeof(gridInfo));
    black=NULL;
    white=NULL;
    currColor=0;
    block=false;
    inGaming=false;
}

bool inMap(int x,int y)
{
    return x>=0&&x<7&&y>=0&&y<7;
}

Game::~Game()
{
    printf("Game Deleted! RoomNumber:%d\n",roomNumber);
    if(black!=NULL)
        black->disconnectFromHost();
    if(white!=NULL)
        white->disconnectFromHost();
}

void Game::procStep(int color, int beginX, int beginY, int endX, int endY)
{
    if(color!=currColor||gridInfo[beginX][beginY]!=color||gridInfo[endX][endY]!=0||abs(beginX-endX)>2||abs(beginY-endY)>2)
    {
        char msg[4]={9,9,9,9};
        //game error code:9
        inGaming=false;
        black->write(msg,4);
        white->write(msg,4);
        return;
    }
    else
    {
        if(abs(endX-beginX)>1||abs(endY-beginY)>1)
            gridInfo[beginX][beginY]=0;
        gridInfo[endX][endY]=color;
        for(int i=-1;i<=1;i++)
            for(int j=-1;j<=1;j++)
                if(inMap(endX+i,endY+j)&&gridInfo[endX+i][endY+j])
                    gridInfo[endX+i][endY+j]=color;
        char msg[4];
        msg[0]=beginX+20;
        msg[1]=beginY+20;
        msg[2]=endX+20;
        msg[3]=endY+20;
        if(color==1)
           white->write(msg,4);
        else black->write(msg,4);
        printf("%d:%d (%d,%d)->(%d,%d)\n",roomNumber,color,beginX,beginY,endX,endY);
        /*for(int i=0;i<7;i++)
        {
            for(int j=0;j<7;j++)
               printf("%3d",gridInfo[j][i]);
            printf("\n");
        }*/
        changeColor();
        return;
    }
}

void Game::changeColor()
{
    int whiteNum=0,blackNum=0;
    bool whiteCanMove=0,blackCanMove=0;
    for(int i=0;i<7;i++)
        for(int j=0;j<7;j++)
        {
            if(gridInfo[i][j]==1)
            {
                blackNum++;
                for(int x=-2;x<=2&&!blackCanMove;x++)
                    for(int y=-2;y<=2&&!blackCanMove;y++)
                        if(inMap(i+x,j+y)&&gridInfo[i+x][j+y]==0)
                            blackCanMove=1;
            }
            if(gridInfo[i][j]==-1)
            {
                whiteNum++;
                for(int x=-2;x<=2&&!whiteCanMove;x++)
                    for(int y=-2;y<=2&&!whiteCanMove;y++)
                        if(inMap(i+x,j+y)&&gridInfo[i+x][j+y]==0)
                            whiteCanMove=1;
            }
        }
    char msg[4]={8,8,8,8};
    if(!blackCanMove)
    {
        msg[0]=8;   //game over code:8
        if(blackNum<49-blackNum)
            msg[1]=2;   //2 means white
        else msg[1]=1;   //1 means black
        inGaming=false;
        black->write(msg,4);
        white->write(msg,4);
        return;
    }
    else if(!whiteCanMove)
    {
        msg[0]=8;
        if(whiteNum<49-whiteNum)
            msg[1]=1;
        else msg[1]=2;
        inGaming=false;
        black->write(msg,4);
        white->write(msg,4);
        return;
    }
    currColor=-currColor;
    return;
}

void Game::ReadyToStart()
{
    if(white&&black)
    {
        gridInfo[0][0]=gridInfo[6][6]=1;
        gridInfo[0][6]=gridInfo[6][0]=-1;
        /*for(int i=0;i<7;i++)
        {
            for(int j=0;j<7;j++)
               printf("%3d",gridInfo[j][i]);
            printf("\n");
        }*/
        currColor=1;
        char msg[4]={7,7,7,7};   //game start code:7
        white->write(msg,4);
        black->write(msg,4);
        inGaming=true;
        return;
    }
}

void Game::slotPlayerQuit(int color)
{
    if(color==1)
        black=NULL;
    if(color==-1)
        white=NULL;
    if(inGaming)
    {
        char msg[4]={10,10,10,10};  //competitor quit code:10
        if(color==1&&white)
        {
            white->write(msg);
        }
        if(color==-1&&black)
        {
            black->write(msg);
        }
    }
    if(black==NULL&&white==NULL)
        emit gameDelete(roomNumber);
}
