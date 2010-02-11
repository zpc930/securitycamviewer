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
	// Pause video if playing for performance 
	PlaybackWidget::Status oldStatus = ui->viewer->status();
	if(oldStatus == PlaybackWidget::Playing)
		ui->viewer->setStatus(PlaybackWidget::Paused);
	
	// Setup the starting date (first of the month)
	QDate date = QDate::currentDate();
	if(year < 0)
		year = date.year();
	if(month < 0)
		month = date.month();
		
	date.setDate(year,month,1);
	
	// Setup the last day of the month (the stopping point for the iterator)
	QDate endOfMonth = date;
	endOfMonth.setDate(year,month,date.daysInMonth());
	
	// Setup the format for use in the calendar to show which days have video
	QTextCharFormat fmtHasFiles;
	fmtHasFiles.setFontWeight(QFont::Bold);
	
	//qDebug() << "updateCalendarWidget("<<year<<","<<month<<"): date:"<<date<<", endOfMonth:"<<endOfMonth;
	
	// Setup a progress dialog because this loading loop can take a few seconds (5 - 30secs sometimes)
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
			
		if(ui->viewer->dateHasVideo(date.toString("yyyy-MM-dd")))
			ui->calendarWidget->setDateTextFormat(date,fmtHasFiles);
			
		date = date.addDays(1);
		
		// update the progress dialog
		progress.setValue(date.day());
		QCoreApplication::processEvents();
		
		if(progress.wasCanceled())
			break;
	}
	
	progress.close();
	
	ui->viewer->setStatus(oldStatus);
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
