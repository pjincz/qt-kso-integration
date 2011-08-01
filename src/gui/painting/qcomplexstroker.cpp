#include "qcomplexstroker.h"
#include "qcomplexstroker_p.h"

#include <QtCore/qmath.h>
#include <algorithm>

static const qreal intersection_epsilon = 1.0e-30;
static const qreal vertex_dist_epsilon = 1.0e-30;

inline qreal cross_product(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3)
{
    return (x3 - x2) * (y2 - y1) - (y3 - y2) * (x2 - x1);
}

inline qreal cross_product(const QPointF &pt1, const QPointF &pt2, const QPointF &pt3)
{
    return cross_product(pt1.x(), pt1.y(), pt2.x(), pt2.y(), pt3.x(), pt3.y());
}

inline qreal calc_distance(qreal x1, qreal y1, qreal x2, qreal y2)
{
    const qreal dx = x2-x1;
    const qreal dy = y2-y1;
    return qSqrt(dx * dx + dy * dy);
}

inline qreal calc_distance(const QPointF& pt1, const QPointF& pt2)
{
    return calc_distance(pt1.x(), pt1.y(), pt2.x(), pt2.y());
}

inline bool calc_intersection(qreal ax, qreal ay, qreal bx, qreal by,
                              qreal cx, qreal cy, qreal dx, qreal dy,
                              qreal* x, qreal* y)
{
    qreal num = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
    qreal den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);
    if (qFabs(den) < intersection_epsilon) return false;
    qreal r = num / den;
    *x = ax + r * (bx-ax);
    *y = ay + r * (by-ay);
    return true;
}

inline bool calc_intersection(const QPointF &pt1, const QPointF &pt2,
                              const QPointF &pt3, const QPointF &pt4,
                              qreal* x, qreal* y)
{
    return calc_intersection(pt1.x(), pt1.y(), pt2.x(), pt2.y(),
        pt3.x(), pt3.y(), pt4.x(), pt4.y(),
        x, y);
}

inline bool calc_intersection(const QPointF &pt1, const QPointF &pt2,
                              const QPointF &pt3, const QPointF &pt4,
                              QPointF& pt)
{
    return calc_intersection(pt1.x(), pt1.y(), pt2.x(), pt2.y(),
        pt3.x(), pt3.y(), pt4.x(), pt4.y(),
        &(pt.rx()), &(pt.ry()));
}

inline qreal calc_line_point_distance(qreal x1, qreal y1, 
                                      qreal x2, qreal y2, 
                                      qreal x,  qreal y)
{
    qreal dx = x2-x1;
    qreal dy = y2-y1;
    qreal d = sqrt(dx * dx + dy * dy);

    if (d < vertex_dist_epsilon)
    {
        return calc_distance(x1, y1, x, y);
    }
    return ((x - x2) * dy - (y - y2) * dx) / d;
}

inline qreal calc_line_point_distance(QPointF pt1, QPointF pt2, QPointF pt)
{
    return calc_line_point_distance(pt1.rx(), pt1.ry(), pt2.rx(), pt2.ry(), pt.rx(), pt.ry());
}
inline void CalcScalePointOnSegment(const QPointF& pt1, const QPointF& pt2, qreal scale, QPointF& pt)
{
    const qreal& dx = pt2.x() - pt1.x();
    const qreal& dy = pt2.y() - pt1.y();
    pt.rx() = pt1.x() + dx * scale;
    pt.ry() = pt1.y() + dy * scale;
}

inline void CalcSegmentCrossRound(const QPointF& centerPt, qreal radiu, 
                                  const QPointF& pt1, const QPointF& pt2, qreal distc1, qreal distc2, QPointF& pt)
{
    Q_ASSERT_X (distc1 <= radiu && distc2 >= radiu, "", "");
    if (distc1 == 0)
    {
        CalcScalePointOnSegment(pt1, pt2, radiu / distc2, pt);
        return;
    }

    if (qFuzzyCompare(distc2, radiu))
    {
        pt = pt2;
        return;
    }

    const qreal& distc3 = calc_line_point_distance(pt1, pt2, centerPt);
    if (qFuzzyIsNull(distc3))
    {
        CalcScalePointOnSegment(centerPt, pt2, radiu / distc2, pt);
        return;
    }

    const qreal& dist12 = calc_distance(pt1, pt2);
    const qreal& dist32 = qSqrt(distc2 * distc2 - distc3 * distc3);
    const qreal& dist34 = qSqrt(radiu * radiu - distc3 * distc3);
    const qreal& dist42 = dist32 - dist34;

    CalcScalePointOnSegment(pt2, pt1, dist42 / dist12, pt);
    return;
}

inline bool IsPointsOnLine(const QPointF *pts, unsigned n)
{
    for (unsigned i = 0; i < n - 2; ++i)
    {
        if (cross_product(pts[i], pts[i+1], pts[i+2]) != 0)
        {
            return false;
        }
    }
    return true;
}

inline bool IsPolygonClockWise(const QPointF *pts, unsigned n)
{
    Q_ASSERT_X(n > 2, "", "");
    Q_ASSERT_X(false == IsPointsOnLine(pts, n), "", "");
    qreal maxx = pts[0].x();
    unsigned maxi = 0;
    for (unsigned i = 1; i < n; ++i)
    {	
        if (pts[i].x() > maxx)
        {
            maxx = pts[i].x();
            maxi = i;
        }
    }

    unsigned prev, next, convex = maxi;
    prev = maxi - 1;
    next = maxi + 1;
    if (0 == convex)
    {
        prev = n - 1;
    }
    if (n - 1 == convex)
    {
        next = 0;
    }

    qreal product = cross_product(pts[prev], pts[convex], pts[next]);
    Q_ASSERT_X(product != 0, "", "");
    return product < 0;
}

struct vertex_dist
{
    QPointF pt;
    qreal dist;

    vertex_dist(const QPointF& p)
        : pt(p), dist(0) 
    {}

    vertex_dist(const QPointF& p, const QPointF& n)
        : pt(p), dist(calc_distance(p, n))
    {}
};

class QVertices
{		
public:
    QVertices(const QPolygonF &subPath);
    int size() const;
    bool isclosed() const;
    void reset(const QPolygonF &polygon);

    const vertex_dist& first() const;
    const vertex_dist& last() const;	
    const vertex_dist& operator[](int index) const;

private:
    std::vector<vertex_dist> m_pVertices;
    bool m_bClosed;
};

QVertices::QVertices(const QPolygonF &subPath)
{
    reset(subPath);
}

int QVertices::size() const
{
    return m_pVertices.size();
}

bool QVertices::isclosed() const
{
    return m_bClosed;
}

void QVertices::reset(const QPolygonF &polygon)
{
    // 可以调节。
    const qreal& zeroDist = 0.000001f;

    m_pVertices.clear();
    m_pVertices.reserve(polygon.size());
    for (int i = 0; i < polygon.size() - 1; ++i) {
        vertex_dist v(polygon[i], polygon[i+1]);
        if (zeroDist < v.dist)
            m_pVertices.push_back(v);
    }

    vertex_dist v(polygon.back(), polygon[0]);
    if (!polygon.isClosed())
        m_pVertices.push_back(v);
    else if (zeroDist < v.dist)
        m_pVertices.push_back(v);
    m_bClosed = polygon.isClosed();
}

const vertex_dist& QVertices::first() const
{
    return m_pVertices.front();
}

const vertex_dist& QVertices::last() const
{
    return m_pVertices.back();
}

const vertex_dist& QVertices::operator[](int index) const
{
    return m_pVertices[index];
}

QMathStroker::QMathStroker()
: m_width(1)
, m_lineJoin(Qt::MiterJoin)
, m_miterLimit(10)
, m_startCap(Qt::FlatCap)
, m_endCap(Qt::FlatCap)
, m_compoundArray()
, m_penAlignment(Qt::PenAlignmentCenter)
, m_scaleX(1)
, m_scaleY(1)
{
}

qreal QMathStroker::GetWidth() const
{
    return m_width;
}

void QMathStroker::SetWidth(qreal width)
{ 	
    m_width = width <= 0 ? 1 : width;
}

Qt::PenJoinStyle QMathStroker::GetLineJoin() const
{
    return m_lineJoin;
}

void QMathStroker::SetLineJoin(Qt::PenJoinStyle linejoin)
{
    m_lineJoin = linejoin;
}

qreal QMathStroker::GetMiterLimit() const
{
    return m_miterLimit;
}

void QMathStroker::SetMiterLimit(qreal miterlimit)
{
    m_miterLimit = miterlimit;
}

Qt::PenCapStyle QMathStroker::GetStartCap() const
{
    return m_startCap;
}

Qt::PenCapStyle QMathStroker::GetEndCap() const
{
    return m_endCap;
}

void QMathStroker::SetStartCap(Qt::PenCapStyle startcap)
{
    if (startcap == Qt::FlatCap || startcap == Qt::SquareCap || startcap == Qt::RoundCap || Qt::TriangleCap == startcap)
        m_startCap = startcap;
    else
        m_startCap = Qt::FlatCap;

}

void QMathStroker::SetEndCap(Qt::PenCapStyle endcap)
{
    if (endcap == Qt::FlatCap || endcap == Qt::SquareCap || endcap == Qt::RoundCap || Qt::TriangleCap == endcap)
        m_endCap = endcap;
    else
        m_endCap = Qt::FlatCap;
}

void QMathStroker::SetLineCap(Qt::PenCapStyle startCap,
                              Qt::PenCapStyle endCap)
{
    SetStartCap(startCap);
    SetEndCap(endCap);
}	


int QMathStroker::GetCompoundArrayCount() const 
{ 
    return m_compoundArray.size();
}

bool QMathStroker::GetCompoundArray(qreal *array, int count) const
{
    if (NULL == array)
    {
        return false;
    }

    Q_ASSERT(count <= m_compoundArray.size());

    for (int i = 0; i < count; ++i)
    {
        array[i] = m_compoundArray[i];
    }
    return true;
}

bool QMathStroker::SetCompoundArray(const qreal *compoundArray, int count) 
{
    if (NULL == compoundArray)
    {
        return false;
    }

    if (count & 1)
    {
        return false;
    }

    int i = 0;
    while (i < count - 1)
    {
        if (compoundArray[i] < 0 ||
            compoundArray[i] > 1 ||
            compoundArray[i] >= compoundArray[i+1])
        {
            return false;
        }

        ++i;
    }

    if (compoundArray[i] < 0 || compoundArray[i] > 1)
    {
        return false;
    }

    m_compoundArray.clear();
    for (int j = 0; j < count; ++j)
    {
        m_compoundArray.push_back(compoundArray[j]);
    }
    return true;
}


