#include "pages.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>

// ============================================================
// helper
// ============================================================
QScrollArea *makeScrollable(QWidget *inner)
{
    // Caller is expected to be a QScrollArea subclass; we just return it
    // already configured. See each page's ctor for setWidget().
    Q_UNUSED(inner);
    return nullptr;
}

// ============================================================
// 0. Sign in
// ============================================================
SignInPage::SignInPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1> \u{1F44B} Welcome back !</h1><p>Sign in to place your order </p>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    auto *form = new QFormLayout;
    m_email = new QLineEdit;
    m_email->setPlaceholderText("you@example.com");
    m_password = new QLineEdit;
    m_password->setEchoMode(QLineEdit::Password);
    form->addRow("Email:", m_email);
    form->addRow("Password:", m_password);
    v->addLayout(form);

    auto *next = new QPushButton("Continue \u2192");
    next->setObjectName("primaryButton");
    v->addSpacing(10);
    v->addWidget(next, 0, Qt::AlignRight);
    v->addStretch();

    connect(next, &QPushButton::clicked, this, &SignInPage::nextRequested);

    setWidget(inner);
}
QString SignInPage::email() const { return m_email->text(); }

// ============================================================
// 1. Restaurant
// ============================================================
RestaurantPage::RestaurantPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Choose a restaurant \u{1F4CD}</h1>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    m_restaurants = new QComboBox;

    m_restaurants->addItem(
        QIcon(":/logos/images/logos.png"),
        "Pizza Pizza — Peanut Plaza"
        );
    m_restaurants->addItem(
        QIcon(":/logos/images/phut.png"),
        "Pizza Hut — 3555 Don Mills Rd"
        );

    m_restaurants->addItem(
        QIcon(":/logos/images/phwings.png"),
        "Pizza Hot Wings — 3883 Don Mills Rd"
        );

    m_restaurants->addItem(
        QIcon(":/logos/images/logos.png"),
        "Pizza Pizza — 5921 Leslie St"
        );

    m_address = new QLabel("<i>123 Main St, open until 11:00 PM</i>");
    m_address->setTextFormat(Qt::RichText);

    v->addWidget(m_restaurants);
    v->addWidget(m_address);

    auto *next = new QPushButton("Continue \u2192");
    next->setObjectName("primaryButton");
    v->addSpacing(10);
    v->addWidget(next, 0, Qt::AlignRight);
    v->addStretch();

    connect(next, &QPushButton::clicked, this, &RestaurantPage::nextRequested);

    setWidget(inner);
}
QString RestaurantPage::selectedRestaurant() const { return m_restaurants->currentText(); }
QString RestaurantPage::selectedAddress() const   { return m_address->text(); }

// ============================================================
// 2. Location
// ============================================================
LocationPage::LocationPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Where should we deliver?</h1>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    auto *form = new QFormLayout;
    m_street = new QLineEdit;
    m_city   = new QLineEdit;
    m_postal = new QLineEdit;
    m_street->setPlaceholderText("Street address");
    m_city->setPlaceholderText("City");
    m_postal->setPlaceholderText("A1A 1A1");

    QRegularExpression postal("[A-Za-z]\\d[A-Za-z]\\d[A-Za-z]\\d");
    m_postal->setValidator(new QRegularExpressionValidator(postal, this));

    form->addRow("Street:", m_street);
    form->addRow("City:",   m_city);
    form->addRow("Postal:", m_postal);
    v->addLayout(form);

    auto *row = new QHBoxLayout;
    auto *back = new QPushButton("\u2190 Back");
    auto *next = new QPushButton("Continue \u2192");
    next->setObjectName("primaryButton");
    row->addWidget(back);
    row->addStretch();
    row->addWidget(next);
    v->addSpacing(10);
    v->addLayout(row);
    v->addStretch();

    connect(next, &QPushButton::clicked, this, &LocationPage::nextRequested);

    setWidget(inner);
}
QString LocationPage::selectedAddress() const
{
    return QString("%1, %2 %3").arg(m_street->text(), m_city->text(), m_postal->text());
}

