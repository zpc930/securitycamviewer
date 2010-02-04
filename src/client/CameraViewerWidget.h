#ifndef CameraViewerWidget_H
#define CameraViewerWidget_H

#include <QWidget>
#include <QFileInfo>
#include <QImage>

class MjpegClient;

public CameraViewerWidget : public QWidget
{
	Q_OBJECT
public:
	CameraViewerWidget(QWidget *parent=0);
	~CameraViewerWidget();
	
	void setDesiredSize(QSize);
	
	MjpegClient * connectTo(QString host, int port=80, QString path="/");
	
	void setPlaybackEnabled(bool flag);
	bool playbackEnabled() { return m_playbackEnabled; }
	
	void setDailyRecordingPath(const QString&);
	QString dailyRecordingPath() { return m_dailyRecordingPath; }
	
	void setPlaybackFps(double);
	double playbackFps() { return m_playbackFps; }
	
	void setLiveFps(double);
	double liveFps() { return m_liveFps; }
	
	void setCurrentFrame(int);
	int currentFrame() { return m_currentFrame; }
	
	// date should be in YYYY-MM-DD
	void loadPlaybackDate(const QString & date);
	QString currentPlaybackDate() { return m_currentPlaybackDate; }
	
	void setLiveMode();
	void setPlaybackMode();
	bool isLiveMode() { return ! m_inPlaybackMode; }
	bool isPlaybackMode() { return m_inPlaybackMode; }
	
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