#ifndef PlaybackWidget_H
#define PlaybackWidget_H

#include <QWidget>
#include <QFileInfo>
#include <QImage>
#include <QTimer>



class PlaybackWidget : public QWidget
{
	Q_OBJECT
public:
	PlaybackWidget(QWidget *parent=0);
	~PlaybackWidget();
	
	// QWidget::
	virtual QSize sizeHint () const;
	
	bool playbackEnabled() { return m_playbackEnabled; }
	
	QString dailyRecordingPath() { return m_dailyRecordingPath; }
	
	double playbackFps() { return m_playbackFps; }
	
	QString currentPlaybackDate() { return m_currentPlaybackDate; }
	int currentFrame() { return m_currentFrame; }
	
	int numFrames() { return m_files.size(); }
	
	typedef enum Status { Playing, Stopped, Paused };
	Status status() { return m_status; }
	void setStatus(Status);
	
public slots:
	void setDesiredSize(QSize);
	
	void setDailyRecordingPath(const QString&);
	
	void setPlaybackFps(double);
	
	void setCurrentFrame(int);
	
	// date should be in YYYY-MM-DD
	void loadPlaybackDate(const QString & date);
	
signals:
	void currentFrameChanged(int);
	void numFramesChanged(int);
	void statusChanged(PlaybackWidget::Status);
	
protected:
	void paintEvent(QPaintEvent *event);

private slots:
	void updateImage();

private:
	QImage m_currentImage;
	
	QTimer m_updateTimer;
		
	bool m_playbackEnabled;
	QSize m_desiredSize;
	
	QString m_dailyRecordingPath;
	bool m_inPlaybackMode;
	QFileInfoList m_files;
	
	int m_currentFrame;
	double m_playbackFps;
	QString m_currentPlaybackDate;
	
	Status m_status;
	
	// used to prevent loops in setCurrentFrame signals
	bool m_lockCurrentFrameChange;
	

};

#endif
