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

#include "qbrush.h"
#include <QPainter>

#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=

class tst_QTextureBrush : public QObject
{
    Q_OBJECT

private:
    const QString m_textureFile;
    QImage m_texture;
    QImage m_canvas;

public:
    tst_QTextureBrush();

private slots:
    void init();
    void cleanup();

    void setters();

    void equal_data();
    void equal();

    void stream_data();
    void stream();

    void tile();
    void tileFlipX();
    void tileFlipY();
    void tileFlipXY();
    void clamp();

    void tileOffset();
    void scale();
    void scaleOffset();
    void scaleOffset2();

    void expand();
    void expandScale1();
    void expandScale2();
    void expandScale3flipx();
    void expandScale4flipy();
    void expandScale5();
    void expandScale6();
    void expandScale7();
    void expandScale8();

    void drawRects();
    void drawEllipse();
    void drawPolygon();
    void drawPath();
};

Q_DECLARE_METATYPE(QBrush)

tst_QTextureBrush::tst_QTextureBrush():
m_textureFile("qt-logo.png")
{
}

void tst_QTextureBrush::init()
{
    m_texture = QImage(m_textureFile);
    m_canvas = QImage(m_texture.width() * 4, m_texture.height() * 4, QImage::Format_ARGB32);
    m_canvas.fill(0x0);
}

void tst_QTextureBrush::cleanup()
{
}

void tst_QTextureBrush::setters()
{
    QBrush brush1(m_texture);
    QBrush brush2 = brush1;
    const QRectF rc(0, 0, 100, 100);
    const qreal dx = 100, dy = 200;
    const qreal sx = 2, sy = 3;
    qreal aa, bb, cc, dd;

    QCOMPARE(brush1, brush2);

    brush1.setTextureWrapMode(Qt::TextureFlippingXY);
    QCOMPARE(brush1.textureWrapMode(), Qt::TextureFlippingXY);

    brush1.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    QCOMPARE(brush1.textureAlignment(), Qt::TextureAlignmentBottomRight);

    //add the ref to 2
    brush2 = brush1;

    brush1.setTextureOffset(dx, dy);
    brush1.getTextureOffset(aa, bb);
    QCOMPARE(aa, dx);
    QCOMPARE(bb, dy);

    brush1.setTextureScale(sx, sy);
    brush1.getTextureScale(aa, bb);
    QCOMPARE(aa, sx);
    QCOMPARE(bb, sy);

    QCOMPARE(brush1.textureWrapMode(), Qt::TextureFlippingXY);
    QCOMPARE(brush1.textureAlignment(), Qt::TextureAlignmentBottomRight);

    QVERIFY(brush1 != brush2);

    brush1.setTextureWrapMode(Qt::TextureFlippingX);
    QCOMPARE(brush1.textureWrapMode(), Qt::TextureFlippingX);
    QCOMPARE(brush1.textureAlignment(), Qt::TextureAlignmentBottomRight);

    //Qt::TextureStretching brush

    brush2.setTextureWrapMode(Qt::TextureStretching);
    brush2.setTextureStretching(dx, dy);
    brush2.getTextureStretchingOffset(aa, bb, cc, dd);
    QCOMPARE(QRectF(aa, bb, cc, dd), QRectF(dx, dy, 0, 0));

    brush2.setTextureStretchingOffset(dx, dy, sx, sy);
    brush2.getTextureStretchingOffset(aa, bb, cc, dd);
    QCOMPARE(QRectF(aa, bb, cc, dd), QRectF(dx, dy, sx, sy));
}

