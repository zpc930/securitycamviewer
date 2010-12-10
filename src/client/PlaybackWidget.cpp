#include "PlaybackWidget.h"

#include "PlaybackWidget.h"

#include <QDate>
#include <QDirIterator>
#include <QPainter>
#include <QMenu>
#include <QDebug>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QMessageBox>

PlaybackWidget::PlaybackWidget(QWidget *parent)
	: QWidget(parent)
	, m_desiredSize(320,240)
	, m_dailyRecordingPath("")
	, m_currentFrame(0)
	, m_playbackFps(30)
	, m_currentPlaybackDate("")
	, m_status(Paused)
	, m_lockCurrentFrameChange(false)
	, m_playDirection(PlayForward)
	#ifdef OPENCV_ENABLED
	, m_counter(0)
	, m_logFilePtr(0)
	#endif
{
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateImage()));
	
	setPlaybackFps(m_playbackFps); // starts timer as well
	
 	QDate date = QDate::currentDate();
 	m_currentPlaybackDate = date.toString("yyyy-MM-dd");
};

PlaybackWidget::~PlaybackWidget()
{
	
}
	
void PlaybackWidget::setDesiredSize(QSize size)
{
	m_desiredSize = size;
}

QSize PlaybackWidget::sizeHint() const
{
	return m_desiredSize;
}

void PlaybackWidget::setDailyRecordingPath(const QString& path)
{
	m_dailyRecordingPath = path;
}

void PlaybackWidget::setPlaybackFps(double d)
{
	m_playbackFps = d;
 	//qDebug() << "PlaybackWidget::setPlaybackFps(): m_playbackFps:"<<m_playbackFps;
 	
	m_updateTimer.stop();
	m_updateTimer.setInterval((int)(1000/m_playbackFps));
	m_updateTimer.start();
	

}

void PlaybackWidget::setPlayDirection(PlayDirection d)
{
	m_playDirection = d;
}

void PlaybackWidget::enableEyeDetection(bool highlightEyes, QString logFile)
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
	
	
void PlaybackWidget::setCurrentFrame(int d)
{
	if(m_lockCurrentFrameChange)
		return;
	m_lockCurrentFrameChange = true;
	
	if(d >= m_files.size() || d<0)
	{
		qDebug() << "PlaybackWidget::setCurrentFrame():"<<d<<">"<<m_files.size()<<"|| d < 0, not changing frame.";
		m_currentImage = QImage();
		m_lockCurrentFrameChange = false;
		return;
	}
	
	m_currentFrame = d;
	
	QFileInfo info = m_files[m_currentFrame];
	m_currentImage.load(info.canonicalFilePath());
			
	if(!m_currentImage.isNull() && m_desiredSize != m_currentImage.size())
	{
		m_desiredSize = m_currentImage.size();
		resize(m_desiredSize);
	}	
	
	#ifdef OPENCV_ENABLED
	if(m_counter)
	{
		QList<EyeCounterResult> faces = m_counter->detectEyes(m_currentImage, true);
		
		QPainter painter(&m_currentImage);
		
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
	
	emit currentFrameChanged(m_currentFrame);
	
	update();
	
	m_lockCurrentFrameChange = false;
}

bool PlaybackWidget_sortQFileInfoByModified(const QFileInfo &a, const QFileInfo &b)
{
	return a.lastModified().toTime_t() < b.lastModified().toTime_t();
}

// date should be in YYYY-MM-DD
void PlaybackWidget::loadPlaybackDate(const QString & date)
{
	QProgressDialog progress(QString(tr("Loading Video from %1...")).arg(date),tr("Stop Loading"),0,0,this);
	progress.setWindowTitle(QString(tr("Loading %1")).arg(date));
	progress.show();
	// let the dialog become visible
	QCoreApplication::processEvents();
		
	m_currentPlaybackDate = date;
	
	QStringList parts = date.split("-");
	QString path = m_dailyRecordingPath;
// 	qDebug() << "loadPlaybackDate("<<date<<"): Original path from config: "<<path;
	
	path.replace("%Y",parts[0]);
	path.replace("%m",parts[1]);
	path.replace("%d",parts[2]);
 	qDebug() << "loadPlaybackDate("<<date<<"): Reading from path"<<path;

	m_files.clear();

	QDirIterator it(path, QDirIterator::Subdirectories);
	while (it.hasNext() && !progress.wasCanceled())
	{
		it.next();
		if(it.fileInfo().isFile())
		{
			m_files << it.fileInfo();
			
			// update the progress dialog
			progress.setValue(m_files.size());
			QCoreApplication::processEvents();
		}
	}
	
	progress.close();
	
	if(!m_files.isEmpty())
		qSort(m_files.begin(),m_files.end(), PlaybackWidget_sortQFileInfoByModified);
	
	qDebug() << "Found "<<m_files.size()<<" files.";
	
	emit numFramesChanged(m_files.size()-1);
	
	setCurrentFrame(0);
	setStatus(Playing);
	
	if(m_files.size() <= 0)
	{
		setStatus(Paused);
		QMessageBox::critical(this,tr("No Video Found"),QString(tr("Sorry, no video was found for %1.")).arg(date));
		return;
	}
	
}	
	
bool PlaybackWidget::dateHasVideo(const QString & date)
{
	QString path = m_dailyRecordingPath;
		
	QStringList parts = date.split("-");
	path.replace("%Y",parts[0]);
	path.replace("%m",parts[1]);
	path.replace("%d",parts[2]);
	
	QDirIterator it(path, QDirIterator::Subdirectories);
	return it.hasNext();
}
	
void PlaybackWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	painter.fillRect(rect(),Qt::black);
	
	if(!m_currentImage.isNull())
	{
		painter.drawImage(rect(),m_currentImage);
		
	}
// 	painter.setPen(QPen(Qt::white,2));
// 	painter.setBrush(Qt::black);
// 	painter.drawText(5,15,QString("%3 - %1/%2").arg(m_currentFrame).arg(m_files.size()).arg(status() == Playing ? "PLAYING" : "PAUSED"));
	
}
void PlaybackWidget::updateImage()
{
	if(status() == Playing)
	{
		if(m_playDirection == PlayForward)
		{
			if(m_currentFrame < m_files.size())
			{
				setCurrentFrame(m_currentFrame);
				m_currentFrame++;
			}
			else
			{
				setStatus(Paused);
				setCurrentFrame(m_files.size()-1);
			}
		}
		else
		if(m_playDirection == PlayBackward)
		{
			if(m_currentFrame > 0)
			{
				setCurrentFrame(m_currentFrame);
				m_currentFrame--;
			}
			else
			{
				setStatus(Paused);
				setCurrentFrame(0);
			}
		}
		
	}
}

void PlaybackWidget::setStatus(Status s)
{
// 	qDebug() << "PlaybackWidget::setStatus(): "<<s;
	m_status = s;
	emit statusChanged(s);
	update();
}
