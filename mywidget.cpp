#include "mywidget.h"
#include "ui_mywidget.h"
#include "myplaylist.h"
#include<QLabel>
#include<QToolBar>
#include<QVBoxLayout>
#include<QTime>
#include<QMessageBox>
#include<QFileInfo>
#include<QFileDialog>
#include<QDesktopServices>

MyWidget::MyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyWidget)
{
    ui->setupUi(this);
    initPlayer();
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
}

MyWidget::~MyWidget()
{
    delete ui;
}

void MyWidget::initPlayer()
{
    setWindowTitle(tr("MiNi���ֲ�����"));
    setWindowIcon(QIcon(":/images/icon.png"));
    setMinimumSize(320, 160);
    setMaximumSize(320, 160);

    mediaObject = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(mediaObject, audioOutput);

    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(updataTime(qint64)));

    //����������ǩ
    topLabel = new QLabel(tr("MiNi������"));
    topLabel->setTextFormat(Qt::RichText);
    topLabel->setOpenExternalLinks(true);
    topLabel->setAlignment(Qt::AlignCenter);

    //�������Ʋ��Ž��ȵĻ���
    Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider(mediaObject, this);

    // �������������б�ͼ�ꡢ��ʾʱ���ǩ��������ͼ��Ĺ�����
    QToolBar *widgetBar = new QToolBar(this);
    timeLabel = new QLabel(tr("00:00 / 00:00"), this);
    timeLabel->setToolTip(tr("��ǰʱ�� / ��ʱ��"));
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // ����ͼ�꣬���ڿ����Ƿ���ʾ�����б�
    QAction *PLAction = new QAction(tr("�����б�"), this);
    PLAction->setShortcut(QKeySequence("F4"));
    PLAction->setToolTip(tr("�����б�(F4)"));
    connect(PLAction, SIGNAL(triggered()), this, SLOT(setPlaylistShown()));

    // ����ͼ�꣬���ڿ����Ƿ���ʾ������
    QAction *LRCAction = new QAction(tr("���"), this);
    LRCAction->setShortcut(QKeySequence("F2"));
    LRCAction->setToolTip(tr("������(F2)"));
    connect(LRCAction, SIGNAL(triggered()), this, SLOT(setLrcShown()));

    // ��ӵ�������
    widgetBar->addAction(PLAction);
    widgetBar->addSeparator();
    widgetBar->addWidget(timeLabel);
    widgetBar->addSeparator();
    widgetBar->addAction(LRCAction);

    //�������ſ��ƶ���������
    QToolBar *toolBar = new QToolBar(this);
    //����
    playAction = new QAction(this);
    playAction->setIcon(QIcon(":/images/play.png"));
    playAction->setText(tr("����(F5)"));
    playAction->setShortcut(QKeySequence(tr("F5")));
    connect(playAction, SIGNAL(triggered()), this, SLOT(setPaused()));

    //ֹͣ
    stopAction = new QAction(this);
    stopAction->setIcon(QIcon(":/images/stop.png"));
    stopAction->setText(tr("ֹͣ(F6)"));
    stopAction->setShortcut(QKeySequence(tr("F6")));
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));

    //��һ��
    skipBackwardAction = new QAction(this);
    skipBackwardAction->setIcon(QIcon(":/images/skipBackward.png"));
    skipBackwardAction->setText(tr("��һ��(Ctrl+Left)"));
    skipBackwardAction->setShortcut(QKeySequence(tr("Ctrl+Left")));
    connect(skipBackwardAction, SIGNAL(triggered()), this, SLOT(skipBackward()));

   //��һ��
    skipForwardAction = new QAction(this);
    skipForwardAction->setIcon(QIcon(":/images/skipForward.png"));
    skipForwardAction->setText(tr("��һ��(Ctrl+Right)"));
    skipForwardAction->setShortcut(QKeySequence(tr("Ctrl+Right")));
    connect(skipForwardAction, SIGNAL(triggered()), this, SLOT(skipForward()));

    //���ļ�
    QAction *openAction = new QAction(this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setText(tr("�����ļ�(Ctrl+O)"));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

    //��������
    Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider(audioOutput, this);
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // ��ӵ�������
    toolBar->addAction(playAction);
    toolBar->addSeparator();
    toolBar->addAction(stopAction);
    toolBar->addSeparator();
    toolBar->addAction(skipBackwardAction);
    toolBar->addSeparator();
    toolBar->addAction(skipForwardAction);
    toolBar->addSeparator();
    toolBar->addWidget(volumeSlider);
    toolBar->addSeparator();
    toolBar->addAction(openAction);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(topLabel);
    mainLayout->addWidget(seekSlider);
    mainLayout->addWidget(widgetBar);
    mainLayout->addWidget(toolBar);
    setLayout(mainLayout);

    //���������б�
    playlist = new MyPlaylist(this);
    connect(playlist, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableClicked(int)));
    connect(playlist, SIGNAL(playlistClean()), this, SLOT(clearSources()));

    //������������ý�����Ϣ��Ԫ��Ϣ������
    metaInformationResolver = new Phonon::MediaObject(this);
    Phonon::AudioOutput *metaInformationAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(metaInformationResolver, metaInformationAudioOutput);

    //����������Ϣ
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(metaStateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));

    //��ʼ������ͼ���״̬
    playAction->setEnabled(false);
    stopAction->setEnabled(false);
    skipBackwardAction->setEnabled(false);
    skipForwardAction->setEnabled(false);
    topLabel->setFocus();

}

