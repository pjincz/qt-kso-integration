#ifndef DRAWTEXTEXT_H
#define DRAWTEXTEXT_H

#include <QtGui/QMainWindow>
#include "ui_drawtextext.h"

class drawtextext : public QMainWindow
{
	Q_OBJECT

public:
	drawtextext(QWidget *parent = 0, Qt::WFlags flags = 0);
	~drawtextext();
	void paintEvent(QPaintEvent *e);

private:
	Ui::drawtextextClass ui;
};

#endif // DRAWTEXTEXT_H
