#include "mainwindow.h"
#include "pages.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFrame>
#include <QScrollArea>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QComboBox>
#include <QTimer>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Pizza Ordering — Wizard");
    buildUi();
    applyStyle();
    goToPage(0);
}

MainWindow::~MainWindow() = default;

// Takes the content out of a page (pages are QScrollAreas) and appends it as
// a section of the single scrolling column. The page object itself stays alive
// (hidden) because it owns the signals, slots and timers.
void MainWindow::addSection(QScrollArea *page)
{
    QWidget *inner = page->takeWidget();
    page->hide();
    m_sections->addWidget(inner);
    m_sectionWidgets.append(inner);
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    // LEFT: Step Rail (fixed, outside the scroll area)
    m_rail = new StepRail;
    m_rail->setFixedWidth(240);
    m_rail->setSteps({
        "Sign in",
        "Choose restaurant",
        "Build your pizza",   // Step 3
        "Delivery details",   // Step 4
        "Payment",            // Step 5
        "Review & confirm",   // Step 6
        "Order progress"      // Step 7
    });

    auto *railFrame = new QFrame;
    railFrame->setObjectName("railFrame");
    auto *railLay = new QVBoxLayout(railFrame);
    railLay->setContentsMargins(0, 0, 0, 0);
    railLay->addWidget(m_rail);
    h->addWidget(railFrame);

    // RIGHT: one scroll area holding all steps stacked vertically
    m_scroll = new QScrollArea;
    m_scroll->setObjectName("mainScroll");
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_content  = new QWidget;
    m_sections = new QVBoxLayout(m_content);
    m_sections->setContentsMargins(0, 0, 0, 0);
    m_sections->setSpacing(0);
    m_scroll->setWidget(m_content);
    h->addWidget(m_scroll, 1);
    setCentralWidget(central);

    // Pages (parented to the window so they are cleaned up; they stay hidden)
    m_signIn     = new SignInPage(this);
    m_restaurant = new RestaurantPage(this);
    m_pizza      = new PizzaPage(this);
    m_delivery   = new DeliveryPage(this);
    m_payment    = new PaymentPage(this);
    m_review     = new ReviewPage(this);
    m_progress   = new ProgressPage(this);
    m_thanks     = new ThankYouPage(this);

    addSection(m_signIn);      // 0
    addSection(m_restaurant);  // 1
    addSection(m_pizza);       // 2
    addSection(m_delivery);    // 3
    addSection(m_payment);     // 4
    addSection(m_review);      // 5
    addSection(m_progress);    // 6
    addSection(m_thanks);      // 7

    // Only the first step is visible at start; the rest unlock as you go.
    for (int i = 1; i < m_sectionWidgets.size(); ++i)
        m_sectionWidgets[i]->hide();
    m_frontier = 0;

    // Each section fills the viewport so a step feels like a full "screen".
    m_scroll->viewport()->installEventFilter(this);
    updateSectionHeights();

    // Mouse wheel over a combo box should scroll the page, not change the value
    // (unless the combo box has focus because the user clicked it).
    const auto combos = m_content->findChildren<QComboBox*>();
    for (QComboBox *cb : combos) {
        cb->setFocusPolicy(Qt::StrongFocus);
        cb->installEventFilter(this);
    }

    // Smooth scrolling between sections
    m_scrollAnim = new QPropertyAnimation(m_scroll->verticalScrollBar(), "value", this);
    m_scrollAnim->setDuration(450);
    m_scrollAnim->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_scrollAnim, &QPropertyAnimation::finished,
            this, &MainWindow::onScrollFinished);
    connect(m_scroll->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MainWindow::onScrolled);

    // Wiring: forward navigation
    connect(m_signIn,     &SignInPage::nextRequested,     this, [this]{ goToPage(1); });
    connect(m_restaurant, &RestaurantPage::nextRequested, this, [this]{ goToPage(2); });
    connect(m_pizza,      &PizzaPage::nextRequested,      this, [this]{ goToPage(3); });
    connect(m_delivery,   &DeliveryPage::nextRequested,   this, [this]{ goToPage(4); });
    connect(m_payment,    &PaymentPage::nextRequested,    this, [this]{ goToPage(5); refreshReview(); });
    connect(m_review,     &ReviewPage::placeRequested,    this, [this]{
        // Safety net: the card may have been edited after leaving the payment step
        if (!m_payment->valid()) { goToPage(4); return; }
        goToPage(6);
        m_progress->start();
    });

    // Backwards
    connect(m_pizza,    &PizzaPage::backRequested,    this, [this]{ goToPage(1); });
    connect(m_delivery, &DeliveryPage::backRequested, this, [this]{ goToPage(2); });
    connect(m_payment,  &PaymentPage::backRequested,  this, [this]{ goToPage(3); });
    connect(m_review,   &ReviewPage::backRequested,   this, [this]{ goToPage(4); });

    // Rail navigation
    connect(m_rail, &StepRail::stepClicked, this, &MainWindow::onStepClicked);
}

