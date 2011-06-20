/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>

#include "../../src/gui/painting/qpathgradient_p.h"
#include "qbrush.h"
#include <QPainter>

#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=
Q_DECLARE_METATYPE(argb8)

class tst_QPathGradientBrush : public QObject
{
    Q_OBJECT

private:
    QPathGradient m_pg;

public:
    tst_QPathGradientBrush();

private slots:
    void init();
    void cleanup();

    //argb8
    void argb8Constructor();
    void argb8Gradient_data();
    void argb8Gradient();
    void argb8Equal_data();
    void argb8Equal();

    //path_gradient_span_gen
    void pgsgGenerate();

    //brush
    void rectGradient();
    void ellipseGradient();
    void heartGradient();
};

Q_DECLARE_METATYPE(QBrush)

tst_QPathGradientBrush::tst_QPathGradientBrush()
{
    m_pg.setColorAt(0, Qt::red);
    m_pg.setColorAt(0.5, Qt::green);
    m_pg.setColorAt(1, Qt::blue);
}

void tst_QPathGradientBrush::init()
{
    QRectF rc(0, 0, 200, 200);
    QPainterPath path;
    path.addRect(rc);
    m_pg.setPath(path);
    m_pg.setCenter(rc.center());
}

void tst_QPathGradientBrush::cleanup()
{
}

//argb8////////////////////////////////////////////////////////////////////
void tst_QPathGradientBrush::argb8Constructor()
{
    const uint *pUint = NULL;
    argb8 clr1;
    QVERIFY(clr1.a == 255 && clr1.r == 0 && clr1.g == 0 && clr1.b == 0);
    pUint = (uint*)&clr1;
    QCOMPARE(*pUint, 0xff000000);

    argb8 clr2(255, 255, 255);
    QVERIFY(clr2.a == 255 && clr2.r == 255 && clr2.g == 255 && clr2.b == 255);
    pUint = (uint*)&clr2;
    QCOMPARE(*pUint, 0xffffffff);

    argb8 clr3(0x12, 0x34, 0x56, 0x78);
    QVERIFY((int)clr3.a == 0x12 && (int)clr3.r == 0x34 && (int)clr3.g == 0x56 && (int)clr3.b == 0x78);
    pUint = (uint*)&clr3;
    QCOMPARE(int(*pUint), 0x12345678);
}

void tst_QPathGradientBrush::argb8Gradient_data()
{
    QTest::addColumn<argb8>("from");
    QTest::addColumn<argb8>("to");
    QTest::addColumn<qreal>("gradient");
    QTest::addColumn<argb8>("result");

    QTest::newRow("black black 0") << argb8() << argb8() << 0.0 << argb8();
    QTest::newRow("black black 0.5") << argb8() << argb8() << 0.5 << argb8();
    QTest::newRow("black black 1") << argb8() << argb8() << 1.0 << argb8();

    QTest::newRow("black white 0") << argb8() << argb8(255, 255, 255) << 0.0 << argb8();
    QTest::newRow("black white 0.3") << argb8() << argb8(255, 255, 255) << 0.3 << argb8(76, 76, 76);
    QTest::newRow("black white 0.5") << argb8() << argb8(255, 255, 255) << 0.5 << argb8(127, 127, 127);
    QTest::newRow("black white 1.0") << argb8() << argb8(255, 255, 255) << 1.0 << argb8(255, 255, 255);

    QTest::newRow("red white 0") << argb8(255, 0, 0) << argb8(255, 255, 255) << 0.0 << argb8(255, 0, 0);
    QTest::newRow("red white 0.3") << argb8(255, 0, 0) << argb8(255, 255, 255) << 0.3 << argb8(255, 76, 76);
    QTest::newRow("red white 0.5") << argb8(255, 0, 0) << argb8(255, 255, 255) << 0.5 << argb8(255, 127, 127);
    QTest::newRow("red white 1.0") << argb8(255, 0, 0) << argb8(255, 255, 255) << 1.0 << argb8(255, 255, 255);

    QTest::newRow("black 0x12345678 0")   << argb8() << argb8(0x12, 0x34, 0x56, 0x78) << 0.0 << argb8();
    QTest::newRow("black 0x12345678 0.3") << argb8() << argb8(0x12, 0x34, 0x56, 0x78) << 0.3 << argb8(183, 15, 25, 36);
    QTest::newRow("black 0x12345678 0.5") << argb8() << argb8(0x12, 0x34, 0x56, 0x78) << 0.5 << argb8(136, 26, 43, 60);
    QTest::newRow("black 0x12345678 1.0") << argb8() << argb8(0x12, 0x34, 0x56, 0x78) << 1.0 << argb8(0x12, 0x34, 0x56, 0x78);

    QTest::newRow("red 0x12345678 0") << argb8(255, 0, 0) << argb8(0x12, 0x34, 0x56, 0x78) << 0.0 << argb8(255, 0, 0);
    QTest::newRow("red 0x12345678 0.3") << argb8(255, 0, 0) << argb8(0x12, 0x34, 0x56, 0x78) << 0.3 << argb8(183, 193, 25, 36);
    QTest::newRow("red 0x12345678 0.5") << argb8(255, 0, 0) << argb8(0x12, 0x34, 0x56, 0x78) << 0.5 << argb8(136, 153, 43, 60);
    QTest::newRow("red 0x12345678 1.0") << argb8(255, 0, 0) << argb8(255, 255, 255) << 1.0 << argb8(255, 255, 255);
}

