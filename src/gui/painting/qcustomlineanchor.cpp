#include "qcustomlineanchor.h"
#include "qcomplexstroker_p.h"
#include "qcustomlineanchor_p.h"

#include <QtCore/qmath.h>
#include <algorithm>

static const qreal vertex_dist_epsilon = 1.0e-30;

extern qreal calc_distance(qreal x1, qreal y1, qreal x2, qreal y2);
extern qreal calc_distance(const QPointF& pt1, const QPointF& pt2);
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
case Qt::CustomAnchor:
    break;
default:
    Q_ASSERT(!"unknow style!");
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

void QCustomLineAnchor::setFlatness(qreal flatness)
{
    if (isValid())
    {
        m_cap->setFlatness(flatness);
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

bool QCustomLineAnchor::operator==(const QCustomLineAnchor &p) const
{
    DataPtr lhs = m_cap;
    DataPtr rhs = p.m_cap;
    return (lhs == rhs)
        || (lhs != NULL && rhs != NULL
            && lhs->GetAnchorType() == rhs->GetAnchorType()
            && lhs->baseInset() == rhs->baseInset()
            && lhs->widthScale() == rhs->widthScale()
            && lhs->strokeStartCap() == rhs->strokeStartCap()
            && lhs->strokeEndCap() == rhs->strokeEndCap()
            && lhs->strokeJoin() == rhs->strokeJoin()
            && lhs->baseCap() == rhs->baseCap()
            && lhs->GetCapPath() == rhs->GetCapPath());
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
                                QPainterPath& generatedCapPath,
                                const qreal flatness) const
{
    QList<QPolygonF> polygons = path2generate.toSubpathPolygons(QTransform(), flatness);
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
, m_flatness(0.25)
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

void QCustomLineAnchorState::setFlatness(qreal flatness)
{
    m_flatness = flatness;
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
    QList<QPolygonF> polygons = path.toSubpathPolygons(QTransform(mtx), m_flatness);
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
    QList<QPolygonF> polygons = m_capPath.toSubpathPolygons(QTransform(), m_flatness);
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

    QList<QPolygonF> polygons = m_capPath.toSubpathPolygons(QTransform(), m_flatness);

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

/*****************************************************************************
  QCustomLineAnchor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const QCustomLineAnchor &anchor)
{
    QCustomLineAnchorState *d = anchor.m_cap;

    s << bool(NULL != d);
    if (NULL != d) {
        s << quint8(d->GetAnchorType());
        s << d->GetCapPath();
        s << double(d->baseInset()) << double(d->widthScale());
        s << quint16(d->strokeStartCap()) << quint16(d->strokeEndCap()) 
            << quint16(d->strokeJoin()) << quint16(d->baseCap());        
    }
    return s;
}

QDataStream &operator>>(QDataStream &s, QCustomLineAnchor &anchor)
{
    bool bNotNull = false;
    s >> bNotNull;
    if (bNotNull) {
        QCustomLineAnchorState *d = NULL;
        quint8 i8;
        QPainterPath capPath;
        double r;
        quint16 i;
        s >> i8;
        s >> capPath;
        if (QCustomLineAnchorState::AnchorTypeFill == i8)
            d = new QCustomFillAnchor(capPath);
        else if (QCustomLineAnchorState::AnchorTypeStroke == i8)
            d = new QCustomStrokeAnchor(capPath);
        else
            Q_ASSERT(false);
        s >> r;
        d->setBaseInset(r);
        s >> r;
        d->setWidthScale(r);
        s >> i;
        d->setStrokeStartCap((Qt::PenCapStyle)i);
        s >> i;
        d->setStrokeEndCap((Qt::PenCapStyle)i);
        s >> i;
        d->setStrokeJoin((Qt::PenJoinStyle)i);
        s >> i;
        d->setBaseCap((Qt::PenCapStyle)i);
        anchor = QCustomLineAnchor(d);
    } else {
        anchor = QCustomLineAnchor();
    }
    return s;
}
#endif