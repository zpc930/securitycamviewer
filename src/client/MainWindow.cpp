#include "MainWindow.h"

#include "MjpegClient.h"
#include "CameraViewerWidget.h"

#include <math.h>

#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>

MainWindow::MainWindow(QString configFile, bool verbose, QWidget *parent)
	: QWidget(parent)
{
	QGridLayout *layout = new QGridLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	
	if(verbose)
		qDebug() << "Viewer: Reading settings from "<<configFile;
	
	
	QSettings settings(configFile,QSettings::IniFormat);
	
	QString title = settings.value("window-title","Camera Viewer").toString();
	setWindowTitle(title);
	
	if(!settings.value("window-pos").isNull())
	{
		QString pos = settings.value("window-pos","10x10").toString();
		QStringList part = pos.split("x");
		move(part[0].toInt(),part[1].toInt());
	}
	
	// Load the frame size (the "small" frame - the final frame size is computed automatically)
	QString size = settings.value("frame-size","640x480").toString();
	QStringList part = size.split("x");
	m_frameSize = QSize(part[0].toInt(),part[1].toInt());
	
	if(verbose)
		qDebug() << "Viewer: Frame size: "<<m_frameSize.width()<<"x"<<m_frameSize.height();
	
	// Setup all the threads and create the labels to view the images
	int numCameras = settings.value("num-cams",0).toInt();
	
	// Load the defaults for use in the cameras section below
	QString mainHost = settings.value("host","localhost").toString();
	int     mainPort = settings.value("port",80).toInt();
	QString mainPath = settings.value("path","/").toString();
	
	if(numCameras == 0 &&
		(! settings.value("host").isNull() ||
		 ! settings.value("port").isNull() ||
		 ! settings.value("path").isNull()))
	{
		numCameras = 1;
		// since the user defined a "main" host/port or path, then
		// add a single camera using those values, since the 
		// load loop below defaults to the main* values read above
	}
	
	if(verbose)
		qDebug() << "Viewer: Using default host"<<mainHost<<", port"<<mainPort<<", path"<<mainPath;
	
	if(verbose)
		qDebug() << "Viewer: Going to read"<<numCameras<<"cameras";
	
	if(!numCameras)
	{
		QMessageBox::critical(this,"No Cameras","Sorry, no cameras were found in viewer.ini");
		QTimer::singleShot(0,this,SLOT(close()));
		return;
	}
	
	int fps = settings.value("fps",2).toInt();
	if(verbose)
		qDebug() << "Viewer: Running at"<<fps<<" frames per second";
		
	
	// Attempt to find an optimum window size to view the cameras in a nice symetric arragmenet
	double sq = sqrt(numCameras);
	
	if(((int)sq) != sq)
	{
		// first, attempt to round up
		int x = (int)ceil(sq);
		int y = (int)floor(sq);
		
		if(x*y >= numCameras)
			// good to go, apply size
			applySize(x,y);
		else
		{
			// add one row then try
			y++;
			if(x*y >= numCameras)
				applySize(x,y);
			else
				// just use the sqrt ceil
				applySize(x,x);
		}
	}
	else
		applySize((int)sq,(int)sq);
	
	int row=0;
	int col=0;
	
	for(int i=0; i<numCameras;i++)
	{
		// Setup all the threads 
		QString group = QString("cam%1").arg(i);
		
		QString hostKey = QString("%1/host").arg(group);
		QString portKey = QString("%1/port").arg(group);
		QString pathKey = QString("%1/path").arg(group);
		
		QString host = settings.value(hostKey,mainHost).toString();
		int     port = settings.value(portKey,mainPort).toInt();
		QString path = settings.value(pathKey,mainPath).toString();
		
		
		CameraViewerWidget * viewer = new CameraViewerWidget(this);
		
		viewer->connectTo(host,port,path);
		viewer->setDesiredSize(m_frameSize);
		viewer->setLiveFps(fps);
		
		layout->addWidget(viewer,row,col);
		
		col++;
		if(col>=m_cols)
		{
			col =0;
			row ++;
		}
		
		if(verbose)
			qDebug() << "Viewer: Setup camera "<<i<<" using host"<<host<<", port"<<port<<", path"<<path;
	}
	
	
	
}

MainWindow::~MainWindow()
{

}

void MainWindow::applySize(int x, int y)
{
	int xpx = x * m_frameSize.width();
	int ypx = y * m_frameSize.height();
	m_rows = x;
	m_cols = y;
	resize(xpx,ypx);
}

