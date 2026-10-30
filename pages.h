
#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
#include <QDateEdit>
#include <QButtonGroup>
#include <QProgressBar>
// Every page is a QScrollArea so content can always scroll vertically,
// even on small screens. Pages emit signals; MainWindow wires them.

// ---------- helper ----------
QScrollArea *makeScrollable(QWidget *inner);

// ---------- 0. Sign in ----------
class SignInPage : public QScrollArea {
    Q_OBJECT
public:
    explicit SignInPage(QWidget *parent = nullptr);
    QString email() const;
signals:
    void nextRequested();
private:
    QLineEdit *m_email;
    QLineEdit *m_password;
};

// ---------- 1. Restaurant ----------
class RestaurantPage : public QScrollArea {
    Q_OBJECT
public:
    explicit RestaurantPage(QWidget *parent = nullptr);
    QString selectedRestaurant() const;
    QString selectedAddress() const;
signals:
    void nextRequested();
private:
    QComboBox *m_restaurants;
    QLabel    *m_address;

};

// ---------- 2. Location ----------
class LocationPage : public QScrollArea {
    Q_OBJECT
public:
    explicit LocationPage(QWidget *parent = nullptr);
    QString selectedAddress() const;
signals:
    void nextRequested();
private:
    QLineEdit *m_street;
    QLineEdit *m_city;
    QLineEdit *m_postal;
};

// ---------- 3. Pizza Builder ----------
class PizzaPage : public QScrollArea {
    Q_OBJECT
public:
    explicit PizzaPage(QWidget *parent = nullptr);

    QString  size() const;
    QStringList toppings() const;
    bool     extraCheese() const;
    void     resetPizza();

signals:
    void backRequested();
    void nextRequested();
    void summaryChanged();

private slots:
    void applyPreset(int index);
    void refreshSummary();

private:
    QComboBox   *m_size;
    QComboBox   *m_presets;
    QCheckBox   *m_ham, *m_pep, *m_gp, *m_mush, *m_pine, *m_onion, *m_art, *m_olv;
    QCheckBox   *m_extraCheese;
    QLabel      *m_summary;
    QPushButton *m_resetBtn;

    QList<QCheckBox*> allToppings() const;
};

// ---------- 4. Delivery Details ----------
class DeliveryPage : public QScrollArea {
    Q_OBJECT
public:
    explicit DeliveryPage(QWidget *parent = nullptr);

    QString name() const;
    QString address() const;
    QString city() const;
    QString postalCode() const;

signals:
    void nextRequested();
    void backRequested();

private slots:
    void validatePostal();

private:
    QLineEdit *m_name;
    QLineEdit *m_street;
    QLineEdit *m_apt;
    QLineEdit *m_city;
    QLineEdit *m_postal;
    QLabel    *m_error;
};

// ---------- 5. Payment ----------
class PaymentPage : public QScrollArea {
    Q_OBJECT
public:
    explicit PaymentPage(QWidget *parent = nullptr);

    bool    valid() const;
    QString maskedCard() const;

signals:
    void nextRequested();
    void backRequested();

private slots:
    void formatCard(const QString &text);
    void formatExpiry(const QString &text);

private:
    QLineEdit *m_holder;
    QLineEdit *m_card;
    QLineEdit *m_expiry;
    QLineEdit *m_cvv;
    QLabel    *m_error;
};

// ---------- 6. Review ----------
class ReviewPage : public QScrollArea {
    Q_OBJECT
public:
    explicit ReviewPage(QWidget *parent = nullptr);
    void setSummary(const QString &html);
signals:
    void placeRequested();
    void backRequested();
private:
    QLabel *m_body;
};

// ---------- 7. Progress ----------
class ProgressPage : public QScrollArea {
    Q_OBJECT
public:
    explicit ProgressPage(QWidget *parent = nullptr);
    void start();
    void stop();
private slots:
    void tick();
private:
    QLabel      *m_status;
    QProgressBar *m_bar;
    QTimer      *m_timer;
    int          m_stage;
};

// ---------- 8. Thank-you ----------
class ThankYouPage : public QScrollArea {
    Q_OBJECT
public:
    explicit ThankYouPage(QWidget *parent = nullptr);
};

#endif // PAGES_H
