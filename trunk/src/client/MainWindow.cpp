#include "MainWindow.h"

#include "MjpegClient.h"
#include "FlowLayout.h"

#include <math.h>

#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	FlowLayout * layout = new FlowLayout(this,0,0,0);
	
	bool verbose = false;
	
	QSettings settings("viewer.ini",QSettings::IniFormat);
	
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
	QSize sameSize(part[0].toInt(),part[1].toInt());
	
	if(verbose)
		qDebug() << "Viewer: Frame size: "<<sameSize.width()<<"x"<<sameSize.height();
	
	
	//QSize sameSize(1280,960); //(320,240);
	
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
		
		
		QLabel * label = new QLabel(this);
		layout->addWidget(label);
		m_labels << label;
		
		MjpegClient * client = new MjpegClient();
		client->connectTo(host,port,path);
			
		client->setAutoResize(sameSize);
		client->start();
		
		m_threads    << client;
		m_images     << QImage();
		m_wasChanged << false;
		
		connect(client, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)));
		
		if(verbose)
			qDebug() << "Viewer: Setup camera "<<i<<" using host"<<host<<", port"<<port<<", path"<<path;
	}
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateFrames()));
	
	int fps = settings.value("fps",2).toInt();
	if(verbose)
		qDebug() << "Viewer: Running at"<<fps<<" frames per second";
		
	m_updateTimer.setInterval(1000/fps);
	m_updateTimer.start();
	
	// Attempt to find an optimum window size to view the cameras in a nice symetric arragmenet
	double sq = sqrt(numCameras);
	if(((int)sq) != sq)
	{
		// first, attempt to round up
		int x = (int)ceil(sq);
		int y = (int)floor(sq);
		
		if(x*y >= numCameras)
			// good to go, apply size
			applySize(x,y,sameSize);
		else
		{
			// add one row then try
			y++;
			if(x*y >= numCameras)
				applySize(x,y,sameSize);
			else
				// just use the sqrt ceil
				applySize(x,x,sameSize);
		}
	}
	else
	{
		applySize((int)sq,(int)sq,sameSize);
	}
	
}

MainWindow::~MainWindow()
{
	while(!m_threads.isEmpty())
	{
		MjpegClient * client = m_threads.takeFirst();
		client->quit();
		client->wait();
		delete client;
		client = 0;
	}
}

void MainWindow::applySize(int x, int y, QSize size)
{
	int xpx = x * size.width();
	int ypx = y * size.height();
	resize(xpx + 1,ypx + 1);
}


void MainWindow::newImage(QImage image)
{
	MjpegClient * client = dynamic_cast<MjpegClient*>(sender());
	if(client)
	{
		int index = m_threads.indexOf(client);
		m_images[index] = image;
		m_wasChanged[index] = true;
		//qDebug() << "newImage(): Received image for thread index"<<index;
	}
}

void MainWindow::updateFrames()
{
	int length = m_images.size();
	for(int i=0;i<length;i++)
	{
		if(i < m_labels.size())
		{
			bool flag = m_wasChanged.at(i);
			if(flag)
			{
				QLabel * label = m_labels.at(i);
				QImage   image = m_images.at(i);
				if(label)
					label->setPixmap(QPixmap::fromImage(image));
				
				m_wasChanged[i] = false;
			}
		}
	}
}
