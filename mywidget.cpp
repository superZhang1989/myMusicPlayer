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
    setWindowTitle(tr("MiNi音乐播放器"));
    setWindowIcon(QIcon(":/images/icon.png"));
    setMinimumSize(320, 160);
    setMaximumSize(320, 160);

    mediaObject = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(mediaObject, audioOutput);

    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(updataTime(qint64)));

    //创建顶部标签
    topLabel = new QLabel(tr("MiNi播放器"));
    topLabel->setTextFormat(Qt::RichText);
    topLabel->setOpenExternalLinks(true);
    topLabel->setAlignment(Qt::AlignCenter);

    //创建控制播放进度的滑块
    Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider(mediaObject, this);

    // 创建包含播放列表图标、显示时间标签和桌面歌词图标的工具栏
    QToolBar *widgetBar = new QToolBar(this);
    timeLabel = new QLabel(tr("00:00 / 00:00"), this);
    timeLabel->setToolTip(tr("当前时间 / 总时间"));
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 创建图标，用于控制是否显示播放列表
    QAction *PLAction = new QAction(tr("播放列表"), this);
    PLAction->setShortcut(QKeySequence("F4"));
    PLAction->setToolTip(tr("播放列表(F4)"));
    connect(PLAction, SIGNAL(triggered()), this, SLOT(setPlaylistShown()));

    // 创建图标，用于控制是否显示桌面歌词
    QAction *LRCAction = new QAction(tr("歌词"), this);
    LRCAction->setShortcut(QKeySequence("F2"));
    LRCAction->setToolTip(tr("桌面歌词(F2)"));
    connect(LRCAction, SIGNAL(triggered()), this, SLOT(setLrcShown()));

    // 添加到工具栏
    widgetBar->addAction(PLAction);
    widgetBar->addSeparator();
    widgetBar->addWidget(timeLabel);
    widgetBar->addSeparator();
    widgetBar->addAction(LRCAction);

    //创建播放控制动作工具栏
    QToolBar *toolBar = new QToolBar(this);
    //播放
    playAction = new QAction(this);
    playAction->setIcon(QIcon(":/images/play.png"));
    playAction->setText(tr("播放(F5)"));
    playAction->setShortcut(QKeySequence(tr("F5")));
    connect(playAction, SIGNAL(triggered()), this, SLOT(setPaused()));

    //停止
    stopAction = new QAction(this);
    stopAction->setIcon(QIcon(":/images/stop.png"));
    stopAction->setText(tr("停止(F6)"));
    stopAction->setShortcut(QKeySequence(tr("F6")));
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));

    //上一首
    skipBackwardAction = new QAction(this);
    skipBackwardAction->setIcon(QIcon(":/images/skipBackward.png"));
    skipBackwardAction->setText(tr("上一首(Ctrl+Left)"));
    skipBackwardAction->setShortcut(QKeySequence(tr("Ctrl+Left")));
    connect(skipBackwardAction, SIGNAL(triggered()), this, SLOT(skipBackward()));

   //下一首
    skipForwardAction = new QAction(this);
    skipForwardAction->setIcon(QIcon(":/images/skipForward.png"));
    skipForwardAction->setText(tr("下一首(Ctrl+Right)"));
    skipForwardAction->setShortcut(QKeySequence(tr("Ctrl+Right")));
    connect(skipForwardAction, SIGNAL(triggered()), this, SLOT(skipForward()));

    //打开文件
    QAction *openAction = new QAction(this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setText(tr("播放文件(Ctrl+O)"));
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

    //音量控制
    Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider(audioOutput, this);
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // 添加到工具栏
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

    //创建播放列表
    playlist = new MyPlaylist(this);
    connect(playlist, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableClicked(int)));
    connect(playlist, SIGNAL(playlistClean()), this, SLOT(clearSources()));

    //创建用来解析媒体的信息的元信息解析器
    metaInformationResolver = new Phonon::MediaObject(this);
    Phonon::AudioOutput *metaInformationAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(metaInformationResolver, metaInformationAudioOutput);

    //解析歌曲信息
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(metaStateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));

    //初始化动作图标的状态
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
    QStringList list = QFileDialog::getOpenFileNames(this, tr("打开音乐文件"), QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
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
                QMessageBox::warning(this, tr("致命错误"),
                                     mediaObject->errorString());
            } else {
                QMessageBox::warning(this, tr("错误"), mediaObject->errorString());
            }
            break;
        case Phonon::PlayingState :
            stopAction->setEnabled(true);
            playAction->setIcon(QIcon(":/images/pause.png"));
            playAction->setText(tr("暂停(F5)"));
            topLabel->setText(QFileInfo(mediaObject->
                                        currentSource().fileName()).baseName());
            //解析当前媒体源对应的歌词文件
            //resolveLrc(mediaObject->currentSource().fileName());
            break;
        case Phonon::StoppedState :
            stopAction->setEnabled(false);
            playAction->setIcon(QIcon(":/images/play.png"));
            playAction->setText(tr("播放(F5)"));
            topLabel->setText(tr("MiNi播放器"));
            timeLabel->setText(tr("00:00 / 00:00"));
            // 停止歌词遮罩
            //lrc->stopLrcMask();
            //lrc->setText(tr("MyPlayer音乐播放器 --- yafeilinux作品"));
            break;
        case Phonon::PausedState :
            stopAction->setEnabled(true);
            playAction->setIcon(QIcon(":/images/play.png"));
            playAction->setText(tr("播放(F5)"));
            topLabel->setText(QFileInfo(mediaObject->
                                        currentSource().fileName()).baseName() + tr(" 已暂停！"));
            // 如果该歌曲有歌词文件
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
    else//注意此处要修改
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
        QMessageBox::warning(this, tr("打开文件是出错"), metaInformationResolver->errorString());
        while(!sources.isEmpty() && !(sources.takeLast() == metaInformationResolver->currentSource()))
        {}
        return;
    }
    if(newState != Phonon::StoppedState && newState != Phonon::PausedState)
        return;
    if(metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
        return;
    QMap<QString, QString> metaData = metaInformationResolver->metaData();

    //获取标题,如果为空,则使用文件名
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
                mediaObject->setCurrentSource(sources.at(0));//默认播放列表中的第一首歌曲
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
    //如果媒体源列表为空
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














