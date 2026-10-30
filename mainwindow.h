#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "steprail.h"

class QScrollArea;
class QVBoxLayout;
class QPropertyAnimation;
class QEvent;

class SignInPage;
class RestaurantPage;
class PizzaPage;
class DeliveryPage;
class PaymentPage;
class ReviewPage;
class ProgressPage;
class ThankYouPage;

// Single vertically-scrolling page: every wizard step is a "section" stacked
// in one QScrollArea. The step rail stays fixed on the left and works as a
// table of contents. Sections below the current step stay hidden (locked)
// until the user reaches them with the Continue buttons, so validation still
// works exactly like in the old stacked wizard.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void goToPage(int index);
    void onStepClicked(int index);
    void refreshReview();
    void onScrolled();          // user scrolled -> keep rail in sync
    void onScrollFinished();    // programmatic scroll animation ended

private:
    void buildUi();
    void applyStyle();
    void addSection(QScrollArea *page);
    void updateSectionHeights();
    void scrollToSection(int index);
    QString composedOrderHtml() const;

    StepRail *m_rail = nullptr;

    // Scrolling content
    QScrollArea        *m_scroll   = nullptr;
    QWidget            *m_content  = nullptr;
    QVBoxLayout        *m_sections = nullptr;
    QList<QWidget*>     m_sectionWidgets;   // one per page, same order as steps
    QPropertyAnimation *m_scrollAnim = nullptr;

    // Pages (kept alive, hidden: they own the buttons' signals and timers)
    SignInPage      *m_signIn      = nullptr;
    RestaurantPage  *m_restaurant  = nullptr;
    PizzaPage       *m_pizza       = nullptr;
    DeliveryPage    *m_delivery    = nullptr;
    PaymentPage     *m_payment     = nullptr;
    ReviewPage      *m_review      = nullptr;
    ProgressPage    *m_progress    = nullptr;
    ThankYouPage    *m_thanks      = nullptr;

    int m_currentStep = 0;   // rail step (0..6)
    int m_viewIndex   = 0;   // section currently in view (0..7)
    int m_frontier    = 0;   // last section that is unlocked/visible
    int m_target      = 0;   // section the last programmatic scroll aimed at
};

#endif // MAINWINDOW_H

// #ifndef MAINWINDOW_H
// #define MAINWINDOW_H

// #include <QMainWindow>
// #include "animatedstack.h" // Included new stack
// #include "steprail.h"

// class SignInPage;
// class RestaurantPage;
// class PizzaPage;
// class DeliveryPage;
// class PaymentPage;
// class ReviewPage;
// class ProgressPage;
// class ThankYouPage;

// class MainWindow : public QMainWindow
// {
//     Q_OBJECT
// public:
//     explicit MainWindow(QWidget *parent = nullptr);
//     ~MainWindow() override;

// private slots:
//     void goToPage(int index);
//     void onStepClicked(int index);
//     void refreshReview();

// private:
//     void buildUi();
//     void applyStyle();
//     QString composedOrderHtml() const;

//     StepRail              *m_rail  = nullptr;
//     AnimatedStackedWidget *m_stack = nullptr; // Changed from QStackedWidget

//     SignInPage      *m_signIn      = nullptr;
//     RestaurantPage  *m_restaurant  = nullptr;
//     PizzaPage       *m_pizza       = nullptr;
//     DeliveryPage    *m_delivery    = nullptr;
//     PaymentPage     *m_payment     = nullptr;
//     ReviewPage      *m_review      = nullptr;
//     ProgressPage    *m_progress    = nullptr;
//     ThankYouPage    *m_thanks      = nullptr;

//     int m_currentStep = 0;
// };

// #endif // MAINWINDOW_H
