#include "myplaylist.h"
#include <QContextMenuEvent>
#include <QMenu>

MyPlaylist::MyPlaylist(QWidget *parent) :
    QTableWidget(parent)
{
    setWindowTitle(tr("�����б�"));
    setWindowFlags(Qt::Window | Qt::WindowTitleHint);

    resize(320, 400);
    setMaximumWidth(320);
    setMinimumWidth(320);

    setRowCount(0);
    setColumnCount(3);

    QStringList list;
    list<<tr("����")<<tr("������")<<tr("����");
    setHorizontalHeaderLabels(list);

    //����ֻ��ѡ����
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setShowGrid(false);

}

void MyPlaylist::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(tr("����б�"), this, SLOT(clearPlaylist()));
    menu.exec(event->globalPos());
}

void MyPlaylist::clearPlaylist()
{
    while(rowCount())
        removeRow(0);
    emit playlistClean();
}

void MyPlaylist::closeEvent(QCloseEvent *event)
{
    if(isVisible())
    {
        hide();
        event->ignore();
    }
}
