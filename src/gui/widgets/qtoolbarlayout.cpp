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

#include <qaction.h>
#include <qwidgetaction.h>
#include <qtoolbar.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qdebug.h>
#include <qmath.h>
#include <qtoolbaritemwidget.h>

#include "qmainwindowlayout_p.h"
#include "qtoolbarextension_p.h"
#include "qtoolbarlayout_p.h"
#include "qtoolbarseparator_p.h"

#ifndef QT_NO_TOOLBAR

QT_BEGIN_NAMESPACE

// qmainwindow.cpp
extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

/******************************************************************************
** QToolBarTitleButton
*/

class QToolBarTitleButton : public QAbstractButton
{
public:
    QToolBarTitleButton(QToolBar *toolbar);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

QToolBarTitleButton::QToolBarTitleButton(QToolBar *toolbar)
    : QAbstractButton(toolbar)
{
    setFocusPolicy(Qt::NoFocus);
#ifndef QT_NO_CURSOR
	setCursor(Qt::ArrowCursor);
#endif
}

void QToolBarTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QToolBarTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QToolBarTitleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOptionToolButton opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;

    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;

    opt.icon = icon();
    opt.subControls = 0;
    opt.activeSubControls = 0;
    opt.features = QStyleOptionToolButton::None;
    opt.arrowType = Qt::NoArrow;
    opt.iconSize = opt.icon.actualSize(QSize(10, 10));
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}

/******************************************************************************
** QToolBarItem
*/

QToolBarItem::QToolBarItem(QWidget *widget)
    : QWidgetItem(widget), action(0), customWidget(false)
{
}

bool QToolBarItem::isEmpty() const
{
    return action == 0 || !action->isVisible();
}

QSize QToolBarItem::sizeHint(Qt::Orientation o) const
{
    QSize s(0, 0);
    if (!isEmpty())
	{
		const QToolBarItemWidget* item = dynamic_cast<const QToolBarItemWidget*>(wid);
		if (item)
		{
			s = item->sizeHint(o)
				.expandedTo(item->minimumSizeHint(o))
				.boundedTo(item->maximumSize(o))
				.expandedTo(item->minimumSize(o));

			s = layoutItemSize(s);
			if (wid->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored)
				s.setWidth(0);
			if (wid->sizePolicy().verticalPolicy() == QSizePolicy::Ignored)
				s.setHeight(0);
		}
		else
		{
			s = QWidgetItem::sizeHint();
		}
    }

	return s;
}

QSize QToolBarItem::minimumSize(Qt::Orientation o) const
{
	QSize s(0, 0);
	if (!isEmpty())
	{
		const QToolBarItemWidget* item = dynamic_cast<const QToolBarItemWidget*>(wid);
		if (item)
		{
			s = qSmartMinSize(item->sizeHint(o),
							  item->minimumSizeHint(o),
							  item->minimumSize(o),
							  item->maximumSize(o),
							  wid->sizePolicy());
			s = layoutItemSize(s);
		}
		else
		{
			s = QWidgetItem::minimumSize();
		}
	}

	return s;
}

QSize QToolBarItem::maximumSize(Qt::Orientation o) const
{
	QSize s(0, 0);
	if (!isEmpty())
	{
		const QToolBarItemWidget* item = dynamic_cast<const QToolBarItemWidget*>(wid);
		if (item)
		{
			s = qSmartMaxSize(item->sizeHint(o).expandedTo(item->minimumSizeHint(o)),
							  item->minimumSize(o),
							  item->maximumSize(o),
							  wid->sizePolicy(),
							  align);
			s = layoutItemSize(s);
		}
		else
		{
			s = QWidgetItem::maximumSize();
		}
	}

	return s;
}

void QToolBarItem::setGeometry(Qt::Orientation o, const QRect& rc)
{
	QToolBarItemWidget* item = dynamic_cast<QToolBarItemWidget*>(wid);
	if (item)
		item->setOrientation(o);

	if (o == Qt::Vertical)
	{
		QSize s = rc.size().boundedTo(maximumSize(o));
		int x = rc.x();
		if (align & Qt::AlignBottom)
		{
			x += (rc.width() - s.width());
		}
		else if (!(align & Qt::AlignTop))
		{
			x += (rc.width() - s.width()) / 2;
		}

		QRect rc2(x, rc.y(), s.width(), s.height());
		QWidgetItem::setGeometry(rc2);
	}
	else
	{
		QWidgetItem::setGeometry(rc);
	}
}

