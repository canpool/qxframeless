/**
 * Copyright (C) 2023 maminjie <canpool@163.com>
 * SPDX-License-Identifier: MulanPSL-2.0
**/
#include "framelesshelper.h"
#include "framelesshelper_p.h"

#include <QWidget>
#include <QApplication>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QIcon>

#ifdef QX_FRAMELESS_NATIVE
#include <QWindow>
#endif // QX_FRAMELESS_NATIVE

#ifdef Q_OS_WINDOWS
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <QScreen>
#endif

QX_FRAMELESS_BEGIN_NAMESPACE


static int s_nBorderWidth = 5;
static int s_nTitleHeight = 30;

/* FramelessHelperPrivate */
FramelessHelperPrivate::FramelessHelperPrivate(QObject *parent)
    : QObject(parent)
{

}

FramelessHelperPrivate::~FramelessHelperPrivate()
{

}

class FramelessWidgetData
{
public:
    explicit FramelessWidgetData(FramelessHelperPrivate *_d, QWidget *widget);
    virtual ~FramelessWidgetData();

    QWidget *widget();
    virtual bool handleWidgetEvent(QEvent *event);

protected:
    FramelessHelperPrivate *d;
    QWidget *m_pWidget;
};


FramelessWidgetData::FramelessWidgetData(FramelessHelperPrivate *_d, QWidget *widget)
{
    Q_ASSERT(widget);
    d = _d;
    m_pWidget = widget;
}

FramelessWidgetData::~FramelessWidgetData()
{

}

QWidget *FramelessWidgetData::widget()
{
    return m_pWidget;
}

bool FramelessWidgetData::handleWidgetEvent(QEvent *event)
{
    Q_UNUSED(event);
    return false;
}


#ifdef QX_FRAMELESS_NATIVE

#ifdef Q_OS_WINDOWS

/* FramelessWidgetDataNativeWin */

class FramelessWidgetDataNativeWin : public FramelessWidgetData
{
public:
    explicit FramelessWidgetDataNativeWin(FramelessHelperPrivate *_d, QWidget *widget);
    virtual ~FramelessWidgetDataNativeWin();

    bool handleNativeWindowsMessage(MSG *msg, QXRESULT *result);

    bool handleNonClinetCalcSize(MSG *msg, QXRESULT *result);
    bool handleNonClientHitTest(MSG *msg, QXRESULT *result);
};

FramelessWidgetDataNativeWin::FramelessWidgetDataNativeWin(FramelessHelperPrivate *_d, QWidget *widget)
    : FramelessWidgetData(_d, widget)
{
    HWND hwnd = (HWND)m_pWidget->winId();
    DWORD style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
    ::SetWindowLongPtr(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
}

FramelessWidgetDataNativeWin::~FramelessWidgetDataNativeWin()
{

}

bool FramelessWidgetDataNativeWin::handleNativeWindowsMessage(MSG *msg, QXRESULT *result)
{
    switch (msg->message) {
    case WM_NCCALCSIZE:
        return handleNonClinetCalcSize(msg, result);
    case WM_NCHITTEST:
        return handleNonClientHitTest(msg, result);
    }

    return false;
}

bool FramelessWidgetDataNativeWin::handleNonClinetCalcSize(MSG *msg, QXRESULT *result)
{
    // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-nccalcsize
    *result = 0;
    // If wParam is FALSE, no need handle
    if (!msg->wParam) {
        return true;
    }
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-nccalcsize_params
    // If wParam is TRUE, lParam points to the NCCALCSIZE_PARAMS structure, which contains information
    // that the application can use to calculate the new size and position of the client rectangle.
    NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);

    const int originalTop = pncsp->rgrc[0].top;
    const RECT originalRect = pncsp->rgrc[0];

    // call default window proc to handle WM_NCCALCSIZE message
    const auto ret = ::DefWindowProc(msg->hwnd, WM_NCCALCSIZE, msg->wParam, msg->lParam);
    if (ret != 0) {
        *result = ret;
        return true;
    }
    pncsp->rgrc[0].top = originalTop;

    bool isMaximized = GetWindowStyle(msg->hwnd) & WS_MAXIMIZE;
    if (isMaximized) {
#ifdef SM_CXPADDEDBORDER
        int border = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
#else
        int border = GetSystemMetrics(SM_CXSIZEFRAME);
#endif
        pncsp->rgrc[0].top += border;
    } else {
        pncsp->rgrc[0] = originalRect;
    }
    return true;
}

