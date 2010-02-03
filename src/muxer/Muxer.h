#ifndef Muxer_H
#define Muxer_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QSize>

#define LISTEN_PORT 8088

class JpegServer;
class MjpegClient;

class Muxer : public QObject
{
	Q_OBJECT
public:
	Muxer(bool verbose = false, QObject *parent =0);
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
	QTimer m_updateTimer;
	
	QImage m_muxedImage;
	
	int m_cols;
	int m_rows;
	QSize m_frameSize;
	
	QList<bool> m_wasChanged;
	
	bool m_verbose;
};

#endif // Muxer_H