/******************************************************************************
** QToolBarLayout
*/

QToolBarLayout::QToolBarLayout(QWidget *parent)
    : QLayout(parent), expanded(false), animating(false), dirty(true),
        expanding(false), empty(true), expandFlag(false), popupMenu(0)
{
    QToolBar *tb = qobject_cast<QToolBar*>(parent);
    if (!tb)
        return;

    extension = new QToolBarExtension(tb);
    extension->setFocusPolicy(Qt::NoFocus);
    extension->hide();
    QObject::connect(tb, SIGNAL(orientationChanged(Qt::Orientation)),
                     extension, SLOT(setOrientation(Qt::Orientation)));

	closebutton = new QToolBarTitleButton(tb);
    QObject::connect(closebutton, SIGNAL(clicked()), tb, SLOT(close()));
	closebutton->setVisible(tb->isClosable() && tb->isFloating());
	closebutton->setIcon(tb->style()->standardIcon(QStyle::SP_TitleBarCloseButton));

    setUsePopupMenu(qobject_cast<QMainWindow*>(tb->parentWidget()) == 0);
}

QToolBarLayout::~QToolBarLayout()
{
    while (!items.isEmpty()) {
        QToolBarItem *item = items.takeFirst();
        if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(item->action)) {
            if (item->customWidget)
                widgetAction->releaseWidget(item->widget());
        }
        delete item;
    }
}

void QToolBarLayout::updateMarginAndSpacing()
{
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return;
    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);
    setMargin(style->pixelMetric(QStyle::PM_ToolBarItemMargin, &opt, tb)
                + style->pixelMetric(QStyle::PM_ToolBarFrameWidth, &opt, tb));
    setSpacing(style->pixelMetric(QStyle::PM_ToolBarItemSpacing, &opt, tb));
}

bool QToolBarLayout::hasExpandFlag() const
{
    return expandFlag;
}

int QToolBarLayout::titleHeight() const
{
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
	return tb ? (tb->fontMetrics().height() + 2) : 0;
}

QSize QToolBarLayout::getMargins(Qt::Orientation o, bool bFloating) const
{
	QSize s;
	QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
	if (tb)
	{
		QStyle *style = tb->style();
		QStyleOptionToolBar opt;
		tb->initStyleOption(&opt);

		const int margin = bFloating ? style->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb) : this->margin();
		s = QSize(margin * 2, margin * 2);
		if (bFloating)
			s.rheight() += titleHeight();
		else if (movable())
			rpick(o, s) += style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, tb);
	}

	return s;
}

QRect QToolBarLayout::dragRect() const
{
	QRect r;
	QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (tb)
	{
		if (tb->isFloating())
		{
			const int margin = tb->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb);
			r = tb->rect();
			r.adjust(margin, margin, -margin, -margin);
			int height = tb->fontMetrics().height() + 2;
			if (tb->isClosable())
				r.setWidth(r.width() - height);
			r.setHeight(height);
		}
		else if (tb->isMovable())
		{
			QStyleOptionToolBar opt;
			tb->initStyleOption(&opt);
			r = tb->style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, tb);
		}
	}

	return r;
}

void QToolBarLayout::setUsePopupMenu(bool set)
{
    if (!dirty && ((popupMenu == 0) == set))
        invalidate();
    if (!set) {
        QObject::connect(extension, SIGNAL(clicked(bool)),
                        this, SLOT(setExpanded(bool)), Qt::UniqueConnection);
        extension->setPopupMode(QToolButton::DelayedPopup);
        extension->setMenu(0);
        delete popupMenu;
        popupMenu = 0;
    } else {
        QObject::disconnect(extension, SIGNAL(clicked(bool)),
                            this, SLOT(setExpanded(bool)));
        extension->setPopupMode(QToolButton::InstantPopup);
        if (!popupMenu) {
            popupMenu = new QMenu(extension);
        }
        extension->setMenu(popupMenu);
    }
}

void QToolBarLayout::checkUsePopupMenu()
{
    QToolBar *tb = static_cast<QToolBar *>(parent());
    QMainWindow *mw = qobject_cast<QMainWindow *>(tb->parent());
    Qt::Orientation o = tb->orientation();
    setUsePopupMenu(!mw || tb->isFloating() || perp(o, expandedSize(mw->size())) >= perp(o, mw->size()));
}

