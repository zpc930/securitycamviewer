#include "Muxer.h"

#include "JpegServer.h"
#include "MjpegClient.h"

#include <math.h>
#include <QPainter>


Muxer::Muxer(QObject *parent)
	: QObject(parent)
	, m_jpegServer(0)
	, m_cols(-1)
	, m_rows(-1)
{
	m_jpegServer = new JpegServer();
	m_jpegServer->setProvider(this, SIGNAL(imageReady(QImage*)));
	
	if (!m_jpegServer->listen(QHostAddress::Any,LISTEN_PORT)) 
	{
		qDebug() << "JpegServer could not start: "<<m_jpegServer->errorString();
	}
	
	
	QList<int> ports;
//	ports << 8082 << 8089 << 8095 << 8093 << 8081 << 8094 << 8083 << 8084 << 8091 << 8090 << 8092 << 8087 << 8096 << 8099 << 8085 << 8086;
	ports << 8081 << 8082 << 8083 << 8084;
	
	m_frameSize = QSize(640,480);
	//320,240);
	
	// Setup all the threads and create the labels to view the images
	foreach(int port, ports)
	{
		MjpegClient * client = new MjpegClient();
		client->connectTo("localhost",port);
			
		client->setAutoResize(m_frameSize);
		client->start();
		
		m_threads    << client;
		m_images     << QImage();
		m_wasChanged << false;
		
		connect(client, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)));
	}
	
	// Attempt to find an optimum window size to view the cameras in a nice symetric arragmenet
	int numItems = m_images.size();
	
	double sq = sqrt(numItems);
	if(((int)sq) != sq)
	{
		// first, attempt to round up
		int x = (int)ceil(sq);
		int y = (int)floor(sq);
		
		if(x*y >= numItems)
			// good to go, apply size
			applySize(x,y);
		else
		{
			// add one row then try
			y++;
			if(x*y >= numItems)
				applySize(x,y);
			else
				// just use the sqrt ceil
				applySize(x,x);
		}
	}
	else
	{
		applySize((int)sq,(int)sq);
	}
	
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateFrames()));
	m_updateTimer.setInterval(1000/2);
	m_updateTimer.start();
}

Muxer::~Muxer()
{
	while(!m_threads.isEmpty())
	{
		MjpegClient * client = m_threads.takeFirst();
		client->quit();
		client->wait();
		delete client;
		client = 0;
	}
	
	delete m_jpegServer;
}

void Muxer::applySize(int x, int y)
{
	m_cols = x;
	m_rows = y;
	int xpx = x * m_frameSize.width();
	int ypx = y * m_frameSize.height();
// 	resize(xpx + 1,ypx + 1);
	m_muxedImage = QImage(xpx,ypx,QImage::Format_RGB32);
	QPainter painter(&m_muxedImage);
	painter.fillRect(m_muxedImage.rect(), Qt::gray);
	painter.end();
	emit imageReady(&m_muxedImage);
}


void Muxer::newImage(QImage image)
{
	MjpegClient * client = dynamic_cast<MjpegClient*>(sender());
	if(client)
	{
		int index = m_threads.indexOf(client);
		m_images[index] = image;
		m_wasChanged[index] = true;
//		qDebug() << "newImage(): Received image for thread index"<<index;
	}
}

void Muxer::updateFrames()
{
	QPainter painter(&m_muxedImage);
	
	bool changed = false;
	
	int length = m_images.size();
	for(int i=0;i<length;i++)
	{
		bool flag = m_wasChanged.at(i);
		if(flag)
		{
			QImage image = m_images.at(i);
			double frac = (double)i / (double)m_cols;
			int row = (int)frac;
			int col = (frac - row) * m_cols;
			
			int x = col * m_frameSize.width();
			int y = row * m_frameSize.height();
// 			qDebug() << "updateFrames(): Painted image index "<<i<<" at row"<<row<<", col"<<col<<", coords "<<x<<"x"<<y;
			painter.drawImage(x,y, image);
			
			m_wasChanged[i] = false;
			
			changed = true;
		}
	}
	
	painter.end();
	
	if(changed)
		emit imageReady(&m_muxedImage);
}

