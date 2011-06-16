#include "drawtextext.h"
#include <QtGui/QPainter>

drawtextext::drawtextext(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
}

drawtextext::~drawtextext()
{

}

void drawtextext::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e)
	QPainter painter(this);
	
	QFont serifFont("宋体", 22);
	painter.setFont(serifFont);
	
	QVector<qreal> advanceDist0;
	advanceDist0.push_back(50.0);
	advanceDist0.push_back(140.0);
	advanceDist0.push_back(55.0);
	advanceDist0.push_back(60.0);
	advanceDist0.push_back(350.0);
	advanceDist0.push_back(130.0);
	advanceDist0.push_back(35.0);
	advanceDist0.push_back(40.0);
	advanceDist0.push_back(55.0);
	advanceDist0.push_back(60.0);
	advanceDist0.push_back(350.0);
	advanceDist0.push_back(60.0);
	advanceDist0.push_back(150.0);

	
	QVector<uint> index;
	index.push_back(18341);
	index.push_back(4677);
	index.push_back(1166);
	//对应宋体中文字“金山人”
	//test drawText, use glyph index
	painter.drawText(QPointF(65.0,327.0), index, 0, QRectF(), advanceDist0);
	
	
	//test drawText, use QString
	QString str1 = QString::fromLocal8Bit("金山软件的世界里");
	painter.drawText(QPointF(65.0,127.0), str1, 0, QRectF(), advanceDist0);
	
	QString str2 = QString::fromLocal8Bit("测试");
	painter.drawText(QPointF(5.0,127.0),str2 );

	QString str3 = QString::fromLocal8Bit("成功的案例实现");
	painter.drawText(QPointF(30.0, 200.0), str3, 0, QRectF(), advanceDist0);

	QString str4 = QString::fromLocal8Bit("验证");
	painter.drawText(QPointF(30.0,250.0), str4); 


	painter.rotate(15.0);
	painter.drawText(QPointF(65.0,327.0), str1, 0, QRectF(), advanceDist0);


	painter.rotate(-15.0);
	painter.scale(1.0, 3.5);
	painter.drawText(QPointF(30.0, 200.0), str3, 0, QRectF(), advanceDist0);

	painter.setWorldTransform(QTransform());
	QVector<qreal> advanceDist1;
	advanceDist1.push_back(100.0);
	advanceDist1.push_back(75.0);
	advanceDist1.push_back(175.0);
	QString str5 = QString::fromLocal8Bit("文字显示");
	QFont font1 = painter.font();
	font1.setUnderline(true);
	painter.setFont(font1);
	painter.drawText(QPointF(30.0, 300.0),str5, 0, QRectF(), advanceDist1);

	QString str6 = QString::fromLocal8Bit("再次文字显示");
	painter.drawText(QPointF(30.0, 350.0),str6, 0, QRectF(), advanceDist1);
}
