#ifndef STEPRAIL_H
#define STEPRAIL_H

#include <QWidget>
#include <QStringList>
#include <QVector>

// Vertical progress rail: circles + connecting line.
// Clicking a step that is <= currentStep emits stepClicked(index).
class StepRail : public QWidget
{
    Q_OBJECT
public:
    explicit StepRail(QWidget *parent = nullptr);

    void setSteps(const QStringList &labels);
    void setCurrentStep(int index);
    void setCompletedUpTo(int index);   // steps 0..index rendered as done

    QSize sizeHint() const override;

signals:
    void stepClicked(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QStringList m_labels;
    QVector<QRect> m_hitRects;   // clickable circle rects
    int m_current = 0;
    int m_completedUpTo = -1;

    int m_radius = 13;
    int m_rowHeight = 78;
    int m_leftMargin = 67;
    //int m_bottomMargin = 100;
};

#endif // STEPRAIL_H