void tst_QPathGradientBrush::argb8Gradient()
{
    QFETCH(argb8, from);
    QFETCH(argb8, to);
    QFETCH(qreal, gradient);
    QFETCH(argb8, result);

    QCOMPARE(from.gradient(to, gradient), result);
}

void tst_QPathGradientBrush::argb8Equal_data()
{
    QTest::addColumn<argb8>("lhs");
    QTest::addColumn<argb8>("rhs");
    QTest::addColumn<bool>("result");

    QTest::newRow("black vs black") << argb8() << argb8() << true;
    QTest::newRow("black vs white") << argb8() << argb8(255, 255, 255) << false;
    QTest::newRow("0x12345678 vs 0x12345678") << argb8(0x12, 0x34, 0x56, 0x78)
        << argb8(0x12, 0x34, 0x56, 0x78) << true;
    QTest::newRow("0x12345678 vs 0x78563412") << argb8(0x12, 0x34, 0x56, 0x78)
        << argb8(0x78, 0x56, 0x34, 0x12) << false;

}

void tst_QPathGradientBrush::argb8Equal()
{
    QFETCH(argb8, lhs);
    QFETCH(argb8, rhs);
    QFETCH(bool, result);

    QCOMPARE(lhs == rhs, result);
}

//path_gradient_span_gen
void tst_QPathGradientBrush::pgsgGenerate()
{
    const QRectF rc(0, 0, 200, 300);
    QPainterPath path;
    path.addRect(rc);
    QGradientStops stops;
    stops << QGradientStop(0, Qt::red)
        << QGradientStop(0.5, Qt::green)
        << QGradientStop(1.0, Qt::blue);
    QPathGradientBrush pgbrush(path, rc.center(), stops, QMatrix());
    path_gradient_span_gen gen(pgbrush);
    QVector<argb8>buffer(rc.width());
    gen.prepare();

    //first row
    gen.generate(&buffer.front(), 0, 0, rc.width());
    for (int i = 0; i < rc.width(); i++)
    {
        const argb8 &r = buffer.at(i);
        QCOMPARE(QColor(r.r, r.g, r.b, r.a), QColor(Qt::blue));
    }

    //last row
    gen.generate(&buffer.front(), 0, rc.bottom(), rc.width());
    for (int i = 0; i < rc.width(); i++)
    {
        const argb8 &r = buffer.at(i);
        QCOMPARE(QColor(r.r, r.g, r.b, r.a), QColor(Qt::blue));
    }

    //first column
    for (int j = 0; j < rc.height(); j++)
    {
    	gen.generate(&buffer.front(), 0, j, 1);
        const argb8 &r = buffer.at(0);
        QCOMPARE(QColor(r.r, r.g, r.b, r.a), QColor(Qt::blue));
    }

    //last column
    /*for (int j = 0; j < rc.height(); j++)
    {
        gen.generate(&buffer.front(), rc.right(), j, 1);
        const argb8 &r = buffer.at(0);
        QCOMPARE(QColor(r.r, r.g, r.b, r.a), QColor(Qt::blue));
    }*/

    {//the same color
        QGradientStops stops;
        stops << QGradientStop(0, Qt::blue)
            << QGradientStop(1.0, Qt::blue);
        QPathGradientBrush pgbrush(path, rc.center(), stops, QMatrix());
        path_gradient_span_gen gen(pgbrush);

        for (int h = 0; h < rc.height(); h++)
        {
            gen.generate(&buffer.front(), 0, h, rc.width());
            for (int w = 0; w < rc.width(); w++)
            {
                const argb8 &r = buffer.at(w);
                QCOMPARE(QColor(r.r, r.g, r.b, r.a), QColor(Qt::blue));
            }
        }
    }
}

