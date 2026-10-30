#include "steprail.h"

#include <QPainter>
#include <QMouseEvent>
#include <QFontMetrics>

StepRail::StepRail(QWidget *parent) : QWidget(parent)
{
    setMinimumWidth(230);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setCursor(Qt::PointingHandCursor);
    //setFocusPolicy(Qt::StrongFocus);
}

void StepRail::setSteps(const QStringList &labels)
{
    m_labels = labels;
    m_hitRects.clear();
    for (int i = 0; i < m_labels.size(); ++i) {
        int y = 55 + i * m_rowHeight;
        m_hitRects.push_back(QRect(m_leftMargin - m_radius, y,
                                   m_radius * 2, m_radius * 2));
    }
    updateGeometry();
    update();
}

void StepRail::setCurrentStep(int index)
{
    m_current = index;
    if (index > m_completedUpTo) m_completedUpTo = index - 1;
    update();
}

void StepRail::setCompletedUpTo(int index)
{
    m_completedUpTo = index;
    update();
}

QSize StepRail::sizeHint() const
{
    int h = 24 + m_labels.size() * m_rowHeight + 24;
    return QSize(230, h);
}

void StepRail::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#ffffff"));

    QFont f = font();
    f.setPointSizeF(f.pointSizeF() + 0.5);
    p.setFont(f);

    for (int i = 0; i < m_labels.size(); ++i) {
        const QRect r = m_hitRects[i];
        QPoint c = r.center();

        // Connector line to next circle
        if (i < m_labels.size() - 1) {
            QPoint next = m_hitRects[i + 1].center();
            QPen linePen(QColor("#d6d6d6"), 3);
            if (i < m_current) linePen.setColor(QColor("#4CAF50"));
            p.setPen(linePen);
            p.drawLine(c.x(), c.y() + m_radius, next.x(), next.y() - m_radius);
        }

        // Circle
        QColor fill = QColor("#e9ecef");
        QColor border = QColor("#c7ccd1");
        QColor text = QColor("#6c757d");

        if (i < m_current) {
            fill   = QColor("#4CAF50");
            border = QColor("#4CAF50");
            text   = Qt::white;
        } else if (i == m_current) {
            fill   = QColor("#2196F3");
            border = QColor("#2196F3");
            text   = Qt::white;
        }

        p.setPen(QPen(border, 2));
        p.setBrush(fill);
        p.drawEllipse(c, m_radius, m_radius);

        p.setPen(text);
        p.drawText(r, Qt::AlignCenter,
                   i < m_current ? QString::fromUtf8("\u2713")
                                 : QString::number(i + 1));

        // Label
        p.setPen(i == m_current ? QColor("#212529") : QColor("#495057"));
        QFont lf = f;
        if (i == m_current) lf.setBold(true);
        p.setFont(lf);

        QRect labelRect(m_leftMargin + m_radius + 14, r.top(),
                        width() - (m_leftMargin + m_radius + 14), r.height());
                        //m_rowHeight);
        //labelRect.moveTop(r.top() - 4);
        p.drawText(labelRect, Qt::AlignVCenter | Qt::TextWordWrap,
                   m_labels[i]);
        p.setFont(f);
    }
}

void StepRail::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < m_hitRects.size(); ++i) {
        // generous hit area: whole row
        QRect row(0, m_hitRects[i].top() - 12, width(), m_rowHeight);
        if (row.contains(event->pos()) && i <= m_current) {
            emit stepClicked(i);
            return;
        }
    }
}