bool FramelessWidgetDataNativeWin::handleNonClientHitTest(MSG *msg, QXRESULT *result)
{
    // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#appendix-c-hittestnca-function
    int borderWidth = s_nBorderWidth;

    long x = GET_X_LPARAM(msg->lParam);
    long y = GET_Y_LPARAM(msg->lParam);
    QPoint pos = m_pWidget->mapFromGlobal(QPoint(x, y));
    int w = m_pWidget->width();
    int h = m_pWidget->height();

    bool left = pos.x() < borderWidth;
    bool right = pos.x() > w - borderWidth;
    bool top = pos.y() < borderWidth;
    bool bottom = pos.y() > h - borderWidth;

    *result = HTNOWHERE;
    if (d->m_bWidgetResizable) {
        if (top && left) {
            *result = HTTOPLEFT;
        } else if (top && right) {
            *result = HTTOPRIGHT;
        } else if (bottom && left) {
            *result = HTBOTTOMLEFT;
        } else if (bottom && right) {
            *result = HTBOTTOMRIGHT;
        } else if (left) {
            *result = HTLEFT;
        } else if (right) {
            *result = HTRIGHT;
        } else if (top) {
            *result = HTTOP;
        } else if (bottom) {
            *result = HTBOTTOM;
        }
    }

    if (*result != HTNOWHERE) {
        return true;
    }

    if (d->m_bWidgetMovable && pos.y() < s_nTitleHeight) {
        QWidget *child = m_pWidget->childAt(pos);
        if (!child || qstrcmp(child->metaObject()->className(), "QWidget") == 0) {
            // A non-QWidget or non-QWidget-derived class in the title bar belongs to the blank area
            *result = HTCAPTION;
            return true;
        }
    }
    return false;
}

#endif // Q_OS_WINDOWS

#else // not QX_FRAMELESS_NATIVE

/* FramelessCursor */

class FramelessCursor
{
public:
    FramelessCursor();

    void reset();
    void update(const QPoint &gMousePos, const QRect &frameRect);

public:
    bool m_bOnEdges;
    bool m_bOnLeftEdge;
    bool m_bOnRightEdge;
    bool m_bOnTopEdge;
    bool m_bOnBottomEdge;
    bool m_bOnTopLeftEdge;
    bool m_bOnBottomLeftEdge;
    bool m_bOnTopRightEdge;
    bool m_bOnBottomRightEdge;
};

FramelessCursor::FramelessCursor()
{
    reset();
}

void FramelessCursor::reset()
{
    m_bOnEdges = false;
    m_bOnLeftEdge = false;
    m_bOnRightEdge = false;
    m_bOnTopEdge = false;
    m_bOnBottomEdge = false;
    m_bOnTopLeftEdge = false;
    m_bOnBottomLeftEdge = false;
    m_bOnTopRightEdge = false;
    m_bOnBottomRightEdge = false;
}

void FramelessCursor::update(const QPoint &gMousePos, const QRect &frameRect)
{
    int globalMouseX = gMousePos.x();
    int globalMouseY = gMousePos.y();
    int frameX       = frameRect.x();
    int frameY       = frameRect.y();
    int frameWidth   = frameRect.width();
    int frameHeight  = frameRect.height();

    m_bOnLeftEdge    = (globalMouseX >= frameX && globalMouseX <= frameX + s_nBorderWidth);
    m_bOnRightEdge   = (globalMouseX >= frameX + frameWidth - s_nBorderWidth &&
                        globalMouseX <= frameX + frameWidth);
    m_bOnTopEdge     = (globalMouseY >= frameY && globalMouseY <= frameY + s_nBorderWidth);
    m_bOnBottomEdge  = (globalMouseY >= frameY + frameHeight - s_nBorderWidth &&
                        globalMouseY <= frameY + frameHeight);

    m_bOnTopLeftEdge     = m_bOnTopEdge && m_bOnLeftEdge;
    m_bOnBottomLeftEdge  = m_bOnBottomEdge && m_bOnLeftEdge;
    m_bOnTopRightEdge    = m_bOnTopEdge && m_bOnRightEdge;
    m_bOnBottomRightEdge = m_bOnBottomEdge && m_bOnRightEdge;

    m_bOnEdges = m_bOnLeftEdge || m_bOnRightEdge || m_bOnTopEdge || m_bOnBottomEdge;
}


/* FramelessWidgetDataQt  */

class FramelessWidgetDataQt : public FramelessWidgetData
{
public:
    explicit FramelessWidgetDataQt(FramelessHelperPrivate *d, QWidget *widget);
    ~FramelessWidgetDataQt();

