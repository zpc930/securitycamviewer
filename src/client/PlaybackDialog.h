#ifndef PLAYBACKDIALOG_H
#define PLAYBACKDIALOG_H

#include <QDialog>
#include <QDate>
#include "PlaybackWidget.h"

class QShowEvent;

namespace Ui {
    class PlaybackDialog;
}

class PlaybackDialog : public QDialog
{
	Q_OBJECT
public:
	PlaybackDialog(QWidget *parent = 0);
	~PlaybackDialog();
	
	QString dailyRecordingPath() { return m_dailyRecordingPath; }
	double playbackFps() { return m_playbackFps; }

protected:
	void showEvent(QShowEvent *);

public slots:
	void setDailyRecordingPath(const QString& path);
	void setPlaybackFps(double d);
	void enableEyeDetection(bool highlightEyes, QString logFile);

private slots:
	void updateCalendarWidget(int year=-1, int month=-1);
	void loadDate(const QDate&);
	
	void numFramesChanged(int);
	void statusChanged(PlaybackWidget::Status);
	void playButtonClicked();
	
	void applyFpsValue(int);

protected:
	void changeEvent(QEvent *e);

private:
	Ui::PlaybackDialog *ui;
	
	QString m_dailyRecordingPath;
	double m_playbackFps;	
	int m_calendarChangeCount;
	bool m_lockStatusChange;
	bool m_statusFromButton;
};

#endif // PLAYBACKDIALOG_H