void MyWidget::updataTime(qint64 time)
{
    qint64 totalTimeValue = mediaObject->totalTime();
    QTime totalTime(0, (totalTimeValue / 60000) % 60, (totalTimeValue / 1000) % 60);
    QTime currentTime(0, (time / 60000) % 60, (time / 1000) % 60);
    QString str = currentTime.toString("mm:ss") + " / " + totalTime.toString("mm:ss");
    timeLabel->setText(str);
}

void MyWidget::setPlaylistShown()
{
    if(playlist->isHidden())
    {
        playlist->move(frameGeometry().bottomLeft());
        playlist->show();
    }
    else
    {
        playlist->hide();
    }
}

void MyWidget::setPaused()
{
    if(mediaObject->state() == Phonon::PlayingState)
        mediaObject->pause();
    else
        mediaObject->play();
}

void MyWidget::skipBackward()
{
    int index = sources.indexOf(mediaObject->currentSource());
    mediaObject->setCurrentSource(sources.at(index - 1));
    mediaObject->play();
}

void MyWidget::skipForward()
{
    int index = sources.indexOf(mediaObject->currentSource());
    mediaObject->setCurrentSource(sources.at(index + 1));
    mediaObject->play();
}

void MyWidget::openFile()
{
    QStringList list = QFileDialog::getOpenFileNames(this, tr("�������ļ�"), QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    if(list.isEmpty())
        return;
    int index = sources.size();

    foreach(QString string, list)
    {
        Phonon::MediaSource source(string);
        sources.append(source);
    }
    if(!sources.isEmpty())
        metaInformationResolver->setCurrentSource(sources.at(index));
}

void MyWidget::setLrcShow()
{

}

void MyWidget::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    switch (newState)
        {
        case Phonon::ErrorState :
            if(mediaObject->errorType() == Phonon::FatalError) {
                QMessageBox::warning(this, tr("��������"),
                                     mediaObject->errorString());
            } else {
                QMessageBox::warning(this, tr("����"), mediaObject->errorString());
            }
            break;
        case Phonon::PlayingState :
            stopAction->setEnabled(true);
            playAction->setIcon(QIcon(":/images/pause.png"));
            playAction->setText(tr("��ͣ(F5)"));
            topLabel->setText(QFileInfo(mediaObject->
                                        currentSource().fileName()).baseName());
            //������ǰý��Դ��Ӧ�ĸ���ļ�
            //resolveLrc(mediaObject->currentSource().fileName());
            break;
        case Phonon::StoppedState :
            stopAction->setEnabled(false);
            playAction->setIcon(QIcon(":/images/play.png"));
            playAction->setText(tr("����(F5)"));
            topLabel->setText(tr("MiNi������"));
            timeLabel->setText(tr("00:00 / 00:00"));
            // ֹͣ�������
            //lrc->stopLrcMask();
            //lrc->setText(tr("MyPlayer���ֲ����� --- yafeilinux��Ʒ"));
            break;
        case Phonon::PausedState :
            stopAction->setEnabled(true);
            playAction->setIcon(QIcon(":/images/play.png"));
            playAction->setText(tr("����(F5)"));
            topLabel->setText(QFileInfo(mediaObject->
                                        currentSource().fileName()).baseName() + tr(" ����ͣ��"));
            // ����ø����и���ļ�
            /*if (!lrcMap.isEmpty()) {
                lrc->stopLrcMask();
                lrc->setText(topLabel->text());
            }*/
            break;
        case Phonon::BufferingState :
            break;
        default :
            ;
        }
}

