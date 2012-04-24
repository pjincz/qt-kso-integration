/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qimage.h"
#include <private/qimage_p.h>
#include <private/qsimd_p.h>
#include <private/qdrawhelper_p.h>
#include <private/qdrawingprimitive_sse2_p.h>
#include <private/qpathgradient_p.h>

#ifdef QT_HAVE_SSE4_1

QT_BEGIN_NAMESPACE

/*!
    \fn inline void QImageEffectsData::transform_sse4(QRgb &rgb) const
    Transform the rgb value use the color matrix.
    Note: The alpha channel is unchanged.
    sa\ transform_cpp(), transform_sse2()
*/
static const __m128i s_lowerbound = _mm_setzero_si128();
void QImageEffectsPrivate::transform_sse4(QRgb &rgb) const
{
    Q_ASSERT(qDetectCPUFeatures() & SSE4_1);

    quint8 *pb = (quint8*)&rgb;
    __m128i rg = _mm_set1_epi32((pb[2] << 16) | pb[1]);
    __m128i ba = _mm_set1_epi32((pb[0] << 16) | pb[3]);

    __m128i mrg = _mm_madd_epi16(rg, ((__m128i*)d.m_mmtxs)[0]);
    __m128i mba = _mm_madd_epi16(ba, ((__m128i*)d.m_mmtxs)[1]);
    __m128i mrgba = _mm_add_epi32(mrg, mba);
    mrgba = _mm_srai_epi32(mrgba, 8);

    if (checkBound) {
        __m128i upperbound = _mm_set1_epi32(pb[3]);
        mrgba = _mm_max_epi32(s_lowerbound, mrgba);
        mrgba = _mm_min_epi32(upperbound, mrgba);
    }

    qint32* cast_rgba = (qint32*)&mrgba;
    pb[2] = cast_rgba[3];
    pb[1] = cast_rgba[2];
    pb[0] = cast_rgba[1];
}

/*!
    \overload
*/
void QImageEffectsPrivate::transform_sse4(QRgb *buffer, int length) const
{
    Q_ASSERT(qDetectCPUFeatures() & SSE4_1);

    const int *p = colorMatrixInt[0];
    __m128i *pSrc = reinterpret_cast<__m128i*>(buffer);
    __m128i *pEnd = pSrc + length / 4;
    const __m128i alphaMask = _mm_set1_epi32(0xff000000);
    const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
    __m128i mmtxr1 = _mm_set1_epi32((p[0] << 16) | p[8]);
    __m128i mmtxr2 = _mm_set1_epi32((p[12] << 16) | p[4]);
    __m128i mmtxg1 = _mm_set1_epi32((p[1] << 16) | p[9]);
    __m128i mmtxg2 = _mm_set1_epi32((p[13] << 16) | p[5]);
    __m128i mmtxb1 = _mm_set1_epi32((p[2] << 16) | p[10]);
    __m128i mmtxb2 = _mm_set1_epi32((p[14] << 16) | p[6]);

    for (; pSrc < pEnd; pSrc++) {
        const __m128i msrc = _mm_loadu_si128(pSrc);
        const __m128i mrb = _mm_and_si128(msrc, colorMask);
        const __m128i mag = _mm_srli_epi16(msrc, 8);
        const __m128i mas = _mm_and_si128(msrc, alphaMask);
        const __m128i upperbound = _mm_srli_epi32(mas, 24);

        //red channel
        const __m128i mr1 = _mm_madd_epi16(mrb, mmtxr1);
        const __m128i mr2 = _mm_madd_epi16(mag, mmtxr2);
        __m128i mrs = _mm_add_epi32(mr1, mr2);
        mrs = _mm_srai_epi32(mrs, 8);
        mrs = _mm_max_epi32(s_lowerbound, mrs);
        mrs = _mm_min_epi32(upperbound, mrs);
        mrs = _mm_slli_epi32(mrs, 16);

        //green channel
        const __m128i mg1 = _mm_madd_epi16(mrb, mmtxg1);
        const __m128i mg2 = _mm_madd_epi16(mag, mmtxg2);
        __m128i mgs = _mm_add_epi32(mg1, mg2);
        mgs = _mm_srai_epi32(mgs, 8);
        mgs = _mm_max_epi32(s_lowerbound, mgs);
        mgs = _mm_min_epi32(upperbound, mgs);
        mgs = _mm_slli_epi32(mgs, 8);

        //blue channel
        const __m128i mb1 = _mm_madd_epi16(mrb, mmtxb1);
        const __m128i mb2 = _mm_madd_epi16(mag, mmtxb2);
        __m128i mbs = _mm_add_epi32(mb1, mb2);
        mbs = _mm_srai_epi32(mbs, 8);
        mbs = _mm_max_epi32(s_lowerbound, mbs);
        mbs = _mm_min_epi32(upperbound, mbs);

        _mm_storeu_si128(pSrc, _mm_or_si128(_mm_or_si128(mas, mrs), _mm_or_si128(mgs, mbs)));
    }

    switch (length % 4) {
    case 3: transform_sse4(buffer[--length]);
    case 2: transform_sse4(buffer[--length]);
    case 1: transform_sse4(buffer[--length]);
    }
}


QT_END_NAMESPACE

#endif // QT_HAVE_SSE4_1

