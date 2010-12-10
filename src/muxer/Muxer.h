#ifndef Muxer_H
#define Muxer_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QImage>
#include <QSize>
#include <QFile>

#define LISTEN_PORT 8088

class JpegServer;
class MjpegClient;
class EyeCounter;

class Muxer : public QObject
{
	Q_OBJECT
public:
	Muxer(QString configFile="muxer.ini", bool verbose = false, QObject *parent =0);
	~Muxer();

signals:
	void imageReady(QImage*);

private slots:
	void newImage(QImage);
	void updateFrames();
	

private:
	void applySize(int x, int y);
	
	JpegServer * m_jpegServer;
	
	QList<MjpegClient*> m_threads;
	QList<QImage> m_images;
	
	QList<int> m_counts;
	QList<int> m_durations;
	QList<QTime> m_time;
	
	QTimer m_updateTimer;
	
	QImage m_muxedImage;
	
	int m_cols;
	int m_rows;
	QSize m_frameSize;
	
	QList<bool> m_wasChanged;
	
	bool m_verbose;
	
	#ifdef OPENCV_ENABLED
	EyeCounter *m_counter;
	QString m_logFile;
	QFile *m_logFilePtr;
	bool m_highlightEyes;
	#endif
};

#endif // Muxer_H