//brush////////////////////////////////////////////////////////////////////
void tst_QPathGradientBrush::rectGradient()
{
    QRect rc(0, 0, 200, 200);
    QImage img(rc.width(), rc.height(), QImage::Format_RGB32);
    QPainter painter(&img);
    painter.fillRect(rc, QBrush(m_pg));
    img.save("rectGradient.png");

    QColor centerClr = m_pg.stops().at(0).second;
    QColor halfClr = m_pg.stops().at(1).second;
    QColor surroundClr = m_pg.stops().at(2).second;

    QCOMPARE(centerClr, QColor(Qt::red));
    QCOMPARE(halfClr, QColor(Qt::green));
    QCOMPARE(surroundClr, QColor(Qt::blue));

    QCOMPARE(img.pixel(100, 100), centerClr.rgba());

    QCOMPARE(img.pixel(rc.topLeft()), surroundClr.rgba());
    QCOMPARE(img.pixel(rc.topRight()), surroundClr.rgba());
    QCOMPARE(img.pixel(rc.bottomLeft()), surroundClr.rgba());
    //QCOMPARE(img.pixel(rc.bottomRight()), surroundClr.rgba());

    QCOMPARE(img, QImage("res/rectGradient.png"));
}

void tst_QPathGradientBrush::ellipseGradient()
{
    QRect rc(0, 0, 200, 200);
    QPainterPath path;
    path.addEllipse(rc);
    m_pg.setPath(path);
    QImage img(rc.width(), rc.height(), QImage::Format_RGB32);
    QPainter painter(&img);
    painter.fillRect(rc, QBrush(m_pg));
    img.save("ellipseGradient.png");

    QColor centerClr = m_pg.stops().at(0).second;
    QColor halfClr = m_pg.stops().at(1).second;
    QColor surroundClr = m_pg.stops().at(2).second;

    QCOMPARE(centerClr, QColor(Qt::red));
    QCOMPARE(halfClr, QColor(Qt::green));
    QCOMPARE(surroundClr, QColor(Qt::blue));

    QCOMPARE(img.pixel(100, 100), centerClr.rgba());
    QCOMPARE(img.pixel(0, 100), surroundClr.rgba());
    QCOMPARE(img.pixel(100, 0), surroundClr.rgba());

    QCOMPARE(img.pixel(rc.topLeft()), QRgb(0));
    QCOMPARE(img.pixel(rc.topRight()), QRgb(0));
    QCOMPARE(img.pixel(rc.bottomLeft()), QRgb(0));
    QCOMPARE(img.pixel(rc.bottomRight()), QRgb(0));

    QCOMPARE(img, QImage("res/ellipseGradient.png"));
}

void tst_QPathGradientBrush::heartGradient()
{
    QPainterPath path;
    path.moveTo(200, 200);
    path.lineTo(300, 0);
    path.lineTo(400, 100);
    path.lineTo(200, 400);
    path.lineTo(0, 100);
    path.lineTo(100, 0);
    path.closeSubpath();
    m_pg.setPath(path);
    m_pg.setCenter(200, 200);

    QRect rc = path.boundingRect().toRect();
    QImage img(rc.width(), rc.height(), QImage::Format_RGB32);
    QPainter painter(&img);
    painter.fillRect(rc, QBrush(m_pg));
    img.save("heartGradient.png");

    QColor centerClr = m_pg.stops().at(0).second;
    QColor halfClr = m_pg.stops().at(1).second;
    QColor surroundClr = m_pg.stops().at(2).second;

    QCOMPARE(centerClr, QColor(Qt::red));
    QCOMPARE(halfClr, QColor(Qt::green));
    QCOMPARE(surroundClr, QColor(Qt::blue));

    QCOMPARE(img.pixel(200, 200), centerClr.rgba());

    QCOMPARE(img.pixel(300, 0), surroundClr.rgba());
    //QCOMPARE(img.pixel(400 - 1, 100), surroundClr.rgba());
    QCOMPARE(img.pixel(200, 400 - 1), surroundClr.rgba());
    QCOMPARE(img.pixel(0, 100), surroundClr.rgba());
    QCOMPARE(img.pixel(100, 0), surroundClr.rgba());

    QCOMPARE(img.pixel(rc.topLeft()), QRgb(0));
    QCOMPARE(img.pixel(rc.topRight()), QRgb(0));
    QCOMPARE(img.pixel(rc.bottomLeft()), QRgb(0));
    QCOMPARE(img.pixel(rc.bottomRight()), QRgb(0));

    QCOMPARE(img, QImage("res/heartGradient.png"));
}

QTEST_MAIN(tst_QPathGradientBrush)
#include "tst_qpathgradientbrush.moc"
