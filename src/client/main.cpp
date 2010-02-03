#include <QApplication>
#include "MainWindow.h"

#include "../common/getopt/getopt.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	// construct class from command line arguments
	GetOpt opts(argc, argv);
	
	bool verbose = false;
	opts.addSwitch("verbose", &verbose);

	// add some switches
	QString configFile;
	opts.addOptionalOption('c',"config", &configFile, "viewer.ini");
	
	// do the parsing and check for errors
	if (!opts.parse()) 
	{
		fprintf(stderr,"Usage: %s [--verbose] [-c|--config configfile]\n", qPrintable(opts.appName()));
		return 1;
	}
	
	if(configFile.isEmpty())
		configFile = "viewer.ini";
			
		
		
	MainWindow viewer(configFile,verbose);
	viewer.show();
	
	return app.exec();
}
