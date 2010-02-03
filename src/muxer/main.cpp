
#include <QCoreApplication>
#include "Muxer.h"

#include <unistd.h>
// #include <systypes.h>

int main(int argc, char *argv[])
{
 	pid_t pid;
 	pid = fork();
 	if(pid == 0)
 	{
		QCoreApplication app(argc, argv);
		Muxer mux;
		return app.exec();
		_exit(app.exec());
	}
	printf("Forked muxer with pid %d\n", (int)pid);
	return 0;
}
