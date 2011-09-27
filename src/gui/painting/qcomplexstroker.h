#ifndef QCOMPLEXSTROKER_H
#define QCOMPLEXSTROKER_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QCustomLineAnchorState;
class QCustomLineAnchor;

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

    QPainterPath createStroke(const QPainterPath &path, const qreal flatness = 0.25) const;

private:
    void detach();

    QComplexStrokerPrivate *d_ptr;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCOMPLEXSTROKER_H