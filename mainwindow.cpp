#include "mainwindow.h"
#include <QMessageBox>
#include <cmath>
#include <QInputDialog>
#include <QFile>
#include <QDataStream>
#include <QDate>
#include <QTime>
#include <QApplication>

#define LEFTBORDER 50
#define TOPBORDER 50
#define INTERVAL 110

int MAX_DEPTH=3;
int MousePassX=-1,MousePassY=-1;
int SelectX=-1,SelectY=-1;
int GridInfo[7][7];
int SelectStep=0;
int turn=0;
int GameMode=0;//0--初始；1--PVP；2--PVE Black；3--PVE White；4--PvP Network；5--尝试连接
int bestStartX,bestStartY,bestEndX,bestEndY;
bool needWarning=1;
bool needWaiting=0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("同化鸡");
    setWindowIcon(QIcon(QStringLiteral(":/images/chicken_64X64.ico")));
    Color=0;
    roomNumber=0;
    empty=new QImage;
    emptyGreen=new QImage;
    emptyYellow=new QImage;
    black=new QImage;
    white=new QImage;
    whiteSelected=new QImage;
    blackSelected=new QImage;
    blackInfo=new QImage;
    whiteInfo=new QImage;
    AITarget=new QImage;
    AIBlack=new QImage;
    AIWhite=new QImage;
    empty->load(":/images/empty.png");
    emptyGreen->load(":/images/emptyGreen.png");
    emptyYellow->load(":/images/emptyYellow.png");
    black->load(":/images/black.png");
    white->load(":/images/white.png");
    whiteSelected->load(":/images/whiteSelected.png");
    blackSelected->load(":/images/blackSelected.png");
    whiteInfo->load(":/images/whiteInfo.png");
    blackInfo->load(":/images/blackInfo.png");
    AITarget->load(":/images/AITarget.png");
    AIBlack->load(":/images/AIBlack.png");
    AIWhite->load(":/images/AIWhite.png");

    blackImage=new QLabel(this);
    blackImage->setGeometry(870,75,100,100);
    blackImage->setPixmap(QPixmap::fromImage(*blackInfo));
    PvPStart=new QPushButton(this);
    whiteImage=new QLabel(this);
    whiteImage->setGeometry(1040,75,100,100);
    whiteImage->setPixmap(QPixmap::fromImage(*whiteInfo));

    QFont ft("Microsoft YaHei",30);
    blackNum=new QLabel(this);
    blackNum->setGeometry(870,185,100,70);
    blackNum->setFont(ft);
    blackNum->setAlignment(Qt::AlignCenter);
    blackNum->setText("0");
    whiteNum=new QLabel(this);
    whiteNum->setGeometry(1040,185,100,70);
    whiteNum->setFont(ft);
    whiteNum->setAlignment(Qt::AlignCenter);
    whiteNum->setText("0");

    GameInfo1=new QLabel(this);
    ft.setPointSize(25);
    GameInfo1->setFont(ft);
    GameInfo1->setGeometry(840,310,320,70);
    GameInfo1->setAlignment(Qt::AlignCenter);
    GameInfo1->setText("同化鸡");
    GameInfo2=new QLabel(this);
    ft.setPointSize(20);
    GameInfo2->setFont(ft);
    GameInfo2->setGeometry(840,380,320,140);
    GameInfo2->setAlignment(Qt::AlignCenter);
    GameInfo2->setText("KunerStudio");

    tm=new QTimer(this);
    connect(tm,SIGNAL(timeout()),this,SLOT(tmTimeout()));
    connectTimer=new QTimer(this);
    connect(connectTimer,SIGNAL(timeout()),this,SLOT(connectTimeOut()));

    serverIP=new QHostAddress;
    port=2333;
    //serverIP->setAddress("128.199.82.227");
    serverIP->setAddress("159.89.243.21");
    tcpSocket=new QTcpSocket(this);
    connect(tcpSocket,SIGNAL(connected()),this,SLOT(slotConnect()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(dataReceived()));

    PvPStart->setGeometry(900,530,200,80);
    PvPStart->setText("新的PVP对局");
    PvEStart=new QPushButton(this);
    PvEStart->setGeometry(900,630,200,80);
    PvEStart->setText("新的人机对局");
    Help=new QPushButton(this);
    Help->setGeometry(900,730,200,80);
    Help->setText("查看游戏帮助");
    for(int i=0;i<7;i++)
        for(int j=0;j<7;j++)
        {
            point[i][j]=new QLabel(this);
            point[i][j]->setGeometry(INTERVAL*i+LEFTBORDER,INTERVAL*j+TOPBORDER,100,100);
            point[i][j]->setMouseTracking(true);
        }
    setFixedSize(1200,865);
    this->setMouseTracking(true);
    connect(PvPStart,SIGNAL(clicked()),this,SLOT(PvP()));
    connect(PvEStart,SIGNAL(clicked()),this,SLOT(PvE()));
    connect(Help,SIGNAL(clicked(bool)),this,SLOT(GameHelp()));
    RecordLoad();
    GridDisplay();
}

