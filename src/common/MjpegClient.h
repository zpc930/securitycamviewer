#ifndef MjpegClient_H
#define MjpegClient_H

#include <QObject>
#include <QByteArray>
#include <QTcpSocket>
#include <QtNetwork>

#include <QThread>
#include <QImage>

//#define MJPEG_TEST 1

#ifdef MJPEG_TEST
  #include <QLabel>
#endif

class MjpegClient: public QThread
{
	Q_OBJECT
public:
	MjpegClient(QObject *parent = 0);
	~MjpegClient();
	
	bool connectTo(const QString& host, int port=-1, QString url = "/", const QString& user="", const QString& pass="");
	void exit();
	QString errorString(){ return m_socket->errorString(); }
	
	QSize autoResize() { return m_autoResize; }
	void setAutoResize(QSize size) { m_autoResize = size; }
	
	bool autoReconnect() { return m_autoReconnect; }
	void setAutoReconnect(bool flag) { m_autoReconnect = flag; }
	
	QString host() { return m_host; }
	int port() { return m_port; }
	QString path() { return m_url; }
	
	bool flipImage() { return m_flipImage; }
	void setFlipImage(bool flip) { m_flipImage = flip; }
	
	bool pollingMode() { return m_pollMode; }
	void setPollingMode(bool flag);
	
	int pollingFps() { return m_pollFps; }
	void setPollingFps(int fps) { m_pollFps = fps; }
	
signals:
	void socketDisconnected();
	void socketError(QAbstractSocket::SocketError);
	void socketConnected();
	
	void newImage(QImage);
	
private slots:
	void dataReady();
	void processBlock();
	void lostConnection();
	void lostConnection(QAbstractSocket::SocketError);
	void reconnect();
	void connectionReady();
	
	// polling slots
	void loadUrl(const QString &url);
	void handleNetworkData(QNetworkReply *networkReply);
	void pollServer();

private:
	void log(const QString&);
	QTcpSocket *m_socket;
	
	QString m_boundary;
	bool m_firstBlock;
	
	QByteArray m_dataBlock;
	
	QSize m_autoResize;
	bool m_autoReconnect;
	
	QString m_host;
	int m_port;
	QString m_url;
	QString m_user;
	QString m_pass;
	
	bool m_flipImage;
	bool m_pollMode;
	int  m_pollFps;
	
#ifdef MJPEG_TEST
	QLabel *m_label;
#endif

};

#endif // MjpegClient_H