void tst_QTextureBrush::equal_data()
{
    QTest::addColumn<QBrush>("lhs");
    QTest::addColumn<QBrush>("rhs");
    QTest::addColumn<bool>("result");

    const QPixmap pix(m_textureFile);
    const QImage img(m_textureFile);

    QTest::newRow("default vs pixmap_texture") << QBrush() << QBrush(pix) << false;
    QTest::newRow("default vs image_texture") << QBrush() << QBrush(img) << false;

    QBrush flipx(pix);
    QBrush flipy(pix);
    QBrush flipxy(pix);
    QBrush clamp(pix);

    flipx.setTextureWrapMode(Qt::TextureFlippingX);
    flipy.setTextureWrapMode(Qt::TextureFlippingY);
    flipxy.setTextureWrapMode(Qt::TextureFlippingXY);
    clamp.setTextureWrapMode(Qt::TextureNoTiling);

    QTest::newRow("pixmap_texture vs flipx") << QBrush(pix) << flipx << false;
    QTest::newRow("flipx vs flipy") << flipx << flipy << false;
    QTest::newRow("flipxy vs clamp") << flipxy << clamp << false;

    QBrush brush = QBrush(pix);
    brush.setTextureWrapMode(Qt::TextureFlippingX);
    QTest::newRow("flipx vs flipx") << flipx << brush << true;
    Q_ASSERT(flipx == brush);

    QBrush flipxAlignbr = flipx;
    flipxAlignbr.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    QTest::newRow("flipx tl vs flipx br") << flipx << flipxAlignbr << false;

    QBrush flipxAlignbr2 = flipx;
    flipxAlignbr2.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    QTest::newRow("flipx br vs another flipx br") << flipxAlignbr << flipxAlignbr2 << true;

    QBrush flipxAlignbr4 = flipx;
    flipxAlignbr4.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    flipxAlignbr4.setTextureScale(2, 3);
    QTest::newRow("flipx br vs another flipx br diff scale") << flipxAlignbr << flipxAlignbr4 << false;

    QBrush flipxAlignbr5 = flipx;
    flipxAlignbr5.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    flipxAlignbr5.setTextureOffset(2, 3);
    QTest::newRow("flipx br vs another flipx br diff offsset") << flipxAlignbr << flipxAlignbr5 << false;

    QBrush expandBrush(pix);
    expandBrush.setTextureStretching();
    QTest::newRow("flipx br vs expand br") << flipx << expandBrush << false;

    QBrush expandBrush2 = expandBrush;
    QTest::newRow("expand vs expand") << expandBrush2 << expandBrush << true;

    expandBrush2 = expandBrush;
    expandBrush2.setTextureStretchingOffset(0.1, 0.2, 0.3, 0.4);
    QTest::newRow("expand vs expand with diff offset") << expandBrush2 << expandBrush << false;

    expandBrush.setTextureStretchingOffset(0.1, 0.2, 0.3, 0.4);
    QTest::newRow("expand vs expand with same offset") << expandBrush2 << expandBrush << true;
}

void tst_QTextureBrush::equal()
{
    QFETCH(QBrush, lhs);
    QFETCH(QBrush, rhs);
    QFETCH(bool, result);

    QCOMPARE(lhs == rhs, result);
}

void tst_QTextureBrush::stream_data()
{
    equal_data();
}

void tst_QTextureBrush::stream()
{
    QFETCH(QBrush, lhs);

    QPixmap pix(m_textureFile);
    QByteArray data;

    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << lhs;
    }

    QBrush cmp;
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        stream >> cmp;
    }

    QCOMPARE(lhs.style(), cmp.style());
    QCOMPARE(lhs.color(), cmp.color());
    QCOMPARE(lhs.transform(), cmp.transform());

    if (lhs.style() == Qt::TexturePattern)
    {
        qreal dx1, dy1, sx1, sy1;
        qreal dx2, dy2, sx2, sy2;
        if (lhs.textureWrapMode() == Qt::TextureStretching)
        {
            lhs.getTextureStretchingOffset(dx1, dy1, sx1, sy1);
            cmp.getTextureStretchingOffset(dx2, dy2, sx2, sy2);
            QCOMPARE(QRectF(dx1, dy1, sx1, sy1), QRectF(dx2, dy2, sx2, sy2));
        }
        else
        {
            lhs.getTextureOffset(dx1, dy1);
            cmp.getTextureOffset(dx2, dy2);
            QCOMPARE(dx1, dx2);
            QCOMPARE(dy1, dy2);

            lhs.getTextureScale(sx1, sy1);
            cmp.getTextureScale(sx2, sy2);
            QCOMPARE(sx1, sx2);
            QCOMPARE(sy1, sy2);

            QCOMPARE(lhs.textureAlignment(), cmp.textureAlignment());
        }
    }
    else
        QCOMPARE(lhs, cmp);
}

void tst_QTextureBrush::tile()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    //QBrush(const QImage &image);
    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();
        m_canvas.save("tile.png");

        /*QImage tl = m_canvas.copy(QRect(rc.topLeft().toPoint(), m_texture.size()));
        QCOMPARE(tl, m_texture);*/
        QCOMPARE(m_canvas, QImage("res/tile.png"));
    }

    //QBrush(const QPixmap &pixmap);
    {        
        QBrush brush = QBrush(QPixmap(m_textureFile));
        brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();

        QCOMPARE(m_canvas, QImage("res/tile.png"));
    }

    {
        const QRectF rc1(rc.topLeft(), rcTexture.size() * 1.5);
        const QRectF rc2(rc1.topRight(), rc1.size());
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc1, brush);
        painter.fillRect(rc2, brush);
        painter.end();

        QCOMPARE(m_canvas.copy(rc1.toRect()), m_canvas.copy(rc2.toRect()));
    }

    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        
        QPainterPath path;
        path.addRect(rc);
        painter.fillPath(path, brush);
        painter.end();
        QCOMPARE(m_canvas, QImage("res/tile.png"));

        QPolygonF poly;
        poly << rc.topLeft() << rc.topRight() << rc.bottomRight() << rc.bottomLeft() << rc.topLeft();
        m_canvas.fill(0x0);
        painter.begin(&m_canvas);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(poly);
        QCOMPARE(m_canvas, QImage("res/tile.png"));        
    }
}