MainWindow::~MainWindow()
{
}

void MainWindow::GameHelp()
{
    QMessageBox::about(this,tr("帮助"),tr("同化棋规则：\n"
                                 "在7*7的棋盘内，对局双方分别执黑白子。\n"
                                 "游戏开始时，在棋盘角落分别有黑白棋子各两颗。"
                                 "每回合，玩家选择一颗己方棋子，并将其复制到相距一格的空位或移动到相距两格的空位,"
                                 "棋子落下后，将使相邻八个位置内的对手棋子变成己方棋子。\n"
                                 "一方无法落子时，跳过该回合，双方均无法落子时，游戏结束，棋子数量多者胜。\n\n注：程序将自动保存棋局信息，除非你删除了名为\"Record.chicken\"的文件\n"
                                 "\n"
                                 "人机和联网对局说明：\n"
                                 "红框标注的棋子和空位为对手本回合着法\n\n"
                                 "注意：人机最高难度可能会造成程序卡顿，请耐心等待AI运算\n"
                                 "\n"
                                 "联网对局中，先进入房间者执黑\n"
                                 "\n"
                                 "Made by KunerStudio\n"
                                 "Powered by Qt"));
}

void MainWindow::RecordLoad()
{
    QFile Record("Record.chicken");
    if(Record.open(QIODevice::ReadOnly)&&QMessageBox::warning(this,
                                                   tr("恢复棋局"),tr("检测到未完成的棋局，是否恢复？"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Ok)
    {
        QDataStream in(&Record);
        in>>GameMode>>turn>>SelectStep;
        if(GameMode==2||GameMode==3)
            in>>MAX_DEPTH;
        switch (GameMode) {
        case 1:
            GameInfo2->setText("PvP");
            break;
        case 2:
            GameInfo2->setText("PvE 人类执黑\n难度:"+QString::number(MAX_DEPTH));
            break;
        case 3:
            GameInfo2->setText("PvE 人类执白\n难度:"+QString::number(MAX_DEPTH));
            break;
        }
        for(int i=0;i<7;i++)
            for(int j=0;j<7;j++)
                in>>GridInfo[i][j];
        Record.close();
        GameCore();
    }
}

void MainWindow::RecordSave()
{
    QFile Record("Record.chicken");
    if(!Record.exists())
    {
        Record.open(QIODevice::ReadWrite);
        Record.close();
    }
    Record.open(QIODevice::ReadWrite|QIODevice::Truncate);
    QDataStream out(&Record);
    out<<GameMode<<turn<<SelectStep;
    if(GameMode==2||GameMode==3)
        out<<MAX_DEPTH;
    for(int i=0;i<7;i++)
        for(int j=0;j<7;j++)
            out<<GridInfo[i][j];
    Record.close();
}

bool InGrid(int x,int y)
{
    return x>=0&&x<7*INTERVAL&&y>=0&&y<7*INTERVAL&&x%INTERVAL<=100&&y%INTERVAL<=100;
}

bool inMap(int x,int y)
{
    return x>=0&&x<7&&y>=0&&y<7;
}

void ProcStep(int startX,int startY,int endX,int endY)
{
    if(abs(endX-startX)>1||abs(endY-startY)>1)
        GridInfo[startX][startY]=0;
    GridInfo[endX][endY]=turn;
    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
            if(inMap(endX+i,endY+j)&&GridInfo[endX+i][endY+j])
                GridInfo[endX+i][endY+j]=turn;
}

bool dfsProcStep(int startX,int startY,int endX,int endY,int currBotColor,int tempBoard[7][7])
{
    if(tempBoard[endX][endY])
        return 0;
    else tempBoard[endX][endY]=currBotColor;
    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
            if(inMap(endX+i,endY+j)&&tempBoard[endX+i][endY+j])
                tempBoard[endX+i][endY+j]=currBotColor;
    if(abs(endX-startX)>1||abs(endY-startY)>1)
        tempBoard[startX][startY]=0;
    return 1;
}

int Onetry(int currBotColor,int lastBoard[7][7],int depth,int alpha)
{
    int beta=99999;
    if(depth>MAX_DEPTH)
    {
        currBotColor=-currBotColor;  //此时并非换手落子，仅是统计分数，而传参时换手， 故还原
        int beatNum=0,totnum=0;
        for(int i=0;i<7;i++)
            for(int j=0;j<7;j++)
            {
                beatNum+=lastBoard[i][j];
                totnum+=(bool)lastBoard[i][j];
            }
        beatNum*=currBotColor;
        if(totnum==49) return beatNum>0?-9999:9999;
        return -beatNum;
    }
    int tempBoard[7][7];
    int currColorAva[49][2];
    int currColorAvaNum=0;
    bool CanMove=false;
    for(int i=0;i<7;i++)
        for(int j=0;j<7;j++)
        {
            if(lastBoard[i][j]==currBotColor)
            {
                currColorAva[currColorAvaNum][0]=i;
                currColorAva[currColorAvaNum++][1]=j;
            }
        }
    int Value,MinValue=99999;
    for(int i=0;i<currColorAvaNum;i++)
        for(int x=-2;x<=2;x++)
            for(int y=-2;y<=2;y++)
            {
                for(int m=0;m<7;m++)
                    for(int n=0;n<7;n++)
                        tempBoard[m][n]=lastBoard[m][n];
                if(inMap(currColorAva[i][0]+x,currColorAva[i][1]+y))
                    if(dfsProcStep(currColorAva[i][0],currColorAva[i][1],currColorAva[i][0]+x,currColorAva[i][1]+y,currBotColor,tempBoard))
                    {
                        CanMove=true;
                        Value=Onetry(-currBotColor,tempBoard,depth+1,-beta);
                        if(Value<alpha) return -Value;
                        if(Value<beta) beta=Value;
                        if(Value<MinValue)
                        {
                            MinValue=Value;
                            if(depth==1)
                            {
                                bestStartX=currColorAva[i][0];
                                bestStartY=currColorAva[i][1];
                                bestEndX=currColorAva[i][0]+x;
                                bestEndY=currColorAva[i][1]+y;
                            }
                        }
                    }
            }
    if(!CanMove)
    {
        for(int i=0;i<7;i++)
            for(int j=0;j<7;j++)
                tempBoard[i][j]=lastBoard[i][j];
        return Onetry(-currBotColor,tempBoard,depth+1,-beta);
    }
    return -MinValue;
}

void MainWindow::AIturn()
{
    QString temp=GameInfo1->text();
    GameInfo1->setText("AI运算中...");
    this->repaint();
    Onetry(turn,GridInfo,1,-99999);
    GameInfo1->setText(temp);
    SelectX=bestStartX;
    SelectY=bestStartY;
    if(turn==1)
        point[SelectX][SelectY]->setPixmap(QPixmap::fromImage(*AIBlack));
    if(turn==-1)
        point[SelectX][SelectY]->setPixmap(QPixmap::fromImage(*AIWhite));
        point[bestEndX][bestEndY]->setPixmap(QPixmap::fromImage(*AITarget));
    tm->start(500);
}

void MainWindow::tmTimeout()
{
    tm->stop();
    SelectX=-1;
    SelectY=-1;
    ProcStep(bestStartX,bestStartY,bestEndX,bestEndY);
    GridDisplay();
    needWaiting=false;
    GameCore();
}

void MainWindow::GameCore()
{
    if(GameMode!=4)
        MainWindow::RecordSave();
    bool BlackAva=0,WhiteAva=0;
    int Black=0,White=0;
    int i,j;
    for(i=0;i<7;i++)
        for(j=0;j<7;j++)
            if(GridInfo[i][j]==1)
            {
                Black++;
                for(int x=-2;x<=2&&!BlackAva;x++)
                    for(int y=-2;y<=2&&!BlackAva;y++)
                        if(inMap(i+x,j+y)&&GridInfo[i+x][j+y]==0)
                            BlackAva=1;
            }
    for(i=0;i<7;i++)
        for(j=0;j<7;j++)
            if(GridInfo[i][j]==-1)
            {
                White++;
                for(int x=-2;x<=2&&!WhiteAva;x++)
                    for(int y=-2;y<=2&&!WhiteAva;y++)
                        if(inMap(i+x,j+y)&&GridInfo[i+x][j+y]==0)
                            WhiteAva=1;
            }
    blackNum->setText(QString::number(Black));
    whiteNum->setText(QString::number(White));
    if(GameMode!=4){
    if(!BlackAva)
    {
        turn=0;
        GameMode=0;
        SelectStep=0;
        if(49-Black>Black)
            GameInfo1->setText("白方获胜");
        else GameInfo1->setText("黑方获胜");
        GameInfo2->setText("Winner Winer!\nChicken dinner!");
        QFile::remove("Record.chicken");
        return;
    }
    if(!WhiteAva)
    {
        turn=0;
        GameMode=0;
        SelectStep=0;
        if(49-White<White)
            GameInfo1->setText("白方获胜");
        else GameInfo1->setText("黑方获胜");
        GameInfo2->setText("Winner Winer!\nChicken dinner!");
        QFile::remove("Record.chicken");
        return;
    }
    }
    turn=-turn;
    if(GameMode==4)
    {
        if(turn==Color)
            GameInfo1->setText("你的回合");
        else
            GameInfo1->setText("对手回合");
    }
    else
    {
        if(turn==1)
            GameInfo1->setText("黑方行棋");
        if(turn==-1)
            GameInfo1->setText("白方行棋");
    }
    if((GameMode==2&&turn==-1)||(GameMode==3&&turn==1))
    {
        SelectStep=0;
        AIturn();
    }
    else if(GameMode==4&&turn!=Color)
        SelectStep=0;
    else SelectStep=1;
    return;
}

void MainWindow::PvP()
{
    if(GameMode==0||
            QMessageBox::warning(this,tr("警告"),tr("开始新的对局将使当前棋局丢失，确定吗？"),
                                 QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel)==QMessageBox::Ok)
    {
        if(GameMode==4)
        {
            needWarning=false;
            tcpSocket->disconnectFromHost();
            GameInfo1->setText("对局终止");
        }
        GameMode=0;
        SelectStep=0;
        turn=0;
        SelectX=SelectY=-1;
        QFile::remove("Record.chicken");
        for(int i=0;i<7;i++)
            for(int j=0;j<7;j++)
                GridInfo[i][j]=0;
        GridDisplay();
        blackNum->setText("0");
        whiteNum->setText("0");
        GameInfo1->setText("同化鸡");
        GameInfo2->setText("KunerStudio");
        bool ok;
        roomNumber=QInputDialog::getInt(this,
                                    tr("联网对局"),tr("请输入房间号(联网:1-255,本地:0):"),
                                                  0,0,255,1,&ok);
        if(!ok)
            return;
        QFile::remove("Record.chicken");
        if(roomNumber==0)
        {
            GameMode=1;
            SelectStep=1;
            turn=1;
            GridInfo[0][0]=GridInfo[6][6]=1;
            GridInfo[0][6]=GridInfo[6][0]=-1;
            blackNum->setText("2");
            whiteNum->setText("2");
            GameInfo1->setText("黑方行棋");
            GameInfo2->setText("PvP 本地");
            GridDisplay();
        }
        else
        {
            GameMode=5;
            PvPStart->setEnabled(false);
            PvEStart->setEnabled(false);
            tcpSocket->connectToHost(*serverIP,port);
            connectTimer->start(3500);
            GameInfo1->setText(tr("尝试连接..."));
        }
    }
}

void MainWindow::connectTimeOut()
{
    connectTimer->stop();
    QMessageBox::information(this,tr("连接失败"),tr("连接服务器失败，请检查网络连接或联系服务器管理员"));
    GameInfo1->setText(tr("同化鸡"));
    GameInfo2->setText(tr("KunerStudio"));
    PvPStart->setEnabled(true);
    PvEStart->setEnabled(true);
    GameMode=0;
}

void MainWindow::slotConnect()
{
    connectTimer->stop();
    GameMode=4;
    PvPStart->setEnabled(true);
    PvEStart->setEnabled(true);
    char *msg;
    msg=new char;
    *msg=roomNumber;
    tcpSocket->write(msg,1);
}

void MainWindow::dataReceived()
{
    while(tcpSocket->bytesAvailable()>0){
    if(Color)
    {
        char msg[4];
        tcpSocket->read(msg,4);
        //GameInfo1->setText(QString::number((int)msg[0])+" "+QString::number((int)msg[1])+" "+QString::number((int)msg[2])+" "+QString::number((int)msg[3]));
        if(msg[0]==10)
        {
            QMessageBox::information(this,"对方断线","对手与服务器断开连接，游戏结束");
            GameInfo1->setText("对局终止");
            needWarning=false;
            tcpSocket->disconnectFromHost();
        }
        else if(msg[0]==9)
        {
            QMessageBox::warning(this,"异常","对局状态异常，游戏强行终止\n"
                                           "（这可能由网络异常导致）");
            GameInfo1->setText("对局终止");
            needWarning=false;
            tcpSocket->disconnectFromHost();
        }
        else if(msg[0]==8)
        {
            if(needWaiting)
            {
                QTime reachTime=QTime::currentTime().addMSecs(600);
                while(QTime::currentTime()<=reachTime)
                    QApplication::processEvents(QEventLoop::AllEvents);
            }
            int Black=0,White=0,i,j;
            if((msg[1]==1&&Color==1)||(msg[1]==2&&Color==-1))
            {
                GameInfo1->setText("胜利！");
                GameInfo2->setText("Winner Winer!\nChicken dinner!");
            }
            else
            {
                GameInfo1->setText("失败！");
                GameInfo2->setText("Good luck\nnext time");
            }
            for(i=0;i<7;i++)
                for(j=0;j<7;j++)
                    if(GridInfo[i][j]==1)
                    {
                        Black++;
                    }
            for(i=0;i<7;i++)
                for(j=0;j<7;j++)
                    if(GridInfo[i][j]==-1)
                    {
                        White++;
                    }
            blackNum->setText(QString::number(Black));
            whiteNum->setText(QString::number(White));
            needWarning=false;
            tcpSocket->disconnectFromHost();
        }
        else if(msg[0]==7)
        {
            QString yourColor;
            turn=1;
            if(Color==1)
            {
                yourColor="黑";
                SelectStep=1;
            }
            else yourColor="白";
            if(Color==1)
                GameInfo1->setText("你的回合");
            else
                GameInfo1->setText("对手回合");
            GridInfo[0][0]=GridInfo[6][6]=1;
            GridInfo[0][6]=GridInfo[6][0]=-1;
            blackNum->setText("2");
            whiteNum->setText("2");
            GridDisplay();
            QMessageBox::information(this,"对局开始","对局开始！你所执子颜色为"+yourColor);
        }
        else
        {
            //借用指示AI着法的工具指示对手着法
            //此处减20的原因为与服务器的数据交换加了20（防止可能的传输0错误）
            bestStartX=msg[0]-20;
            bestStartY=msg[1]-20;
            bestEndX=msg[2]-20;
            bestEndY=msg[3]-20;
            SelectX=bestStartX;
            SelectY=bestStartY;
            if(turn==1)
                point[SelectX][SelectY]->setPixmap(QPixmap::fromImage(*AIBlack));
            if(turn==-1)
                point[SelectX][SelectY]->setPixmap(QPixmap::fromImage(*AIWhite));
            point[bestEndX][bestEndY]->setPixmap(QPixmap::fromImage(*AITarget));
            tm->start(500);
            needWaiting=true;
        }
    }
    else
    {
        char *msg=new char;
        tcpSocket->read(msg,1);
        if(*msg==0)
        {
            Color=1;
            GameInfo1->setText(tr("等待对手..."));
            GameInfo2->setText(tr("PvP 房间号:")+QString::number(roomNumber));
        }
        if(*msg==1)
        {
            Color=-1;
            GameInfo2->setText(tr("PvP 房间号:")+QString::number(roomNumber));
        }
        if(*msg==2)
        {
            QMessageBox::warning(this,tr("进入房间失败"),tr("此房间有正在进行的对局，请重试"));
            needWarning=false;
            tcpSocket->disconnectFromHost();
            GameInfo1->setText("同化鸡");
        }
    }
    }
}

void MainWindow::slotDisconnect()
{
    SelectStep=0;
    GameMode=0;
    Color=0;
    roomNumber=0;
    turn=0;
    SelectX=SelectY=-1;
    if(needWarning)
    {
        QMessageBox::information(this,tr("断开连接"),tr("与服务器的连接异常终止，游戏结束"));
        GameInfo1->setText("对局终止");
    }
    else needWarning=true;
}

void MainWindow::PvE()
{
    if(GameMode==0||
            QMessageBox::warning(this,tr("警告"),tr("开始新的对局将使当前棋局丢失，确定吗？"),
                                 QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel)==QMessageBox::Ok)
    {

        if(GameMode==4)
        {
            needWarning=false;
            tcpSocket->disconnectFromHost();
            GameInfo1->setText("对局终止");
        }
        GameMode=0;
        SelectStep=0;
        turn=0;
        SelectX=SelectY=-1;
        QFile::remove("Record.chicken");
        for(int i=0;i<7;i++)
            for(int j=0;j<7;j++)
                GridInfo[i][j]=0;
        GridDisplay();
        blackNum->setText("0");
        whiteNum->setText("0");
        GameInfo1->setText("同化鸡");
        GameInfo2->setText("KunerStudio");
        QStringList AIitem;
        AIitem<<tr("人类执黑")<<tr("人类执白");
        bool ok;
        QString choice=QInputDialog::getItem(this,tr("选择位置"),tr("请选择人类的位置："),
                                             AIitem,0,false,&ok);
        if(ok && !choice.isEmpty())
        {
            bool ok;
            int Dif=QInputDialog::getInt(this,
                       tr("难度选择"),tr("请输入难度（1-5，默认为3）："),3,1,5,1,&ok);
            if(ok)
                MAX_DEPTH=Dif;
            else MAX_DEPTH=3;
            GridInfo[0][0]=GridInfo[6][6]=1;
            GridInfo[0][6]=GridInfo[6][0]=-1;
            GridDisplay();
            blackNum->setText("2");
            whiteNum->setText("2");
            turn=1;
            GameInfo1->setText("黑方行棋");
            if(choice=="人类执黑")
            {
                GameMode=2;
                SelectStep=1;
                GameInfo2->setText("PvE 人类执黑\n难度:"+QString::number(MAX_DEPTH));
            }
            if(choice=="人类执白")
            {
                GameMode=3;
                GameInfo2->setText("PvE 人类执白\n难度:"+QString::number(MAX_DEPTH));
                AIturn();
            }
        }
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    int x=e->x()-LEFTBORDER;
    int y=e->y()-TOPBORDER;
    if(InGrid(x,y))
    {
        x/=INTERVAL;
        y/=INTERVAL;
        if(MousePassX>=0&&MousePassY>=0&&!(SelectX==MousePassX&&SelectY==MousePassY))
        {
            if(GridInfo[MousePassX][MousePassY]==1)
                point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*black));
            if(GridInfo[MousePassX][MousePassY]==-1)
                point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*white));
        }
        MousePassX=x;
        MousePassY=y;
        if(GridInfo[MousePassX][MousePassY]==1)
            point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*blackSelected));
        if(GridInfo[MousePassX][MousePassY]==-1)
            point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*whiteSelected));
    }
    else if(MousePassX>=0&&MousePassY>=0&&!(SelectX==MousePassX&&SelectY==MousePassY))
    {
        if(GridInfo[MousePassX][MousePassY]==1)
            point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*black));
        if(GridInfo[MousePassX][MousePassY]==-1)
            point[MousePassX][MousePassY]->setPixmap(QPixmap::fromImage(*white));
        MousePassX=-1;
        MousePassY=-1;
    }
}

