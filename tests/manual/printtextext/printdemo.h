#ifndef PRINTDEMO_H
#define PRINTDEMO_H

#include <QtGui/QMainWindow>
#include "ui_printdemo.h"
#include <QtGui/QPrinter>
#include <QtGui/QFont>

class printdemo : public QMainWindow
{
	Q_OBJECT

public:
	printdemo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~printdemo();
private slots:
	void printText();
	void printPDF();
	void selectFont();
	void printChinese();

private:
	void renderChinese(QPainter *painter);
	void renderText(QPainter *painter);
	Ui::printdemoClass ui;
	QPrinter printer;
	QFont font;
	bool fontOk;

	QStringList fontData;

};

#endif // PRINTDEMO_H