void QToolBarLayout::addItem(QLayoutItem*)
{
    qWarning() << "QToolBarLayout::addItem(): please use addAction() instead";
    return;
}

QLayoutItem *QToolBarLayout::itemAt(int index) const
{
    if (index < 0 || index >= items.count())
        return 0;
    return items.at(index);
}

QLayoutItem *QToolBarLayout::takeAt(int index)
{
    if (index < 0 || index >= items.count())
        return 0;
    QToolBarItem *item = items.takeAt(index);

    if (popupMenu)
        popupMenu->removeAction(item->action);

    QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(item->action);
    if (widgetAction != 0 && item->customWidget) {
        widgetAction->releaseWidget(item->widget());
    } else {
        // destroy the QToolButton/QToolBarSeparator
        item->widget()->hide();
        item->widget()->deleteLater();
    }

    invalidate();
    return item;
}

void QToolBarLayout::insertAction(int index, QAction *action)
{
    index = qMax(0, index);
    index = qMin(items.count(), index);

    QToolBarItem *item = createItem(action);
    if (item) {
        items.insert(index, item);
        invalidate();
    }
}

int QToolBarLayout::indexOf(QAction *action) const
{
    for (int i = 0; i < items.count(); ++i) {
        if (items.at(i)->action == action)
            return i;
    }
    return -1;
}

int QToolBarLayout::count() const
{
    return items.count();
}

bool QToolBarLayout::isEmpty() const
{
    if (dirty)
        updateGeomArray();
    return empty;
}

void QToolBarLayout::invalidate()
{
    dirty = true;
    QLayout::invalidate();
}

Qt::Orientations QToolBarLayout::expandingDirections() const
{
    if (dirty)
        updateGeomArray();
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return Qt::Orientations(0);
    Qt::Orientation o = tb->orientation();
    return expanding ? Qt::Orientations(o) : Qt::Orientations(0);
}

bool QToolBarLayout::movable() const
{
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return false;
    QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget());
    return tb->isMovable() && win != 0;
}

void QToolBarLayout::updateGeomArray() const
{
    if (!dirty)
        return;

    QToolBarLayout *that = const_cast<QToolBarLayout*>(this);

    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return;

    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);

	const Qt::Orientation o = tb->orientation();
	const bool bFloating = tb->isFloating() && o == Qt::Horizontal;
	const int nToolBarHandleExtent = style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, tb);
	const int topMargin = bFloating ? titleHeight() : 0;
    const int handleExtent = movable() && !bFloating ? nToolBarHandleExtent : 0;
	const int margin = bFloating ? style->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb) : this->margin();
    const int spacing = this->spacing();
    const int extensionExtent = style->pixelMetric(QStyle::PM_ToolBarExtensionExtent, &opt, tb);

    that->minSize = QSize(0, 0);
    that->hint = QSize(0, 0);
    rperp(o, that->minSize) = nToolBarHandleExtent;
    rperp(o, that->hint) = nToolBarHandleExtent;

    that->expanding = false;
    that->empty = false;

    QVector<QLayoutStruct> a(items.count() + 1); // + 1 for the stretch

    int count = 0;
    for (int i = 0; i < items.count(); ++i)
	{
        QToolBarItem *item = items.at(i);

        QSize max = item->maximumSize(o);
        QSize min = item->minimumSize(o);
        QSize hint = item->sizeHint(o);
        Qt::Orientations exp = item->expandingDirections();
        bool empty = item->isEmpty();

        that->expanding = expanding || exp & o;

        if (item->widget())
		{
            if ((item->widget()->sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag))
			{
                that->expandFlag = true;
            }
        }

        if (!empty)
		{
			if (!bFloating)
			{
            if (count == 0) // the minimum size only displays one widget
                rpick(o, that->minSize) += pick(o, min);
            int s = perp(o, minSize);
            rperp(o, that->minSize) = qMax(s, perp(o, min));
			}

            //we only add spacing before item (ie never before the first one)
            rpick(o, that->hint) += (count == 0 ? 0 : spacing) + pick(o, hint);
            int s = perp(o, that->hint);
            rperp(o, that->hint) = qMax(s, perp(o, hint));
            ++count;
        }

        a[i].sizeHint = pick(o, hint);
        a[i].maximumSize = pick(o, max);
        a[i].minimumSize = pick(o, min);
        a[i].expansive = exp & o;
        if (o == Qt::Horizontal)
            a[i].stretch = item->widget()->sizePolicy().horizontalStretch();
        else
            a[i].stretch = item->widget()->sizePolicy().verticalStretch();
        a[i].empty = empty;
    }

    that->geomArray = a;
    that->empty = count == 0;

	QSize margins(2 * margin, 2 * margin);
	rpick(o, margins) += handleExtent;
	rperp(o, margins) += topMargin;
    that->hint += margins;
	if (bFloating)
	{
		that->minSize = that->hint;
		that->updateDockSize();
	}
	else
	{
		that->minSize += margins;
    if (items.count() > 1)
        rpick(o, that->minSize) += spacing + extensionExtent;
	}

