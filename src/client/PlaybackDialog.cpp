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
	, m_lockStatusChange(false)
	, m_statusFromButton(false)
{
	ui->setupUi(this);
	
	ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	
	ui->playButton->setEnabled(true);
	
	connect(ui->viewer, SIGNAL(currentFrameChanged(int)), ui->frameSlider, SLOT(setValue(int)));
	connect(ui->viewer, SIGNAL(numFramesChanged(int)), this, SLOT(numFramesChanged(int)));
	connect(ui->viewer, SIGNAL(statusChanged(PlaybackWidget::Status)), this, SLOT(statusChanged(PlaybackWidget::Status)));
	
	connect(ui->calendarWidget, SIGNAL(currentPageChanged(int,int)), this, SLOT(updateCalendarWidget(int,int)));
	connect(ui->calendarWidget, SIGNAL(activated(const QDate&)), this, SLOT(loadDate(const QDate&)));
	connect(ui->calendarWidget, SIGNAL(clicked(const QDate&)), this, SLOT(loadDate(const QDate&)));
	
	connect(ui->frameSlider, SIGNAL(valueChanged(int)), ui->viewer, SLOT(setCurrentFrame(int)));
	connect(ui->playButton, SIGNAL(clicked()), this, SLOT(playButtonClicked()));
	connect(ui->fps, SIGNAL(valueChanged(int)), this, SLOT(applyFpsValue(int)));
	
	setWindowTitle("Recorded Footage");
}


PlaybackDialog::~PlaybackDialog()
{
	delete ui;
}

void PlaybackDialog::enableEyeDetection(bool highlightEyes, QString logFile)
{
	ui->viewer->enableEyeDetection(highlightEyes, logFile);
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
	// this will trigger applyFpsValue(), below
	ui->fps->setValue((int)d);
}


void PlaybackDialog::applyFpsValue(int d)
{
// 	qDebug() << "PlaybackDialog::applyFpsValue(): d:"<<d;
	 	
	if(d == 0)
		d = 1;
		
	if(d < 0)
	{
// 		qDebug() << "PlaybackDialog::applyFpsValue(): [BACKWARD] calling setPlaybackFps(d * -1) ="<<(d * -1);
		if(ui->viewer->playDirection() != PlaybackWidget::PlayBackward)
		{
			ui->fps->setStyleSheet("background:yellow;color:black");
			ui->viewer->setPlayDirection(PlaybackWidget::PlayBackward);
		}
		ui->viewer->setPlaybackFps(d * -1);
	}
	else
	{
// 		qDebug() << "PlaybackDialog::applyFpsValue():  [FORWARD] calling setPlaybackFps(d) ="<<d;
		if(ui->viewer->playDirection() != PlaybackWidget::PlayForward)
		{
			ui->fps->setStyleSheet("background:green;color:white");
			ui->viewer->setPlayDirection(PlaybackWidget::PlayForward);
		}
		ui->viewer->setPlaybackFps(d);
	}
	
	if(ui->viewer->status() == PlaybackWidget::Paused && ! m_lockStatusChange)
		ui->viewer->setStatus(PlaybackWidget::Playing);
	
	m_playbackFps = (double)d;
}


void PlaybackDialog::statusChanged(PlaybackWidget::Status s)
{
// 	qDebug() << "PlaybackDialog::statusChanged(): "<<s;
	
	// If the statusChanged() signal is from the user clicking a button,
	// set m_lockStatusChange so that the applyFpsValue() doesn't start
	// playing when the user moves the slider. See applyFpsValue() for more 
	// thoughts on this issue. 
	if(m_statusFromButton)
		m_lockStatusChange = true;
	else
		m_lockStatusChange = false;
	m_statusFromButton = false;
	
	if(s == PlaybackWidget::Paused)
	{
		ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	}
	else
	{
		ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	}
}

void PlaybackDialog::playButtonClicked()
{
	// Used in statusChanged()
	m_statusFromButton = true;
	if(ui->viewer->status() == PlaybackWidget::Playing)
	{
		ui->viewer->setStatus(PlaybackWidget::Paused);
	}
	else
	{
		ui->viewer->setStatus(PlaybackWidget::Playing);
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