void tst_QTextureBrush::tileFlipX()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    //QBrush(const QImage &image);
    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentTopRight);
        brush.setTextureWrapMode(Qt::TextureFlippingX);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();
        m_canvas.save("tileflipx.png");

        QCOMPARE(m_canvas, QImage("res/tileflipx.png"));
    }

    //QBrush(const QPixmap &pixmap);
    {        
        QBrush brush = QBrush(QPixmap(m_textureFile));
        brush.setTextureAlignment(Qt::TextureAlignmentTopRight);
        brush.setTextureWrapMode(Qt::TextureFlippingX);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();

        QCOMPARE(m_canvas, QImage("res/tileflipx.png"));
    }
}

void tst_QTextureBrush::tileFlipY()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    //QBrush(const QImage &image);
    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentBottomLeft);
        brush.setTextureWrapMode(Qt::TextureFlippingY);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();
        m_canvas.save("tileflipy.png");

        QCOMPARE(m_canvas, QImage("res/tileflipy.png"));
    }

    //QBrush(const QPixmap &pixmap);
    {        
        QBrush brush = QBrush(QPixmap(m_textureFile));
        brush.setTextureAlignment(Qt::TextureAlignmentBottomLeft);
        brush.setTextureWrapMode(Qt::TextureFlippingY);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();

        QCOMPARE(m_canvas, QImage("res/tileflipy.png"));
    }
}

void tst_QTextureBrush::tileFlipXY()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    //QBrush(const QImage &image);
    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
        brush.setTextureWrapMode(Qt::TextureFlippingXY);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();
        m_canvas.save("tileflipxy.png");

        QCOMPARE(m_canvas, QImage("res/tileflipxy.png"));
    }

    //QBrush(const QPixmap &pixmap);
    {        
        QBrush brush = QBrush(QPixmap(m_textureFile));
        brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
        brush.setTextureWrapMode(Qt::TextureFlippingXY);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();

        QCOMPARE(m_canvas, QImage("res/tileflipxy.png"));
    }
}

void tst_QTextureBrush::clamp()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    //QBrush(const QImage &image);
    {
        QBrush brush(m_texture);
        brush.setTextureAlignment(Qt::TextureAlignmentCenter);
        brush.setTextureWrapMode(Qt::TextureNoTiling);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);
        painter.end();
        m_canvas.save("clamp.png");

        QCOMPARE(m_canvas, QImage("res/clamp.png"));
    }

    //QBrush(const QPixmap &pixmap);
    {
        QBrush brush = QBrush(QPixmap(m_textureFile));
        brush.setTextureAlignment(Qt::TextureAlignmentCenter);
        brush.setTextureWrapMode(Qt::TextureNoTiling);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.fillRect(rc, brush);

        QCOMPARE(m_canvas, QImage("res/clamp.png"));
    }
}

void tst_QTextureBrush::tileOffset()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);
    brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
    brush.setTextureOffset(m_texture.width(), m_texture.height());
    QPainter painter(&m_canvas);
    painter.fillRect(rc, brush);
    painter.end();
    m_canvas.save("tileoffset.png");

    /*QImage tl = m_canvas.copy(QRect(rc.topLeft().toPoint(), m_texture.size()));
    QCOMPARE(tl, m_texture.mirrored(true, true));*/
    QCOMPARE(m_canvas, QImage("res/tileoffset.png"));
}

void tst_QTextureBrush::scale()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);
    brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
    brush.setTextureScale(0.5, 0.5);
    QPainter painter(&m_canvas);
    painter.fillRect(rc, brush);
    painter.end();
    m_canvas.save("scale.png");

    QCOMPARE(m_canvas, QImage("res/scale.png"));
}

void tst_QTextureBrush::scaleOffset()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);
    brush.setTextureAlignment(Qt::TextureAlignmentTopLeft);
    brush.setTextureScale(0.5, 0.5);
    brush.setTextureOffset(m_texture.width(), m_texture.height());
    QPainter painter(&m_canvas);
    painter.fillRect(rc, brush);
    painter.end();
    m_canvas.save("scaleoffset.png");

    QCOMPARE(m_canvas, QImage("res/scaleoffset.png"));
    QCOMPARE(m_canvas, QImage("res/scale.png"));
}

