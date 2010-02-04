#ifndef CameraViewerWidget_H
#define CameraViewerWidget_H

#include <QWidget>
#include <QFileInfo>
#include <QImage>
#include <QTimer>

class MjpegClient;

class CameraViewerWidget : public QWidget
{
	Q_OBJECT
public:
	CameraViewerWidget(QWidget *parent=0);
	~CameraViewerWidget();
	
	
	// QWidget::
	virtual QSize sizeHint () const;
	
	bool playbackEnabled() { return m_playbackEnabled; }
	
	QString dailyRecordingPath() { return m_dailyRecordingPath; }
	
	double playbackFps() { return m_playbackFps; }
	
	double liveFps() { return m_liveFps; }

	bool isLiveMode() { return ! m_inPlaybackMode; }
	bool isPlaybackMode() { return m_inPlaybackMode; }
	QString currentPlaybackDate() { return m_currentPlaybackDate; }
	int currentFrame() { return m_currentFrame; }
	
	
public slots:
	void setDesiredSize(QSize);
	
	MjpegClient * connectTo(QString host, int port=80, QString path="/");
	
	void setPlaybackEnabled(bool flag);
	
	void setDailyRecordingPath(const QString&);
	
	void setPlaybackFps(double);
	
	void setLiveFps(double);
	
	void setCurrentFrame(int);
	
	// date should be in YYYY-MM-DD
	void loadPlaybackDate(const QString & date);
	
	void setLiveMode();
	void setPlaybackMode();
	
protected:
	void paintEvent(QPaintEvent *event);


private slots:
	void newImage(QImage);
	void updateImage();
	void showCustomContextMenu(const QPoint&pos);

private:
	QImage m_currentImage;
	MjpegClient * m_client;
	
	QTimer m_updateTimer;
	bool m_playbackEnabled;
	QSize m_desiredSize;
	
	QString m_dailyRecordingPath;
	bool m_inPlaybackMode;
	QFileInfoList m_files;
	
	int m_currentFrame;
	bool m_paused;
	double m_playbackFps;
	QString m_currentPlaybackDate;
	
	double m_liveFps;
};

#endif //CameraViewerWidget_H
