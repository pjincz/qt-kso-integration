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

#include <QPainter>

class tst_QImageEffects : public QObject
{
    Q_OBJECT
private:
    const QImage m_image;

public:
    tst_QImageEffects();

private slots:
    void init();
    void cleanup();

    void data();

    void gray_data();
    void gray();
    void bilevel_data();
    void bilevel();
    void bright_data();
    void bright();
    void contrast_data();
    void contrast();
    void colorKey_data();
    void colorKey();
    void watermark();

    void recolor_data();
    void recolor();

    void setColorMatrix_data();
    void setColorMatrix();
    void unsetColorMatrix_data();
    void unsetColorMatrix();

private:
    void recolor(QPainter &painter, QImage &image);
};

Q_DECLARE_METATYPE(QBrush)

tst_QImageEffects::tst_QImageEffects():
m_image(QImage("res/colorful_rgb.bmp"))
{
}

void tst_QImageEffects::init()
{
}

void tst_QImageEffects::cleanup()
{
}

void tst_QImageEffects::data()
{
    QTest::addColumn<QImage>("image");

    QTest::newRow("Format_Mono") << m_image.convertToFormat(QImage::Format_Mono);
    QTest::newRow("Format_MonoLSB") << m_image.convertToFormat(QImage::Format_MonoLSB);
    QTest::newRow("Format_Indexed8") << m_image.convertToFormat(QImage::Format_Indexed8);
    QTest::newRow("Format_RGB32") << m_image.convertToFormat(QImage::Format_RGB32);
    QTest::newRow("Format_ARGB32") << m_image.convertToFormat(QImage::Format_ARGB32);
    QTest::newRow("Format_ARGB32_Premultiplied") << m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QTest::newRow("Format_RGB16") << m_image.convertToFormat(QImage::Format_RGB16);
    QTest::newRow("Format_ARGB8565_Premultiplied") << m_image.convertToFormat(QImage::Format_ARGB8565_Premultiplied);
    QTest::newRow("Format_RGB666") << m_image.convertToFormat(QImage::Format_RGB666);
    QTest::newRow("Format_ARGB6666_Premultiplied") << m_image.convertToFormat(QImage::Format_ARGB6666_Premultiplied);
    QTest::newRow("Format_RGB555") << m_image.convertToFormat(QImage::Format_RGB555);
    QTest::newRow("Format_ARGB8555_Premultiplied") << m_image.convertToFormat(QImage::Format_ARGB8555_Premultiplied);
    QTest::newRow("Format_RGB888") << m_image.convertToFormat(QImage::Format_RGB888);
    QTest::newRow("Format_RGB444") << m_image.convertToFormat(QImage::Format_RGB444);
    QTest::newRow("Format_ARGB4444_Premultiplied") << m_image.convertToFormat(QImage::Format_ARGB4444_Premultiplied);

    QImage img(80, 80, QImage::Format_ARGB32);
    for (int y = 0; y < 80; y++) {
        QRgb *scanline = (QRgb*)img.scanLine(y);
    	for (int x = 0; x < 80; x++) {
            scanline[x] = qRgba(255, 0, 0, y * 3);
    	}
    }
    QTest::newRow("ARGB32(x, 255, 0, 0)") << img;

    for (int y = 0; y < 80; y++) {
        QRgb *scanline = (QRgb*)img.scanLine(y);
        for (int x = 0; x < 80; x++) {
            scanline[x] = qRgba(100, 200, 0, y * 3);
        }
    }
    QTest::newRow("ARGB32(x, 100, 200, 0)") << img;
}

void tst_QImageEffects::gray_data()
{
    data();
}

void tst_QImageEffects::gray()
{
    QFETCH(QImage, image);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImageEffects effects;
    effects.setDuotone(255, 255, 255);

    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QCOMPARE(qRed(rgb), qGreen(rgb));
            QCOMPARE(qGreen(rgb), qBlue(rgb));
        }
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, 1, 1), &effects);
    for (int h = 0; h < cavas.height(); h++) {
    	for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QCOMPARE(qRed(rgb), qGreen(rgb));
            QCOMPARE(qGreen(rgb), qBlue(rgb));
    	}
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, image.width() / 2, image.height() / 2), &effects);
    for (int h = 0; h < cavas.height(); h++) {
    	for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QCOMPARE(qRed(rgb), qGreen(rgb));
            QCOMPARE(qGreen(rgb), qBlue(rgb));
    	}
    }
}