Qt::PenAlignment QMathStroker::GetAlignment() const
{
    return m_penAlignment;
}

void QMathStroker::SetAlignment(Qt::PenAlignment penAlignment)
{
    m_penAlignment = penAlignment;
}

void QMathStroker::GetScale(qreal *sx, qreal *sy) const
{
    if (sx)
    {
        *sx = m_scaleX;
    }
    if (sy)
    {
        *sy = m_scaleY;
    }
}

void QMathStroker::SetScale(qreal sx, qreal sy)
{
    m_scaleX = sx;
    m_scaleY = sy;
}


void QMathStroker::StrokePath(const QList<QPolygonF>& path2stroke, QPainterPath& outPath) const
{
    for (int i = 0; i < path2stroke.size(); ++i)
        StrokeSubPath(path2stroke[i], outPath);
}

void QMathStroker::StrokePath(const QPainterPath& path2stroke, QPainterPath& outPath) const
{	
    QList<QPolygonF> polygons = path2stroke.toSubpathPolygons();
    for (int i = 0; i < polygons.size(); ++i)
        StrokeSubPath(polygons[i], outPath);
}


// ------------------------------------------------------------------------p
void QMathStroker::StrokeSubPath(const QPolygonF &subPath,
                                 QPainterPath& outPath) const
{
    if (subPath.isClosed())
    {
        StrokeCloseSubPath(subPath, outPath);
    }
    else
    {
        StrokeOpenSubPath(subPath, outPath);
    }
}

void QMathStroker::StrokeCloseSubPath(const QPolygonF &subPath, QPainterPath &outPath) const
{
    QVertices vertices(subPath);
    const int& s = vertices.size();
    Q_ASSERT(s >= 2);

    QVector<QPointF> firstpts, secondpts;
    QVector<QVector<QPointF> > compoundpts(m_compoundArray.size());

    for (int i = 0; i < s - 2; ++i)
    {
        CalcPoints(vertices, i, i+1, i+2, firstpts, secondpts, compoundpts);			
    }
    CalcPoints(vertices, s - 2, s - 1, 0, firstpts, secondpts, compoundpts);
    CalcPoints(vertices, s - 1, 0, 1, firstpts, secondpts, compoundpts);

    if (m_compoundArray.empty())
    {
        outPath.addPolygon(firstpts);
        outPath.closeSubpath();
        std::reverse(secondpts.begin(), secondpts.end());
        outPath.addPolygon(secondpts);
        outPath.closeSubpath();
    }
    else
    {
        for (int i = 0; i < m_compoundArray.size(); i += 2)
        {
            outPath.addPolygon(compoundpts[i]);
            outPath.closeSubpath();
            std::reverse(compoundpts[i+1].begin(), compoundpts[i+1].end());
            outPath.addPolygon(compoundpts[i+1]);
            outPath.closeSubpath();
        }
    }
}

void QMathStroker::StrokeOpenSubPath(const QPolygonF &subPath, QPainterPath &outPath) const
{	
    QVertices vertices(subPath);
    const int& s = vertices.size();
    Q_ASSERT(s >= 2);

    QVector<QPointF> firstpts, secondpts;
    QVector<QVector<QPointF> > compoundpts(m_compoundArray.size());
    QVector<QPointF> startCapPts, endCapPts;

    CalcStartPoints(vertices, firstpts, secondpts, compoundpts, startCapPts);
    for (int i = 0; i < s - 2; ++i)
    {
        CalcPoints(vertices, i, i+1, i+2, firstpts, secondpts, compoundpts);
    }
    CalcEndPoints(vertices, firstpts, secondpts, compoundpts, endCapPts);


    if (m_compoundArray.empty())
    {
        outPath.addPolygon(firstpts);
        outPath.connectPolygon(endCapPts);
        std::reverse(secondpts.begin(), secondpts.end());
        outPath.connectPolygon(secondpts);
        outPath.connectPolygon(startCapPts);
        outPath.closeSubpath();	
    }
    else
    {
        for (int i = 0; i < m_compoundArray.size(); i += 2)
        {
            outPath.addPolygon(compoundpts[i]);
            std::reverse(compoundpts[i+1].begin(), compoundpts[i+1].end());
            outPath.connectPolygon(compoundpts[i+1]);
            outPath.closeSubpath();
        }
        outPath.addPolygon(startCapPts);
        outPath.closeSubpath();
        outPath.addPolygon(endCapPts);
        outPath.closeSubpath();
    }
}

void QMathStroker::CalcPoints(const QVertices& vertices, int first, int second,
                              int third, QVector<QPointF>& firstPts, 
                              QVector<QPointF>& secondPts,
                              QVector<QVector<QPointF> >& compoundPts) const
{
    QVector<QPointF> firstptstemp;
    QVector<QPointF> secondptstemp;

    CalcJoin(firstptstemp, secondptstemp, vertices[first].pt, vertices[second].pt,
        vertices[third].pt, vertices[first].dist, vertices[second].dist);
    ScalePoints(firstptstemp, vertices[second].pt);
    ScalePoints(secondptstemp, vertices[second].pt);
    firstPts << firstptstemp;
    secondPts << secondptstemp;

    if (!compoundPts.empty())
    {
        Q_ASSERT(compoundPts.size() == m_compoundArray.size());
        QVector<QVector<QPointF> > compoundptstemp(m_compoundArray.size());
        CalcCompoundPoints(firstptstemp, secondptstemp, compoundptstemp);
        for (int i = 0; i < m_compoundArray.size(); ++i)
        {
            compoundPts[i] << compoundptstemp[i];
        }
    }
}

void QMathStroker::CalcStartPoints(const QVertices& vertices, 
                                   QVector<QPointF>& firstPts, 
                                   QVector<QPointF>& secondPts,
                                   QVector<QVector<QPointF> > & compoundPts,
                                   QVector<QPointF>& startCapPts) const
{
    QPointF startFirstPt, startSecondPt;	
    CalcStartPoints(vertices[0].pt, vertices[1].pt, vertices[0].dist, startFirstPt, startSecondPt);

    QVector<QPointF> firstPtsTemp, secondPtsTemp;	
    firstPtsTemp.push_back(startFirstPt);
    secondPtsTemp.push_back(startSecondPt);
    ScalePoints(firstPtsTemp, vertices[0].pt);
    ScalePoints(secondPtsTemp, vertices[0].pt);

    firstPts << firstPtsTemp;
    secondPts << secondPtsTemp;

    if (!compoundPts.empty())
    {
        Q_ASSERT(compoundPts.size() == m_compoundArray.size());
        QVector<QVector<QPointF> >compoundPtsTemp(m_compoundArray.size());
        CalcCompoundPoints(firstPtsTemp, secondPtsTemp, compoundPtsTemp);
        for (int i = 0; i < m_compoundArray.size(); ++i)
        {
            compoundPts[i] << compoundPtsTemp[i];
        }
    }

    QVector<QPointF> startCapPtsTemp;
    CalcCap(vertices[1].pt, vertices[0].pt, vertices[0].dist, m_startCap, startCapPtsTemp);
    ScalePoints(startCapPtsTemp, vertices[0].pt);
    startCapPts << startCapPtsTemp;
}

void QMathStroker::CalcEndPoints(const QVertices& vertices,
                                 QVector<QPointF>& firstPts, 
                                 QVector<QPointF>& secondPts,
                                 QVector<QVector<QPointF> > & compoundPts,
                                 QVector<QPointF>& endCapPts) const
{
    QPointF endFirstPt, endSecondPt;	
    const int& s = vertices.size();
    CalcEndPoints(vertices[s-2].pt, vertices[s-1].pt, vertices[s-2].dist, endFirstPt, endSecondPt);

    QVector<QPointF> firstPtsTemp, secondPtsTemp;
    firstPtsTemp.push_back(endFirstPt);
    secondPtsTemp.push_back(endSecondPt);
    ScalePoints(firstPtsTemp, vertices[s-1].pt);
    ScalePoints(secondPtsTemp, vertices[s-1].pt);

    firstPts << firstPtsTemp;
    secondPts << secondPtsTemp;

    if (!compoundPts.empty())
    {
        Q_ASSERT(compoundPts.size() == m_compoundArray.size());
        QVector<QVector<QPointF> >compoundPtsTemp(m_compoundArray.size());
        CalcCompoundPoints(firstPtsTemp, secondPtsTemp, compoundPtsTemp);
        for (int i = 0; i < m_compoundArray.size(); ++i)
        {
            compoundPts[i] << compoundPtsTemp[i];
        }
    }

    QVector<QPointF> endCapPtsTemp;
    CalcCap(vertices[s-2].pt, vertices[s-1].pt, vertices[s-2].dist, m_endCap, endCapPtsTemp);
    ScalePoints(endCapPtsTemp, vertices[s-1].pt);
    endCapPts << endCapPtsTemp;
}

void QMathStroker::CalcStartPoints(const QPointF& p0, const QPointF& p1, qreal len01,
                                   QPointF& firstPt, QPointF& secondPt) const
{
    const qreal& temp = m_width / 2 / len01;
    const qreal& dx = (p1.y() - p0.y()) * temp;
    const qreal& dy = (p1.x() - p0.x()) * temp;
    firstPt.rx() = p0.x() + dx;
    firstPt.ry() = p0.y() - dy;
    secondPt.rx() = p0.x() - dx;
    secondPt.ry() = p0.y() + dy;
}

void QMathStroker::CalcEndPoints(const QPointF& p0, const QPointF& p1, qreal len01,
                                 QPointF& firstPt, QPointF& secondPt) const
{
    const qreal& temp = m_width / 2 / len01;
    const qreal& dx = (p1.y() - p0.y()) * temp;
    const qreal& dy = (p1.x() - p0.x()) * temp;
    firstPt.rx() = p1.x() + dx;
    firstPt.ry() = p1.y() - dy;
    secondPt.rx() = p1.x() - dx;
    secondPt.ry() = p1.y() + dy;
}