#ifdef Q_WS_MAC
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget()->parentWidget())) {
        if (mw->unifiedTitleAndToolBarOnMac()
                && mw->toolBarArea(static_cast<QToolBar *>(parentWidget())) == Qt::TopToolBarArea) {
            if (expandFlag) {
                tb->setMaximumSize(0xFFFFFF, 0xFFFFFF);
            } else {
               tb->setMaximumSize(hint);
            }
        }
    }
#endif

    that->dirty = false;
}

void QToolBarLayout::updateDockSize()
{
	QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return;

    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);

	const int nToolBarHandleExtent = style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, tb);
    const int handleExtent = movable() ? nToolBarHandleExtent : 0;
	const int margin = this->margin() * 2;
    const int spacing = this->spacing();
    const int extensionExtent = style->pixelMetric(QStyle::PM_ToolBarExtensionExtent, &opt, tb);

	dockHint = dockMinSize = QSize(0, nToolBarHandleExtent);
	dockHintVert = dockMinSizeVert = QSize(nToolBarHandleExtent, 0);

    int count = 0;
    for (int i = 0; i < items.count(); ++i)
	{
        QToolBarItem *item = items.at(i);
        bool empty = item->isEmpty();

        if (!empty)
		{
			QSize min = item->minimumSize(Qt::Horizontal);
			QSize minV = item->minimumSize(Qt::Vertical);
            if (count == 0) // the minimum size only displays one widget
			{
				dockMinSize.rwidth() += min.width() + spacing + extensionExtent;
				dockMinSizeVert.rheight() += min.height() + spacing + extensionExtent;
			}
			//int s = dockMinSize.rheight();// perp(o, minSize);
			dockMinSize.rheight() = qMax(dockMinSize.height(), min.height());
			dockMinSizeVert.rwidth() = qMax(dockMinSizeVert.width(), minV.width());

			//we only add spacing before item (ie never before the first one)
			QSize hint = item->sizeHint(Qt::Horizontal);
			QSize hintV = item->sizeHint(Qt::Vertical);
			int space = count == 0 ? 0 : spacing;
			dockHint.rwidth() += space + hint.width();
			dockHintVert.rheight() += space + hintV.height();

			dockHint.rheight() = qMax(dockHint.height(), hint.height());
			dockHintVert.rwidth() = qMax(dockHintVert.width(), hintV.width());
 			++count;
       }
    }

	QSize margins(margin + handleExtent, margin);
    dockHint += margins;
    dockMinSize += margins;

	margins.transpose();
    dockHintVert += margins;
    dockMinSizeVert += margins;
}

static bool defaultWidgetAction(QToolBarItem *item)
{
    QWidgetAction *a = qobject_cast<QWidgetAction*>(item->action);
    return a != 0 && a->defaultWidget() == item->widget();
}

