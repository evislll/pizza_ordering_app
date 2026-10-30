#ifndef ANIMATEDSTACK_H
#define ANIMATEDSTACK_H

#include <QStackedWidget>

class QResizeEvent;

class AnimatedStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit AnimatedStackedWidget(QWidget *parent = nullptr);

    void slideToIndex(int index);
    void slideToWidget(QWidget *widget);
    void setAnimationDuration(int ms);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int m_duration = 480;
    bool m_isAnimating = false;
};

#endif // ANIMATEDSTACK_H
