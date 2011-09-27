#ifndef QCOMPLEXSTROKER_P_H
#define QCOMPLEXSTROKER_P_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QVertices;
struct vertex_dist;

class QMathStroker 
{
public:	
	explicit QMathStroker();

	qreal GetWidth() const;
	void SetWidth(qreal width);
	Qt::PenJoinStyle GetLineJoin() const;
	void SetLineJoin(Qt::PenJoinStyle linejoin); 
	qreal GetMiterLimit() const;
	void SetMiterLimit(qreal miterlimit);
	Qt::PenCapStyle GetStartCap() const;
	Qt::PenCapStyle GetEndCap() const;
	void SetLineCap(Qt::PenCapStyle startCap,  Qt::PenCapStyle endCap);
	void SetStartCap(Qt::PenCapStyle startcap);
	void SetEndCap(Qt::PenCapStyle endcap);


	int  GetCompoundArrayCount() const;
	bool GetCompoundArray(qreal *compoundArray,  int count) const;
	bool SetCompoundArray(const qreal *compoundArray,  int count);

	Qt::PenAlignment GetAlignment() const;
	void SetAlignment(Qt::PenAlignment penAlignment);

	void GetScale(qreal *sx, qreal *sy) const;
	void SetScale(qreal sx, qreal sy);

	void StrokePath(const QList<QPolygonF>& path2stroke, QPainterPath& outPath) const;
	void StrokePath(const QPainterPath& path2stroke, QPainterPath& outPath, const qreal flatness = 0.25) const;

private:	
    void StrokeSubPath(const QPolygonF &subPath, QPainterPath& outPath) const;
	void StrokeCloseSubPath(const QPolygonF &subPath, QPainterPath &outPath) const;
	void StrokeOpenSubPath(const QPolygonF &subPath, QPainterPath &outPath) const;

	void CalcPoints(const QVertices& vertices, int first, int second, int third, 
		QVector<QPointF>& firstPts, QVector<QPointF>& secondPts,
		QVector<QVector<QPointF> > & compoundPts) const; 

	void CalcStartPoints(const QVertices& vertices, 
		QVector<QPointF>& firstPts, QVector<QPointF>& secondPts,
		QVector<QVector<QPointF> > & compoundPts, QVector<QPointF>& startCapPts) const;
	void CalcEndPoints(const QVertices& vertices,
		QVector<QPointF>& firstPts, QVector<QPointF>& secondPts,
		QVector<QVector<QPointF> > & compoundPts, QVector<QPointF>& endCapPts) const;

	void CalcStartPoints(const QPointF& p0, const QPointF& p1, qreal len01, QPointF& firstPt, QPointF& secondPt) const;
	void CalcEndPoints(const QPointF& p0, const QPointF& p1, qreal len01, QPointF& firstPt, QPointF& secondPt) const;

	void CalcCompoundPoints(const QVector<QPointF>& firstpts, const QVector<QPointF>& secondpts, QVector<QVector<QPointF> >& compoundpts) const;

	void ScalePoints(QVector<QPointF>& pts, const QPointF& centerPt) const;

	void CalcCap(const QPointF& p0, const QPointF& p1, qreal len01, Qt::PenCapStyle lineCap, QVector<QPointF>& capPts) const;
	void CalcJoin(QVector<QPointF>& firstpts, QVector<QPointF>& secondpts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal len01, qreal len12) const;

	void CalcCapFlat(const QPointF& p0, const QPointF& p1, qreal dx, qreal dy, QVector<QPointF>& capPts) const;
	void CalcCapSquare(const QPointF& p0, const QPointF& p1, qreal dx, qreal dy, QVector<QPointF>& capPts) const;
	void CalcCapTriangle(const QPointF& p0, const QPointF& p1, qreal dx, qreal dy, QVector<QPointF>& capPts) const;
	void CalcCapRound(const QPointF& p0, const QPointF& p1, qreal dx, qreal dy, QVector<QPointF>& capPts) const;