void QToolBarLayout::setGeometry(const QRect &rect)
{
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return;
    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);
	const Qt::Orientation o = tb->orientation();
	const bool bFloating = tb->isFloating() && o == Qt::Horizontal;
	const int topMargin = bFloating ? titleHeight() : 0;
	const int margin = bFloating ? style->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb) : this->margin();
    const int extensionExtent = style->pixelMetric(QStyle::PM_ToolBarExtensionExtent, &opt, tb);

    QLayout::setGeometry(rect);

    bool ranOutOfSpace = false;
    if (!animating)
        ranOutOfSpace = layoutActions(rect.size());

    if (expanded || animating || ranOutOfSpace) {
        Qt::ToolBarArea area = Qt::TopToolBarArea;
        if (QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget()))
            area = win->toolBarArea(tb);
        QSize hint = sizeHint();

        QPoint pos;
        rpick(o, pos) = pick(o, rect.bottomRight()) - margin - extensionExtent + 2;
        if (area == Qt::LeftToolBarArea || area == Qt::TopToolBarArea)
            rperp(o, pos) = perp(o, rect.topLeft()) + margin + topMargin;
        else
            rperp(o, pos) = perp(o, rect.bottomRight()) - margin - topMargin - (perp(o, hint) - 2*margin) + 1;
        QSize size;
        rpick(o, size) = extensionExtent;
        rperp(o, size) = perp(o, hint) - 2*margin - topMargin;
        QRect r(pos, size);

        if (o == Qt::Horizontal)
            r = QStyle::visualRect(parentWidget()->layoutDirection(), rect, r);

        extension->setGeometry(r);

        if (extension->isHidden())
            extension->show();
    } else {
        if (!extension->isHidden())
            extension->hide();
    }

	if (tb->isClosable() && bFloating)
	{
		QRect rcClose(rect.width() - margin - topMargin, margin, topMargin, topMargin);
		closebutton->setGeometry(rcClose);
        if (closebutton->isHidden())
            closebutton->show();
	}
	else
	{
        if (!closebutton->isHidden())
            closebutton->hide();
	}

	if (bFloating)
	{
		QSize hint = tb->sizeHint();
		if (hint != rect.size())
			tb->resize(hint);
	}

#ifdef Q_WS_MAC
    // Nothing to do for Carbon... probably   
#  ifdef QT_MAC_USE_COCOA
    if (QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget())) {
        Qt::ToolBarArea area = win->toolBarArea(tb);
        if (win->unifiedTitleAndToolBarOnMac() && area == Qt::TopToolBarArea) {
            qt_mainwindow_layout(win)->fixSizeInUnifiedToolbar(tb);
        }
    }
#  endif
#endif
    
}