void tst_QTextureBrush::scaleOffset2()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);
    brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    brush.setTextureScale(2, 2);
    brush.setTextureOffset(m_texture.width() * 2, m_texture.height() * 2);
    QPainter painter(&m_canvas);
    painter.fillRect(rc, brush);
    painter.end();
    m_canvas.save("scaleoffset2.png");

    QCOMPARE(m_canvas, QImage("res/scaleoffset2.png"));
}

void tst_QTextureBrush::expand()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching();
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rc);
    painter.end();
    m_canvas.save("expand.png");

    QCOMPARE(m_canvas, QImage("res/expand.png"));
}

void tst_QTextureBrush::expandScale1()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0.1, 0.1, 0.1, 0.1);
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rc);
    painter.end();
    m_canvas.save("expandscale1.png");

    QCOMPARE(m_canvas, QImage("res/expandscale1.png"));
}

void tst_QTextureBrush::expandScale2()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(-0.1, -0.1, -0.1, -0.1);
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rc);
    painter.end();
    m_canvas.save("expandscale2.png");

    QCOMPARE(m_canvas, QImage("res/expandscale2.png"));
}

void tst_QTextureBrush::expandScale3flipx()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(1, 1, 0, 0);//flip x
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rc);
    painter.end();
    m_canvas.save("expandscale3flipx.png");

    QCOMPARE(m_canvas, QImage("res/expandscale3flipx.png"));
}

void tst_QTextureBrush::expandScale4flipy()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0, 0, 1, 1);//flip y
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rc);
    painter.end();
    m_canvas.save("expandscale4flipy.png");

    QCOMPARE(m_canvas, QImage("res/expandscale4flipy.png"));
}

void tst_QTextureBrush::expandScale5()
{
    const QRectF rcLeft(30, 50, (m_canvas.width() - 60) / 2.0, m_canvas.height() - 100);
    const QRectF rcRight(rcLeft.topRight(), rcLeft.size());
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0, -1, 0, 0);
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rcLeft);

    brush.setTextureStretching(-1, 0, 0, 0);
    painter.setBrush(brush);
    painter.drawRect(rcRight);

    painter.end();
    m_canvas.save("expandscale5.png");

    QCOMPARE(m_canvas, QImage("res/expandscale5.png"));
}

void tst_QTextureBrush::expandScale6()
{
    const QRectF rcLeftTop(30, 50, (m_canvas.width() - 60) / 2.0, (m_canvas.height() - 100) / 2.0);
    const QRectF rcRightTop(rcLeftTop.topRight(), rcLeftTop.size());
    const QRectF rcBottomLeft(rcLeftTop.bottomLeft(), rcLeftTop.size());
    const QRectF rcBottomRight(rcLeftTop.bottomRight(), rcLeftTop.size());

    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0, -1, 0, -1);
    QPainter painter(&m_canvas);
    painter.setBrush(brush);
    painter.drawRect(rcLeftTop);

    brush.setTextureStretching(-1, 0, 0, -1);
    painter.setBrush(brush);
    painter.drawRect(rcRightTop);

    brush.setTextureStretching(0, -1, -1, 0);
    painter.setBrush(brush);
    painter.drawRect(rcBottomLeft);

    brush.setTextureStretching(-1, 0, -1, 0);
    painter.setBrush(brush);
    painter.drawRect(rcBottomRight);

    painter.end();
    m_canvas.save("expandscale6.png");

    QCOMPARE(m_canvas, QImage("res/expandscale6.png"));
}

void tst_QTextureBrush::expandScale7()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0.5, 0.5, 0, 0);
    m_canvas.fill(0x0);
    const QImage cmp = m_canvas;

    QPainter painter(&m_canvas);
    painter.rotate(30);
    painter.fillRect(rc, brush);
    painter.end();

    QCOMPARE(m_canvas, cmp);
}

void tst_QTextureBrush::expandScale8()
{
    const QRectF rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);
    const QRectF rcTexture(0, 0, m_texture.width(), m_texture.height());

    QBrush brush(m_texture);
    brush.setTextureStretching(0, 0, 0.3, 0.7);
    m_canvas.fill(0x0);
    const QImage cmp = m_canvas;

    QPainter painter(&m_canvas);
    painter.rotate(30);
    painter.fillRect(rc, brush);
    painter.end();

    QCOMPARE(m_canvas, cmp);
}

