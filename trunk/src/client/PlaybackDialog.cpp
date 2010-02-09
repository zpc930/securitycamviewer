#include "PlaybackDialog.h"
#include "ui_PlaybackDialog.h"

#include <QDebug>
#include <QStyle>
#include <QDirIterator>
#include <QTextCharFormat>
#include <QProgressDialog>

PlaybackDialog::PlaybackDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::PlaybackDialog)
	, m_calendarChangeCount(0)
{
	ui->setupUi(this);
	
	ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	ui->stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	
	ui->playButton->setEnabled(true);
	ui->stopButton->setEnabled(false);
	
	connect(ui->viewer, SIGNAL(currentFrameChanged(int)), ui->frameSlider, SLOT(setValue(int)));
	connect(ui->viewer, SIGNAL(numFramesChanged(int)), this, SLOT(numFramesChanged(int)));
	connect(ui->viewer, SIGNAL(statusChanged(PlaybackWidget::Status)), this, SLOT(statusChanged(PlaybackWidget::Status)));
	
	connect(ui->frameSlider, SIGNAL(valueChanged(int)), ui->viewer, SLOT(setCurrentFrame(int)));
	
	connect(ui->playButton, SIGNAL(clicked()), this, SLOT(playButtonClicked()));
	connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
	
	connect(ui->fps, SIGNAL(valueChanged(int)), this, SLOT(setFps(int)));
	
	connect(ui->calendarWidget, SIGNAL(currentPageChanged(int,int)), this, SLOT(updateCalendarWidget(int,int)));
	connect(ui->calendarWidget, SIGNAL(activated(const QDate&)), this, SLOT(loadDate(const QDate&)));
	connect(ui->calendarWidget, SIGNAL(clicked(const QDate&)), this, SLOT(loadDate(const QDate&)));
	
	setWindowTitle("Recorded Footage");
}


PlaybackDialog::~PlaybackDialog()
{
	delete ui;
}

void PlaybackDialog::showEvent(QShowEvent *)
{
	ui->viewer->loadPlaybackDate(ui->viewer->currentPlaybackDate());
}

void PlaybackDialog::loadDate(const QDate& date)
{
	ui->viewer->loadPlaybackDate(date.toString("yyyy-MM-dd"));
}


void PlaybackDialog::numFramesChanged(int num)
{
	ui->frameSlider->setMaximum(num);
	ui->frameBox->setMaximum(num);
}

void PlaybackDialog::setDailyRecordingPath(const QString& dailyPath)
{
	ui->viewer->setDailyRecordingPath(dailyPath);
	m_dailyRecordingPath = dailyPath;
	
	
	updateCalendarWidget();
	
}

void PlaybackDialog::updateCalendarWidget(int year, int month)
{
	QDate date = QDate::currentDate();
	if(year < 0)
		year = date.year();
	if(month < 0)
		month = date.month();
		
	date.setDate(year,month,1);
	
	QDate endOfMonth = date;
	endOfMonth.setDate(year,month,date.daysInMonth());
	
	QTextCharFormat fmtHasFiles;
	fmtHasFiles.setFontWeight(QFont::Bold);
	
	QTextCharFormat fmtNoFiles;
	fmtNoFiles.setFontWeight(QFont::Normal);
	//qDebug() << "updateCalendarWidget("<<year<<","<<month<<"): date:"<<date<<", endOfMonth:"<<endOfMonth;
	
	
	QProgressDialog progress(QString(tr("Loading Calendar")),tr("Stop Loading"),1,date.daysInMonth(),this);
	progress.setWindowTitle(QString(tr("Loading Calendar...")));
	progress.show();
	// let the dialog become visible
	QCoreApplication::processEvents();
	
	// Since the m_calendarChangeCount is changed every time we enter this loop,
	// that means that if the user changes the month on us before we finish loading
	// the calendar completly, we can break the loop by checking to see if
	// m_calendarChangeCount has changed
	m_calendarChangeCount ++;
	int startedAtCount = m_calendarChangeCount;
	
	while(date <= endOfMonth)
	{
		// break the loop if user has changed months on us
		if(m_calendarChangeCount != startedAtCount)
			break;
			
		QString path = m_dailyRecordingPath;
		
		QStringList parts = date.toString("yyyy-MM-dd").split("-");
		path.replace("%Y",parts[0]);
		path.replace("%m",parts[1]);
		path.replace("%d",parts[2]);
		
		QDirIterator it(path, QDirIterator::Subdirectories);
		if(it.hasNext())
			ui->calendarWidget->setDateTextFormat(date,fmtHasFiles);
		else
			ui->calendarWidget->setDateTextFormat(date,fmtNoFiles);
			
		date = date.addDays(1); 
		
		// update the progress dialog
		progress.setValue(date.day());
		QCoreApplication::processEvents();
		
		if(progress.wasCanceled())
			break;
	}
	
	progress.close();
}


void PlaybackDialog::setPlaybackFps(double d)
{
	ui->viewer->setPlaybackFps(d);
	ui->fps->setValue((int)d);
	m_playbackFps = d;
}


void PlaybackDialog::setFps(int d)
{
	ui->viewer->setPlaybackFps((double)d);
	m_playbackFps = (double)d;
}


void PlaybackDialog::statusChanged(PlaybackWidget::Status s)
{
// 	qDebug() << "PlaybackDialog::statusChanged(): "<<s;
	if(s == PlaybackWidget::Stopped)
	{
		ui->stopButton->setEnabled(false);
		ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	}
	else
	{
		ui->stopButton->setEnabled(true);
		if(s == PlaybackWidget::Playing)
		{
			ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		}
		else
		{
			ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		}
	}
}

void PlaybackDialog::stopButtonClicked()
{
	ui->viewer->setStatus(PlaybackWidget::Stopped);
}

void PlaybackDialog::playButtonClicked()
{
	if(ui->viewer->status() != PlaybackWidget::Playing)
	{
		ui->viewer->setStatus(PlaybackWidget::Playing);
	}
	else
	{
		ui->viewer->setStatus(PlaybackWidget::Paused);
	}
}

void PlaybackDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}
