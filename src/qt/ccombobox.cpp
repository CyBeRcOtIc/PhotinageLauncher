#include "qt/ccombobox.h"
#include "io/styleloader.h"

CComboBox::CComboBox(QWidget* parent) : QComboBox(parent) {
    this->m_showPopupStyle = IO::StyleLoader::loadStyle(":/styles/version_combo_box_open.qss");
    this->m_hidePopupStyle = IO::StyleLoader::loadStyle(":/styles/version_combo_box_close.qss");
}

void CComboBox::showPopup() {
    if(this->count() > 0){
        this->setStyleSheet(this->m_showPopupStyle);
    }
    QComboBox::showPopup();
}

void CComboBox::hidePopup(){
    if(this->count() > 0){
        this->setStyleSheet(this->m_hidePopupStyle);
    }
    QComboBox::hidePopup();
}
