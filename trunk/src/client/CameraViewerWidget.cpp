#include "CameraViewerWidget.h"
#include "MjpegClient.h"
#include "PlaybackWidget.h"

#include <QDate>
#include <QDirIterator>
#include <QPainter>
#include <QMenu>

CameraViewerWidget::CameraViewerWidget(QWidget *parent)
	: QWidget(parent)
	, m_client(0)
	, m_playbackEnabled(false)
	, m_desiredSize(320,240)
	, m_dailyRecordingPath("")
	, m_playbackFps(2)
	, m_liveFps(2)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCustomContextMenu(QPoint)));
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
	
	m_updateTimer.setInterval((int)(1000/m_liveFps));
	m_updateTimer.start();

};

CameraViewerWidget::~CameraViewerWidget()
{
	if(m_client)
	{
		m_client->quit();
		m_client->wait();
		delete m_client;
		m_client = 0;
	}
}
	
void CameraViewerWidget::setLiveFps(double fps)
{
	m_liveFps = fps;
	
	m_updateTimer.stop();
	m_updateTimer.setInterval((int)(1000/m_liveFps));
	m_updateTimer.start();
}

void CameraViewerWidget::setDesiredSize(QSize size)
{
	if(m_client)
		m_client->setAutoResize(size);
	m_desiredSize = size;
}

QSize CameraViewerWidget::sizeHint() const
{
	return m_desiredSize;
}
	
MjpegClient * CameraViewerWidget::connectTo(QString host, int port, QString path)
{
	if(m_client)
	{
		m_client->quit();
		m_client->wait();
		delete m_client;
	}
	
	m_client = new MjpegClient();
	m_client->connectTo(host,port,path);
	m_client->setAutoResize(rect().size());
	// TODO catch resize event and update autoresize accordingly
	m_client->start();
	
	connect(m_client, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)));
	
	return m_client;	
}

	
void CameraViewerWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);

	if(!m_currentImage.isNull())
	{
	
		painter.drawImage(rect(),m_currentImage);
	}
	else
	{
 		painter.fillRect(rect(),Qt::black);
 		painter.setPen(Qt::white);
 		painter.drawText(5,15,QString("Waiting for video from %1:%2...").arg(m_client->host()).arg(m_client->port()));
	}
}

void CameraViewerWidget::newImage(QImage image)
{
	m_currentImage = image;
}

void CameraViewerWidget::showCustomContextMenu(const QPoint&pos)
{

	QMenu contextMenu(tr("Context menu"), this);
	
	QAction *today = new QAction(tr("View Recorded Footage"),this);
	connect(today, SIGNAL(triggered()), this, SLOT(showPlaybackDialog()));
	
// 	QAction *live = new QAction(tr("View Live Feed"),this);
// 	connect(live, SIGNAL(triggered()), this, SLOT(setLiveMode()));
	
	contextMenu.addAction(today);
// 	contextMenu.addAction(live);
	
	contextMenu.exec(mapToGlobal(pos));
}

void CameraViewerWidget::showPlaybackDialog()
{
	//qDebug() << "TO BE DONE";
	PlaybackWidget * w = new PlaybackWidget();
	w->adjustSize();
	w->setWindowTitle("Playback Test - Cam 2");
	w->loadPlaybackDate(w->currentPlaybackDate());
	w->show();
}