// ============================================================
// 3. Pizza Builder
// ============================================================
PizzaPage::PizzaPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Build your pizza \u{1F355}</h1>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    // Presets
    m_presets = new QComboBox;
    m_presets->addItem("— Start from scratch —");
    m_presets->addItem("Margherita (cheese only)");
    m_presets->addItem("Pepperoni Classic");
    m_presets->addItem("Veggie Delight");
    m_presets->addItem("Hawaiian");
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(new QLabel("Preselected pizzas:"));
    presetRow->addWidget(m_presets, 1);
    v->addLayout(presetRow);

    // Size
    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(new QLabel("Size:"));
    m_size = new QComboBox;
    m_size->addItems({"Small", "Medium", "Large"});
    m_size->setCurrentIndex(1);
    sizeRow->addWidget(m_size);
    sizeRow->addStretch();
    m_resetBtn = new QPushButton("Reset Pizza ⟳");
    sizeRow->addWidget(m_resetBtn);
    v->addLayout(sizeRow);

    // Toppings — no limit any more
    auto *tGroup = new QGroupBox("Toppings");
    auto *grid = new QGridLayout(tGroup);
    m_ham   = new QCheckBox("Ham \u{1F953} (+$1.25)");
    m_pep   = new QCheckBox("Beef \u{1F969} (+$1.25)");
    m_gp    = new QCheckBox("Green Pepper 🫑 (+$1.25)");
    m_mush  = new QCheckBox("Mushroom 🍄‍🟫(+$1.25)");
    m_pine  = new QCheckBox("Pineapple \u{1F34D} (+$1.25)");
    m_onion = new QCheckBox("Onion \u{1F9C5} (+$1.25)");
    m_art   = new QCheckBox("Artichokes \u{1F96C} (+$1.25)");
    m_olv   = new QCheckBox("Extra Sauce \u{1F96B} (+$1.25)");
    int r = 0, c = 0;
    for (auto *cb : allToppings()) {
        grid->addWidget(cb, r, c);
        if (++c == 2) { c = 0; ++r; }
    }
    v->addWidget(tGroup);

    // Extra cheese — always available (restriction removed)
    m_extraCheese = new QCheckBox("Extra Cheese \u{1F9C0} (+$1.50)");
    grid->addWidget(m_extraCheese, r, 0, 1, 2);

    // Summary
    auto *sumGroup = new QGroupBox("Order Summary");
    auto *sumLay = new QVBoxLayout(sumGroup);
    m_summary = new QLabel("No order yet.");
    m_summary->setWordWrap(true);
    m_summary->setTextFormat(Qt::RichText);
    sumLay->addWidget(m_summary);
    v->addWidget(sumGroup);

    // Buttons
    auto *row = new QHBoxLayout;
    //m_resetBtn = new QPushButton("Reset Pizza ⟳");
    //row->addWidget(m_resetBtn);
    auto *back = new QPushButton("\u2190 Back");
    row->addWidget(back);
    auto *next = new QPushButton("Continue \u2192");
    next->setObjectName("primaryButton");

    row->addStretch();
    row->addWidget(next);
    v->addLayout(row);
    v->addStretch();

    setWidget(inner);

    // Wiring
    connect(m_presets, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PizzaPage::applyPreset);
    for (auto *cb : allToppings())
        connect(cb, &QCheckBox::stateChanged, this, &PizzaPage::refreshSummary);
    connect(m_extraCheese, &QCheckBox::stateChanged, this, &PizzaPage::refreshSummary);
    connect(m_size, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PizzaPage::refreshSummary);
    connect(m_resetBtn, &QPushButton::clicked, this, [this]{
        m_presets->setCurrentIndex(0);
        resetPizza();
    });
    connect(back, &QPushButton::clicked, this, &PizzaPage::backRequested);
    connect(next, &QPushButton::clicked, this, &PizzaPage::nextRequested);

    refreshSummary();
}

QList<QCheckBox*> PizzaPage::allToppings() const
{
    return {m_ham, m_pep, m_gp, m_mush, m_pine, m_onion, m_art, m_olv};
}

QString PizzaPage::size() const { return m_size->currentText(); }

