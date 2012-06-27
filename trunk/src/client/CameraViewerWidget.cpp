#include "CameraViewerWidget.h"
#include "MjpegClient.h"
#include "PlaybackDialog.h"

#include <QDate>
#include <QDirIterator>
#include <QPainter>
#include <QMenu>
#include <QDebug>

#ifdef OPENCV_ENABLED
#include "EyeCounter.h"
#endif

CameraViewerWidget::CameraViewerWidget(QWidget *parent)
	: QWidget(parent)
	, m_client(0)
	, m_playbackEnabled(false)
	, m_desiredSize(320,240)
	, m_dailyRecordingPath("")
	, m_playbackFps(2)
	, m_liveFps(2)
	#ifdef OPENCV_ENABLED
	, m_counter(0)
	, m_logFilePtr(0)
	#endif
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

void CameraViewerWidget::enableEyeDetection(bool highlightEyes, QString logFile)
{
	#ifdef OPENCV_ENABLED
	m_highlightEyes = highlightEyes;
	m_logFile = logFile;

	m_counter = new EyeCounter();
	
	if(!m_logFile.isEmpty())
	{
		m_logFilePtr = new QFile(m_logFile);
		
		if (!m_logFilePtr->open(QIODevice::WriteOnly | QIODevice::Text))
		{
			qDebug() << "CameraViewerWidget: Unable to open"<<m_logFile<<" for writing.";
			delete m_logFilePtr ;
			m_logFilePtr  = 0;
		}
	}
	#endif
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
	
MjpegClient * CameraViewerWidget::connectTo(QString host, int port, QString path, const QString& user, const QString& pass)
{
	if(m_client)
	{
		m_client->quit();
		m_client->wait();
		delete m_client;
	}
	
	m_client = new MjpegClient();
	m_client->connectTo(host,port,path,user,pass);
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
		
		painter.setPen(Qt::black);
 		painter.drawText(6,16,QString("%1:%2").arg(m_client->host()).arg(m_client->port()));
 		
 		painter.setPen(Qt::black);
 		painter.drawText(4,16,QString("%1:%2").arg(m_client->host()).arg(m_client->port()));
 		
 		painter.setPen(Qt::black);
 		painter.drawText(6,14,QString("%1:%2").arg(m_client->host()).arg(m_client->port()));
 		
 		painter.setPen(Qt::black);
 		painter.drawText(4,14,QString("%1:%2").arg(m_client->host()).arg(m_client->port()));
 		
 		painter.setPen(Qt::white);
 		painter.drawText(5,15,QString("%1:%2").arg(m_client->host()).arg(m_client->port()));
 		
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
	#ifdef OPENCV_ENABLED
	if(m_counter)
	{
		QList<EyeCounterResult> faces = m_counter->detectEyes(image, true);
		
		QPainter painter(&image);
		
		int facesWithEyesCount;
		int eyesCount;
		foreach(EyeCounterResult res, faces)
		{
			if(!res.allEyes.isEmpty())
				facesWithEyesCount ++;
			eyesCount += res.allEyes.size();
				
			if(m_highlightEyes)
			{
				painter.setPen(Qt::red);
				painter.drawRect(res.face);
				
				painter.setPen(Qt::green);
				foreach(QRect eye, res.allEyes)
					painter.drawRect(eye);
			}
		}
		
		if(m_logFilePtr)
		{
			QTextStream out(m_logFilePtr);
			out << QDateTime::currentDateTime ().toString("yyyy-dd-MM hh:mm:ss") << "," << faces.size() << facesWithEyesCount << eyesCount;
		}
		
	}
	#endif
	
	m_currentImage = image;
}

void CameraViewerWidget::showCustomContextMenu(const QPoint&pos)
{
	if(playbackEnabled())
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
}

void CameraViewerWidget::showPlaybackDialog()
{
	if(playbackEnabled())
	{
// 		PlaybackWidget * w = new PlaybackWidget();
// 		w->adjustSize();
// 		w->setWindowTitle("Playback Test - Cam 2");
// 		w->loadPlaybackDate(w->currentPlaybackDate());
// 		w->show();
		PlaybackDialog * d = new PlaybackDialog();
		connect(d, SIGNAL(finished(int)), d, SLOT(deleteLater()));
		
		//qDebug() << "CameraViewerWidget::showPlaybackDialog(): dailyRecordingPath():"<<dailyRecordingPath();
		d->setDailyRecordingPath(dailyRecordingPath());
		d->setPlaybackFps(playbackFps());
		
		#ifdef OPENCV_ENABLED
		if(m_counter)
			d->enableEyeDetection(m_highlightEyes, m_logFile);
		#endif
		
		d->adjustSize();
		d->show();
	}
}