void MyWidget::sourceChanged(const Phonon::MediaSource &source)
{
    int index = sources.indexOf(source);
    playlist->selectRow(index);
    changeActionState();
}

void MyWidget::aboutToFinish()
{
    int index = sources.indexOf(mediaObject->currentSource()) + 1;
    if(sources.size() > index)
    {
        mediaObject->enqueue(sources.at(index));
        mediaObject->seek(mediaObject->totalTime());
    }
    else//ע��˴�Ҫ�޸�
    {
        mediaObject->setCurrentSource(sources.at(0));
        mediaObject->play();
        //mediaObject->stop();
    }
}

void MyWidget::metaStateChanged(Phonon::State newState, Phonon::State oldState)
{
    if(newState == Phonon::ErrorState)
    {
        QMessageBox::warning(this, tr("���ļ��ǳ���"), metaInformationResolver->errorString());
        while(!sources.isEmpty() && !(sources.takeLast() == metaInformationResolver->currentSource()))
        {}
        return;
    }
    if(newState != Phonon::StoppedState && newState != Phonon::PausedState)
        return;
    if(metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
        return;
    QMap<QString, QString> metaData = metaInformationResolver->metaData();

    //��ȡ����,���Ϊ��,��ʹ���ļ���
    QString title = metaData.value("TITLE");
    if(title == "")
    {
        QString str = metaInformationResolver->currentSource().fileName();
        title = QFileInfo(str).baseName();
    }

    QTableWidgetItem *titleItem = new QTableWidgetItem(title);
    titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);

    QTableWidgetItem *artistItem = new QTableWidgetItem(metaData.value("ARTIST"));
    artistItem->setFlags(artistItem->flags() ^ Qt::ItemIsEditable);

    qint64 totalTime = metaInformationResolver->totalTime();
    QTime time(0, (totalTime / 60000) % 60, (totalTime / 1000) % 60);
    QTableWidgetItem *timeItem = new QTableWidgetItem(time.toString("mm:ss"));

    int currentRow = playlist->rowCount();
    playlist->insertRow(currentRow);
    playlist->setItem(currentRow, 0, titleItem);
    playlist->setItem(currentRow, 1, artistItem);
    playlist->setItem(currentRow, 2, timeItem);

    int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
    if(sources.size() > index)
    {
        metaInformationResolver->setCurrentSource(sources.at(index));
    }
    else
    {
        if(playlist->selectedItems().isEmpty())
        {
            if(mediaObject->state() != Phonon::PlayingState &&
               mediaObject->state() != Phonon::PausedState)
                mediaObject->setCurrentSource(sources.at(0));//Ĭ�ϲ����б��еĵ�һ�׸���
            else
            {
                playlist->selectRow(0);
                changeActionState();
            }
       }
        else
        {
            changeActionState();
        }
    }


}

void MyWidget::tableClicked(int row)
{
    bool wasPlaying = mediaObject->state() == Phonon::PlayingState;
    mediaObject->stop();
    mediaObject->clearQueue();

    if(row >= sources.size())
        return;

    mediaObject->setCurrentSource(sources.at(row));

    if(wasPlaying)
        mediaObject->play();
    else
        mediaObject->play();
}

void MyWidget::clearSources()
{
    sources.clear();
    changeActionState();
}

void MyWidget::changeActionState()
{
    //���ý��Դ�б�Ϊ��
    if(sources.count() == 0)
    {
        if(mediaObject->state() != Phonon::PlayingState &&
           mediaObject->state() != Phonon::PausedState)
        {
            playAction->setEnabled(false);
            stopAction->setEnabled(false);
        }
        skipBackwardAction->setEnabled(false);
        skipForwardAction->setEnabled(false);
    }
    else
    {
        playAction->setEnabled(true);
        stopAction->setEnabled(true);
        if(sources.count() == 1)
        {
            skipBackwardAction->setEnabled(false);
            skipForwardAction->setEnabled(false);
        }
        else
        {
            skipBackwardAction->setEnabled(true);
            skipForwardAction->setEnabled(true);
            int index = playlist->currentRow();
            if(index == 0)
                skipBackwardAction->setEnabled(false);
            if(index + 1 == sources.count())
                skipForwardAction->setEnabled(false);
        }
    }
}