QStringList PizzaPage::toppings() const
{
    QStringList out;
    for (auto *cb : allToppings())
        if (cb->isChecked()) out << cb->text();
    return out;
}

bool PizzaPage::extraCheese() const { return m_extraCheese->isChecked(); }

void PizzaPage::resetPizza()
{
    for (auto *cb : allToppings()) cb->setChecked(false);
    m_extraCheese->setChecked(false);
    m_size->setCurrentIndex(1);
    refreshSummary();
}

void PizzaPage::applyPreset(int index)
{
    if (index <= 0) return;              // "scratch"
    for (auto *cb : allToppings()) cb->setChecked(false);
    m_extraCheese->setChecked(false);

    switch (index) {
    case 1: // Margherita
        break;
    case 2: // Pepperoni
        m_pep->setChecked(true); break;
    case 3: // Veggie
        m_gp->setChecked(true); m_mush->setChecked(true);
        m_onion->setChecked(true); m_olv->setChecked(true); break;
    case 4: // Hawaiian
        m_ham->setChecked(true); m_pine->setChecked(true); break;
    }
    refreshSummary();
}

void PizzaPage::refreshSummary()
{
    double price = 0.0;
    if (m_size->currentText() == "Small")      price = 8.99;
    else if (m_size->currentText() == "Medium") price = 10.99;
    else                                        price = 12.99;

    const QStringList tops = toppings();
    price += tops.size() * 1.25;
    if (m_extraCheese->isChecked()) price += 1.50;

    QString html = QString("<b>Size:</b> %1<br><b>Toppings:</b> %2<br>")
                       .arg(m_size->currentText(),
                            tops.isEmpty() ? "None" : tops.join(", "));
    if (m_extraCheese->isChecked())
        html += "<b>Extra cheese:</b> yes (+$1.50)<br>";
    html += QString("<span style='color:#2e7d32;font-size:16px;'>"
                    "<b>Total: $%1</b></span>")
                .arg(QString::number(price, 'f', 2));

    m_summary->setText(html);
    emit summaryChanged();
}

// ============================================================
// 4. Delivery Details (moved to the END of the flow)
// ============================================================
DeliveryPage::DeliveryPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Delivery details \u{1F3E0}</h1>"
                             "<p>Almost done — where should the driver go?</p>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    auto *form = new QFormLayout;
    m_name   = new QLineEdit; m_name->setPlaceholderText("Your full name");
    m_street = new QLineEdit; m_street->setPlaceholderText("Street address");
    m_apt    = new QLineEdit; m_apt->setPlaceholderText("Optional");
    m_city   = new QLineEdit; m_city->setPlaceholderText("City");
    m_postal = new QLineEdit; m_postal->setPlaceholderText("A1A 1A1");

    QRegularExpression postalRe("[A-Za-z]\\d[A-Za-z]\\d[A-Za-z]\\d");
    m_postal->setValidator(new QRegularExpressionValidator(postalRe, this));

    form->addRow("Name:",         m_name);
    form->addRow("Street:",       m_street);
    form->addRow("Apartment:",    m_apt);
    form->addRow("City:",         m_city);
    form->addRow("Postal code:",  m_postal);
    v->addLayout(form);

    m_error = new QLabel;
    m_error->setStyleSheet("color:#c62828;font-weight:bold;");
    m_error->setWordWrap(true);
    v->addWidget(m_error);

    auto *row = new QHBoxLayout;
    auto *back = new QPushButton("\u2190 Back");
    auto *next = new QPushButton("Continue to payment \u2192");
    next->setObjectName("primaryButton");
    row->addWidget(back);
    row->addStretch();
    row->addWidget(next);
    v->addLayout(row);
    v->addStretch();

    setWidget(inner);

    connect(m_postal, &QLineEdit::textChanged, this, &DeliveryPage::validatePostal);
    connect(back, &QPushButton::clicked, this, &DeliveryPage::backRequested);
    connect(next, &QPushButton::clicked, this, &DeliveryPage::nextRequested);
}

void DeliveryPage::validatePostal()
{
    const QString t = m_postal->text();
    if (t.isEmpty() || m_postal->hasAcceptableInput())
        m_postal->setStyleSheet("");
    else
        m_postal->setStyleSheet("QLineEdit{border:2px solid #c62828;}");
}

