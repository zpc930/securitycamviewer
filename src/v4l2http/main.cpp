
#include <QCoreApplication>
#include "Muxer.h"

#include "../common/getopt/getopt.h"

#include <unistd.h>
// #include <systypes.h>

int muxer(int argc, char *argv[],bool verbose)
{
	QCoreApplication app(argc, argv);
	
	QCoreApplication::setOrganizationName("Josiah Bryan");
	QCoreApplication::setApplicationName("securitycam-muxer");
	
	// construct class from command line arguments
	GetOpt opts(argc, argv);
	
	QString verboseTmp;
	opts.addOptionalOption('s',"verbose", &verboseTmp, "false");

	
	// add some switches
	QString configFile;
	opts.addOptionalOption('c',"config", &configFile, "muxer.ini");
	
	// do the parsing and check for errors
	if (!opts.parse()) 
	{
		fprintf(stderr,"Usage: %s [-s] [-c]\nNote: -s must be the first argument if used.\n", qPrintable(opts.appName()));
		return 1;
	}
	
	if(configFile.isEmpty())
		configFile = "muxer.ini";

	verbose = true;
	Muxer mux(configFile, verbose);
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