void QMathStroker::CalcCompoundPoints(const QVector<QPointF>& firstpts,
                                      const QVector<QPointF>& secondpts, 
                                      QVector<QVector<QPointF> >& compoundpts) const
{
    if (m_compoundArray.empty())
    {
        return;
    }

    const int& fs = firstpts.size();
    const int& ss = secondpts.size();
    if (fs > 1 && ss > 1)
    {
    }
    else if (fs > 1 && ss == 1)
    {
        for (int i = 0; i < fs; ++i)
        {
            const QPointF& pt0 = firstpts[i];
            const QPointF& pt1 = secondpts[0];
            const qreal& dx = pt1.x() - pt0.x();
            const qreal& dy = pt1.y() - pt0.y();
            const int& cs = m_compoundArray.size();
            for (int j = 0; j < cs; ++j)
            {
                const qreal& scale = m_compoundArray[j];
                compoundpts[j].push_back(QPointF(pt0.x() + scale * dx, pt0.y() + scale * dy));
            }
        }
    }
    else if (ss > 1 && fs == 1)
    {
        for (int i = 0; i < ss; ++i)
        {
            const QPointF& pt0 = firstpts[0];
            const QPointF& pt1 = secondpts[i];
            const qreal& dx = pt1.x() - pt0.x();
            const qreal& dy = pt1.y() - pt0.y();
            const int& cs = m_compoundArray.size();
            for (int j = 0; j < cs; ++j)
            {
                const qreal& scale = m_compoundArray[j];
                compoundpts[j].push_back(QPointF(pt0.x() + scale * dx, pt0.y() + scale * dy));
            }
        }
    }
    else if (fs == 1 && ss == 1)
    {
        const QPointF& pt0 = firstpts[0];
        const QPointF& pt1 = secondpts[0];
        const qreal& dx = pt1.x() - pt0.x();
        const qreal& dy = pt1.y() - pt0.y();
        const int& cs = m_compoundArray.size();
        for (int j = 0; j < cs; ++j)
        {
            const qreal& scale = m_compoundArray[j];
            compoundpts[j].push_back(QPointF(pt0.x() + scale * dx, pt0.y() + scale * dy));
        }
    }
    else
    {
        Q_ASSERT(0);
    }
}

void QMathStroker::ScalePoints(QVector<QPointF>& pts, const QPointF& centerPt) const
{
    if (m_scaleX != 1 && m_scaleY != 1)
    {
        // 		Matrix mtx;
        // 		mtx.Translate(centerPt.X, centerPt.Y);
        // 		mtx.Scale(m_scaleX, m_scaleY);
        // 		mtx.Translate(-centerPt.X, -centerPt.Y);
        // 		mtx.TransformPoints(&pts[0], pts.size());
        for (int i = 0; i < pts.size(); ++i)
        {
            pts[i].rx() = centerPt.x() + (pts[i].x() - centerPt.x()) * m_scaleX;
            pts[i].ry() = centerPt.y() + (pts[i].y() - centerPt.y()) * m_scaleY;
        }
    }
}

void QMathStroker::CalcCap(const QPointF& p0,
                           const QPointF& p1, 
                           qreal len01, 
                           Qt::PenCapStyle lineCap, 
                           QVector<QPointF>& capPts) const
{
    const qreal& twidth = m_width / 2;
    const qreal& tempd1 = twidth / len01;
    const qreal& dx1 = tempd1 * (p1.y() - p0.y());
    const qreal& dy1 = tempd1 * (p1.x() - p0.x());

    switch (lineCap)
    {
    case Qt::FlatCap:
        CalcCapFlat(p0, p1, dx1, dy1, capPts);
        break;

    case Qt::SquareCap:
        CalcCapSquare(p0, p1, dx1, dy1, capPts);
        break;

    case Qt::TriangleCap:
        CalcCapTriangle(p0, p1, dx1, dy1, capPts);
        break;

    case Qt::RoundCap:
        CalcCapRound(p0, p1, dx1, dy1, capPts);
        break;

    default:
        Q_ASSERT_X(0, "", "Is this a simple stroke?");
        break;
    }
}

void QMathStroker::CalcCapFlat(const QPointF& p0, 
                               const QPointF& p1,
                               qreal dx, qreal dy,
                               QVector<QPointF>& capPts) const
{
    Q_UNUSED(p0);
    capPts.push_back(QPointF(p1.x() + dx, p1.y() - dy));
    capPts.push_back(QPointF(p1.x() - dx, p1.y() + dy));
}

void QMathStroker::CalcCapSquare(const QPointF& p0,
                                 const QPointF& p1, 
                                 qreal dx, qreal dy, 
                                 QVector<QPointF>& capPts) const
{
    Q_UNUSED(p0);
    capPts.push_back(QPointF(p1.x() + dx, p1.y() - dy));
    capPts.push_back(QPointF(p1.x() + dx + dy, p1.y() - dy + dx));
    capPts.push_back(QPointF(p1.x() - dx + dy, p1.y() + dy + dx));
    capPts.push_back(QPointF(p1.x() - dx, p1.y() + dy));
}

void QMathStroker::CalcCapTriangle(const QPointF& p0, 
                                   const QPointF& p1, 
                                   qreal dx, qreal dy, 
                                   QVector<QPointF>& capPts) const
{
    Q_UNUSED(p0);
    capPts.push_back(QPointF(p1.x() + dx, p1.y() - dy));
    capPts.push_back(QPointF(p1.x() + dy, p1.y() + dx));
    capPts.push_back(QPointF(p1.x() - dx, p1.y() + dy));
}

void QMathStroker::CalcCapRound(const QPointF& p0, 
                                const QPointF& p1, 
                                qreal dx, qreal dy, 
                                QVector<QPointF>& capPts) const
{
    Q_UNUSED(p0);
    capPts.push_back(QPointF(p1.x() + dx, p1.y() - dy));

    const qreal& calcWidth = m_width / 2;
    const qreal& approxscale = 1;
    qreal da = qAcos(calcWidth / (calcWidth + 0.125f / approxscale)) * 2;
    const int n = (int)(M_PI / da);				
    da = M_PI / (n + 1);
    qreal a1 = M_PI + qAtan2(dy, dx);
    a1 -= da;

    for (int i = 0; i < n; ++i)
    {
        const qreal& costemp = qCos(a1) * calcWidth;
        const qreal& sintemp = qSin(a1) * calcWidth;
        capPts.push_back(QPointF(p1.x() - costemp, p1.y() + sintemp));
        a1 -= da;
    }

    capPts.push_back(QPointF(p1.x() - dx, p1.y() + dy));
}

void QMathStroker::CalcJoin(QVector<QPointF>& firstpts,
                            QVector<QPointF>& secondpts,
                            const QPointF& p0,
                            const QPointF& p1, 
                            const QPointF& p2, 
                            qreal len01, 
                            qreal len12) const
{
    const qreal& cp = cross_product(p0, p1, p2);
    const qreal& twidth = m_width / 2;
    const qreal& tempd1 = twidth / len01;
    const qreal& tempd2 = twidth / len12;
    const qreal& dx1 = tempd1 * (p1.y() - p0.y());
    const qreal& dy1 = tempd1 * (p1.x() - p0.x());
    const qreal& dx2 = tempd2 * (p2.y() - p1.y());
    const qreal& dy2 = tempd2 * (p2.x() - p1.x());

    if (cp > 0)
    {
        CalcInnerJoin(firstpts, p0, p1, p2, dx1, dy1, dx2, dy2, len01, len12);
        CalcOuterJoin(secondpts, p2, p1, p0, -dx2, -dy2, -dx1, -dy1);
    }
    else if (cp < 0)
    {
        CalcOuterJoin(firstpts, p0, p1, p2, dx1, dy1, dx2, dy2);
        CalcInnerJoin(secondpts, p2, p1, p0, -dx2, -dy2, -dx1, -dy1, len12, len01);
    }
    else
    {
        CalcStraightJoin(firstpts, secondpts, p0, p1, p2, dx1, dy1, dx2, dy2);
    }
    std::reverse(secondpts.begin(), secondpts.end());
}

void QMathStroker::CalcInnerJoin(QVector<QPointF>& pts,
                                 const QPointF& p0, 
                                 const QPointF& p1,
                                 const QPointF& p2, 
                                 qreal dx1, qreal dy1,
                                 qreal dx2, qreal dy2,
                                 qreal len01, qreal len12) const
{
    if (m_compoundArray.empty())
    {
        pts.push_back(QPointF(p1.x() + dx1, p1.y() - dy1));
        pts.push_back(QPointF(p1.x() + dx2, p1.y() - dy2));
    }
    else
    {
        const QPointF p010(p0.x() + dx1, p0.y() - dy1);
        const QPointF p011(p1.x() + dx1, p1.y() - dy1);
        const QPointF p121(p1.x() + dx2, p1.y() - dy2);
        const QPointF p122(p2.x() + dx2, p2.y() - dy2);
        bool biOut = cross_product(p010, p1, p122) < 0 ? TRUE : FALSE;
        if (biOut)
        {
            if (len01 > len12)
            {
                pts.push_back(p122);
            }
            else
            {
                pts.push_back(p010);
            }
        }
        else
        {
            QPointF pt;
            bool bOk = calc_intersection(p010, p011, p121, p122, pt);
            Q_ASSERT(bOk);
            Q_UNUSED(bOk);
            pts.push_back(pt);
        }
    }
}

void QMathStroker::CalcOuterJoin(QVector<QPointF>& pts,
                                 const QPointF& p0, 
                                 const QPointF& p1,
                                 const QPointF& p2, 
                                 qreal dx1, qreal dy1,
                                 qreal dx2, qreal dy2) const
{
    switch (m_lineJoin)
    {
    case Qt::MiterJoin:
        CalcLineJoinMiter(pts, p0, p1, p2, dx1, dy1, dx2, dy2);
        break;

    case Qt::SvgMiterJoin:
        CalcLineJoinMiterClipped(pts, p0, p1, p2, dx1, dy1, dx2, dy2);
        break;

    case Qt::BevelJoin:
        CalcLineJoinBevel(pts, p0, p1, p2, dx1, dy1, dx2, dy2);
        break;

    case Qt::RoundJoin:
        CalcLineJoinRound(pts, p0, p1, p2, dx1, dy1, dx2, dy2);
        break;

    default:
        Q_ASSERT(0);
        break;
    }
}

void QMathStroker::CalcStraightJoin(QVector<QPointF>& firstpts,
                                    QVector<QPointF>& secondpts,
                                    const QPointF& p0, 
                                    const QPointF& p1,
                                    const QPointF& p2, 
                                    qreal dx1, qreal dy1,
                                    qreal dx2, qreal dy2) const
{
    const QPointF pt011(p1.x() + dx1, p1.y() - dy1);
    const bool& b1 = cross_product(p0, pt011, p1) > 0 ? TRUE : FALSE;
    const bool& b2 = cross_product(p2, pt011, p1) > 0 ? TRUE : FALSE;
    if (b1 == b2)
    {
        // 同侧
        const QPointF pt120(p1.x() + dx2, p1.y() - dy2);
        firstpts.push_back(pt011);
        firstpts.push_back(pt120);

        secondpts.push_back(pt011);
        secondpts.push_back(pt120);
    }
    else
    {
        // 直线
        firstpts.push_back(pt011);
        secondpts.push_back(QPointF(p1.x() - dx1, p1.y() + dy1));
    }	
}

