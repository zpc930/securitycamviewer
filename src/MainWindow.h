#ifndef MainWindow_H
#define MainWindow_H

class MjpegClient;

#include <QImage>
#include <QLabel>
#include <QTimer>

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent=0);
	~MainWindow();
	
private slots:
	void newImage(QImage);
	void updateFrames();
	
private:
	void applySize(int x, int y, QSize imgSize);
	
	QList<MjpegClient*> m_threads;
	QList<QImage> m_images;
	QList<QLabel*> m_labels;
	QTimer m_updateTimer;
	QList<bool> m_wasChanged;

};

#endif // MotionViewer_H