    bool handleWidgetEvent(QEvent *event) override;

private:
    void updateCursorShape(const QPoint &gMousePos);
    void resizeWidget(const QPoint &gMousePos);
    void moveWidget(const QPoint &gMousePos);

    bool handleMousePressEvent(QMouseEvent *event);
    bool handleMouseReleaseEvent(QMouseEvent *event);
    bool handleMouseMoveEvent(QMouseEvent *event);
    bool handleLeaveEvent(QEvent *event);
    bool handleHoverMoveEvent(QHoverEvent *event);
    bool handleDoubleClickedMouseEvent(QMouseEvent *event);

private:
    QPoint m_ptDragPos;
    FramelessCursor m_pressedMousePos;
    FramelessCursor m_moveMousePos;
    bool m_bLeftButtonPressed;
    bool m_bCursorShapeChanged;
    bool m_bLeftButtonTitlePressed;
    Qt::WindowFlags m_windowFlags;
};

FramelessWidgetDataQt::FramelessWidgetDataQt(FramelessHelperPrivate *_d, QWidget *widget)
    : FramelessWidgetData(_d, widget)
{
    m_bLeftButtonPressed = false;
    m_bCursorShapeChanged = false;
    m_bLeftButtonTitlePressed = false;

    m_windowFlags = m_pWidget->windowFlags();
    m_pWidget->setMouseTracking(true);
    m_pWidget->setAttribute(Qt::WA_Hover, true);
}

FramelessWidgetDataQt::~FramelessWidgetDataQt()
{
    m_pWidget->setMouseTracking(false);
    m_pWidget->setAttribute(Qt::WA_Hover, false);
}

bool FramelessWidgetDataQt::handleWidgetEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return handleMousePressEvent(static_cast<QMouseEvent *>(event));
    case QEvent::MouseButtonRelease:
        return handleMouseReleaseEvent(static_cast<QMouseEvent *>(event));
    case QEvent::MouseMove:
        return handleMouseMoveEvent(static_cast<QMouseEvent *>(event));
    case QEvent::Leave:
        return handleLeaveEvent(event);
    case QEvent::HoverMove:
        return handleHoverMoveEvent(static_cast<QHoverEvent *>(event));
    case QEvent::MouseButtonDblClick:
        return handleDoubleClickedMouseEvent(static_cast<QMouseEvent *>(event));
    default:
        break;
    }
    return false;
}

void FramelessWidgetDataQt::updateCursorShape(const QPoint &gMousePos)
{
    if (m_pWidget->isFullScreen() || m_pWidget->isMaximized()) {
        if (m_bCursorShapeChanged) {
            m_pWidget->unsetCursor();
        }
        return;
    }

    m_moveMousePos.update(gMousePos, m_pWidget->frameGeometry());

    if (m_moveMousePos.m_bOnTopLeftEdge || m_moveMousePos.m_bOnBottomRightEdge) {
        m_pWidget->setCursor(Qt::SizeFDiagCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnTopRightEdge || m_moveMousePos.m_bOnBottomLeftEdge) {
        m_pWidget->setCursor(Qt::SizeBDiagCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnLeftEdge || m_moveMousePos.m_bOnRightEdge) {
        m_pWidget->setCursor(Qt::SizeHorCursor);
        m_bCursorShapeChanged = true;
    } else if (m_moveMousePos.m_bOnTopEdge || m_moveMousePos.m_bOnBottomEdge) {
        m_pWidget->setCursor(Qt::SizeVerCursor);
        m_bCursorShapeChanged = true;
    } else {
        if (m_bCursorShapeChanged) {
            m_pWidget->unsetCursor();
            m_bCursorShapeChanged = false;
        }
    }
}

void FramelessWidgetDataQt::resizeWidget(const QPoint &gMousePos)
{
    QRect origRect = m_pWidget->frameGeometry();

    int left = origRect.left();
    int top = origRect.top();
    int right = origRect.right();
    int bottom = origRect.bottom();

    origRect.getCoords(&left, &top, &right, &bottom);

    int minWidth = m_pWidget->minimumWidth();
    int minHeight = m_pWidget->minimumHeight();

    if (m_pressedMousePos.m_bOnTopLeftEdge) {
        left = gMousePos.x();
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomLeftEdge) {
        left = gMousePos.x();
        bottom = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnTopRightEdge) {
        right = gMousePos.x();
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomRightEdge) {
        right = gMousePos.x();
        bottom = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnLeftEdge) {
        left = gMousePos.x();
    } else if (m_pressedMousePos.m_bOnRightEdge) {
        right = gMousePos.x();
    } else if (m_pressedMousePos.m_bOnTopEdge) {
        top = gMousePos.y();
    } else if (m_pressedMousePos.m_bOnBottomEdge) {
        bottom = gMousePos.y();
    }

    QRect newRect(QPoint(left, top), QPoint(right, bottom));

    if (newRect.isValid()) {
        if (minWidth > newRect.width()) {
            if (left != origRect.left()) {
                newRect.setLeft(origRect.left());
            } else {
                newRect.setRight(origRect.right());
            }
        }
        if (minHeight > newRect.height()) {
            if (top != origRect.top()) {
                newRect.setTop(origRect.top());
            } else {
                newRect.setBottom(origRect.bottom());
            }
        }

        m_pWidget->setGeometry(newRect);
    }
}

void FramelessWidgetDataQt::moveWidget(const QPoint &gMousePos)
{
    m_pWidget->move(gMousePos - m_ptDragPos);
}

bool FramelessWidgetDataQt::handleMousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftButtonPressed = true;
        m_bLeftButtonTitlePressed = event->pos().y() < s_nTitleHeight;

        QRect frameRect = m_pWidget->frameGeometry();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_pressedMousePos.update(event->globalPos(), frameRect);
        m_ptDragPos = event->globalPos() - frameRect.topLeft();
#else
        m_pressedMousePos.update(event->globalPosition().toPoint(), frameRect);
        m_ptDragPos = event->globalPosition().toPoint() - frameRect.topLeft();
#endif

        if (m_pressedMousePos.m_bOnEdges) {
            if (m_pWidget->isMaximized() || m_pWidget->isFullScreen()) {
                return false;
            }
        }
    }
    return false;
}

