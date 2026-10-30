#include "animatedstack.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QPixmap>
#include <QEasingCurve>
#include <QPointer>
#include <QWidget>
#include <QtGlobal>

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void AnimatedStackedWidget::setAnimationDuration(int ms)
{
    // Keep the duration in a useful range so transitions remain smooth.
    m_duration = qBound(150, ms, 1500);
}

void AnimatedStackedWidget::resizeEvent(QResizeEvent *event)
{
    QStackedWidget::resizeEvent(event);
}

void AnimatedStackedWidget::slideToIndex(int index)
{
    if (index < 0 || index >= count() || index == currentIndex() || m_isAnimating)
        return;

    slideToWidget(widget(index));
}

void AnimatedStackedWidget::slideToWidget(QWidget *targetWidget)
{
    if (!targetWidget || targetWidget == currentWidget() || m_isAnimating)
        return;

    const int oldIndex = currentIndex();
    const int newIndex = indexOf(targetWidget);
    QWidget *oldWidget = currentWidget();
    if (!oldWidget || newIndex < 0)
        return;

    const QRect area = contentsRect();
    if (area.width() <= 0 || area.height() <= 0) {
        setCurrentWidget(targetWidget);
        return;
    }

    m_isAnimating = true;

    // Take snapshots: QStackedWidget's layout controls its pages' geometry,
    // so animating the real pages directly can cause them to snap back.
    QPixmap oldPixmap(oldWidget->size());
    oldPixmap.fill(Qt::transparent);
    oldWidget->render(&oldPixmap);

    setCurrentWidget(targetWidget);
    targetWidget->ensurePolished();
    targetWidget->updateGeometry();

    QPixmap newPixmap(targetWidget->size());
    newPixmap.fill(Qt::transparent);
    targetWidget->render(&newPixmap);

    // Animate image overlays instead of the layout-managed pages.
    auto *oldLayer = new QLabel(this);
    auto *newLayer = new QLabel(this);
    for (QLabel *layer : {oldLayer, newLayer}) {
        layer->setAlignment(Qt::AlignCenter);
        layer->setScaledContents(true);
        layer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        layer->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    }
    oldLayer->setPixmap(oldPixmap);
    newLayer->setPixmap(newPixmap);

    auto *oldOpacity = new QGraphicsOpacityEffect(oldLayer);
    auto *newOpacity = new QGraphicsOpacityEffect(newLayer);
    oldLayer->setGraphicsEffect(oldOpacity);
    newLayer->setGraphicsEffect(newOpacity);

    const bool forward = newIndex > oldIndex;
    const int verticalOffset = qMax(50, area.height() / 3);

    // Vertical wheel/scroll transition:
    // forward navigation brings the next page up from below;
    // backward navigation brings the previous page down from above.
    // A subtle scale + fade gives the pages a little depth, like an iPhone picker.
    const int direction = forward ? 1 : -1;
    const QRect fullRect = area;
    const QRect incomingStart(
        area.x(),
        area.y() + direction * verticalOffset,
        qRound(area.width() * 0.95),
        qRound(area.height() * 0.95));
    const QRect outgoingEnd(
        area.x(),
        area.y() - direction * verticalOffset,
        qRound(area.width() * 0.97),
        qRound(area.height() * 0.97));

    oldLayer->setGeometry(fullRect);
    newLayer->setGeometry(incomingStart);
    oldLayer->show();
    newLayer->show();
    oldLayer->raise();
    newLayer->raise();

    auto *group = new QParallelAnimationGroup(this);

    auto addAnimation = [this, group](QObject *target,
                                      const QByteArray &property,
                                      const QVariant &start,
                                      const QVariant &end) {
        auto *animation = new QPropertyAnimation(target, property, group);
        animation->setDuration(m_duration);
        animation->setStartValue(start);
        animation->setEndValue(end);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        group->addAnimation(animation);
    };

    addAnimation(oldLayer, "geometry", fullRect, outgoingEnd);
    addAnimation(newLayer, "geometry", incomingStart, fullRect);
    addAnimation(oldOpacity, "opacity", 2.0, 0.0);
    addAnimation(newOpacity, "opacity", 0.0, 3.0);

    connect(group, &QParallelAnimationGroup::finished, this,
            [this, oldLayer, newLayer, group]() {
                oldLayer->deleteLater();
                newLayer->deleteLater();
                m_isAnimating = false;
                group->deleteLater();
            });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}