void MainWindow::updateSectionHeights()
{
    const int h = m_scroll->viewport()->height();
    for (int i = 0; i < m_sectionWidgets.size(); ++i)
        m_sectionWidgets[i]->setMinimumHeight(h);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Viewport resized: make sections fill it again and re-align to the
    // section currently in view.
    if (m_scroll && obj == m_scroll->viewport() && event->type() == QEvent::Resize) {
        updateSectionHeights();
        QTimer::singleShot(0, this, [this]{
            if (m_scrollAnim->state() == QAbstractAnimation::Running) return;
            if (m_viewIndex >= 0 && m_viewIndex < m_sectionWidgets.size())
                m_scroll->verticalScrollBar()->setValue(m_sectionWidgets[m_viewIndex]->y());
        });
    }
    // Unfocused combo boxes pass the wheel on to the scroll area
    else if (event->type() == QEvent::Wheel) {
        if (auto *cb = qobject_cast<QComboBox*>(obj)) {
            if (!cb->hasFocus()) {
                event->ignore();
                return true;   // ignored + handled => propagates to the parent
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::applyStyle()
{
    setStyleSheet(R"(
        /* Global */
        QWidget {
            font-family: 'Segoe UI', 'Helvetica Neue', sans-serif;
            font-size: 14px;
            color: #333333;
        }

        /* Left Rail Background */
        QFrame#railFrame {
            background-color: #f4f6f8;
            border-right: 1px solid #e1e4e8;
        }

        /* Form Elements */
        QLineEdit, QComboBox {
            padding: 10px 12px;
            border: 1px solid #d1d5da;
            border-radius: 6px;
            background: #ffffff;
            min-height: 20px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #2196F3;
            background: #f8fbff;
        }

        /* Fix for Invisible ComboBox Dropdown */
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            border: 1px solid #d1d5da;
            selection-background-color: #2196F3;
            selection-color: #ffffff;
            outline: 0px;
            padding: 4px;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #666;
            margin-right: 10px;
        }

        /* Group Boxes (Cards) */
        QGroupBox {
            font-weight: bold;
            font-size: 15px;
            border: 1px solid #e1e4e8;
            border-radius: 10px;
            margin-top: 15px;
            padding: 40px 15px 15px 15px;
            background: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: padding;
            left: 5px;
            padding: 15px;
            color: #2c3e50;
        }

        /* Buttons */
        QPushButton {
            padding: 10px 20px;
            border-radius: 6px;
            border: 1px solid #d1d5da;
            background: #ffffff;
            font-weight: 500;
        }
        QPushButton:hover {
            background: #f1f3f5;
            border-color: #b0b8c1;
        }
        QPushButton#primaryButton {
            background: #4CAF50;
            color: white;
            border: none;
            font-weight: bold;
            padding: 12px 24px;
        }
        QPushButton#primaryButton:hover {
            background: #43a047;
        }

        /* Checkboxes */
        QCheckBox { padding: 6px; font-size: 14px; }
        QCheckBox::indicator { width: 18px; height: 18px; }

        /* Scroll Areas */
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            border: none;
            background: #f1f1f1;
            width: 10px;
            margin: 0px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #c1c1c1;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover { background: #a8a8a8; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");
}

// Unlocks sections up to `index` (if needed) and scrolls to it.
// Going backwards re-locks every section below the target once the scroll
// has finished, so earlier steps have to be confirmed again with Continue.
void MainWindow::goToPage(int index)
{
    if (index < 0 || index >= m_sectionWidgets.size()) return;

    // Leaving the order-progress step: confirm cancelling
    if (m_frontier == 6 && index < 6) {
        auto reply = QMessageBox::question(
            this, "Cancel order?",
            "Are you sure you want to cancel the order?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
        m_progress->stop();
    }

    m_viewIndex   = index;
    m_target      = index;
    m_currentStep = qMin(index, 6);          // Max rail step is 6 (Order Progress)
    m_rail->setCurrentStep(m_currentStep);

    while (m_frontier < index)
        m_sectionWidgets[++m_frontier]->show();

    // Wait one event-loop turn so newly shown sections have their geometry.
    QTimer::singleShot(0, this, [this, index]{
        if (index == m_target) scrollToSection(index);
    });
}

void MainWindow::scrollToSection(int index)
{
    if (index < 0 || index >= m_sectionWidgets.size()) return;

    QScrollBar *bar = m_scroll->verticalScrollBar();
    const int target = m_sectionWidgets[index]->y();

    m_scrollAnim->stop();
    if (bar->value() == target) {
        onScrollFinished();
        return;
    }
    m_scrollAnim->setStartValue(bar->value());
    m_scrollAnim->setEndValue(target);
    m_scrollAnim->start();
}

void MainWindow::onScrollFinished()
{
    // Re-lock sections below the target (after a Back / rail navigation)
    while (m_frontier > m_target)
        m_sectionWidgets[m_frontier--]->hide();

    // Make sure we end exactly on the section's top edge
    if (m_target >= 0 && m_target < m_sectionWidgets.size())
        m_scroll->verticalScrollBar()->setValue(m_sectionWidgets[m_target]->y());
}

// Keeps the rail highlighting whatever section the user scrolled to.
void MainWindow::onScrolled()
{
    if (m_scrollAnim->state() == QAbstractAnimation::Running) return;

    const int probe = m_scroll->verticalScrollBar()->value()
                      + m_scroll->viewport()->height() / 2;

    int idx = 0;
    for (int i = 0; i <= m_frontier && i < m_sectionWidgets.size(); ++i)
        if (m_sectionWidgets[i]->y() <= probe) idx = i;

    if (idx == m_viewIndex) return;

    m_viewIndex   = idx;
    m_currentStep = qMin(idx, 6);
    m_rail->setCurrentStep(m_currentStep);
    if (idx == 5) refreshReview();   // keep the summary fresh when scrolling back to it
}

void MainWindow::onStepClicked(int index)
{
    if (index < 0 || index > m_frontier) return;   // can't skip ahead
    goToPage(index);                               // (cancel-order guard is inside)
}

void MainWindow::refreshReview()
{
    m_review->setSummary(composedOrderHtml());
}

QString MainWindow::composedOrderHtml() const
{
    QString html;
    html += "<h3>Your Order</h3>";
    html += QString("<b>Restaurant:</b> %1<br>")
                .arg(m_restaurant->selectedRestaurant());

    html += "<hr style='border: 1px solid #eee;'>";
    html += QString("<b>Pizza size:</b> %1<br>").arg(m_pizza->size());
    const auto tops = m_pizza->toppings();
    html += QString("<b>Toppings:</b> %1<br>")
                .arg(tops.isEmpty() ? "None" : tops.join(", "));
    if (m_pizza->extraCheese())
        html += "<b>Extra cheese:</b> yes (+$1.50)<br>";

    html += "<hr style='border: 1px solid #eee;'>";
    html += QString("<b>Recipient:</b> %1<br>").arg(m_delivery->name());
    html += QString("<b>Address:</b> %1, %2 %3<br>")
                .arg(m_delivery->address(), m_delivery->city(),
                     m_delivery->postalCode());
    html += QString("<b>Card:</b> %1<br>").arg(m_payment->maskedCard());
    return html;
}


// #include "mainwindow.h"
// #include "pages.h"

// #include <QHBoxLayout>
// #include <QVBoxLayout>
// #include <QMessageBox>
// #include <QFrame>

// MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
// {
//     setWindowTitle("Pizza Ordering — Wizard");
//     buildUi();
//     applyStyle();
//     goToPage(0);
// }

// MainWindow::~MainWindow() = default;

// void MainWindow::buildUi()
// {
//     auto *central = new QWidget(this);
//     auto *h = new QHBoxLayout(central);
//     h->setContentsMargins(0, 0, 0, 0);
//     h->setSpacing(0);

//     // LEFT: Step Rail
//     m_rail = new StepRail;
//     m_rail->setFixedWidth(240);
//     m_rail->setSteps({
//         "Sign in",
//         "Choose restaurant",
//         "Build your pizza",   // Step 3 (was location)
//         "Delivery details",   // Step 4
//         "Payment",            // Step 5
//         "Review & confirm",   // Step 6
//         "Order progress"      // Step 7
//     });

//     // Optional: wrap rail in a frame for a distinct background color
//     auto *railFrame = new QFrame;
//     railFrame->setObjectName("railFrame");
//     auto *railLay = new QVBoxLayout(railFrame);
//     railLay->setContentsMargins(0, 0, 0, 0);
//     railLay->addWidget(m_rail);
//     h->addWidget(railFrame);

//     // RIGHT: Animated Stack
//     m_stack = new AnimatedStackedWidget;
//     m_signIn     = new SignInPage;
//     m_restaurant = new RestaurantPage;
//     m_pizza      = new PizzaPage;
//     m_delivery   = new DeliveryPage;
//     m_payment    = new PaymentPage;
//     m_review     = new ReviewPage;
//     m_progress   = new ProgressPage;
//     m_thanks     = new ThankYouPage;

//     m_stack->addWidget(m_signIn);      // 0
//     m_stack->addWidget(m_restaurant);  // 1
//     m_stack->addWidget(m_pizza);       // 2 (was location)
//     m_stack->addWidget(m_delivery);    // 3
//     m_stack->addWidget(m_payment);     // 4
//     m_stack->addWidget(m_review);      // 5
//     m_stack->addWidget(m_progress);    // 6
//     m_stack->addWidget(m_thanks);      // 7

//     h->addWidget(m_stack, 1);
//     setCentralWidget(central);

//     // Wiring: forward navigation
//     connect(m_signIn,     &SignInPage::nextRequested,     this, [this]{ goToPage(1); });
//     connect(m_restaurant, &RestaurantPage::nextRequested, this, [this]{ goToPage(2); });
//     connect(m_pizza,      &PizzaPage::nextRequested,      this, [this]{ goToPage(3); });
//     connect(m_delivery,   &DeliveryPage::nextRequested,   this, [this]{ goToPage(4); });
//     connect(m_payment,    &PaymentPage::nextRequested,    this, [this]{ goToPage(5); refreshReview(); });
//     connect(m_review,     &ReviewPage::placeRequested,    this, [this]{ goToPage(6); m_progress->start(); });

//     // Backwards
//     connect(m_pizza,    &PizzaPage::backRequested,    this, [this]{ goToPage(1); });
//     connect(m_delivery, &DeliveryPage::backRequested, this, [this]{ goToPage(2); });
//     connect(m_payment,  &PaymentPage::backRequested,  this, [this]{ goToPage(3); });
//     connect(m_review,   &ReviewPage::backRequested,   this, [this]{ goToPage(4); });

//     // Rail back navigation
//     connect(m_rail, &StepRail::stepClicked, this, &MainWindow::onStepClicked);
// }

// void MainWindow::applyStyle()
// {
//     setStyleSheet(R"(
//         /* Global */
//         QWidget {
//             font-family: 'Segoe UI', 'Helvetica Neue', sans-serif;
//             font-size: 14px;
//             color: #333333;
//         }

//         /* Left Rail Background */
//         QFrame#railFrame {
//             background-color: #f4f6f8;
//             border-right: 1px solid #e1e4e8;
//         }

//         /* Right Content Area Background */
//         AnimatedStackedWidget {
//             background-color: #ffffff;
//         }

//         /* Form Elements */
//         QLineEdit, QComboBox {
//             padding: 10px 12px;
//             border: 1px solid #d1d5da;
//             border-radius: 6px;
//             background: #ffffff;
//             min-height: 20px;
//         }
//         QLineEdit:focus, QComboBox:focus {
//             border: 2px solid #2196F3;
//             background: #f8fbff;
//         }

//         /* Fix for Invisible ComboBox Dropdown */
//         QComboBox QAbstractItemView {
//             background-color: #ffffff;
//             border: 1px solid #d1d5da;
//             selection-background-color: #2196F3;
//             selection-color: #ffffff;
//             outline: 0px;
//             padding: 4px;
//         }
//         QComboBox::drop-down {
//             border: none;
//             width: 30px;
//         }
//         QComboBox::down-arrow {
//             image: none;
//             border-left: 5px solid transparent;
//             border-right: 5px solid transparent;
//             border-top: 5px solid #666;
//             margin-right: 10px;
//         }

//         /* Group Boxes (Cards) */
//         QGroupBox {
//             font-weight: bold;
//             font-size: 15px;
//             border: 1px solid #e1e4e8;
//             border-radius: 10px;
//             margin-top: 15px;
//             padding: 40px 15px 15px 15px;
//             background: #ffffff;
//         }
//         QGroupBox::title {
//             subcontrol-origin: padding;
//             left: 5px;
//             padding: 15px;
//             color: #2c3e50;
//         }

//         /* Buttons */
//         QPushButton {
//             padding: 10px 20px;
//             border-radius: 6px;
//             border: 1px solid #d1d5da;
//             background: #ffffff;
//             font-weight: 500;
//         }
//         QPushButton:hover {
//             background: #f1f3f5;
//             border-color: #b0b8c1;
//         }
//         QPushButton#primaryButton {
//             background: #4CAF50;
//             color: white;
//             border: none;
//             font-weight: bold;
//             padding: 12px 24px;
//         }
//         QPushButton#primaryButton:hover {
//             background: #43a047;
//         }

//         /* Checkboxes */
//         QCheckBox { padding: 6px; font-size: 14px; }
//         QCheckBox::indicator { width: 18px; height: 18px; }

//         /* Scroll Areas */
//         QScrollArea { background: transparent; border: none; }
//         QScrollBar:vertical {
//             border: none;
//             background: #f1f1f1;
//             width: 10px;
//             margin: 0px;
//             border-radius: 5px;
//         }
//         QScrollBar::handle:vertical {
//             background: #c1c1c1;
//             min-height: 20px;
//             border-radius: 5px;
//         }
//         QScrollBar::handle:vertical:hover { background: #a8a8a8; }
//         QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
//     )");
// }

// void MainWindow::goToPage(int index)
// {
//     if (index < 0 || index >= m_stack->count()) return;

//     m_currentStep = qMin(index, 6); // Max step is 6 (Order Progress)
//     m_rail->setCurrentStep(m_currentStep);

//     if (index == 6) {
//         m_stack->slideToWidget(m_progress);
//     } else if (index == 7) {
//         m_stack->slideToWidget(m_thanks);
//     } else {
//         m_stack->slideToIndex(index);
//     }
// }

// void MainWindow::onStepClicked(int index)
// {
//     // Guard leaving the review/progress step without placing
//     if (m_stack->currentWidget() == m_progress && index != 6) {
//         auto reply = QMessageBox::question(
//             this, "Cancel order?",
//             "Are you sure you want to cancel the order?",
//             QMessageBox::Yes | QMessageBox::No);
//         if (reply != QMessageBox::Yes) return;
//         m_progress->stop();
//     }

//     // Map Step Rail indices to Stack indices
//     if (index <= 4) {
//         goToPage(index);
//     } else if (index == 5) {
//         goToPage(5); // Review
//     } else if (index == 6) {
//         goToPage(6); // Progress
//     }
// }

// void MainWindow::refreshReview()
// {
//     m_review->setSummary(composedOrderHtml());
// }

// QString MainWindow::composedOrderHtml() const
// {
//     QString html;
//     html += "<h3>Your Order</h3>";
//     html += QString("<b>Restaurant:</b> %1<br>")
//                 .arg(m_restaurant->selectedRestaurant());

//     html += "<hr style='border: 1px solid #eee;'>";
//     html += QString("<b>Pizza size:</b> %1<br>").arg(m_pizza->size());
//     const auto tops = m_pizza->toppings();
//     html += QString("<b>Toppings:</b> %1<br>")
//                 .arg(tops.isEmpty() ? "None" : tops.join(", "));
//     if (m_pizza->extraCheese())
//         html += "<b>Extra cheese:</b> yes (+$1.50)<br>";

//     html += "<hr style='border: 1px solid #eee;'>";
//     html += QString("<b>Recipient:</b> %1<br>").arg(m_delivery->name());
//     html += QString("<b>Address:</b> %1, %2 %3<br>")
//                 .arg(m_delivery->address(), m_delivery->city(),
//                      m_delivery->postalCode());
//     html += QString("<b>Card:</b> %1<br>").arg(m_payment->maskedCard());
//     return html;
// }