void tst_QTextureBrush::drawRects()
{
    QVector<QRect> rcs;
    rcs << QRect(0, 0, m_canvas.width(), m_canvas.height());
    rcs << QRect(rcs[0].bottomRight(), rcs[0].size());

    QBrush brush(m_texture);
    brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);

    //QRect
    {
        QImage canvas(m_canvas.width() * 2, m_canvas.height() * 2, m_canvas.format());
        canvas.fill(0x0);
        QPainter painter(&canvas);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRects(rcs);
        painter.end();
        canvas.save("drawrects.png");

        QCOMPARE(canvas.copy(rcs[0]), canvas.copy(rcs[1]));
        QCOMPARE(canvas, QImage("res/drawrects.png"));
    }

    //QRectF
    {
        QVector<QRectF> rcfs;
        rcfs << rcs[0] << rcs[1];
        QImage canvas(m_canvas.width() * 2, m_canvas.height() * 2, m_canvas.format());
        canvas.fill(0x0);
        QPainter painter(&canvas);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRects(rcfs);
        painter.end();

        QCOMPARE(canvas.copy(rcfs[0].toRect()), canvas.copy(rcfs[1].toRect()));
        QCOMPARE(canvas, QImage("res/drawrects.png"));
    }
}

void tst_QTextureBrush::drawEllipse()
{
    const QRect rc(30, 50, m_canvas.width() - 60, m_canvas.height() - 100);

    QBrush brush(m_texture);
    brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);

    //QRect
    {
        QPainter painter(&m_canvas);
        painter.setBrush(brush);
        painter.fillRect(rc,brush);
        painter.drawEllipse(rc);
        painter.end();
        m_canvas.save("drawellipse.png");

        QCOMPARE(m_canvas, QImage("res/drawellipse.png"));
    }

    //QRectF
    {
        QRectF rcf(rc);
        m_canvas.fill(0x0);
        QPainter painter(&m_canvas);
        painter.setBrush(brush);
        painter.fillRect(rcf, brush);
        painter.drawEllipse(rcf);
        painter.end();

        QCOMPARE(m_canvas, QImage("res/drawellipse.png"));
    }
}


void tst_QTextureBrush::drawPolygon()
{
    const int nWidth = m_canvas.width();
    const int nHeight = m_canvas.height();

    QPolygon poly;
    poly << QPoint(0, 0) << QPoint(nWidth, 0) << QPoint(nWidth, nHeight)
        << QPoint(nWidth * 2, nHeight) << QPoint(nWidth * 2, nHeight * 2)
        << QPoint(0, nHeight * 2) << QPoint(0, 0);

    QBrush brush(m_texture);
    brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);

    //QPolygon
    {
        QImage canvas(m_canvas.width() * 2, m_canvas.height() * 2, m_canvas.format());
        canvas.fill(0x0);
        QPainter painter(&canvas);
        painter.setBrush(brush);
        painter.drawPolygon(poly);
        painter.end();
        canvas.save("drawpolygon.png");

        QCOMPARE(canvas, QImage("res/drawpolygon.png"));
    }

    //QPolygonF
    {
        QPolygonF polyf(poly);
        QImage canvas(m_canvas.width() * 2, m_canvas.height() * 2, m_canvas.format());
        canvas.fill(0x0);
        QPainter painter(&canvas);
        painter.setBrush(brush);
        painter.drawPolygon(poly);
        painter.end();
        canvas.save("drawpolygon.png");

        QCOMPARE(canvas, QImage("res/drawpolygon.png"));
    }
}

void tst_QTextureBrush::drawPath()
{
    const int nWidth = m_canvas.width();
    const int nHeight = m_canvas.height();

    QPolygonF poly;
    poly << QPointF(0, 0) << QPointF(nWidth, 0) << QPointF(nWidth, nHeight)
        << QPointF(nWidth * 2, nHeight) << QPointF(nWidth * 2, nHeight * 2)
        << QPointF(0, nHeight * 2) << QPointF(0, 0);
    QPainterPath path;
    path.addPolygon(poly);

    QBrush brush(m_texture);
    brush.setTextureAlignment(Qt::TextureAlignmentBottomRight);
    brush.setTextureWrapMode(Qt::TextureFlippingXY);

    QImage canvas(m_canvas.width() * 2, m_canvas.height() * 2, m_canvas.format());
    canvas.fill(0x0);
    QPainter painter(&canvas);
    painter.setBrush(brush);
    painter.drawPath(path);
    painter.end();

    QCOMPARE(canvas, QImage("res/drawpolygon.png"));
}

QTEST_MAIN(tst_QTextureBrush)
#include "tst_qtexturebrush.moc"
