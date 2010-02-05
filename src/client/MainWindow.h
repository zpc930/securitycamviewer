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
	MainWindow(QString configFile="viewer.ini", bool verbose=false, QWidget *parent=0);
	~MainWindow();
	
private slots:
/*	void newImage(QImage);
	void updateFrames();*/
	
private:
	void applySize(int x, int y);
	int m_rows;
	int m_cols;
	QSize m_frameSize;
	
};

#endif // MotionViewer_H
