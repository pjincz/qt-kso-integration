#ifndef QCUSTOMLINEANCHOR_H
#define QCUSTOMLINEANCHOR_H

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

    void setFlatness(qreal flatness);

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

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCUSTOMLINEANCHOR_H