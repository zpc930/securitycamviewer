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

	
	
public slots:
	void setDesiredSize(QSize);
	
	MjpegClient * connectTo(QString host, int port=80, QString path="/", const QString& user="", const QString& pass="");
	
 	void setPlaybackEnabled(bool flag) {  m_playbackEnabled = flag; }
 	void setDailyRecordingPath(const QString& path) { m_dailyRecordingPath = path; }
 	void setPlaybackFps(double d) { m_playbackFps = d; }
	void setLiveFps(double);
	
protected:
	void paintEvent(QPaintEvent *event);


private slots:
	void newImage(QImage);
	void showCustomContextMenu(const QPoint&pos);
	void showPlaybackDialog();

private:
	QImage m_currentImage;
	MjpegClient * m_client;
	
	QTimer m_updateTimer;
 	bool m_playbackEnabled;
	QSize m_desiredSize;
	
 	QString m_dailyRecordingPath;
 	double m_playbackFps;
	
	double m_liveFps;
};

#endif //CameraViewerWidget_H