bool QToolBarLayout::layoutActions(const QSize &size)
{
    if (dirty)
        updateGeomArray();

    QRect rect(0, 0, size.width(), size.height());

    QList<QWidget*> showWidgets, hideWidgets;

    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return false;
    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);
	const Qt::Orientation o = tb->orientation();
	const bool bFloating = tb->isFloating() && o == Qt::Horizontal;
	const int topMargin = bFloating ? titleHeight() : 0;
    const int handleExtent = movable() && !bFloating
            ? style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, tb) : 0;
	const int margin = bFloating ? style->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb) : this->margin();
    const int spacing = this->spacing();
    const int extensionExtent = style->pixelMetric(QStyle::PM_ToolBarExtensionExtent, &opt, tb);
    bool extensionMenuContainsOnlyWidgetActions = true;

    int space = pick(o, rect.size()) - 2*margin - handleExtent;
    if (space <= 0)
        return false;  // nothing to do.

    if (popupMenu)
        popupMenu->clear();

    bool ranOutOfSpace = false;
    int rows = 0;
    int rowPos = perp(o, rect.topLeft()) + margin + topMargin;
    int i = 0;
    while (i < items.count()) {
        QVector<QLayoutStruct> a = geomArray;

        int start = i;
        int size = 0;
        int prev = -1;
        int rowHeight = 0;
        int count = 0;
        int maximumSize = 0;
        bool expansiveRow = false;
        for (; i < items.count(); ++i) {
            if (a[i].empty)
                continue;

            int newSize = size + (count == 0 ? 0 : spacing) + a[i].minimumSize;
            if (prev != -1 && newSize > space) {
                if (rows == 0)
                    ranOutOfSpace = true;
                // do we have to move the previous item to the next line to make space for
                // the extension button?
                if (count > 1 && size + spacing + extensionExtent > space)
                    i = prev;
                break;
            }

            if (expanded)
                rowHeight = qMax(rowHeight, perp(o, items.at(i)->sizeHint(o)));
            expansiveRow = expansiveRow || a[i].expansive;
            size = newSize;
            maximumSize += spacing + (a[i].expansive ? a[i].maximumSize : a[i].smartSizeHint());
            prev = i;
            ++count;
        }

        // stretch at the end
        a[i].sizeHint = 0;
        a[i].maximumSize = QWIDGETSIZE_MAX;
        a[i].minimumSize = 0;
        a[i].expansive = true;
        a[i].stretch = 0;
        a[i].empty = true;

        if (expansiveRow && maximumSize < space) {
            expansiveRow = false;
            a[i].maximumSize = space - maximumSize;
        }

        qGeomCalc(a, start, i - start + (expansiveRow ? 0 : 1), 0,
                    space - (ranOutOfSpace ? (extensionExtent + spacing) : 0),
                    spacing);

        for (int j = start; j < i; ++j) {
            QToolBarItem *item = items.at(j);

            if (a[j].empty) {
                if (!item->widget()->isHidden())
                    hideWidgets << item->widget();
                continue;
            }

            QPoint pos;
            rpick(o, pos) = margin + handleExtent + a[j].pos;
            rperp(o, pos) = rowPos;
            QSize size;
            rpick(o, size) = a[j].size;
            if (expanded)
                rperp(o, size) = rowHeight;
            else
                rperp(o, size) = perp(o, rect.size()) - 2*margin - topMargin;
            QRect r(pos, size);

            if (o == Qt::Horizontal)
                r = QStyle::visualRect(parentWidget()->layoutDirection(), rect, r);

            item->setGeometry(o, r);

            if (item->widget()->isHidden())
                showWidgets << item->widget();
        }

        if (!expanded) {
            for (int j = i; j < items.count(); ++j) {
                QToolBarItem *item = items.at(j);
                if (!item->widget()->isHidden())
                    hideWidgets << item->widget();
                if (popupMenu) {
                    if (!defaultWidgetAction(item)) {
                        popupMenu->addAction(item->action);
                        extensionMenuContainsOnlyWidgetActions = false;
                    }
                }
            }
            break;
        }

        rowPos += rowHeight + spacing;
        ++rows;
    }

    // if we are using a popup menu, not the expadning toolbar effect, we cannot move custom
    // widgets into the menu. If only custom widget actions are chopped off, the popup menu
    // is empty. So we show the little extension button to show something is chopped off,
    // but we make it disabled.
    extension->setEnabled(popupMenu == 0 || !extensionMenuContainsOnlyWidgetActions);

    // we have to do the show/hide here, because it triggers more calls to setGeometry :(
    for (int i = 0; i < showWidgets.count(); ++i)
        showWidgets.at(i)->show();
    for (int i = 0; i < hideWidgets.count(); ++i)
        hideWidgets.at(i)->hide();

    return ranOutOfSpace;
}

QSize QToolBarLayout::expandedSize(const QSize &size) const
{
    if (dirty)
        updateGeomArray();

    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return QSize(0, 0);
    QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget());
    QStyle *style = tb->style();
    QStyleOptionToolBar opt;
    tb->initStyleOption(&opt);
	const Qt::Orientation o = tb->orientation();
	const bool bFloating = tb->isFloating() && o == Qt::Horizontal;
	const int topMargin = bFloating ? titleHeight() : 0;
    const int handleExtent = movable() && !bFloating
            ? style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, tb) : 0;
	const int margin = bFloating ? style->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, tb) : this->margin();
    const int spacing = this->spacing();
    const int extensionExtent = style->pixelMetric(QStyle::PM_ToolBarExtensionExtent, &opt, tb);

    int total_w = 0;
    int count = 0;
    for (int x = 0; x < items.count(); ++x) {
        if (!geomArray[x].empty) {
            total_w += (count == 0 ? 0 : spacing) + geomArray[x].minimumSize;
            ++count;
        }
    }
    if (count == 0)
        return QSize(0, 0);

    int min_w = pick(o, size);
    int rows = (int)qSqrt(qreal(count));
    if (rows == 1)
        ++rows;      // we want to expand to at least two rows
    int space = total_w/rows + spacing + extensionExtent;
    space = qMax(space, min_w - 2*margin - handleExtent);
    if (win != 0)
        space = qMin(space, pick(o, win->size()) - 2*margin - handleExtent);

    int w = 0;
    int h = topMargin;
    int i = 0;
    while (i < items.count()) {
        int count = 0;
        int size = 0;
        int prev = -1;
        int rowHeight = 0;
        for (; i < items.count(); ++i) {
            if (geomArray[i].empty)
                continue;

            int newSize = size + (count == 0 ? 0 : spacing) + geomArray[i].minimumSize;
            rowHeight = qMax(rowHeight, perp(o, items.at(i)->sizeHint(o)));
            if (prev != -1 && newSize > space) {
                if (count > 1 && size + spacing + extensionExtent > space) {
                    size -= spacing + geomArray[prev].minimumSize;
                    i = prev;
                }
                break;
            }

            size = newSize;
            prev = i;
            ++count;
        }

        w = qMax(size, w);
        h += rowHeight + spacing;
    }

    w += 2*margin + handleExtent + spacing + extensionExtent;
    w = qMax(w, min_w);
    if (win != 0)
        w = qMin(w, pick(o, win->size()));
    h += 2*margin - spacing; //there is no spacing before the first row

    QSize result;
    rpick(o, result) = w;
    rperp(o, result) = h;
    return result;
}

