#include "colorpanel.h"

#include <QPainter>
#include <QGridLayout>

ColorButton::ColorButton(const QColor &color, QWidget *parent)
    : QPushButton(parent) {
    m_color = color;
    setFixedSize(20, 20);
}

void ColorButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(m_color);

    painter.setBrush(QBrush(m_color));

    QPen pen;
    pen.setWidth(2);
    pen.setColor(m_color);
    painter.setPen(pen);
    painter.drawRoundedRect(this->rect(), 4, 4);
}

ColorButton::~ColorButton() {}

ColorPanel::ColorPanel(QWidget *parent)
    : QWidget(parent) {
    QStringList colList;
    colList << QString("#ff0c0c") << QString("#fe3c3b") << QString("#fd6867") << QString("#fd9694")
            << QString("#fcc4c1") << QString("#f8e0d6") << QString("#e4c299") << QString("#f2aa46")
            << QString("#fd9d0f") << QString("#f6b443") << QString("#eecb77") << QString("#f0ee4e")
            << QString("#f4fb00") << QString("#f6f96d") << QString("#f4f6a6") << QString("f3f3d6")
            << QString("#e9eedc") << QString("#dde8cb") << QString("#ccdfb0") << QString("#9cd972")
            << QString("#4ec918") << QString("#5cc850") << QString("#6bc989") << QString("#53ac6d")
            << QString("#72b88e") << QString("#7cc8cd") << QString("#97d1d4") << QString("#c9e1e1")
            << QString("#c1dee7") << QString("#93ceed") << QString("#76c3f1") << QString("#49b2f6")
            << QString("#119fff") << QString("#0192ea") << QString("#3d7ddd") << QString("#92cdfb")
            << QString("#99cffa") << QString("#ececf8") << QString("#ccc9f9") << QString("#b2acf9")
            << QString("#958ef9") << QString("#7c6ffa") << QString("#8a47fb") << QString("#6b1aef")
            << QString("#952dfd") << QString("#af39e4") << QString("#c174da") << QString("#c587d9")
            << QString("#dbb4c1") << QString("#cf8c86") << QString("#b45f51") << QString("#865e4f")
            << QString("#694d48") << QString("#ffffff") << QString("#d4d4d4") << QString("#919191")
            << QString("#626262") << QString("#404040") << QString("#000000");

    QList<ColorButton*> cButtonList;

    QGridLayout* gLayout = new QGridLayout(this);

    for(int i = 1; i < colList.length(); i++) {
        ColorButton* cb = new ColorButton(QColor(colList[i]), this);
        cButtonList.append(cb);
        gLayout->addWidget(cb, i/10, i%10);
    }

    setLayout(gLayout);
}

ColorPanel::~ColorPanel() {}
