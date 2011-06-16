#include "drawtextext.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	drawtextext w;
	w.show();
	return a.exec();
}