void QMathStroker::CalcLineJoinMiter(QVector<QPointF>& pts,
                                     const QPointF& p0, 
                                     const QPointF& p1, 
                                     const QPointF& p2, 
                                     qreal dx1, qreal dy1, 
                                     qreal dx2, qreal dy2) const
{
    const QPointF pt010(p0.x() + dx1, p0.y() - dy1);
    const QPointF pt011(p1.x() + dx1, p1.y() - dy1);
    const QPointF pt120(p1.x() + dx2, p1.y() - dy2);
    const QPointF pt121(p2.x() + dx2, p2.y() - dy2);
    QPointF outPt;	
    calc_intersection(pt010, pt011, pt120, pt121, outPt);
    const qreal& dist1out = calc_distance(p1, outPt);
    const qreal& limdist = m_width / 2 * m_miterLimit;
    const bool& bExceedLim = (dist1out > limdist) ? TRUE : FALSE;

    if (bExceedLim)
    {
        QPointF bevelpt;
        calc_intersection(pt011, pt120, p1, outPt, bevelpt);
        const qreal& dist1bevel = calc_distance(p1, bevelpt);
        if (dist1bevel > limdist)
        {
            // CalcBevel
            pts.push_back(pt011);
            pts.push_back(pt120);
        }
        else
        {
            const qreal& limbevel = (dist1out - limdist) / 
                calc_distance(outPt, bevelpt) * 
                calc_distance(pt011, bevelpt);
            const qreal& scale = limdist / dist1out;
            const QPointF limpt(p1.x() + (outPt.x() - p1.x()) * scale, 
                p1.y() + (outPt.y() - p1.y()) * scale);

            const qreal& temp = limbevel / calc_distance(p1, limpt);
            const qreal& dx = temp * (limpt.y() - p1.y());
            const qreal& dy = temp * (limpt.x() - p1.x());

            pts.push_back(QPointF(limpt.x() + dx, limpt.y() - dy));
            pts.push_back(QPointF(limpt.x() - dx, limpt.y() + dy));
        }
    }
    else
    {
        pts.push_back(outPt);
    }
}

void QMathStroker::CalcLineJoinMiterClipped(QVector<QPointF>& pts,
                                            const QPointF& p0, 
                                            const QPointF& p1, 
                                            const QPointF& p2, 
                                            qreal dx1, qreal dy1, 
                                            qreal dx2, qreal dy2) const
{
    const QPointF pt010(p0.x() + dx1, p0.y() - dy1);
    const QPointF pt011(p1.x() + dx1, p1.y() - dy1);
    const QPointF pt120(p1.x() + dx2, p1.y() - dy2);
    const QPointF pt121(p2.x() + dx2, p2.y() - dy2);
    QPointF outPt;	
    calc_intersection(pt010, pt011, pt120, pt121, outPt);
    const qreal& dist1out = calc_distance(p1, outPt);
    const qreal& limdist = m_width / 2 * m_miterLimit;
    const bool& bExceedLim = (dist1out >= limdist) ? TRUE : FALSE;

    if (bExceedLim)
    {
        // CalcBevel
        pts.push_back(pt011);
        pts.push_back(pt120);
    }
    else
    {
        pts.push_back(outPt);
    }
}

void QMathStroker::CalcLineJoinBevel(QVector<QPointF>& pts, 
                                     const QPointF& p0, 
                                     const QPointF& p1, 
                                     const QPointF& p2, 
                                     qreal dx1, qreal dy1, 
                                     qreal dx2, qreal dy2) const
{
    Q_UNUSED(p0);
    Q_UNUSED(p2);
    const QPointF pt011(p1.x() + dx1, p1.y() - dy1);
    const QPointF pt120(p1.x() + dx2, p1.y() - dy2);
    pts.push_back(pt011);
    pts.push_back(pt120);
}

void QMathStroker::CalcLineJoinRound(QVector<QPointF>& pts, 
                                     const QPointF& p0, 
                                     const QPointF& p1, 
                                     const QPointF& p2, 
                                     qreal dx1, qreal dy1, 
                                     qreal dx2, qreal dy2) const
{	
    Q_UNUSED(p0);
    Q_UNUSED(p2);
    pts.push_back(QPointF(p1.x() + dx1, p1.y() - dy1));

    qreal a1 = qAtan2(-dy1, dx1);
    qreal a2 = qAtan2(-dy2, dx2);
    const qreal& approxscale = 1;
    const qreal& calcWidth = m_width / 2;
    qreal da = qAcos(calcWidth / (calcWidth + 0.125f / approxscale)) * 2;

    if (a1 > a2) 
    {
        a2 += 2 * M_PI;
    }	
    const int& n = (int)((a2 - a1) / da);
    da = (a2 - a1) / (n + 1);
    a1 += da;

    for (int i = 0; i < n; ++i)
    {
        QPointF pt;
        pt.rx() = p1.x() + cos(a1) * calcWidth;
        pt.ry() = p1.y() + sin(a1) * calcWidth;
        pts.push_back(pt);
        a1 += da;
    }
    pts.push_back(QPointF(p1.x() + dx2, p1.y() - dy2));
}

QSimplePolygon::QSimplePolygon(const QPolygonF & polygon)
: m_bClosed(false)
, m_bClockWise(false)
{
    m_pts = polygon;

    if (EraseStraightPointsOnPolygon(m_pts))
    {
        m_bClosed = polygon.isClosed();
        m_bClockWise = IsPolygonClockWise(&m_pts[0], m_pts.size());
    }
    else
    {
        m_pts.clear();
    }
}

bool QSimplePolygon::IsValid() const
{
    return !m_pts.empty();
}

bool QSimplePolygon::IsClosed() const
{
    Q_ASSERT_X(IsValid(), "", "");
    return m_bClosed;
}

bool QSimplePolygon::IsClockWise() const
{
    Q_ASSERT_X(IsValid(), "", "");
    return m_bClockWise;
}

unsigned QSimplePolygon::GetPointsCount() const
{
    Q_ASSERT_X(IsValid(), "", "");
    return m_pts.size();
}

const QVector<QPointF> &QSimplePolygon::GetPoints() const
{	
    return m_pts;
}

bool QSimplePolygon::GetZoomPoint(QPointF *pts, unsigned index, qreal offset) const
{
    if (NULL == pts)
    {
        return false;
    }

    Q_ASSERT_X (IsValid(), "", "");
    Q_ASSERT_X (index < unsigned(m_pts.size()), "", "");

    const unsigned& s = m_pts.size();
    if (0 == index)
    {
        if (m_bClosed)
        {
            QPointF pt010, pt011, pt120, pt121;
            GetEdgeOffsetPts(m_pts.back(), m_pts.front(), offset, pt010, pt011);
            GetEdgeOffsetPts(m_pts.front(), m_pts[1], offset, pt120, pt121);
            bool bOk = calc_intersection(pt010, pt011, pt120, pt121, *pts);
            Q_ASSERT_X (bOk, "", "");
            Q_UNUSED(bOk);
        }
        else
        {
            QPointF pt121;
            GetEdgeOffsetPts(m_pts.front(), m_pts[1], offset, *pts, pt121);
        }
    }
    else if (s - 1 == index)
    {
        if (m_bClosed)
        {
            QPointF pt010, pt011, pt120, pt121;
            GetEdgeOffsetPts(m_pts[s-2], m_pts[s-1], offset, pt010, pt011);
            GetEdgeOffsetPts(m_pts[s-1], m_pts[0], offset, pt120, pt121);
            bool bOk = calc_intersection(pt010, pt011, pt120, pt121, *pts);
            Q_ASSERT_X (bOk, "", "");
            Q_UNUSED(bOk);
        }
        else
        {
            QPointF pt010;
            GetEdgeOffsetPts(m_pts[s-2], m_pts[s-1], offset, pt010, *pts);
        }
    }
    else
    {
        QPointF pt010, pt011, pt120, pt121;
        GetEdgeOffsetPts(m_pts[index-1], m_pts[index], offset, pt010, pt011);
        GetEdgeOffsetPts(m_pts[index], m_pts[index+1], offset, pt120, pt121);
        bool bOk = calc_intersection(pt010, pt011, pt120, pt121, *pts);
        Q_ASSERT_X (bOk, "", "");
        Q_UNUSED(bOk);
    }
    return true;
}

bool QSimplePolygon::GetZoomPoints(QPointF *pts, qreal offset) const
{
    if (NULL == pts)
    {
        return false;
    }

    Q_ASSERT_X (IsValid(), "", "");

    if (m_bClosed)
    {
        QPointF pt010, pt011, pt120, pt121;
        GetEdgeOffsetPts(m_pts.back(), m_pts.front(), offset, pt010, pt011);
        GetEdgeOffsetPts(m_pts.front(), m_pts[1], offset, pt120, pt121);
        bool bOk = calc_intersection(pt010, pt011, pt120, pt121, pts[0]);
        Q_ASSERT_X (bOk, "", "");
        Q_UNUSED(bOk);
    }
    else
    {
        QPointF pt121;
        GetEdgeOffsetPts(m_pts.front(), m_pts[1], offset, pts[0], pt121);
    }

    const unsigned& s = m_pts.size(); 
    for (unsigned i = 1; i < s - 1; ++i)
    {
        QPointF pt010, pt011, pt120, pt121;
        GetEdgeOffsetPts(m_pts[i-1], m_pts[i], offset, pt010, pt011);
        GetEdgeOffsetPts(m_pts[i], m_pts[i+1], offset, pt120, pt121);
        bool bOk = calc_intersection(pt010, pt011, pt120, pt121, pts[i]);
        Q_ASSERT_X (bOk, "", "");
        Q_UNUSED(bOk);
    }

    if (m_bClosed)
    {
        QPointF pt010, pt011, pt120, pt121;
        GetEdgeOffsetPts(m_pts[s-2], m_pts[s-1], offset, pt010, pt011);
        GetEdgeOffsetPts(m_pts[s-1], m_pts[0], offset, pt120, pt121);
        bool bOk = calc_intersection(pt010, pt011, pt120, pt121, pts[s-1]);
        Q_ASSERT_X (bOk, "", "");
        Q_UNUSED(bOk);
    }
    else
    {
        QPointF pt010;
        GetEdgeOffsetPts(m_pts[s-2], m_pts[s-1], offset, pt010, pts[s-1]);
    }
    return true;
}

