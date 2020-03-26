/*
 * Copyright (C) 2019 ~ %YEAR% Deepin Technology Co., Ltd.
 *
 * Author:     Renranv
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "linewidget.h"
#include "drawshape/cdrawparamsigleton.h"
#include "widgets/bordercolorbutton.h"
#include "widgets/seperatorline.h"
#include "widgets/toolbutton.h"
#include "widgets/csidewidthwidget.h"
#include "widgets/ccheckbutton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "service/cmanagerattributeservice.h"

#include <QHBoxLayout>
#include <QDebug>

const int BTN_SPACNT = 10;
const int TEXT_SIZE = 12;

LineWidget::LineWidget(DWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnection();
}

LineWidget::~LineWidget()
{
}

void LineWidget::changeButtonTheme()
{
    m_sideWidthWidget->changeButtonTheme();
    m_sep1Line->updateTheme();
    int themeType = CManageViewSigleton::GetInstance()->getThemeType();
    m_straightline->setCurrentTheme(themeType);
    m_arrowline->setCurrentTheme(themeType);
}

void LineWidget::updateMultCommonShapWidget(QMap<EDrawProperty, QVariant> propertys)
{
    m_lineTypeLabel->setVisible(false);
    m_straightline->setVisible(false);
    m_arrowline->setVisible(false);
    m_strokeBtn->setVisible(false);
    m_lwLabel->setVisible(false);
    m_sideWidthWidget->setVisible(false);
    for (int i = 0; i < propertys.size(); i++) {
        EDrawProperty property = propertys.keys().at(i);
        switch (property) {
        case LineColor:
            m_strokeBtn->setVisible(true);
            if (propertys[property].type() == QVariant::Invalid) {
                m_strokeBtn->setIsMultColorSame(false);
            } else {
                m_strokeBtn->setColor(propertys[property].value<QColor>());
            }
            m_strokeBtn->update();
            break;
        case LineWidth:
            m_lwLabel->setVisible(true);
            m_sideWidthWidget->setVisible(true);
            if (propertys[property].type() == QVariant::Invalid) {
                m_sideWidthWidget->setMenuButtonICon("—— ——", QIcon());
            } else {
                m_sideWidthWidget->setSideWidth(propertys[property].toInt());
            }
            m_sideWidthWidget->update();
            break;
        case LineArrowType:
            m_lineTypeLabel->setVisible(true);
            m_straightline->setVisible(true);
            m_arrowline->setVisible(true);
            if (propertys[property].type() == QVariant::Invalid) {
                m_straightline->setChecked(false);
                m_arrowline->setChecked(false);
            } else {
                if (propertys[property].value<ELineType>() == straightType) {
                    m_straightline->setChecked(true);
                } else {
                    m_arrowline->setChecked(true);
                }
            }
            m_sideWidthWidget->update();
            break;
        default:
            break;
        }
    }
}

void LineWidget::initUI()
{
    QFont ft;
    ft.setPixelSize(TEXT_SIZE);

    m_lineTypeLabel = new DLabel(this);
    m_lineTypeLabel->setObjectName("Line Type");
    m_lineTypeLabel->setText(tr("Type"));
    m_lineTypeLabel->setFont(ft);

    QMap<int, QMap<CCheckButton::EButtonSattus, QString> > pictureMap;

    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Normal] = QString(":/theme/light/images/attribute/line tool_normal.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Hover] = QString(":/theme/light/images/attribute/line tool_hover.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Press] = QString(":/theme/light/images/attribute/line tool_press.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Active] = QString(":/theme/light/images/attribute/line tool_checked.svg");

    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Normal] = QString(":/theme/dark/images/attribute/line tool_normal.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Hover] = QString(":/theme/dark/images/attribute/line tool_hover.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Press] = QString(":/theme/dark/images/attribute/line tool_press.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Active] = QString(":/theme/dark/images/attribute/line tool_checked.svg");

    m_straightline = new CCheckButton(pictureMap, QSize(36, 36), this);
    m_actionButtons.append(m_straightline);


    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Normal] = QString(":/theme/light/images/attribute/arrow tool_normal.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Hover] = QString(":/theme/light/images/attribute/arrow tool_hover.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Press] = QString(":/theme/light/images/attribute/arrow tool_press.svg");
    pictureMap[DGuiApplicationHelper::LightType][CCheckButton::Active] = QString(":/theme/light/images/attribute/arrow tool_checked.svg");

    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Normal] = QString(":/theme/dark/images/attribute/arrow tool_normal.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Hover] = QString(":/theme/dark/images/attribute/arrow tool_hover.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Press] = QString(":/theme/dark/images/attribute/arrow tool_press.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CCheckButton::Active] = QString(":/theme/dark/images/attribute/arrow tool_checked.svg");

    m_arrowline = new CCheckButton(pictureMap, QSize(36, 36), this);
    m_actionButtons.append(m_arrowline);

    if (CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->getLineType() == straightType) {
        m_straightline->setChecked(true);
    } else {
        m_arrowline->setChecked(true);
    }

//    DLabel *strokeLabel = new DLabel(this);
//    strokeLabel->setObjectName("StrokeLabel");
//    strokeLabel->setText(tr("描边"));
//    ft.setPixelSize(TEXT_SIZE);
//    strokeLabel->setFont(ft);

    m_strokeBtn = new BorderColorButton(this);

    m_sep1Line = new SeperatorLine(this);

    m_lwLabel = new DLabel(this);
    m_lwLabel->setObjectName("Border Label");
    m_lwLabel->setText(tr("Width"));
    QFont ft1;
    ft1.setPixelSize(TEXT_SIZE - 1);
    m_lwLabel->setFont(ft1);

    m_sideWidthWidget = new CSideWidthWidget(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_lineTypeLabel);
    layout->addWidget(m_straightline);
    layout->addWidget(m_arrowline);
    layout->setSpacing(BTN_SPACNT);
    layout->addSpacing(16);
    layout->addWidget(m_strokeBtn);
    //layout->addWidget(strokeLabel);

    layout->addWidget(m_sep1Line, 0, Qt::AlignCenter);

    layout->addWidget(m_lwLabel);
    layout->addWidget(m_sideWidthWidget);
    layout->addStretch();
    setLayout(layout);
}

void LineWidget::initConnection()
{
    connect(m_strokeBtn, &BorderColorButton::btnCheckStateChanged, this, [ = ](bool show) {

        QPoint btnPos = mapToGlobal(m_strokeBtn->pos());
        QPoint pos(btnPos.x() + m_strokeBtn->width() / 2,
                   btnPos.y() + m_strokeBtn->height());

        showColorPanel(DrawStatus::Stroke, pos, show);
    });

    connect(this, &LineWidget::resetColorBtns, this, [ = ] {
        m_strokeBtn->resetChecked();
    });

    ///线宽
    connect(m_sideWidthWidget, &CSideWidthWidget::signalSideWidthChange, this, [ = ] () {
        emit signalLineAttributeChanged();
    });
    connect(m_sideWidthWidget, &CSideWidthWidget::signalSideWidthMenuShow, this, [ = ] () {
        //隐藏调色板
        showColorPanel(DrawStatus::Stroke, QPoint(), false);
    });
    //描边粗细
    connect(m_sideWidthWidget, SIGNAL(signalSideWidthChoosed(int)), this, SLOT(slotSideWidthChoosed(int)));

    connect(m_straightline, &CCheckButton::buttonClick, [this]() {
        clearOtherSelections(m_straightline);
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineType(straightType);
        emit signalLineAttributeChanged();
        //隐藏调色板
        showColorPanel(DrawStatus::Stroke, QPoint(), false);
    });

    connect(m_arrowline, &CCheckButton::buttonClick, [this]() {
        clearOtherSelections(m_arrowline);
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineType(arrowType);
        emit signalLineAttributeChanged();
        //隐藏调色板
        showColorPanel(DrawStatus::Stroke, QPoint(), false);
    });


}

void LineWidget::updateLineWidget()
{
    m_strokeBtn->updateConfigColor();
    m_sideWidthWidget->updateSideWidth();

    if (CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->getLineType() == straightType) {
        m_straightline->setChecked(true);
        clearOtherSelections(m_straightline);
    } else {
        m_arrowline->setChecked(true);
        clearOtherSelections(m_arrowline);
    }

    m_lineTypeLabel->setVisible(true);
    m_straightline->setVisible(true);
    m_arrowline->setVisible(true);
    m_strokeBtn->setVisible(true);
    m_lwLabel->setVisible(true);
    m_sideWidthWidget->setVisible(true);
    CManagerAttributeService::getInstance()->refreshSelectedCommonProperty();
}

void LineWidget::slotSideWidthChoosed(int width)
{
    CManagerAttributeService::getInstance()->setItemsCommonPropertyValue(LineWidth, width);
}

void LineWidget::clearOtherSelections(CCheckButton *clickedButton)
{
    foreach (CCheckButton *button, m_actionButtons) {
        if (button->isChecked() && button != clickedButton) {
            button->setChecked(false);
            return;
        }
    };
}




