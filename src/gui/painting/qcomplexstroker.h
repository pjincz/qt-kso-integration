#ifndef QCOMPLEXSTROKER_H
#define QCOMPLEXSTROKER_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QCustomLineAnchorState;
class QCustomLineAnchor;

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QCustomLineAnchor &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QCustomLineAnchor &);
#endif

class Q_GUI_EXPORT QCustomLineAnchor 
{
public:
    enum AnchorPathUsage {
        PathFill,
        PathStroke,
    };

    QCustomLineAnchor();
    QCustomLineAnchor(Qt::PenAnchorStyle anchor);
    QCustomLineAnchor(const QPainterPath &anchorPath, AnchorPathUsage usage, Qt::PenCapStyle baseCap = Qt::FlatCap, qreal baseInset = 0);
    QCustomLineAnchor(const QCustomLineAnchor &);
    ~QCustomLineAnchor();
    QCustomLineAnchor &operator=(const QCustomLineAnchor &);

    Qt::PenCapStyle strokeStartCap() const;
    void setStrokeStartCap(Qt::PenCapStyle startCap);
    Qt::PenCapStyle strokeEndCap() const;
    void setStrokeEndCap(Qt::PenCapStyle endCap);
    Qt::PenJoinStyle strokeJoin() const;
    void setStrokeJoin(Qt::PenJoinStyle lineJoin);

    Qt::PenCapStyle baseCap() const;
    void setBaseCap(Qt::PenCapStyle baseCap);
    qreal baseInset() const;
    void setBaseInset(qreal inset);

    qreal widthScale() const;	
    void setWidthScale(qreal widthScale);

    bool isValid() const;

    bool operator==(const QCustomLineAnchor &p) const;
    inline bool operator!=(const QCustomLineAnchor &p) const {return !(operator==(p));};

private:
    QCustomLineAnchor(QCustomLineAnchorState *capState);
    void reset(QCustomLineAnchorState *capState);
    static QCustomLineAnchorState *createLineCapState(
        const QPainterPath &anchorPath, AnchorPathUsage usage);

private:
    QCustomLineAnchorState *m_cap;

    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QCustomLineAnchor &);
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QCustomLineAnchor &);

public:
    typedef const QCustomLineAnchorState *DataPtr;
    inline DataPtr data_ptr() const { return m_cap; }
};


class QComplexStrokerPrivate;

class Q_GUI_EXPORT QComplexStroker
{
    Q_DECLARE_PRIVATE(QComplexStroker)
public:
    QComplexStroker();
    QComplexStroker(const QComplexStroker &);
    ~QComplexStroker();

    QComplexStroker &operator=(const QComplexStroker &);

    QVector<qreal> dashPattern() const;
    void setDashPattern(const QVector<qreal> &pattern);

    qreal dashOffset() const;
    void setDashOffset(qreal doffset);

    qreal miterLimit() const;
    void setMiterLimit(qreal limit);

    qreal width() const;
    void setWidth(qreal width);

    Qt::PenJoinStyle joinStyle() const;
    void setJoinStyle(Qt::PenJoinStyle pcs);

    QVector<qreal> compoundArray() const;
    void setCompoundArray(const QVector<qreal> &pattern);

    Qt::PenAnchorStyle startAnchorStyle() const;
    void setStartAnchorStyle(Qt::PenAnchorStyle);
    const QCustomLineAnchor &startAnchor() const;
    void setStartAnchor(const QCustomLineAnchor &);

    Qt::PenAnchorStyle endAnchorStyle() const;
    void setEndAnchorStyle(Qt::PenAnchorStyle);
    const QCustomLineAnchor &endAnchor() const;
    void setEndAnchor(const QCustomLineAnchor &);

    Qt::PenAlignment alignment() const;
    void setAlignment(Qt::PenAlignment alignment);

    Qt::PenCapStyle startCapStyle() const;
    void setStartCapStyle(Qt::PenCapStyle s);

    Qt::PenCapStyle endCapStyle() const;
    void setEndCapStyle(Qt::PenCapStyle s);

    Qt::PenCapStyle dashCapStyle() const;
    void setDashCapStyle(Qt::PenCapStyle s);

    QPainterPath createStroke(const QPainterPath &path) const;

private:
    void detach();

    QComplexStrokerPrivate *d_ptr;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCOMPLEXSTROKER_H