QString DeliveryPage::name()       const { return m_name->text(); }
QString DeliveryPage::address()    const { return m_street->text(); }
QString DeliveryPage::city()       const { return m_city->text(); }
QString DeliveryPage::postalCode() const { return m_postal->text(); }

// ============================================================
// 5. Payment
// ============================================================
PaymentPage::PaymentPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Payment \u{1F4B3}</h1><p>All transactions are encrypted.</p>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    auto *form = new QFormLayout;
    m_holder = new QLineEdit; m_holder->setPlaceholderText("Name on card");
    m_card   = new QLineEdit; m_card->setPlaceholderText("1234 5678 9012 3456");
    m_expiry = new QLineEdit; m_expiry->setPlaceholderText("MM/YY");
    m_cvv    = new QLineEdit; m_cvv->setPlaceholderText("123");
    m_cvv->setEchoMode(QLineEdit::Password);
    m_cvv->setMaxLength(4);
    m_expiry->setMaxLength(5);

    form->addRow("Card holder:", m_holder);
    form->addRow("Card number:", m_card);
    form->addRow("Expiry:",      m_expiry);
    form->addRow("CVV:",         m_cvv);
    v->addLayout(form);

    m_error = new QLabel;
    m_error->setStyleSheet("color:#c62828;font-weight:bold;");
    m_error->setWordWrap(true);
    v->addWidget(m_error);

    auto *row = new QHBoxLayout;
    auto *back = new QPushButton("\u2190 Back");
    auto *next = new QPushButton("Review order \u2192");
    next->setObjectName("primaryButton");
    row->addWidget(back);
    row->addStretch();
    row->addWidget(next);
    v->addLayout(row);
    v->addStretch();

    setWidget(inner);

    connect(m_card, &QLineEdit::textChanged, this, &PaymentPage::formatCard);
    connect(m_expiry, &QLineEdit::textChanged, this, &PaymentPage::formatExpiry);
    connect(back, &QPushButton::clicked, this, &PaymentPage::backRequested);
    connect(next, &QPushButton::clicked, this, [this]{
        if (valid()) emit nextRequested();
    });
}

void PaymentPage::formatCard(const QString &text)
{
    QString digits = text;
    digits.remove(QRegularExpression("[^0-9]"));
    digits.truncate(16);
    QString out;
    for (int i = 0; i < digits.size(); ++i) {
        if (i && i % 4 == 0) out += ' ';
        out += digits[i];
    }
    if (out != text) {
        QSignalBlocker b(m_card);
        m_card->setText(out);
    }
}

// void PaymentPage::formatExpiry(const QString &text)
// {
//     QString digits = text;
//     digits.remove(QRegularExpression("[^0-9]"));
//     digits.truncate(4);
//     QString out;
//     for (int i = 0; i < digits.size(); ++i) {
//         if (i == 2) out += '/';
//         out += digits[i];
//     }
//     if (out != text) {
//         QSignalBlocker b(m_expiry);
//         m_expiry->setText(out);
//     }

// }
void PaymentPage::formatExpiry(const QString &text)
{
    QString digits = text;
    digits.remove(QRegularExpression("[^0-9]"));
    digits.truncate(4);

    // Validate month (01–12)
    if (digits.size() >= 2) {
        int month = digits.left(2).toInt();

        if (month < 1 || month > 12) {
            digits.remove(1, 1);
        }
    }

    // Validate year (current year to 10 years ahead)
    if (digits.size() == 4) {
        int month = digits.left(2).toInt();
        int year = 2000 + digits.mid(2, 2).toInt();

        QDate currentDate = QDate::currentDate();

        if (year < currentDate.year() ||
            (year == currentDate.year() && month < currentDate.month()) ||
            year > currentDate.year() + 10) {
            digits.truncate(2);
        }
    }

    QString out;

    for (int i = 0; i < digits.size(); ++i) {
        if (i == 2) out += '/';
        out += digits[i];
    }

    if (out != text) {
        QSignalBlocker b(m_expiry);
        m_expiry->setText(out);
        m_expiry->setCursorPosition(out.size());
    }
}