void MainWindow::GridDisplay()
{
    for(int i=0;i<7;i++)
        for(int j=0;j<7;j++)
        {
            switch(GridInfo[i][j])
            {
            case 0:point[i][j]->setPixmap(QPixmap::fromImage(*empty));break;
            case 1:point[i][j]->setPixmap(QPixmap::fromImage(*black));break;
            case -1:point[i][j]->setPixmap(QPixmap::fromImage(*white));break;
            }
        }
    this->repaint();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    int x=e->x()-LEFTBORDER;
    int y=e->y()-TOPBORDER;
    if(InGrid(x,y))
    {
        x/=INTERVAL;
        y/=INTERVAL;
    }
    else return;
    if(SelectStep==1)
    {
        if(GridInfo[x][y]==turn)
        {
            SelectX=x;
            SelectY=y;
            for(int i=-2;i<=2;i++)
                for(int j=-2;j<=2;j++)
                {
                    if(inMap(x+i,y+j)&&GridInfo[x+i][y+j]==0)
                    {
                        if(abs(i)>1||abs(j)>1)
                            point[x+i][y+j]->setPixmap(QPixmap::fromImage(*emptyYellow));
                        else
                            point[x+i][y+j]->setPixmap(QPixmap::fromImage(*emptyGreen));
                    }
                }
            SelectStep=2;
            return;
        }
    }
    if(SelectStep==2)
    {
        if(GridInfo[x][y]==turn)
        {
            for(int i=-2;i<=2;i++)
                for(int j=-2;j<=2;j++)
                    if(inMap(SelectX+i,SelectY+j)&&GridInfo[SelectX+i][SelectY+j]==0)
                        point[SelectX+i][SelectY+j]->setPixmap(QPixmap::fromImage(*empty));
            point[SelectX][SelectY]->setPixmap((QPixmap::fromImage(turn==1?(*black):(*white))));
            SelectX=x;
            SelectY=y;
            for(int i=-2;i<=2;i++)
                for(int j=-2;j<=2;j++)
                {
                    if(inMap(x+i,y+j)&&GridInfo[x+i][y+j]==0)
                    {
                        if(abs(i)>1||abs(j)>1)
                            point[x+i][y+j]->setPixmap(QPixmap::fromImage(*emptyYellow));
                        else
                            point[x+i][y+j]->setPixmap(QPixmap::fromImage(*emptyGreen));
                    }
                }
            return;
        }
        if(GridInfo[x][y]==0&&abs(x-SelectX)<=2&&abs(y-SelectY)<=2)
        {
            for(int i=-2;i<=2;i++)
                for(int j=-2;j<=2;j++)
                    if(inMap(SelectX+i,SelectY+j)&&GridInfo[SelectX+i][SelectY+j]==0)
                        point[SelectX+i][SelectY+j]->setPixmap(QPixmap::fromImage(*empty));
            ProcStep(SelectX,SelectY,x,y);
            GridDisplay();
            if(GameMode==4)
            {
                char msg[4];
                msg[0]=SelectX+20;
                msg[1]=SelectY+20;
                msg[2]=x+20;
                msg[3]=y+20;
                tcpSocket->write(msg,4);
            }
            SelectX=-1;
            SelectY=-1;
            GridDisplay();
            GameCore();
        }
    }
}
