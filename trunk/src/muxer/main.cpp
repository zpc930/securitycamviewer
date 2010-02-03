
#include <QCoreApplication>
#include "Muxer.h"

#include <unistd.h>
// #include <systypes.h>

int muxer(int argc, char *argv[],bool verbose)
{
	QCoreApplication app(argc, argv);
	
	QCoreApplication::setOrganizationName("Josiah Bryan");
	QCoreApplication::setApplicationName("securitycam-muxer");

	Muxer mux(verbose);
	return app.exec();
}

int main(int argc, char *argv[])
{
	if(argc >= 2 && strcmp(argv[1],"-s") == 0)
 	{
 		return muxer(argc,argv,true);
 	}
 	else
 	{
 		pid_t pid;
		pid = fork();
		if(pid == 0)
			_exit(muxer(argc,argv,false));
		printf("Forked muxer with pid %d\n", (int)pid);
		return 0;
	}
}