bool PaymentPage::valid() const
{
    QStringList err;
    if (m_holder->text().trimmed().isEmpty()) err << "Card holder is required.";

    QString card = m_card->text();
    card.remove(' ');
    if (card.size() != 16) err << "Card number must be 16 digits.";

    if (!QRegularExpression("^\\d{2}/\\d{2}$").match(m_expiry->text()).hasMatch())
         err << "Expiry must be MM/YY.";
    if (m_cvv->text().size() < 3) err << "CVV must be at least 3 digits.";

    m_error->setText(err.join(" "));
    return err.isEmpty();
}

QString PaymentPage::maskedCard() const
{
    QString card = m_card->text();
    card.remove(' ');
    if (card.size() < 4) return "****";
    return "**** **** **** " + card.right(4);
}

// ============================================================
// 6. Review / Confirm (inline page, not a popup)
// ============================================================
ReviewPage::ReviewPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Confirm your order \u{1F6D2}</h1>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    auto *box = new QGroupBox("Summary");
    auto *boxLay = new QVBoxLayout(box);
    m_body = new QLabel;
    m_body->setTextFormat(Qt::RichText);
    m_body->setWordWrap(true);
    boxLay->addWidget(m_body);
    v->addWidget(box);

    auto *row = new QHBoxLayout;
    auto *back = new QPushButton("\u2190 Modify order");
    auto *place = new QPushButton("Place order \u2714");
    place->setObjectName("primaryButton");
    row->addWidget(back);
    row->addStretch();
    row->addWidget(place);
    v->addLayout(row);
    v->addStretch();

    setWidget(inner);

    connect(back, &QPushButton::clicked, this, &ReviewPage::backRequested);
    connect(place, &QPushButton::clicked, this, &ReviewPage::placeRequested);
}

void ReviewPage::setSummary(const QString &html)
{
    m_body->setText(html);
}

// ============================================================
// 7. Order progress (inline page, replaces the "Order placed" popup)
// ============================================================
ProgressPage::ProgressPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 40, 40, 40);
    v->setSpacing(16);

    auto *title = new QLabel("<h1>Order placed \u2714</h1>"
                             "<p>Thank you! Your pizza order has been placed.</p>");
    title->setTextFormat(Qt::RichText);
    v->addWidget(title);

    m_bar = new QProgressBar;
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    m_bar->setTextVisible(true);
    v->addWidget(m_bar);

    m_status = new QLabel("<i>Preparing…</i>");
    m_status->setTextFormat(Qt::RichText);
    v->addWidget(m_status);
    v->addStretch();

    setWidget(inner);

    m_timer = new QTimer(this);
    m_timer->setInterval(900);
    connect(m_timer, &QTimer::timeout, this, &ProgressPage::tick);
}

void ProgressPage::start()
{
    m_stage = 0;
    m_bar->setValue(0);
    m_status->setText("<i>Order received…</i>");
    m_timer->start();
}
void ProgressPage::stop() { m_timer->stop(); }

void ProgressPage::tick()
{
    ++m_stage;
    static const QStringList msgs = {
        "Order received…",
        "Dough is being stretched…",
        "In the oven \u{1F525}",
        "Quality check…",
        "Out for delivery \u{1F6F5}",
        "Delivered — enjoy! \u{1F355}"
    };
    if (m_stage >= msgs.size()) { m_timer->stop(); m_bar->setValue(300); return; }
    m_bar->setValue(qMin(100, m_stage * 100 / (msgs.size() - 1)));
    m_status->setText(QString("<i>%1</i>").arg(msgs[m_stage]));
}

// ============================================================
// 8. Thank-you
// ============================================================
ThankYouPage::ThankYouPage(QWidget *parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    auto *inner = new QWidget;
    auto *v = new QVBoxLayout(inner);
    v->setContentsMargins(40, 60, 40, 40);
    auto *lbl = new QLabel(
        "<h1>Thank you!</h1>"
        "<p>Your order is on its way. You can close this window.</p>");
    lbl->setTextFormat(Qt::RichText);
    v->addWidget(lbl);
    v->addStretch();
    setWidget(inner);
}