void tst_QImageEffects::bilevel_data()
{
    data();
}

void tst_QImageEffects::bilevel()
{
    QFETCH(QImage, image);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImageEffects effects;

    effects.setBilevel(0);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            if (qAlpha(rgb) == 255)
                QCOMPARE(rgb & 0x00ffffff, 0xffffffu);
            else if (qAlpha(rgb) != 0)
                QVERIFY((rgb & 0x00ffffff) >= 0xfefefe);
        }
    }    

    effects.setBilevel(25);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 255)
                QVERIFY(rgb == 0xffffffff || rgb == 0xff000000);
            else
                QVERIFY(qRed(rgb) >= 0xfe || qRed(rgb) == 0);
        }
    }

    effects.setBilevel(50);
    painter.drawImage(cavas.rect(), image, QRectF(0, 0, 1, 1), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 255)
                QVERIFY(rgb == 0xffffffff || rgb == 0xff000000);
            else
                QVERIFY(qRed(rgb) >= 0xfe || qRed(rgb) == 0);
        }
    }

    effects.setBilevel(75);
    painter.drawImage(cavas.rect(), image, QRectF(0, 0, image.width() / 2, image.height() / 2), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 255)
                QVERIFY(rgb == 0xffffffff || rgb == 0xff000000);
            else
                QVERIFY(qRed(rgb) >= 0xfe || qRed(rgb) == 0);
        }
    }

    effects.setBilevel(100);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            if (qAlpha(rgb) != 0)
                QCOMPARE(rgb & 0x00ffffff, 0x0u);
        }
    }
}

void tst_QImageEffects::bright_data()
{
    data();
}

void tst_QImageEffects::bright()
{
    QFETCH(QImage, image);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImageEffects effects;

    effects.setBrightness(-1.0);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY((rgb | 0xff000000) == 0xff000000);
        }
    }

    effects.setBrightness(1.0);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 0)
                QCOMPARE(qRed(rgb), 0);
            else if (qAlpha(rgb) != 255)
                QVERIFY(qRed(rgb) >= 0xfe);
            else
                QVERIFY(qRed(rgb) == 0xff);
        }
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, 1, 1), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 0)
                QCOMPARE(qRed(rgb), 0);
            else if (qAlpha(rgb) != 255)
                QVERIFY(qRed(rgb) >= 0xfe);
            else
                QVERIFY(qRed(rgb) == 0xff);
        }
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, image.width() / 2, image.height() / 2), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (qAlpha(rgb) == 0)
                QCOMPARE(qRed(rgb), 0);
            else if (qAlpha(rgb) != 255)
                QVERIFY(qRed(rgb) >= 0xfe);
            else
                QVERIFY(qRed(rgb) == 0xff);
        }
    }
}

void tst_QImageEffects::contrast_data()
{
    data();
}

void tst_QImageEffects::contrast()
{
    QFETCH(QImage, image);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImageEffects effects;

    effects.setContrast(0.0);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            QVERIFY(qIsGray(rgb));
            if (!image.hasAlphaChannel())
                QCOMPARE(cavas.pixel(0, 0) & 0x00ffffff, rgb & 0x00ffffff);
        }
    }
    

    effects.setContrast(1 << 10);
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            if (qAlpha(rgb) == 255) {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) == 0xff);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) == 0xff);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) == 0xff);
            } else {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) >= 0xfe);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) >= 0xfe);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) >= 0xfe);
            }
        }
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, 1, 1), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            if (qAlpha(rgb) == 255) {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) == 0xff);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) == 0xff);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) == 0xff);
            } else {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) >= 0xfe);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) >= 0xfe);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) >= 0xfe);
            }
        }
    }

    painter.drawImage(cavas.rect(), image, QRectF(0, 0, image.width() / 2, image.height() / 2), &effects);
    for (int h = 0; h < cavas.height(); h++) {
        for (int w = 0; w < cavas.width(); w++) {
            QRgb rgb = cavas.pixel(w, h);
            if (qAlpha(rgb) == 255) {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) == 0xff);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) == 0xff);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) == 0xff);
            } else {
                QVERIFY(qRed(rgb) == 0 || qRed(rgb) >= 0xfe);
                QVERIFY(qGreen(rgb) == 0 || qGreen(rgb) >= 0xfe);
                QVERIFY(qBlue(rgb) == 0 || qBlue(rgb) >= 0xfe);
            }
        }
    }
}

void tst_QImageEffects::colorKey_data()
{
    data();
}

void tst_QImageEffects::colorKey()
{
    QFETCH(QImage, image);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    const uint clr(0xff123456);
    cavas.fill(clr);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QImageEffects effects;
    QPoint pt(10, 10);

    effects.setColorKey(image.pixel(pt));
    painter.drawImage(cavas.rect(), image, image.rect(), &effects);
    const QRgb rgb = image.pixel(pt);

    for (int h = 0; h < cavas.height(); h++)
        for (int w = 0; w < cavas.width(); w++)
            if (image.pixel(w, h) == rgb)
                QCOMPARE(cavas.pixel(w, h), clr);
}

void tst_QImageEffects::watermark()
{
    QImage image = m_image.convertToFormat(QImage::Format_ARGB32);

    QImage cavas(image.width(), image.height(), QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImageEffects effects;
    effects.setBrightness(0.7);
    effects.setContrast(0.3);

    painter.drawImage(cavas.rect(), image, image.rect(), &effects);

    QCOMPARE(cavas, QImage("res/watermark.png").convertToFormat(QImage::Format_ARGB32));
}

void tst_QImageEffects::recolor_data()
{
    QTest::addColumn<QImage>("image");   

    QImage alphaImg("res/colorful_alpha128.png");
    QTest::newRow("RGB32") << alphaImg.convertToFormat(QImage::Format_RGB32);
    QTest::newRow("ARGB32 alpha=128") << alphaImg.convertToFormat(QImage::Format_ARGB32);
    QTest::newRow("ARGB32_Premultiplied alpha=128") << alphaImg.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

void tst_QImageEffects::recolor()
{
    QFETCH(QImage, image);

    QImage cavas((image.width() + 10) * 7, (image.height() + 10) * 3, QImage::Format_ARGB32);
    cavas.fill(0x0);
    QPainter painter(&cavas);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    recolor(painter, image);

    QCOMPARE(cavas, QImage(QString("res/recolor_format%1.png").arg(image.format())).convertToFormat(QImage::Format_ARGB32));
}

void tst_QImageEffects::recolor(QPainter &painter, QImage &image)
{
    painter.save();

    QImageEffects effect;

    //first row    
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(255, 255, 255);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(245, 228, 208);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone();
    effect.setBrightness(0.7);
    effect.setContrast(0.3);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    effect.setBrightness(0.0);
    effect.setContrast(1.0);
    painter.translate(image.width() + 10, 0);    
    effect.setBilevel(25);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setBilevel(50);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setBilevel(75);
    painter.drawImage(image.rect(), image, image.rect(), &effect);

    //second row
    painter.resetMatrix();
    painter.translate(0, image.height() + 10);
    effect.setDuotone(177, 192, 229);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(160, 198, 255);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(255, 160, 158);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(224, 255, 165);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(205, 182, 242);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(154, 242, 255);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(255, 187, 122);
    painter.drawImage(image.rect(), image, image.rect(), &effect);

    //third row
    painter.resetMatrix();
    painter.translate(0, (image.height() + 10) * 2);
    effect.setDuotone(87, 89, 99, 168, 166, 156);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(217, 169, 110, 38, 86, 145);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(107, 216, 219, 148, 39, 36);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(145, 113, 208, 110, 142, 47);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(169, 196, 136, 86, 59, 119);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(222, 127, 102, 33, 128, 153);
    painter.drawImage(image.rect(), image, image.rect(), &effect);
    painter.translate(image.width() + 10, 0);
    effect.setDuotone(60, 155, 232, 195, 100, 23);
    painter.drawImage(image.rect(), image, image.rect(), &effect);

    painter.restore();
}

void tst_QImageEffects::setColorMatrix_data()
{
    data();
}

void tst_QImageEffects::setColorMatrix()
{
    QFETCH(QImage, image);

    //gray
    {
        QImage cavas1(image.width(), image.height(), QImage::Format_ARGB32);
        {
            QPainter painter1(&cavas1);
            painter1.setCompositionMode(QPainter::CompositionMode_Source);
            QImageEffects effects1;
            effects1.setDuotone(255, 255, 255);
            painter1.drawImage(cavas1.rect(), image, cavas1.rect(), &effects1);
        }

        QImage cavas2(image.width(), image.height(), QImage::Format_ARGB32);
        {
            QPainter painter2(&cavas2);
            painter2.setCompositionMode(QPainter::CompositionMode_Source);
            //row-major order
            qreal values[16] = {
                0.299, 0.587, 0.114, 0,
                0.299, 0.587, 0.114, 0,
                0.299, 0.587, 0.114, 0,
                0,     0,     0, 1
            };
            QImageEffects effects2;
            effects2.setColorMatrix(QMatrix4x4(values));
            painter2.drawImage(cavas2.rect(), image, cavas2.rect(), &effects2);
        }
        QCOMPARE(cavas1, cavas2); 
    }

    //watermark
    {
        QImage cavas1(image.width(), image.height(), QImage::Format_ARGB32);
        {
            QPainter painter1(&cavas1);
            painter1.setCompositionMode(QPainter::CompositionMode_Source);
            QImageEffects effects1;
            effects1.setBrightness(0.7);
            effects1.setContrast(0.3);
            painter1.drawImage(cavas1.rect(), image, cavas1.rect(), &effects1);
        }

        QImage cavas2(image.width(), image.height(), QImage::Format_ARGB32);
        {
            QPainter painter2(&cavas2);
            painter2.setCompositionMode(QPainter::CompositionMode_Source);
            //row-major order
            qreal values[16] = {
                0.3, 0, 0, 0.805,
                0, 0.3, 0, 0.805,
                0, 0, 0.3, 0.805,
                0, 0, 0, 1
            };
            QImageEffects effects2;
            effects2.setColorMatrix(QMatrix4x4(values));
            painter2.drawImage(cavas2.rect(), image, cavas2.rect(), &effects2);
        }
        QCOMPARE(cavas1, cavas2); 
    }
}

void tst_QImageEffects::unsetColorMatrix_data()
{
    data();
}

void tst_QImageEffects::unsetColorMatrix()
{
    QFETCH(QImage, image);

    QImageEffects effects;
    effects.setDuotone(255, 255, 255);
    effects.setBrightness(0.5);
    effects.setContrast(0.7);
    QImage cavas1(image.width(), image.height(), QImage::Format_ARGB32);
    {
        QPainter painter1(&cavas1);
        painter1.setCompositionMode(QPainter::CompositionMode_Source);
        painter1.drawImage(cavas1.rect(), image, cavas1.rect(), &effects);
    }

    //row-major order
    qreal values[16] = {
        0.299, 0.587, 0.114, 0,
        0.299, 0.587, 0.114, 0,
        0.299, 0.587, 0.114, 0,
        0,     0,     0, 1
    };
    effects.setColorMatrix(QMatrix4x4(values));

    QImage cavas2(image.width(), image.height(), QImage::Format_ARGB32);
    {
        QPainter painter2(&cavas2);
        painter2.setCompositionMode(QPainter::CompositionMode_Source);
        effects.unsetColorMatrix();
        painter2.drawImage(cavas2.rect(), image, cavas2.rect(), &effects);
    }
    QCOMPARE(cavas1, cavas2); 
}

QTEST_MAIN(tst_QImageEffects)
#include "tst_qimageeffects.moc"