// static
bool QSimplePolygon::EraseStraightPointsOnPolygon(QVector<QPointF>& pts)
{
    unsigned s = pts.size();
    if (s < 3)
    {
        return false;
    }

    if (3 == s)
    {
        if (0 == cross_product(pts[0], pts[1], pts[2]))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    for (unsigned i = 0; i < s - 2; ++i)
    {
        if (0 == cross_product(pts[i], pts[i+1], pts[i+2]))
        {
            pts.erase(pts.begin() + i + 1);
            --s;
            --i;
        }
    }

    if (s < 3)
    {
        return false;
    }

    if (0 == cross_product(pts[s-2], pts[s-1], pts[0]))
    {
        pts.erase(pts.end() - 1);
        --s;
    }

    if (s < 3)
    {
        return false;
    }

    if (0 == cross_product(pts[s-1], pts[0], pts[1]))
    {
        pts.erase(pts.begin());
        --s;
    }

    if (s < 3)
    {
        return false;
    }
    return true;
}

// static
void QSimplePolygon::GetEdgeOffsetPts(const QPointF& pt0, const QPointF& pt1, qreal offset,
                                      QPointF& opt0, QPointF& opt1)
{
    const qreal& dist = calc_distance(pt0, pt1);
    Q_ASSERT_X(0 != dist, "", "");
    const qreal& scale = offset / dist;
    const qreal& dx = (pt1.y() - pt0.y()) * scale;
    const qreal& dy = (pt1.x() - pt0.x()) * scale;
    opt0.rx() = pt0.x() + dx;
    opt0.ry() = pt0.y() - dy;
    opt1.rx() = pt1.x() + dx;
    opt1.ry() = pt1.y() - dy;
}

QPathZoomer::QPathZoomer(qreal offset /* = 0 */)
: m_offset(offset)
{
}

void QPathZoomer::SetOffset(qreal offset)
{
    m_offset = offset;
}

qreal QPathZoomer::GetOffSet() const
{
    return m_offset;
}

void QPathZoomer::ZoomPath(const QPainterPath& path2zoom, QPainterPath& pathAfterZoom) const
{
    if (0 == m_offset) {
        pathAfterZoom.addPath(path2zoom);
        return;
    }

    if (m_offset > 0) {
        QList<QPolygonF> polygons = path2zoom.toSubpathPolygons();
        for (int subIndex = 0; subIndex < polygons.size(); ++ subIndex) {
            QSimplePolygon sp(polygons[subIndex]);
            if (sp.IsValid()) {
                const unsigned ps = sp.GetPointsCount();
                bool bClockWise = sp.IsClockWise();

                // 判断路径的内外侧
                QPointF pt0, pt1;
                sp.GetZoomPoint(&pt0, 0, 0.1);
                sp.GetZoomPoint(&pt1, 0, -0.1);
                bool b0 = path2zoom.contains(pt0);
                bool b1 = path2zoom.contains(pt1);
                if (pt0 != pt1) {
                    qreal offset = m_offset;
                    if (b0 && !b1) {
                        // pt0方向代表内侧
                        offset = -m_offset;
                    } else {// !b0 && b1
                    }
                    QVector<QPointF> zoomedPts(ps);
                    sp.GetZoomPoints(&zoomedPts[0], offset);

                    // 判断扩大之后的顶点顺序是否和原来相同
                    if (bClockWise == IsPolygonClockWise(&zoomedPts[0], ps)) {
                        pathAfterZoom.connectPolygon(zoomedPts);
                        if (sp.IsClosed())
                            pathAfterZoom.closeSubpath();
                    }
                } else {
                    Q_ASSERT_X(0, "", "NotImplment");
                }
            } else {
                // 可能为直线，直接添加进去
                pathAfterZoom.connectPolygon(sp.GetPoints());
                if (sp.IsClosed())
                    pathAfterZoom.closeSubpath();
            }
        }
    } else {
        Q_ASSERT_X(0, "","PenAlgnmentInset NotImplment");
    }
}

QPathDasher::QPathDasher() 
: m_dashOffset(0)
, m_dashPattern()
, m_width(1)
{
}

qreal QPathDasher::GetWidth() const
{
    return m_width;
}

qreal QPathDasher::GetDashOffset() const
{
    return m_dashOffset;
}

int QPathDasher::GetDashPatternCount() const
{
    return m_dashPattern.size();
}

bool QPathDasher::GetDashPattern(qreal *dashArray, int count) const
{
    if (NULL == dashArray)
    {
        return false;
    }

    Q_ASSERT(count < m_dashPattern.size());

    for (int i = 0; i < count; ++i)
    {
        dashArray[i] = m_dashPattern[i];
    }
    return true;
}

bool QPathDasher::SetDashPattern(const qreal* dasharray, int count)
{
    if (NULL == dasharray || count <= 1 || ((count & 0x01) == 1))
    {
        return false;
    }

    int i = 0;
    while (i < count)
    {
        if (dasharray[i] <= 0)
        {
            return false;
        }			
        ++i;
    }

    m_dashPattern.resize(count);
    while (count-- > 0)
    {
        m_dashPattern[count] = dasharray[count];
    }
    return true;
}

void QPathDasher::SetDashOffset(qreal dashoffset)
{
    m_dashOffset = dashoffset;
}

void QPathDasher::SetWidth(qreal width)
{
    m_width = width <= 0 ? 1 : width; 
}

void QPathDasher::GetDashedPath(const QPainterPath& path2dash, 
                                QPainterPath& dashedPath) const
{
    QList<QPolygonF> polygons = path2dash.toSubpathPolygons();
    for (int i = 0; i < polygons.size(); ++i)
        GetDashedPath(polygons[i], dashedPath);
}

void QPathDasher::GetDashedPath(const QPainterPath& path2dash,  QPainterPath& openStartPath,
                                QPainterPath& openMiddleAndClosePath, QPainterPath& openEndPath) const
{
    QList<QPolygonF> polygons = path2dash.toSubpathPolygons();
    for (int subIndex = 0; subIndex < polygons.size(); ++subIndex) {
        if (polygons[subIndex].isClosed())
            continue;

        const QPolygonF &poly = polygons[subIndex];
        QPointF fpt, lpt;
        fpt = poly.front();
        lpt = poly.back();

        QPainterPath tempDashedPath;
        GetDashedPath(poly, tempDashedPath);
        QList<QPolygonF> dashedPolygons = tempDashedPath.toSubpathPolygons();
        int ns = dashedPolygons.count();
        for (int n = 0; n < ns; ++n) {
            if (0 == n) {
                QPointF tfpt = dashedPolygons[n].front();
                if (tfpt == fpt)
                    openStartPath.addPolygon(dashedPolygons[n]);
                else 
                    openMiddleAndClosePath.addPolygon(dashedPolygons[n]);
            } else if ((ns - 1) == n) {
                QPointF tlpt = dashedPolygons[n].back();
                if (tlpt == lpt)
                    openEndPath.addPolygon(dashedPolygons[n]);
                else
                    openMiddleAndClosePath.addPolygon(dashedPolygons[n]);
            } else {
                openMiddleAndClosePath.addPolygon(dashedPolygons[n]);
            }
        }
    }
}

void QPathDasher::GetDashedPath(const QPolygonF& subPath, QPainterPath& outPath) const
{	
    if (m_dashPattern.empty()) {
        outPath.addPolygon(subPath);
    } else {
        QVertices vertices(subPath);
        GenerateDash(vertices, outPath);
    }
}

void QPathDasher::GenerateDash(const QVertices& vertices,
                               QPainterPath& outPath) const
{
    Q_ASSERT_X(m_dashPattern.size() > 0,"","");
    Q_ASSERT_X((m_dashPattern.size() & 1) == 0, "", "必须是偶数个");
    Q_ASSERT_X(m_width > 0,"", "");

    qreal currentDashLen = 0;
    int currentDashIndex = 0;
    CalcDashStart(currentDashLen, currentDashIndex, m_dashPattern, m_dashOffset, m_width);
    bool bConnect = FALSE;
    const bool& bClosed = vertices.isclosed();	
    if (bClosed)
    {
        qreal current_strt = 0;
        qreal current_rest = vertices.last().dist;

        while (current_rest > 0)
        {
            if (currentDashLen > current_rest)
            {
                if ((currentDashIndex | 1) != currentDashIndex) // ----
                {
                    AddStart(vertices.last(), vertices.first(), current_strt, outPath);
                    AddPoint(vertices.last(), vertices.first(), current_strt + current_rest, outPath);							
                }
                bConnect = TRUE;
                currentDashLen -= current_rest;
                break;
            }
            else
            {
                if ((currentDashIndex | 1) != currentDashIndex) // ----
                {
                    AddStart(vertices.last(), vertices.first(), current_strt, outPath);
                    AddPoint(vertices.last(), vertices.first(), current_strt + currentDashLen, outPath);							
                }
                current_strt += currentDashLen;
                current_rest -= currentDashLen;
                IncCurrentDash(currentDashIndex, currentDashLen, m_dashPattern, m_width);
            }
        }
    }

    int i = 0;
    while (i < vertices.size() - 1)
    {
        qreal current_strt = 0;
        qreal current_rest = vertices[i].dist;

        while (current_rest > 0)
        {
            if (currentDashLen > current_rest)
            {
                if ((currentDashIndex | 1) != currentDashIndex)
                {
                    if (!bConnect)
                    {
                        AddStart(vertices[i], vertices[i+1], current_strt, outPath);
                    }
                    AddPoint(vertices[i], vertices[i+1], current_strt + current_rest, outPath);							
                }
                bConnect = TRUE;
                currentDashLen -= current_rest;
                break;					
            }
            else
            {
                if ((currentDashIndex | 1) != currentDashIndex)
                {
                    if (!bConnect)
                    {
                        AddStart(vertices[i], vertices[i+1], current_strt, outPath);
                    }
                    AddPoint(vertices[i], vertices[i+1], current_strt + currentDashLen, outPath);
                }
                bConnect = FALSE;
                current_strt += currentDashLen;
                current_rest -= currentDashLen;
                IncCurrentDash(currentDashIndex, currentDashLen, m_dashPattern, m_width);
            }
        }
        ++i;
    }
}

// static
void QPathDasher::CalcDashStart(qreal& currentDashLen,
                                int& currentDashIndex, 
                                const QVector<qreal>& pattern,
                                qreal dashOffset,
                                qreal width)
{	
    qreal dashLen = 0;
    const int& s = pattern.size();
    for (int i = 0; i < s; ++i)
    {
        dashLen = dashLen + pattern[i];
    }

    qreal offset = dashOffset;
    while (offset >= dashLen)
    {
        offset -= dashLen;
    }

    for (int j = 0; j < s; ++j)
    {
        if (offset < pattern[j])
        {
            currentDashLen = (pattern[j] - offset) * width;
            currentDashIndex = j;
            return;
        }
        offset = offset - pattern[j];
    }

    Q_ASSERT_X(0, "", "Should never run here");
}

// static 
void QPathDasher::IncCurrentDash(int& currentDashIndex,
                                 qreal& currentDashLen,
                                 const QVector<qreal>& pattern,
                                 qreal width)
{
    ++currentDashIndex;

    if (currentDashIndex >= pattern.size())
    {
        currentDashIndex = 0;
    }
    currentDashLen = pattern[currentDashIndex] * width;
}

// static
void QPathDasher::AddStart(const vertex_dist& v0,
                           const vertex_dist& v1,
                           qreal dist,
                           QPainterPath& outPath)
{
    outPath.moveTo(v0.pt.x() + dist / v0.dist * (v1.pt.x() - v0.pt.x()),
        v0.pt.y() + dist / v0.dist * (v1.pt.y() - v0.pt.y()));
}

// static
void QPathDasher::AddPoint(const vertex_dist& v0,
                           const vertex_dist& v1,
                           qreal dist,
                           QPainterPath& outPath)
{
    QPointF pt(v0.pt.x() + dist / v0.dist * (v1.pt.x() - v0.pt.x()),
        v0.pt.y() + dist / v0.dist * (v1.pt.y() - v0.pt.y()));
    outPath.lineTo(pt);
}
QAnchorGenerator::QAnchorGenerator(const QCustomLineAnchor& startCap,
                                     const QCustomLineAnchor& endCap,
                                     qreal width)
                                     : m_width(width < 1 ? 1 : width)
                                     , m_pCustomStartCap(startCap.data_ptr())
                                     , m_pCustomEndCap(endCap.data_ptr())
{	
}

void QAnchorGenerator::Generate(const QPainterPath& path2generate,
                                 QPainterPath& pathAfterGenerate,
                                 QPainterPath& generatedCapPath) const
{
    QList<QPolygonF> polygons = path2generate.toSubpathPolygons();
    for (int i = 0; i < polygons.size(); ++i) {
        if (polygons[i].isClosed())
            pathAfterGenerate.addPolygon(polygons[i]);
        else
            GenerateCap(polygons[i].constData(), polygons[i].count(), pathAfterGenerate, generatedCapPath);
    }
}


void QAnchorGenerator::GenerateCap(const QPointF *pts, const unsigned count,
                                    QPainterPath& pathAfterGenerate,
                                    QPainterPath& generatedCapPath) const
{
    Q_ASSERT(count > 1);
    QPainterPath capPath;
    QVector<QPointF> strokePts;
    std::copy(pts, pts + count, std::back_inserter(strokePts));

    if (m_pCustomStartCap)
    {
        QPointF devidePt;
        int prevDevide;
        if (m_pCustomStartCap->GetDevidePoint(m_width, pts, count, devidePt, prevDevide))
        {
            const QPointF& centerPt = pts[0];
            m_pCustomStartCap->GenerateCap(m_width, devidePt, centerPt, centerPt, capPath);

            QPointF insetPt;
            const qreal& insetScale = m_pCustomStartCap->calcInsetScale(m_width);
            CalcScalePointOnSegment(centerPt, devidePt, insetScale, insetPt);
            strokePts.erase(strokePts.begin(), strokePts.begin() + prevDevide + 1);
            strokePts.insert(strokePts.begin(), insetPt);
        }
        else
        {
            const QPointF& centerPt = pts[0];
            const QPointF& endPt = pts[count-1];
            m_pCustomStartCap->GenerateCap(m_width, endPt, centerPt, centerPt, capPath);
            strokePts.clear();
        }
    }

    if (m_pCustomEndCap)
    {
        // 开始圈外点数大于0
        if (!strokePts.empty())
        {
            QVector<QPointF> invertPts(strokePts);
            std::reverse(invertPts.begin(), invertPts.end());

            QPointF devidePt;
            int prevDevide;
            if (m_pCustomEndCap->GetDevidePoint(m_width, &invertPts[0], invertPts.size(), devidePt, prevDevide))
            {
                const QPointF& centerPt = invertPts[0];
                m_pCustomEndCap->GenerateCap(m_width, devidePt, centerPt, centerPt, capPath);
                QPointF insetPt;
                const qreal& insetScale = m_pCustomEndCap->calcInsetScale(m_width);
                CalcScalePointOnSegment(centerPt, devidePt, insetScale, insetPt);
                invertPts.erase(invertPts.begin(), invertPts.begin() + prevDevide + 1);
                invertPts.insert(invertPts.begin(), insetPt);

                std::reverse(invertPts.begin(), invertPts.end());
                strokePts = invertPts;
            }
            else
            {
                // 没有割点
                if (m_pCustomStartCap && count > 2)
                {
                    capPath = QPainterPath();
                }
                else
                {
                    m_pCustomEndCap->GenerateCap(m_width, strokePts.front(), strokePts.back(), strokePts.back(), capPath);
                }
                strokePts.clear();
            }
        }
        else // just for end cap
        {
            std::vector<QPointF> innerPts;
            for (unsigned i = 1; i < count - 1; ++i)
            {
                innerPts.push_back(pts[i]);
            }

            QPointF insetPt;
            const qreal& insetScale = 
                m_pCustomStartCap->baseInset() * m_pCustomStartCap->widthScale();
            CalcScalePointOnSegment(pts[0], pts[count-1], insetScale, insetPt);
            innerPts.push_back(insetPt);

            QPointF devidePt;
            int prevDevide;
            if (m_pCustomEndCap->GetDevidePoint(m_width, &innerPts[0], innerPts.size(), devidePt, prevDevide))
            {
                m_pCustomEndCap->GenerateCap(m_width, devidePt, insetPt, pts[count-1], capPath);
            }
            else
            {
                if (0 < m_pCustomStartCap->baseInset())
                {
                    m_pCustomEndCap->GenerateCap(m_width, pts[0], pts[count-1], pts[count-1], capPath);
                }
            }

            // stroke empty
            strokePts.clear();
        }
    }

    pathAfterGenerate.addPolygon(strokePts);
    generatedCapPath.addPath(capPath);
}

QCustomLineAnchorState::QCustomLineAnchorState()
: m_baseInset(0)
, m_widthScale(1)
, m_strokeStartCap(Qt::FlatCap)
, m_strokeEndCap(Qt::FlatCap)
, m_strokeLineJoin(Qt::MiterJoin)
, m_baseCap(Qt::FlatCap)
{
}

QCustomLineAnchorState::~QCustomLineAnchorState()
{
}

qreal QCustomLineAnchorState::baseInset() const
{
    return m_baseInset;
}

qreal QCustomLineAnchorState::widthScale() const
{
    return m_widthScale;
}

Qt::PenCapStyle QCustomLineAnchorState::strokeStartCap() const
{
    return m_strokeStartCap;
}

void QCustomLineAnchorState::setStrokeStartCap(Qt::PenCapStyle startCap)
{
    m_strokeStartCap = startCap;
}

Qt::PenCapStyle QCustomLineAnchorState::strokeEndCap() const
{
    return m_strokeEndCap;
}

void QCustomLineAnchorState::setStrokeEndCap(Qt::PenCapStyle endCap)
{
    m_strokeEndCap = endCap;
}

Qt::PenJoinStyle QCustomLineAnchorState::strokeJoin() const
{
    return m_strokeLineJoin;
    /*		return m_mathStroke.GetLineJoin();*/
}

Qt::PenCapStyle QCustomLineAnchorState::baseCap() const
{
    return m_baseCap;
}

void QCustomLineAnchorState::setBaseInset(qreal inset)
{
    m_baseInset = inset;
}	

void QCustomLineAnchorState::setWidthScale(qreal widthScale)
{
    m_widthScale = widthScale;
}

void QCustomLineAnchorState::setStrokeJoin(Qt::PenJoinStyle lineJoin)
{
    m_strokeLineJoin = lineJoin;
    /*		m_mathStroke.SetLineJoin(lineJoin);*/
}

void QCustomLineAnchorState::setBaseCap(Qt::PenCapStyle baseCap)
{
    m_baseCap = baseCap;
}

qreal QCustomLineAnchorState::calcInsetScale(qreal penWidth) const
{
    return penWidth * m_baseInset * m_widthScale / GetDevideDistance(penWidth);
}

bool QCustomLineAnchorState::GetDevidePoint(qreal width, const QPointF *pts, unsigned count, QPointF &devidePt, int &prevPtOffset) const
{
    const qreal& devideDist = GetDevideDistance(width);
    qreal prevDist = 0;
    for (unsigned i = 1; i < count; ++i)
    {
        const QPointF& centerPt = pts[0];
        const qreal& dist = calc_distance(centerPt, pts[i]);
        if (dist >= devideDist)
        {
            prevPtOffset = i - 1;
            CalcSegmentCrossRound(pts[0], devideDist, pts[prevPtOffset],
                pts[i], prevDist, dist, devidePt);
            return true;
        }
        prevDist = dist;
    }
    return false;
}

void QCustomLineAnchorState::CalcTransform(qreal width, const QPointF& fromPt, const QPointF& toPt,
                                           const QPointF& centerPt, QMatrix& mtx) const
{
    const qreal& len = calc_distance(fromPt, toPt);
    const qreal& dy = toPt.y() - fromPt.y();
    const qreal& dx = toPt.x() - fromPt.x();
    const qreal& ctheta = dy / len;
    const qreal& stheta = -dx / len;

    mtx.setMatrix(ctheta, stheta, -stheta, ctheta, 0, 0);
    QMatrix scaleMatrix(width * m_widthScale, 0.0, 0.0, width * m_widthScale, 0.0, 0.0);
    mtx *= scaleMatrix;

    QMatrix translateMatrix(1.0, 0.0, 0.0, 1.0, centerPt.x(), centerPt.y());
    mtx *= translateMatrix;
}

bool QCustomLineAnchorState::CalcCrossYPts(const QPainterPath& path, std::vector<qreal>& dists,
                                           qreal width) const
{
    QMatrix mtx;
    const qreal& scale = width * widthScale();
    QMatrix scaleMatrix(scale, 0.0, 0.0, scale, 0.0, 0.0);
    mtx = scaleMatrix * mtx;
    QList<QPolygonF> polygons = path.toSubpathPolygons(mtx);
    for (int subIndex = 0; subIndex < polygons.size(); ++subIndex) {
        const QPolygonF &poly = polygons[subIndex];
        QVector<int> flags(poly.count());
        for (int i = 0; i < poly.count(); ++i) {
            const qreal& x = poly[i].x();
            if (x > 0) flags[i] = 1;
            else if (x < 0) flags[i] = -1;
            else flags[i] = 0;
        }
        for (int i = 0; i < poly.count() - 1; ++i)
            CalcCrossYPt(poly[i], poly[i+1], flags[i], flags[i+1], dists);
    }
    std::sort(dists.begin(), dists.end());
    return !dists.empty();
}

void QCustomLineAnchorState::CopyTo(QCustomLineAnchorState& capState) const
{
    if (this != &capState)
    {
        capState.m_baseInset = m_baseInset;
        capState.m_widthScale = m_widthScale;
        capState.m_strokeStartCap = m_strokeStartCap;
        capState.m_strokeEndCap = m_strokeEndCap;
        capState.m_strokeLineJoin = m_strokeLineJoin;
        capState.m_baseCap = m_baseCap;
    }
}

void QCustomLineAnchorState::CalcCrossYPt(const QPointF& pt1, const QPointF& pt2, int flag1,
                                          int flag2, std::vector<qreal>& dists)
{
    const int& f = flag1 * flag2;		
    if (f < 0)
    {
        const qreal& dist = (pt2.x() * pt1.y() - pt1.x() * pt2.y()) / (pt2.x() - pt1.x());
        dists.push_back(dist);
        return;
    }

    if (f == 0)
    {
        if (flag1 == 0)
        {
            dists.push_back(pt1.y());
        }
        if (flag2 == 0)
        {
            dists.push_back(pt2.y());
        }
        return;
    }
}

QCustomFillAnchor::QCustomFillAnchor(const QPainterPath& path)
: m_capPath(path)
{
}

QCustomFillAnchor::~QCustomFillAnchor()
{
}

QCustomLineAnchorState *QCustomFillAnchor::Clone() const
{
    QCustomFillAnchor *pClone = new QCustomFillAnchor(m_capPath);
    CopyTo(*pClone);
    return pClone;
}

qreal QCustomFillAnchor::GetDevideDistance(qreal width) const
{
    std::vector<qreal> dists;
    if (CalcCrossYPts(m_capPath, dists, width)) {
        if (dists[0] >= 0)
            return 0;
        else
            return -dists[0];
    }
    return 0;
}

void QCustomFillAnchor::GenerateCap(qreal width, const QPointF& fromPt, const QPointF& toPt,
                                    const QPointF& centerPt, QPainterPath& capPath) const
{
    QPainterPath pPathClone(m_capPath);
    QMatrix mtx;
    CalcTransform(width, fromPt, toPt, centerPt, mtx);
    pPathClone = mtx.map(pPathClone);
    capPath.addPath(pPathClone);
}

qreal QCustomFillAnchor::GetMaxDistance(qreal width) const
{
    QMatrix mtx;
    const qreal& scale = widthScale() * width;

    QMatrix scaleMatrix(scale, 0.0, 0.0, scale, 0.0, 0.0);
    mtx = scaleMatrix * mtx;

    qreal maxDist = 0;
    QPointF origin(0.0, 0.0);
    QList<QPolygonF> polygons = m_capPath.toSubpathPolygons();
    for (int i = 0; i < polygons.size(); ++i) {
        const QPolygonF &poly = polygons[i];
        foreach(QPointF pt, poly) {
            qreal dist = calc_distance(pt, origin);
            if (dist > maxDist)
                maxDist = dist;
        }
    }
    return maxDist;
}

QCustomStrokeAnchor::QCustomStrokeAnchor(const QPainterPath& path)
: m_capPath(path)
{
}

QCustomStrokeAnchor::~QCustomStrokeAnchor()
{
}

QCustomLineAnchorState *QCustomStrokeAnchor::Clone() const
{
    QCustomStrokeAnchor *pClone = new QCustomStrokeAnchor(m_capPath);
    CopyTo(*pClone);
    return pClone;
}

qreal QCustomStrokeAnchor::GetDevideDistance(qreal width) const
{
    std::vector<qreal> dists;
    if (CalcCrossYPts(m_capPath, dists, width)) {
        if (dists[0] >= 0)
            return width;
        else
            return -dists[0];
    }
    return width;
}

void QCustomStrokeAnchor::GenerateCap(qreal width, const QPointF& fromPt, const QPointF& toPt,
                                      const QPointF& centerPt, QPainterPath& capPath) const
{
    QMathStroker mathStroker;
    mathStroker.SetWidth(widthScale() * width);
    mathStroker.SetLineCap(strokeStartCap(), strokeEndCap());
    mathStroker.SetLineJoin(strokeJoin());
    QPainterPath pPathClone(m_capPath);
    QMatrix mtx;
    CalcTransform(width, fromPt, toPt, centerPt, mtx);
    pPathClone = mtx.map(pPathClone);
    mathStroker.StrokePath(pPathClone, capPath);
}

qreal QCustomStrokeAnchor::GetMaxDistance(qreal width) const
{
    QMatrix mtx;
    const qreal& scale = widthScale() * width;

    QMatrix scaleMatrix(scale, 0.0, 0.0, scale, 0.0, 0.0);
    mtx = scaleMatrix * mtx;

    QList<QPolygonF> polygons = m_capPath.toSubpathPolygons();

    QMathStroker mathStroker;
    mathStroker.SetWidth(width);
    mathStroker.SetLineCap(strokeStartCap(), strokeEndCap());
    mathStroker.SetLineJoin(strokeJoin());
    QPainterPath widenPath;
    mathStroker.StrokePath(polygons, widenPath);

    qreal maxDist = 0;
    const int s = widenPath.elementCount();
    for (int i = 0; i < s; ++i)
    {
        const QPainterPath::Element &e = widenPath.elementAt(i);
        const qreal& dist = calc_distance(e.x, e.y, 0.0, 0.0);
        if (dist > maxDist)
        {
            maxDist = dist;
        }
    }
    return maxDist;
}

QCustomLineAnchor::QCustomLineAnchor()
: m_cap(NULL)
{

}

QCustomLineAnchor::QCustomLineAnchor(Qt::PenAnchorStyle anchor)
: m_cap(NULL)
{
    QPainterPath capPath;
    switch (anchor) {
case Qt::ArrowAnchor: {
    capPath.moveTo(1.0, -1.732051);
    capPath.lineTo(0, 0);
    capPath.lineTo(-1.0, -1.732051);
    capPath.closeSubpath();
    break;
                      }

case Qt::RoundAnchor: {
    capPath.addEllipse(-1, -1, 2, 2);
    break;
                      }

case Qt::DiamondAnchor: {
    capPath.moveTo(0, -1);
    capPath.lineTo(1, 0);
    capPath.lineTo(0, 1);
    capPath.lineTo(-1, 0);
    capPath.closeSubpath();
    break;
                        }

case Qt::SquareAnchor: {
    capPath.moveTo(1, -1);
    capPath.lineTo(1, 1);
    capPath.lineTo(-1, 1); 
    capPath.lineTo(-1, -1);
    capPath.closeSubpath();
    break;
                       }
    }

    if (!capPath.isEmpty())
        m_cap = new QCustomFillAnchor(capPath);
    if (anchor == Qt::ArrowAnchor)
        m_cap->setBaseInset(1.0f);
}

QCustomLineAnchor::QCustomLineAnchor(const QPainterPath &anchorPath,
                                     AnchorPathUsage usage,
                                     Qt::PenCapStyle baseCap,
                                     qreal baseInset)
                                     : m_cap(createLineCapState(anchorPath, usage))
{
    if (m_cap->GetDevideDistance(1) <= 0)
    {
        // 这一约束主要针对fillPath。GDI+未来版本这里可能会有变化。
        reset(NULL);

    }
    else
    {
        m_cap->setBaseCap(baseCap);
        m_cap->setBaseInset(baseInset);
    }
}

QCustomLineAnchor::QCustomLineAnchor(QCustomLineAnchorState* capState)
: m_cap(capState)
{
    Q_ASSERT(capState);
    Q_ASSERT(isValid());
}

QCustomLineAnchor::QCustomLineAnchor(const QCustomLineAnchor &rhs)
: m_cap(NULL)
{
    if (rhs.m_cap)
        m_cap = rhs.m_cap->Clone();
}

QCustomLineAnchor::~QCustomLineAnchor()
{
    reset(NULL);
}

QCustomLineAnchor &QCustomLineAnchor::operator=(const QCustomLineAnchor &rhs)
{
    delete m_cap;
    m_cap = NULL;
    if (rhs.m_cap)
        m_cap = rhs.m_cap->Clone();
    return *this;
}

Qt::PenCapStyle QCustomLineAnchor::strokeStartCap() const
{
    return m_cap->strokeStartCap();
}

void QCustomLineAnchor::setStrokeStartCap(Qt::PenCapStyle startCap)
{
    return m_cap->setStrokeStartCap(startCap);
}

Qt::PenCapStyle QCustomLineAnchor::strokeEndCap() const
{
    return m_cap->strokeEndCap();
}

void QCustomLineAnchor::setStrokeEndCap(Qt::PenCapStyle endCap)
{
    return m_cap->setStrokeEndCap(endCap);
}

Qt::PenCapStyle QCustomLineAnchor::baseCap() const	
{ 
    if (isValid())
    {
        return m_cap->baseCap();
    }
    return Qt::FlatCap;
}

qreal QCustomLineAnchor::baseInset() const
{
    if (isValid())
    {
        return m_cap->baseInset();
    }
    return 0;
}

Qt::PenJoinStyle QCustomLineAnchor::strokeJoin() const	
{
    if (isValid())
    {
        return m_cap->strokeJoin();
    }
    return Qt::MiterJoin;
}

qreal QCustomLineAnchor::widthScale() const	
{
    if (isValid())
    {
        return m_cap->widthScale();
    }
    return 1;
}

void QCustomLineAnchor::setBaseCap(Qt::PenCapStyle baseCap)	
{
    if (isValid())
    {
        m_cap->setBaseCap(baseCap);
        return;
    }

}

void QCustomLineAnchor::setBaseInset(qreal inset)
{
    if (isValid())
    {
        m_cap->setBaseInset(inset);
        return;
    }

}

void QCustomLineAnchor::setStrokeJoin(Qt::PenJoinStyle lineJoin)
{
    if (isValid())
    {
        m_cap->setStrokeJoin(lineJoin);
        return;
    }
    return;
}

void QCustomLineAnchor::setWidthScale(qreal widthScale)
{
    if (isValid())
    {
        m_cap->setWidthScale(widthScale);
        return;
    }
    return;
}

bool QCustomLineAnchor::isValid() const
{
    if (m_cap)
    {
        return true;
    }
    else
    {
        return false;
    }
}


// ------------------------------------------------------------------------p
// static 
QCustomLineAnchorState *QCustomLineAnchor::createLineCapState(
    const QPainterPath &anchorPath, AnchorPathUsage usage)
{
    if (usage == PathFill)
        return new QCustomFillAnchor(anchorPath);
    else if (usage == PathStroke)
        return new QCustomStrokeAnchor(anchorPath);
    return NULL;
}

void QCustomLineAnchor::reset(QCustomLineAnchorState* capState)
{
    if (m_cap) {
        Q_ASSERT(capState != m_cap);
        delete m_cap;
    }
    m_cap = capState;
}

QComplexStrokerPrivate::QComplexStrokerPrivate()
: width(1.0),
//      dashPattern,
joinStyle(Qt::MiterJoin),
dashOffset(0.0),
miterLimit(4.0),
//      compoundArray,
//customStartCap(),
//customEndCap(),
alignment(Qt::PenAlignmentCenter),
startCap(Qt::FlatCap),
endCap(Qt::FlatCap),
dashCap(Qt::FlatCap)
{
}

bool QComplexStrokerPrivate::isSolid() const
{
    return dashPattern.isEmpty();
}

qreal QComplexStrokerPrivate::alignmentOffset()
{
    switch (alignment)
    {
    case Qt::PenAlignmentCenter:
        return 0;
    case Qt::PenAlignmentOutset:
        return width / 2;
    case Qt::PenAlignmentInset:
        return -width / 2;
    default:
        return 0;
    }
}

QMathStroker *QComplexStrokerPrivate::prepareMathStroker()
{
    QMathStroker *mathStroker(new QMathStroker);
    mathStroker->SetWidth(width);
    mathStroker->SetLineJoin(joinStyle);
    mathStroker->SetMiterLimit(miterLimit);
    if (isSolid()) {
        mathStroker->SetStartCap(startCap);
        mathStroker->SetEndCap(endCap);
    } else {
        mathStroker->SetStartCap(dashCap);
        mathStroker->SetEndCap(dashCap);
    }
    if (!compoundArray.isEmpty())
        mathStroker->SetCompoundArray(&compoundArray[0], compoundArray.size());
    mathStroker->SetAlignment(alignment);
    //QMatrix mtx;
    ////pen.GetTransform(&mtx);
    //m_scaleX = sqrt(mtx.m11() * mtx.m11() + mtx.m21() * mtx.m21());
    //m_scaleY = sqrt(mtx.m12() * mtx.m12() + mtx.m22() * mtx.m22());
    ////mtx.GetScalingAbs(m_scaleX, m_scaleY);
    return mathStroker;
}

QPathDasher *QComplexStrokerPrivate::prepareDasher()
{
    QPathDasher *dasher = new QPathDasher;
    dasher->SetDashOffset(dashOffset);

    QVector<qreal> dp = dashPattern;
    if (dashCap == Qt::RoundCap || dashCap == Qt::TriangleCap) {
        const int s = dp.size();
        int i = 0;
        while (i != s)
        {
            if (i & 1)
                dp[i] += 0.99;
            else
                dp[i] -= 0.99;
            ++i;
        }

    }
    dasher->SetDashPattern(&dp[0], dp.size());

    dasher->SetWidth(width);
    return dasher;
}

QComplexStroker::QComplexStroker()
: d_ptr(new QComplexStrokerPrivate)
{
    d_ptr->ref.ref();
}

QComplexStroker::QComplexStroker(const QComplexStroker &rhs)
{
    d_ptr = rhs.d_ptr;
    d_ptr->ref.ref();
}

QComplexStroker::~QComplexStroker()
{
    if (!d_ptr->ref.deref())
        delete d_ptr;
}

QComplexStroker &QComplexStroker::operator=(const QComplexStroker &rhs)
{
    qAtomicAssign(d_ptr, rhs.d_ptr);
    return *this;
}

void QComplexStroker::detach()
{
    if (d_ptr->ref == 1)
        return;

    QComplexStrokerPrivate *x = new QComplexStrokerPrivate(*d_ptr);
    if (!d_ptr->ref.deref())
        delete d_ptr;
    x->ref = 1;
    d_ptr = x;
}

QVector<qreal> QComplexStroker::dashPattern() const
{
    return d_ptr->dashPattern;
}

void QComplexStroker::setDashPattern(const QVector<qreal> &pattern)
{
    if (pattern.isEmpty() || d_ptr->dashPattern == pattern)
        return;
    detach();
    d_ptr->dashPattern = pattern;
}

qreal QComplexStroker::dashOffset() const
{
    return d_ptr->dashOffset;
}

void QComplexStroker::setDashOffset(qreal doffset)
{
    if (qFuzzyCompare(d_ptr->dashOffset, doffset))
        return;
    detach();
    d_ptr->dashOffset = doffset;
}

qreal QComplexStroker::miterLimit() const
{
    return d_ptr->miterLimit;
}

void QComplexStroker::setMiterLimit(qreal limit)
{
    if (qFuzzyCompare(d_ptr->miterLimit, limit))
        return;
    detach();
    d_ptr->miterLimit = limit;
}

qreal QComplexStroker::width() const
{
    return d_ptr->width;
}

void QComplexStroker::setWidth(qreal width)
{
    if (qFuzzyCompare(d_ptr->width, width))
        return;
    detach();
    d_ptr->width = width;
    if (d_ptr->width < 0)
        d_ptr->width = 1;
}

Qt::PenJoinStyle QComplexStroker::joinStyle() const
{
    return d_ptr->joinStyle;
}

void QComplexStroker::setJoinStyle(Qt::PenJoinStyle pcs)
{
    if (d_ptr->joinStyle = pcs)
        return;
    detach();
    d_ptr->joinStyle = pcs;
}

QVector<qreal> QComplexStroker::compoundArray() const
{
    return d_ptr->compoundArray;
}

void QComplexStroker::setCompoundArray(const QVector<qreal> &pattern)
{
    if (pattern.isEmpty() || d_ptr->compoundArray == pattern)
        return;
    detach();
    d_ptr->compoundArray = pattern;
}

Qt::PenAnchorStyle QComplexStroker::startAnchorStyle() const
{
    return d_ptr->startAnchorStyle;
}

void QComplexStroker::setStartAnchorStyle(Qt::PenAnchorStyle anchor)
{
    if (d_ptr->startAnchorStyle == anchor)
        return;
    detach();
    d_ptr->startAnchorStyle = anchor;
    d_ptr->startAnchor = QCustomLineAnchor(anchor);
}

const QCustomLineAnchor &QComplexStroker::startAnchor() const
{
    return d_ptr->startAnchor;
}

void QComplexStroker::setStartAnchor(const QCustomLineAnchor &anchor)
{
    detach();
    d_ptr->startAnchorStyle = Qt::CustomAnchor;
    d_ptr->startAnchor = anchor;
}

Qt::PenAnchorStyle QComplexStroker::endAnchorStyle() const
{
    return d_ptr->endAnchorStyle;
}

void QComplexStroker::setEndAnchorStyle(Qt::PenAnchorStyle anchor)
{
    if (d_ptr->endAnchorStyle == anchor)
        return;
    detach();
    d_ptr->endAnchorStyle = anchor;
    d_ptr->endAnchor = QCustomLineAnchor(anchor);
}

const QCustomLineAnchor &QComplexStroker::endAnchor() const
{
    return d_ptr->endAnchor;
}

void QComplexStroker::setEndAnchor(const QCustomLineAnchor &anchor)
{
    detach();
    d_ptr->endAnchorStyle = Qt::CustomAnchor;
    d_ptr->endAnchor = anchor;
}

Qt::PenAlignment QComplexStroker::alignment() const
{
    return d_ptr->alignment;
}

void QComplexStroker::setAlignment(Qt::PenAlignment alignment)
{
    if (d_ptr->alignment = alignment)
        return;
    detach();
    d_ptr->alignment = alignment;
}

Qt::PenCapStyle QComplexStroker::startCapStyle() const
{
    return d_ptr->startCap;
}

void QComplexStroker::setStartCapStyle(Qt::PenCapStyle s)
{
    if (d_ptr->startCap == s)
        return;
    detach();
    d_ptr->startCap = s;
}

Qt::PenCapStyle QComplexStroker::endCapStyle() const
{
    return d_ptr->endCap;
}

void QComplexStroker::setEndCapStyle(Qt::PenCapStyle s)
{
    if (d_ptr->endCap == s)
        return;
    detach();
    d_ptr->endCap = s;
}

Qt::PenCapStyle QComplexStroker::dashCapStyle() const
{
    return d_ptr->dashCap;
}

void QComplexStroker::setDashCapStyle(Qt::PenCapStyle s)
{
    if (d_ptr->dashCap == s)
        return;
    detach();
    d_ptr->dashCap = s;
}

QPainterPath QComplexStroker::createStroke(const QPainterPath &path) const
{
    QPainterPath result;
    QPainterPath pathAfterGenerateCap;
    QAnchorGenerator capGenerator(d_ptr->startAnchor, d_ptr->endAnchor, d_ptr->width);
    capGenerator.Generate(path, pathAfterGenerateCap, result);

    QPainterPath pathAfterZoom;
    QPathZoomer pathZoomer(d_ptr->alignmentOffset());
    pathZoomer.ZoomPath(pathAfterGenerateCap, pathAfterZoom);

    QScopedPointer<QMathStroker> mathStroker(d_ptr->prepareMathStroker());

    if (d_ptr->isSolid()) {
        mathStroker->StrokePath(pathAfterZoom, result);
    } else {
        QScopedPointer<QPathDasher> dasher(d_ptr->prepareDasher());

        Qt::PenCapStyle dc = dashCapStyle();
        Qt::PenCapStyle slc = startCapStyle();
        Qt::PenCapStyle elc = endCapStyle();

        if (dc == slc && slc == elc) {
            QPainterPath pathAfterDash;
            dasher->GetDashedPath(pathAfterZoom, pathAfterDash);
            mathStroker->StrokePath(pathAfterDash, result);
        } else {
            QPainterPath openStartpath, openMiddleAndClosePath, openEndpath;
            dasher->GetDashedPath(pathAfterZoom,  openStartpath, openMiddleAndClosePath, openEndpath);

            mathStroker->SetStartCap(slc);
            mathStroker->StrokePath(openStartpath, result);

            mathStroker->SetStartCap(dc);
            mathStroker->StrokePath(openMiddleAndClosePath, result);

            mathStroker->SetEndCap(elc);
            mathStroker->StrokePath(openEndpath, result);
        }
    }

    return result;
}