bool FramelessWidgetDataQt::handleMouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftButtonPressed = false;
        m_bLeftButtonTitlePressed = false;
        m_pressedMousePos.reset();
    }
    return false;
}

bool FramelessWidgetDataQt::handleMouseMoveEvent(QMouseEvent *event)
{
    if (m_bLeftButtonPressed) {
        if (d->m_bWidgetResizable && m_pressedMousePos.m_bOnEdges) {
            if (m_pWidget->isMaximized() || m_pWidget->isFullScreen()) {
                return false;
            }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            resizeWidget(event->globalPos());
#else
            resizeWidget(event->globalPosition().toPoint());
#endif
            return true;
        } else if (d->m_bWidgetMovable && m_bLeftButtonTitlePressed) {
            if (m_pWidget->isFullScreen()) {
                return false;
            }
            if (m_pWidget->isMaximized()) {
                QRect normalGeometry = m_pWidget->normalGeometry();
                m_pWidget->showNormal();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QPoint p = event->globalPos();
#else
                QPoint p = event->globalPosition().toPoint();
#endif
                p.ry() -= 10;
                p.rx() -= (normalGeometry.width() / 2);
                m_pWidget->move(p);
                m_ptDragPos = QPoint(normalGeometry.width() / 2, 10);
                return true;
            }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            moveWidget(event->globalPos());
#else
            moveWidget(event->globalPosition().toPoint());
#endif
            return true;
        }
        return false;
    } else if (d->m_bWidgetResizable) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        updateCursorShape(event->globalPos());
#else
        updateCursorShape(event->globalPosition().toPoint());
#endif
    }
    return false;
}

bool FramelessWidgetDataQt::handleLeaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (!m_bLeftButtonPressed) {
        m_pWidget->unsetCursor();
        return true;
    }
    return false;
}

bool FramelessWidgetDataQt::handleHoverMoveEvent(QHoverEvent *event)
{
    if (d->m_bWidgetResizable) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        updateCursorShape(m_pWidget->mapToGlobal(event->pos()));
#else
        updateCursorShape(m_pWidget->mapToGlobal(event->position().toPoint()));
#endif
    }
    return false;
}

bool FramelessWidgetDataQt::handleDoubleClickedMouseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_pWidget->windowFlags() & Qt::WindowMaximizeButtonHint) {
            bool titlePressed = event->pos().y() < s_nTitleHeight;
            if (titlePressed && !m_pWidget->isFullScreen()) {
                if (m_pWidget->isMaximized()) {
                    m_pWidget->showNormal();
                } else {
                    m_pWidget->showMaximized();
                }
                return true;
            }
        }
    }
    return false;
}

#endif // QX_FRAMELESS_NATIVE

/* FramelessHelper */
FramelessHelper::FramelessHelper(QObject *parent)
    : QObject(parent)
{
    QX_INIT_PRIVATE(FramelessHelper)
    if (parent) {
        if (QWidget *w = qobject_cast<QWidget *>(parent)) {
            w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            addWidget(w);
        }
    }
    qApp->installNativeEventFilter(this);
}

