#include "MainWindow.h"

#include "MjpegClient.h"
#include "FlowLayout.h"

#include <math.h>

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	FlowLayout * layout = new FlowLayout(this,0,0,0);
	
	setWindowTitle("Camera Viewer");
	
	// This setup code is specific to my setup - need to generalize eventually
	QList<int> ports;
	//ports << 8082 << 8089 << 8095 << 8093 << 8081 << 8094 << 8083 << 8084 << 8091 << 8090 << 8092 << 8087 << 8096 << 8099 << 8085 << 8086;
	ports << 8088;
	
	QSize sameSize(1280,960); //(320,240);
	
	// Setup all the threads and create the labels to view the images
	foreach(int port, ports)
	{
		QLabel * label = new QLabel(this);
		layout->addWidget(label);
		m_labels << label;
		
		MjpegClient * client = new MjpegClient();
// 		if(port == 8096)
// 		{
// 			// just for fun
// 			client->connectTo("10.10.9.41",80,"/axis-cgi/mjpg/video.cgi?camera=1");
// 		}
// 		else
			client->connectTo("localhost",port);
			
		client->setAutoResize(sameSize);
		client->start();
		
		m_threads    << client;
		m_images     << QImage();
		m_wasChanged << false;
		
		connect(client, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)));
	}
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateFrames()));
	m_updateTimer.setInterval(1000/2);
	m_updateTimer.start();
	
	// Attempt to find an optimum window size to view the cameras in a nice symetric arragmenet
	int numItems = m_labels.size();
	
	double sq = sqrt(numItems);
	if(((int)sq) != sq)
	{
		// first, attempt to round up
		int x = (int)ceil(sq);
		int y = (int)floor(sq);
		
		if(x*y >= numItems)
			// good to go, apply size
			applySize(x,y,sameSize);
		else
		{
			// add one row then try
			y++;
			if(x*y >= numItems)
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
