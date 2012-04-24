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

#include <private/qdrawhelper_x86_p.h>
#include <private/qdrawingprimitive_sse2_p.h>
#include <private/qpathgradient_p.h>
#include <private/qsimd_p.h>
#include <emmintrin.h>

#ifdef QT_HAVE_SSE4_1

QT_BEGIN_NAMESPACE

__m128i gradient_sse4(const __m128i &from, const __m128i &to, const __m128i &pos, const int &shift)
{
    Q_ASSERT(qDetectCPUFeatures() & SSE4_1);

    __m128i result = _mm_sub_epi32(to, from);
    result = _mm_mullo_epi32(result, pos);
    result = _mm_srai_epi32(result, shift);
    result = _mm_add_epi32(from, result);

    qint32* cast_m128 = (qint32*)&result;
    Q_ASSERT(0 <= cast_m128[0] && cast_m128[0] <= 255);
    Q_ASSERT(0 <= cast_m128[1] && cast_m128[1] <= 255);
    Q_ASSERT(0 <= cast_m128[2] && cast_m128[2] <= 255);
    Q_ASSERT(0 <= cast_m128[3] && cast_m128[3] <= 255);

    return result;
}

void generate_gradient_sse4(argb8 *buffer, int length,
                            const argb8 &clrFrom, const argb8 &clrTo,
                            const qreal &start, const qreal &dx)
{

    Q_ASSERT(sizeof(argb8) == sizeof(uint));
    Q_ASSERT(qDetectCPUFeatures() & SSE4_1);

    enum {
        base_shift = 16,
        base_scale = 1 << base_shift
    };
    const __m128i from[4] = {
        _mm_set1_epi32(clrFrom.b),
        _mm_set1_epi32(clrFrom.g),
        _mm_set1_epi32(clrFrom.r),
        _mm_set1_epi32(clrFrom.a)
    };
    const __m128i to[4] = {
        _mm_set1_epi32(clrTo.b),
        _mm_set1_epi32(clrTo.g),
        _mm_set1_epi32(clrTo.r),
        _mm_set1_epi32(clrTo.a)
    };
    __m128i *pSrc = (__m128i *)buffer;
    __m128i *pEnd = pSrc + length / 4;
    __m128i result[4];
    for (int i = 0; pSrc < pEnd; pSrc++, i += 4) {
        const int pos[4] = {
            int((start + i * dx) * base_scale + 0.5),
            int((start + (i + 1) * dx) * base_scale + 0.5),
            int((start + (i + 2) * dx) * base_scale + 0.5),
            int((start + (i + 3) * dx) * base_scale + 0.5)
        };
        const __m128i mpos = _mm_set_epi32(pos[3], pos[2], pos[1], pos[0]);
        result[3] = gradient_sse4(from[3], to[3], mpos, base_shift); // alpha
        result[2] = gradient_sse4(from[2], to[2], mpos, base_shift); // red
        result[1] = gradient_sse4(from[1], to[1], mpos, base_shift); // green
        result[0] = gradient_sse4(from[0], to[0], mpos, base_shift); // blue

        result[3] = _mm_or_si128(_mm_slli_epi32(result[3], 24),  _mm_slli_epi32(result[2], 16));
        result[1] = _mm_or_si128(_mm_slli_epi32(result[1], 8),  result[0]);
        _mm_storeu_si128(pSrc,  _mm_or_si128(result[3], result[1]));
    }
    switch (length % 4) {
            case 3: --length; buffer[length] = clrFrom.gradient(clrTo, start + length * dx);
            case 2: --length; buffer[length] = clrFrom.gradient(clrTo, start + length * dx);
            case 1: --length; buffer[length] = clrFrom.gradient(clrTo, start + length * dx);
    }
}

QT_END_NAMESPACE

#endif // QT_HAVE_SSE4_1