void QToolBarLayout::setExpanded(bool exp, bool bAnimating)
{
    if (exp == expanded)
        return;

    expanded = exp;
    extension->setChecked(expanded);

    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return;
    if (QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget())) {
        QMainWindowLayout *layout = qt_mainwindow_layout(win);
        if (expanded) {
            tb->raise();
        } else {
            QList<int> path = layout->layoutState.indexOf(tb);
            if (!path.isEmpty()) {
                QRect rect = layout->layoutState.itemRect(path);
                layoutActions(rect.size());
            }
        }
		layout->layoutState.toolBarAreaLayout.apply(bAnimating);
	}
}

void QToolBarLayout::setExpanded(bool exp)
{
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
	if (tb)
	{
		if (QMainWindow *win = qobject_cast<QMainWindow*>(tb->parentWidget())) {
#ifdef QT_NO_DOCKWIDGET
			animating = false;
#else
			animating = !tb->isWindow() && win->isAnimated();
#endif
		}
    }
	setExpanded(exp, animating);
}

QSize QToolBarLayout::minimumSize() const
{
    if (dirty)
        updateGeomArray();
    return minSize;
}

QSize QToolBarLayout::sizeHint() const
{
    if (dirty)
        updateGeomArray();
    return hint;
}

QSize QToolBarLayout::dockSizeHint(Qt::Orientation o) const
{
    if (dirty)
        updateGeomArray();

	return o == Qt::Horizontal ? dockHint : dockHintVert;
}

QSize QToolBarLayout::dockMinimumSize(Qt::Orientation o) const
{
    if (dirty)
        updateGeomArray();

	return o == Qt::Horizontal ? dockMinSize : dockMinSizeVert;
}

QToolBarItem *QToolBarLayout::createItem(QAction *action)
{
    bool customWidget = false;
    bool standardButtonWidget = false;
    QWidget *widget = 0;
    QToolBar *tb = qobject_cast<QToolBar*>(parentWidget());
    if (!tb)
        return (QToolBarItem *)0;

    if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action)) {
        widget = widgetAction->requestWidget(tb);
        if (widget != 0) {
            widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
            customWidget = true;
        }
    } else if (action->isSeparator()) {
        QToolBarSeparator *sep = new QToolBarSeparator(tb);
        connect(tb, SIGNAL(orientationChanged(Qt::Orientation)),
                sep, SLOT(setOrientation(Qt::Orientation)));
        widget = sep;
    }

    if (!widget) {
        QToolButton *button = new QToolButton(tb);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(tb->iconSize());
        button->setToolButtonStyle(tb->toolButtonStyle());
        QObject::connect(tb, SIGNAL(iconSizeChanged(QSize)),
                         button, SLOT(setIconSize(QSize)));
        QObject::connect(tb, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                         button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
        button->setDefaultAction(action);
        QObject::connect(button, SIGNAL(triggered(QAction*)), tb, SIGNAL(actionTriggered(QAction*)));
        widget = button;
        standardButtonWidget = true;
    }

    widget->hide();
    QToolBarItem *result = new QToolBarItem(widget);
    if (standardButtonWidget)
        result->setAlignment(Qt::AlignJustify);
    result->customWidget = customWidget;
    result->action = action;
    return result;
}

QT_END_NAMESPACE

#endif // QT_NO_TOOLBAR
