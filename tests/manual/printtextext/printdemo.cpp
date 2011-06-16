#include "printdemo.h"
#include <QtGui/QPrintDialog>
#include <QtGui/QPainter>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtGui/QFileDialog>
#include <QtGui/QFontDialog>
#include <QtGui/QFontDatabase>

printdemo::printdemo(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	connect(ui.printAction, SIGNAL(triggered()), this, SLOT(printText()));
	connect(ui.exportAction, SIGNAL(triggered()), this, SLOT(printPDF()));
	connect(ui.selectFontAction, SIGNAL(triggered()), this, SLOT(selectFont()));
	connect(ui.printChineseAction, SIGNAL(triggered()), this, SLOT(printChinese()));
	QFontDatabase database;
	fontData = database.families(QFontDatabase::SimplifiedChinese);
}

printdemo::~printdemo()
{

}

void printdemo::selectFont()
{
	font = QFontDialog::getFont(&fontOk,this);
}


void printdemo::renderChinese(QPainter *painter)
{
	int x = 10.0;
	int y = 10.0;
	QVector<qreal> advanceDist;
	for (int i = 0; i < 10; ++i)
	{
		if (i % 2 == 0)
			advanceDist.push_back(30.0);
		else
			advanceDist.push_back(60.0);			
	}

	QVector<uint> index;
	index.push_back(18341);
	index.push_back(4677);
	index.push_back(1166);
	QString str = QString::fromLocal8Bit("金山办公软件子公司");

	QStringList::const_iterator constIterator;
	for (constIterator = fontData.constBegin(); constIterator != fontData.constEnd();++constIterator)
	{
		QFont newfont(*constIterator);
		painter->setFont(newfont);
		painter->drawText(QPointF(x, y), str, 0, QRectF(), advanceDist);
		y = y + 18.0;
		painter->drawText(QPointF(x,y), index, 0, QRect(), advanceDist);
		y = y + 18.0;
	}
	
	painter->drawText(QPointF(x, y), str);
}

void printdemo::printChinese()
{
	QPrintDialog printDialog(&printer, this);
	if (printDialog.exec())
	{
		QPainter painter(&printer);
		renderChinese(&painter);
		painter.end();
	}
}

void printdemo::renderText(QPainter *painter)
{
	QFont oldFont = painter->font();
	if (fontOk)
	{
		painter->setFont(font);
	}

	QVector<qreal> advanceDist;
	for (int i = 0; i < 8; ++i)
	{
		if (i % 2 == 0)
			advanceDist.push_back(30.0);
		else
			advanceDist.push_back(60.0);			
	}
	QString str = QString::fromLocal8Bit("测试打印文字效果");

	painter->drawText(QPointF(30.0, 30.0), str, 0, QRectF(), advanceDist);
	painter->drawText(QPointF(30.0, 80.0), QString::fromLocal8Bit("横坐标"));
	painter->drawText(QPointF(400.0, 30.0), QString::fromLocal8Bit("纵坐标"));
	painter->drawText(QPointF(450.0, 30.0), QString::fromLocal8Bit("abcdef"));


	QVector<uint> index;
	index.push_back(18341);
	index.push_back(4677);
	index.push_back(1166);
	painter->drawText(QPointF(0.0,100.0), index, 0, QRect(), advanceDist);
	painter->drawText(QPointF(0.0, 140.0), QString::fromLocal8Bit("横坐标"));
	painter->drawText(QPointF(400.0, 100.0), QString::fromLocal8Bit("纵坐标"));		
	painter->drawText(QPointF(450.0, 100.0), QString::fromLocal8Bit("ghijklmn"));

	painter->drawText(QPointF(1000.0,800.0), index, 0, QRect(), advanceDist);
	painter->drawText(QPointF(1000.0, 240.0), QString::fromLocal8Bit("横坐标"));
	painter->drawText(QPointF(1200.0, 800.0), QString::fromLocal8Bit("纵坐标"));	

	if (fontOk)
	{
		painter->setFont(oldFont);
	}
}

void printdemo::printText()
{
	QPrintDialog printDialog(&printer, this);
	if (printDialog.exec())
	{
		QPainter painter(&printer);
		renderText(&painter);
		painter.end();
	}

}

void printdemo::printPDF()
{
	QString fileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("导出PDF"), ".", QString::fromLocal8Bit("PDF files(*.pdf)"));
	if (!fileName.isEmpty())
	{
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(fileName);
		QPainter painter(&printer);
		renderText(&painter);
		painter.end();
	}



}