FramelessHelper::~FramelessHelper()
{
    Q_D(FramelessHelper);
    qApp->removeNativeEventFilter(this);
    QList<QWidget *> keys = d->m_widgetDataHash.keys();
    int size = keys.size();

    for (int i = 0; i < size; ++i) {
        keys[i]->removeEventFilter(this);
        delete d->m_widgetDataHash.take(keys[i]);
    }
    QX_FINI_PRIVATE()
}

void FramelessHelper::addWidget(QWidget *w)
{
    Q_D(FramelessHelper);
    if (!d->m_widgetDataHash.contains(w)) {
#ifdef QX_FRAMELESS_NATIVE
#ifdef Q_OS_WINDOWS
        FramelessWidgetDataNativeWin *data = new FramelessWidgetDataNativeWin(d, w);
#else
        FramelessWidgetData *data = new FramelessWidgetData(d, w);
#endif
#else // QX_FRAMELESS_NATIVE
        FramelessWidgetDataQt *data = new FramelessWidgetDataQt(d, w);
#endif
        d->m_widgetDataHash.insert(w, data);
        w->installEventFilter(this);
    }
}

void FramelessHelper::removeWidget(QWidget *w)
{
    Q_D(FramelessHelper);
    FramelessWidgetData *data = d->m_widgetDataHash.take(w);
    if (data) {
        w->removeEventFilter(this);
        delete data;
    }
}

bool FramelessHelper::widgetResizable() const
{
    Q_D(const FramelessHelper);
    return d->m_bWidgetResizable;
}

void FramelessHelper::setWidgetResizable(bool resizable)
{
    Q_D(FramelessHelper);
    d->m_bWidgetResizable = resizable;
}

bool FramelessHelper::widgetMovable() const
{
    Q_D(const FramelessHelper);
    return d->m_bWidgetMovable;
}

void FramelessHelper::setWidgetMovable(bool movable)
{
    Q_D(FramelessHelper);
    d->m_bWidgetMovable = movable;
}

int FramelessHelper::titleHeight() const
{
    return s_nTitleHeight;
}

void FramelessHelper::setTitleHeight(int height)
{
    if (height > 0) {
        s_nTitleHeight = height;
    }
}

int FramelessHelper::borderWidth() const
{
    return s_nBorderWidth;
}

void FramelessHelper::setBorderWidth(int width)
{
    if (width > 0) {
        s_nBorderWidth = width;
    }
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
{
    Q_D(FramelessHelper);
    QWidget *widget = static_cast<QWidget *>(object);
    FramelessWidgetData *data = d->m_widgetDataHash.value(widget);
    if (!data) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::Leave: {
        return data->handleWidgetEvent(event);
    }
    case QEvent::WindowTitleChange:
        emit windowTitleChanged(widget->windowTitle());
        return true;
    case QEvent::WindowIconChange:
        emit windowIconChanged(widget->windowIcon());
        return true;
    case QEvent::WindowStateChange:
        emit windowStateChanged(widget->windowState());
        return true;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

bool FramelessHelper::nativeEventFilter(const QByteArray &eventType, void *message, QXRESULT *result)
{
    Q_D(FramelessHelper);
#ifdef Q_OS_WINDOWS
    if (eventType == "windows_generic_MSG") {
        MSG *msg = reinterpret_cast<MSG *>(message);
        QWidget *widget = QWidget::find(reinterpret_cast<WId>(msg->hwnd));
        FramelessWidgetData *data = d->m_widgetDataHash.value(widget);
        if (data == nullptr) {
            return false;
        }
        switch (msg->message) {
            case WM_GETMINMAXINFO: {
                // prevent taskbar is covered when maximized
                if (widget->isMaximized()) {
                    const QRect rc = widget->screen()->availableGeometry();
                    MINMAXINFO *p = (MINMAXINFO *)(msg->lParam);
                    p->ptMaxPosition.x = 0;
                    p->ptMaxPosition.y = 0;
                    p->ptMaxSize.x = rc.width();
                    p->ptMaxSize.y = rc.height();
                    *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
                    return true;
                }
            }
        }
#ifdef QX_FRAMELESS_NATIVE
        FramelessWidgetDataNativeWin *winData = reinterpret_cast<FramelessWidgetDataNativeWin *>(data);
        if (winData == nullptr) {
            return false;
        }
        return winData->handleNativeWindowsMessage(msg, result);
#endif // QX_FRAMELESS_NATIVE
    }
#endif // Q_OS_WINDOWS

    return false;
}

QX_FRAMELESS_END_NAMESPACE
