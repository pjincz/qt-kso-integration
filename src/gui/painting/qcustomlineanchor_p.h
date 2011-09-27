#ifndef QCUSTOMLINEANCHOR_P_H
#define QCUSTOMLINEANCHOR_P_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QVertices;
struct vertex_dist;

class QAnchorGenerator
{
public:
	QAnchorGenerator(const QCustomLineAnchor& startCap,
		             const QCustomLineAnchor& endCap,
					 qreal width);

	void Generate(const QPainterPath& path2generate,
		QPainterPath& pathAfterGenerate,
		QPainterPath& generatedCapPath,
        const qreal flatness) const;

private:
	static QCustomLineAnchor *GetLineCap(Qt::PenAnchorStyle as, const QCustomLineAnchor& customLineCap);

	// for open sub path
	void GenerateCap(const QPointF *pts, const unsigned count,
		QPainterPath& pathAfterGenerate,
		QPainterPath& generatedCapPath) const;

private:
	Q_DISABLE_COPY(QAnchorGenerator)
	qreal m_width;
    qreal m_flatness;
	
	const QCustomLineAnchorState* m_pCustomStartCap;
	const QCustomLineAnchorState* m_pCustomEndCap;
};

class QCustomLineAnchorState 
{
private:
    Q_DISABLE_COPY(QCustomLineAnchorState)

public:
    enum AnchorType {AnchorTypeFill, AnchorTypeStroke};

    QCustomLineAnchorState();
    virtual ~QCustomLineAnchorState();

    virtual QCustomLineAnchorState *Clone() const = 0;
    virtual qreal GetDevideDistance(qreal width) const = 0;
    virtual void GenerateCap(qreal width, const QPointF& fromPt, const QPointF& toPt,
        const QPointF& centerPt, QPainterPath& capPath) const = 0;
    virtual qreal GetMaxDistance(qreal width) const = 0;
    virtual QPainterPath GetCapPath() const = 0;
    virtual AnchorType GetAnchorType() const = 0;

    qreal baseInset() const;
    qreal widthScale() const;
    Qt::PenCapStyle strokeStartCap() const;
    void setStrokeStartCap(Qt::PenCapStyle startCap);
    Qt::PenCapStyle strokeEndCap() const;
    void setStrokeEndCap(Qt::PenCapStyle endCap);
    Qt::PenJoinStyle strokeJoin() const;
    Qt::PenCapStyle baseCap() const;
    void setBaseInset(qreal inset);
    void setWidthScale(qreal widthScale);
    void setStrokeJoin(Qt::PenJoinStyle lineJoin);
    void setBaseCap(Qt::PenCapStyle baseCap);
    qreal calcInsetScale(qreal penWidth) const;
    bool GetDevidePoint(qreal width, const QPointF *pts, unsigned count, QPointF &devidePt, int &prevPtOffset) const;

    void setFlatness(qreal flatness);

protected:
    void CalcTransform(qreal width, const QPointF& fromPt, const QPointF& toPt, const QPointF& centerPt, QMatrix& mtx) const;
    bool CalcCrossYPts(const QPainterPath& path, std::vector<qreal>& dists, qreal width = 1) const;
    void CopyTo(QCustomLineAnchorState& capState) const;
    static void CalcCrossYPt(const QPointF& pt1, const QPointF& pt2, int flag1, int flag2, std::vector<qreal>& dists);

protected:
    qreal m_baseInset;
    qreal m_widthScale;
    /*	MathStroker m_mathStroke;*/
    Qt::PenCapStyle m_strokeStartCap, m_strokeEndCap;
    Qt::PenJoinStyle m_strokeLineJoin;
    Qt::PenCapStyle	m_baseCap;
    qreal m_flatness;
};


class QCustomFillAnchor : public QCustomLineAnchorState
{
public:
    QCustomFillAnchor(const QPainterPath& path);
    virtual ~QCustomFillAnchor();
    virtual QCustomLineAnchorState *Clone() const;
    virtual qreal GetDevideDistance(qreal width) const;
    virtual void GenerateCap(qreal width, const QPointF& fromPt, const QPointF& toPt,
        const QPointF& centerPt, QPainterPath& capPath) const;
    virtual qreal GetMaxDistance(qreal width) const;
    virtual QPainterPath GetCapPath() const {return m_capPath;}
    virtual AnchorType GetAnchorType() const {return AnchorTypeFill;}

private:
    QPainterPath m_capPath;
};


class QCustomStrokeAnchor : public QCustomLineAnchorState
{
public:
    QCustomStrokeAnchor(const QPainterPath& path);
    virtual ~QCustomStrokeAnchor();
    virtual QCustomLineAnchorState *Clone() const;
    virtual qreal GetDevideDistance(qreal width) const;
    virtual void GenerateCap(qreal width, const QPointF& fromPt, const QPointF& toPt,
        const QPointF& centerPt, QPainterPath& capPath) const;
    virtual qreal GetMaxDistance(qreal width) const;
    virtual QPainterPath GetCapPath() const {return m_capPath;}
    virtual AnchorType GetAnchorType() const {return AnchorTypeStroke;}

private:
    QPainterPath m_capPath;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCUSTOMLINEANCHOR_P_H