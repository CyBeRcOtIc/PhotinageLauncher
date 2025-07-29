#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr, const int cornerRadius = 30);
    ~TitleBar();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private slots:
    void toggleMaximizeButtonSlot();
private:
    void setSlots();
    Ui::TitleBar *ui;
    bool m_dragging = false;
    QPoint m_dragPosition;
    int m_cornerRadius;
    QRect m_normalGeometry;
};

#endif // TITLEBAR_H
