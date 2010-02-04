#include "CameraViewerWidget.h"
#include "MjpegClient.h"

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
	, m_inPlaybackMode(false)
	, m_currentFrame(0)
	, m_paused(false)
	, m_playbackFps(3)
	, m_currentPlaybackDate("")
	, m_liveFps(2)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCustomContextMenu(QPoint)));
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateImage()));
	m_updateTimer.start();
	setLiveMode(); // apply fps to update timer
	
	QDate date = QDate::currentDate();
	m_currentPlaybackDate = date.toString("yyyy-MM-dd");
}

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
	
void CameraViewerWidget::setPlaybackEnabled(bool flag)
{
	m_playbackEnabled = flag;
}

void CameraViewerWidget::setDailyRecordingPath(const QString& path)
{
	m_dailyRecordingPath = path;
}

void CameraViewerWidget::setPlaybackFps(double d)
{
	m_playbackFps = d;
	if(isPlaybackMode())
		setPlaybackMode(); // re-apply fps interval to timer
}


void CameraViewerWidget::setLiveFps(double d)
{
	m_liveFps = d;
	if(isLiveMode())
		setLiveMode(); // re-apply fps interval to timer
}

void CameraViewerWidget::setCurrentFrame(int d)
{
	m_currentFrame = d;
	update();
}

// date should be in YYYY-MM-DD
void CameraViewerWidget::loadPlaybackDate(const QString & date)
{
	m_currentPlaybackDate = date;
	
	QStringList parts = date.split("-");
	QString path = m_dailyRecordingPath;
	path.replace("%Y",parts[0]);
	path.replace("%m",parts[1]);
	path.replace("%d",parts[2]);

	path = "S:\\Security Camera Recordings\\cam1\\2010\\01\\31";
	qDebug() << "loadPlaybackDate("<<date<<"): Reading from path"<<path<<" (hardocded!)";
	//QDir dir(path);
	
	//m_files = dir.entryInfoList(QStringList() << "*.jpg" << "*.JPG", QDir::NoFilter, QDir::Time);

	m_files.clear();

	QDirIterator it(path, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		qDebug() << it.next();
		m_files << it.fileInfo();
	}



	qDebug() << "Found "<<m_files.size()<<" files.";
	m_currentFrame = 0;
	
	if(isPlaybackMode())
		update();
}	
	
void CameraViewerWidget::setLiveMode()
{
	m_inPlaybackMode = false;
	m_updateTimer.stop();
	m_updateTimer.setInterval((int)(1000/m_liveFps));
	m_updateTimer.start();
}

void CameraViewerWidget::setPlaybackMode()
{
	m_inPlaybackMode = true;
	m_updateTimer.stop();
	m_updateTimer.setInterval((int)(1000/m_playbackFps));
	m_updateTimer.start();
	
	if(m_files.isEmpty())
	{
		loadPlaybackDate(currentPlaybackDate());
	}
	
	
}
	
	
void CameraViewerWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	painter.fillRect(rect(),Qt::black);
	
	if(!m_currentImage.isNull())
	{
		painter.drawImage(rect(),m_currentImage);
		
	}

	if(isLiveMode())
	{
		painter.drawText(0,15,"LIVE");
	}
	else
	{
		painter.drawText(0,15,QString("PLAYBACK - %1/%2").arg(m_currentFrame).arg(m_files.size()));
	}

}

void CameraViewerWidget::newImage(QImage image)
{
	if(isLiveMode())
		m_currentImage = image;
}

void CameraViewerWidget::updateImage()
{
	if(isPlaybackMode())
	{
		if(m_currentFrame < m_files.size())
		{
			QFileInfo info = m_files[m_currentFrame];
			m_currentImage.load(info.canonicalFilePath());
			m_currentFrame++;
		}
	}
	
	update();
}

void CameraViewerWidget::showCustomContextMenu(const QPoint&pos)
{
// 	QModelIndex idx = m_groupView->currentIndex();
// 	SlideGroup *s = m_docModel.groupFromIndex(idx);
// 	qDebug() << "MainWindow::slotListContextMenu(): selected group#:"<<s->groupNumber()<<", title:"<<s->groupTitle()<<", pos:"<<pos<<", mapToGlobal:"<<mapToGlobal(pos);

	QMenu contextMenu(tr("Context menu"), this);
	//contextMenu.addAction(new QAction(tr("View Recorded Content"), this));
	//contextMenu.addAction(new QAction(tr("View Recorded Content"), this));
	QAction *today = new QAction(tr("View Today's Recordings"),this);
	connect(today, SIGNAL(triggered()), this, SLOT(setPlaybackMode()));
	
	QAction *live = new QAction(tr("View Live Feed"),this);
	connect(live, SIGNAL(triggered()), this, SLOT(setLiveMode()));
	
	contextMenu.addAction(today);
	contextMenu.addAction(live);
	
	contextMenu.exec(mapToGlobal(pos));
}

