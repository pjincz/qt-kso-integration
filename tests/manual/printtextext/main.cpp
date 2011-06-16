#include "printdemo.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	printdemo w;
	w.show();
	return a.exec();
}
