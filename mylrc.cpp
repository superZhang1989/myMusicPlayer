#include "mylrc.h"
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>

MyLrc::MyLrc(QWidget *parent) :
    QLabel(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    setAttribute(Qt::WA_TranslucentBackground);
    setText(tr("MiNi播放器--张超作品"));

    setMaximumSize(800, 60);
    setMinimumSize(800, 60);

    //歌词的线性渐变填充
    linearGradient.setStart(0, 10);
    linearGradient.setFinalStop(0, 40);
    linearGradient.setColorAt(0.1, QColor(14, 179, 255));
    linearGradient.setColorAt(0.5, QColor(114, 32, 255));
    linearGradient.setColorAt(0.9, QColor(14, 179, 255));

    //遮罩的线性渐变填充
    masklinearGradient.setStart(0, 10);
    masklinearGradient.setFinalStop(0, 40);
    masklinearGradient.setColorAt(0.1, QColor(222, 54, 4));
    masklinearGradient.setColorAt(0.5, QColor(222, 72, 16));
    masklinearGradient.setColorAt(0.9, QColor(222, 54, 4));

    //设置字体
    font.setFamily("Times New Roman");
    font.setBold(true);
    font.setPointSize(30);

    //设定定时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    lrcMaskWidth = 0;
    lrcMaskWidthInterval = 0;
}

void MyLrc::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(font);

    //绘制底层文字
    painter.setPen(QColor(0, 0, 0, 200));
    painter.drawText(1, 1, 800, 60, Qt::AlignLeft, text());

    //绘制上面渐变文字
    painter.setPen(QPen(linearGradient, 0));
    painter.drawText(0, 0, 800, 60, Qt::AlignLeft, text());

    //设置歌词遮罩
    painter.setPen(QPen(masklinearGradient, 0));
    painter.drawText(0, 0, lrcMaskWidth, 60, Qt::AlignLeft, text());
}

void MyLrc::startLrcMask(qint64 intervalTime)
{
    qreal count = intervalTime / 30;

    lrcMaskWidthInterval = 800 / count;
    lrcMaskWidth = 0;
    timer->start(30);
}

void MyLrc::stopLrcMask()
{
    timer->stop();
    lrcMaskWidth = 0;
    update();
}

void MyLrc::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        offset = event->globalPos() - frameGeometry().topLeft();
}

void MyLrc::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        setCursor(Qt::PointingHandCursor);
        move(event->globalPos() - offset);
    }
}

void MyLrc::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(tr("隐藏"), this, SLOT(hide()));
    menu.exec(event->globalPos());
}

void MyLrc::timeout()
{
    lrcMaskWidth += lrcMaskWidthInterval;
    update();
}