	void CalcInnerJoin(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2, qreal len01, qreal len12) const;
	void CalcOuterJoin(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;	
	void CalcStraightJoin(QVector<QPointF>& firstpts, QVector<QPointF>& secondpts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;

	void CalcLineJoinMiter(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;
	void CalcLineJoinMiterClipped(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;
	void CalcLineJoinBevel(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;
	void CalcLineJoinRound(QVector<QPointF>& pts, const QPointF& p0, const QPointF& p1, const QPointF& p2, qreal dx1, qreal dy1, qreal dx2, qreal dy2) const;

private:
	Q_DISABLE_COPY(QMathStroker)
	qreal	 m_width;
	Qt::PenJoinStyle m_lineJoin;
	qreal	 m_miterLimit;
	Qt::PenCapStyle	 m_startCap;
	Qt::PenCapStyle  m_endCap;
	QVector<qreal> m_compoundArray;
	Qt::PenAlignment m_penAlignment; 

	qreal m_scaleX, m_scaleY;
};	

class QSimplePolygon
{
public:
	QSimplePolygon(const QPolygonF &polygon);

	bool IsValid() const;
	bool IsClosed() const;
	bool IsClockWise() const;
	unsigned GetPointsCount() const;
	const QVector<QPointF> &GetPoints() const;

	bool GetZoomPoint(QPointF *pts, unsigned index, qreal offset) const;
	bool GetZoomPoints(QPointF *pts, qreal offset) const;

private:
	static bool EraseStraightPointsOnPolygon(QVector<QPointF>& pts);

	static void GetEdgeOffsetPts(const QPointF& pt0, const QPointF& pt1, qreal offset,
		QPointF& opt0, QPointF& opt1);

private:
	QVector<QPointF> m_pts;
	bool m_bClosed;
	bool m_bClockWise;
};

class QPathZoomer
{
public:
	explicit QPathZoomer(qreal offset = 0);
	void SetOffset(qreal offset);
	qreal GetOffSet() const;
	void ZoomPath(const QPainterPath& path2zoom, QPainterPath& pathAfterZoom, const qreal flatness) const;

private:
	qreal m_offset;
};


class QPathDasher
{	
public:	
	QPathDasher();

	qreal GetWidth() const;
	int GetDashPatternCount() const;
	bool GetDashPattern(qreal *dashArray, int count) const;
	qreal GetDashOffset() const;

	bool SetDashPattern(const qreal* dasharray, int count);
	void SetDashOffset(qreal dashoffset);
	void SetWidth(qreal width);
	
	void GetDashedPath(const QPainterPath& path2dash,  QPainterPath& dashedPath, const qreal flatness) const;
	void GetDashedPath(const QPainterPath& path2dash,  QPainterPath& openStartPath,
		QPainterPath& openMiddleAndClosePath, QPainterPath& openEndPath, const qreal flatness) const;
	
private:
	void GetDashedPath(const QPolygonF& subPath, QPainterPath& outPath) const;

	void GenerateDash(const QVertices& vertices, QPainterPath& outPath) const;
	
	static void CalcDashStart(qreal& currentDashLen, int& currentDashIndex, const QVector<qreal>& pattern, qreal dashOffset, qreal width);
	static void IncCurrentDash(int& currentDashIndex, qreal& currentDashLen, const QVector<qreal>& pattern, qreal width);
	static void AddStart(const vertex_dist& v0, const vertex_dist& v1, qreal dist, QPainterPath& outPath);
	static void AddPoint(const vertex_dist& v0, const vertex_dist& v1, qreal dist, QPainterPath& outPath);
	
private:
	qreal m_dashOffset;	
	QVector<qreal> m_dashPattern;

	qreal m_width;
};

class QComplexStrokerPrivate
{
public:
    QComplexStrokerPrivate();
    bool isSolid() const;
    qreal alignmentOffset();

    QMathStroker *prepareMathStroker();
    QPathDasher *prepareDasher();

    QAtomicInt ref;
    qreal width;
    QVector<qreal> dashPattern;
    Qt::PenJoinStyle joinStyle;
    qreal dashOffset;
    qreal miterLimit;
    QVector<qreal> compoundArray;
    Qt::PenAnchorStyle startAnchorStyle;
    QCustomLineAnchor startAnchor;
    Qt::PenAnchorStyle endAnchorStyle;
    QCustomLineAnchor endAnchor;
    Qt::PenAlignment alignment;
    Qt::PenCapStyle startCap;
    Qt::PenCapStyle endCap;
    Qt::PenCapStyle dashCap;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCOMPLEXSTROKER_P_H