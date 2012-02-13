#ifndef QCUSTOMLINEANCHOR_P_H
#define QCUSTOMLINEANCHOR_P_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QVertices;
struct vertex_dist;

template <typename T>
class QUnshareVector
{
public:
	QUnshareVector()
		:m_size(0)
		,m_pData(NULL)
	{
	}
	inline int size() const
	{
		return m_size;
	}

	inline bool isEmpty() const
	{
		return m_size == 0;
	}

	inline void resize(int size)
	{
		Q_ASSERT(size >= 0);

		if (size > m_vec.size())
		{
			m_vec.resize(size);
			updatePointer();
		}
		m_size = size;
	}

	inline void reserve(int size)
	{
		if (m_vec.capacity() >= size)
			return;

		m_vec.reserve(size);
		updatePointer();
	}
	inline void push_back(const T &t)
	{
		Q_ASSERT(m_size <= m_vec.size());

		if (m_size == m_vec.size())
		{
			m_vec.push_back(t);
			updatePointer();
		}
		else
		{
			Q_ASSERT(NULL != m_pData);
			m_pData[m_size] = t;
		}

		m_size++;
	}
	inline T* begin()
	{
		return m_vec.begin();
	}
	inline const T* begin() const
	{
		return m_vec.constBegin();
	}
	inline T* end()
	{
		return m_vec.begin() + m_size;
	}
	inline const T* end() const
	{
		return m_vec.constBegin() + m_size;
	}
	inline const T* data() const
	{
		return m_pData;
	}
	inline const T* constData() const
	{
		return m_pData;
	}
	inline T* data()
	{
		return m_pData;
	}
	inline const T& first() const
	{
		Q_ASSERT(!isEmpty() && NULL != m_pData);

		return m_pData[0];
	}
	inline const T& last() const
	{
		Q_ASSERT(!isEmpty() && NULL != m_pData);

		return m_pData[m_size - 1];
	}
	inline operator const QPolygonF&()
	{
		poly.resize(m_size);
		memcpy(poly.data(), m_pData, sizeof(QPointF) * m_size);
		return poly;
	}
	inline const T& at(int i) const
	{
		Q_ASSERT(i < m_size && NULL != m_pData);

		return m_pData[i];
	}
	inline const T& operator[](int i) const
	{
		return at(i);
	}
	inline T& operator[](int i)
	{
		Q_ASSERT(i < m_size && NULL != m_pData);
		return m_pData[i];
	}
	inline QVector<T> &operator+=(const T &t)
	{
		push_back(t);
		return *this;
	}
	inline QVector<T> &operator<< (const T &t)
	{
		push_back(t);
		return *this;
	}
	inline QVector<T> &operator<<(const QVector<T> &l)
	{
		Q_ASSERT(l.size() > 0);

		const int newSize = m_size + l.size();
		if (newSize > m_vec.size())
		{
			m_vec.resize(newSize);
			updatePointer();
		}
		Q_ASSERT(NULL != m_pData);
		const T *p = l.constData();
		for (int i = m_size; i < newSize; i++)
			m_pData[i] = p[i - m_size];			

		m_size = newSize;
		return *this; 
	}
	inline QUnshareVector<T> &operator<<(const QUnshareVector<T> &l)
	{
		Q_ASSERT(l.size() > 0);

		const int newSize = m_size + l.size();
		if (newSize > m_vec.size())
		{
			m_vec.resize(newSize);
			updatePointer();
		}
		Q_ASSERT(NULL != m_pData);
		const T *p = l.constData();
		for (int i = m_size; i < newSize; i++)
			m_pData[i] = p[i - m_size];			

		m_size = newSize;
		return *this; 
	}
private:
	void updatePointer()
	{
		m_pData = m_vec.data();
	}

protected:
	int			m_size;
	QVector<T>	m_vec;		//it is not shareable
	T			*m_pData;	//points to the data of m_vec
	QPolygonF poly;
};

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
    bool CalcCrossYPts(const QPainterPath& path, QUnshareVector<qreal>& dists, qreal width = 1) const;
    void CopyTo(QCustomLineAnchorState& capState) const;
    static void CalcCrossYPt(const QPointF& pt1, const QPointF& pt2, int flag1, int flag2, QUnshareVector<qreal>& dists);

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