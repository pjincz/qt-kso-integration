/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QIMAGE_P_H
#define QIMAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#include <QVector>

#ifndef QT_NO_IMAGE_TEXT
#include <QMap>
#endif

#include <private/qsimd_p.h>

QT_BEGIN_NAMESPACE

class QImageWriter;

struct Q_GUI_EXPORT QImageData {        // internal image data
    QImageData();
    ~QImageData();
    static QImageData *create(const QSize &size, QImage::Format format, int numColors = 0);
    static QImageData *create(uchar *data, int w, int h,  int bpl, QImage::Format format, bool readOnly);

    QAtomicInt ref;

    int width;
    int height;
    int depth;
    int nbytes;               // number of bytes data
    QVector<QRgb> colortable;
    uchar *data;
#ifdef QT3_SUPPORT
    uchar **jumptable;
#endif
    QImage::Format format;
    int bytes_per_line;
    int ser_no;               // serial number
    int detach_no;

    qreal  dpmx;                // dots per meter X (or 0)
    qreal  dpmy;                // dots per meter Y (or 0)
    QPoint  offset;           // offset in pixels

    uint own_data : 1;
    uint ro_data : 1;
    uint has_alpha_clut : 1;
    uint is_cached : 1;

    bool checkForAlphaPixels() const;

    // Convert the image in-place, minimizing memory reallocation
    // Return false if the conversion cannot be done in-place.
    bool convertInPlace(QImage::Format newFormat, Qt::ImageConversionFlags);

#ifndef QT_NO_IMAGE_TEXT
    QMap<QString, QString> text;
#endif
    bool doImageIO(const QImage *image, QImageWriter* io, int quality) const;

    QPaintEngine *paintEngine;
};

#ifdef QT_HAVE_SSE2

struct Q_GUI_EXPORT QM128Data {
    void *m_mmtxs;

    QM128Data();

    QM128Data(const QM128Data &rhs);

    ~QM128Data();

    QM128Data& operator=(const QM128Data &rhs);

    void* operator new(size_t size);

    void operator delete(void *p);
};
#endif

typedef QMap<QRgb, QRgb> ColorMap;

struct Q_GUI_EXPORT QImageEffectsPrivate
{
public:
    enum base_scale_e
    {
        base_shift = 8,
        base_scale = 1 << base_shift,
    };

    QImageEffectsPrivate();
    ~QImageEffectsPrivate();

    void updateColorMatrixInt();
    void updateColorMatrix();
    void prepare();
    void transform(QRgb *buffer, int length) const;
    void transform_cpp(QRgb &rgb) const;
    void transform_cpp(QRgb *buffer, int length) const;
    void setTransformFunc();

    QMatrix4x4 createDuotoneMatrix(const QRgb clr1, const QRgb clr2) const;
    void resetState();

    QAtomicInt ref;
    bool hasColorMatirx;
    bool hasColorKey;
    bool hasDuotone;
    bool hasBilevel;

    bool checkBound;

    QMatrix4x4 colorMatrix;
    int colorMatrixInt[4][4];
    QRgb colorKey;
    ColorMap colorMap;
    quint8 tolerance;
    quint8 colorKeyLow[3];
    quint8 colorKeyHight[3];
    quint8 bilevelThreshold;
    QRgb duotoneColor1;
    QRgb duotoneColor2;
    qreal brightness;
    qreal contrast;
    typedef void (QImageEffectsPrivate::*TransformProc)(QRgb *buffer, int length) const;
    TransformProc m_pTransformProc;

#ifdef QT_HAVE_SSE2    
    QM128Data d;
    void transform_sse2(QRgb &rgb) const;
    void transform_sse2(QRgb *buffer, int length) const;
#endif

#ifdef QT_HAVE_SSE4_1
    void transform_sse4(QRgb &rgb) const;
    void transform_sse4(QRgb *buffer, int length) const;
#endif

private:
    friend class QRasterPaintEngine;
};

void qInitImageConversions();

QT_END_NAMESPACE

#endif
