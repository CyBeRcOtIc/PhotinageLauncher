#ifndef CCOMBOBOX_H
#define CCOMBOBOX_H

#include <QComboBox>
#include <QString>

class CComboBox : public QComboBox{
    Q_OBJECT
public:
    explicit CComboBox(QWidget* parent = nullptr);
signals:
    void showPopupSignal();
    void hidePopupSignal();
protected:
    void showPopup() override;
    void hidePopup() override;
private:
    QString m_showPopupStyle;
    QString m_hidePopupStyle;
};

#endif // CCOMBOBOX_H
