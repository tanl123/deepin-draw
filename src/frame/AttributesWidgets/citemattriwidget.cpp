/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author: Ji Xianglong<jixianglong@uniontech.com>
*
* Maintainer: Ji Xianglong <jixianglong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "citemattriwidget.h"
#include "cgraphicsitem.h"
#include "cgraphicsitemselectedmgr.h"
#include "cgraphicsrectitem.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicslineitem.h"
#include "cgraphicsitemselectedmgr.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspenitem.h"
#include "cgraphicstextitem.h"
#include "cgraphicscutitem.h"
#include "cgraphicstriangleitem.h"
#include "cpictureitem.h"
#include "cdrawscene.h"
#include "cundoredocommand.h"
#include "cviewmanagement.h"
#include "cgraphicsview.h"
#include "textwidget.h"
#include "ccutwidget.h"
#include "cmanagerattributeservice.h"
#include "cdrawtoolmanagersigleton.h"
#include "ccuttool.h"
#include "blurwidget.h"
#include "cgraphicsmasicoitem.h"
#include "cpicturewidget.h"
#include "cpictureitem.h"

#include <DComboBox>

#include <QLineEdit>

CItemAttriWidget::CItemAttriWidget(QWidget *parent)
    : QWidget(parent)
{
    QMetaObject::invokeMethod(this, "initUiWhenShow", Qt::QueuedConnection);
}

void CItemAttriWidget::setGraphicItem(CGraphicsItem *pItem)
{
    _pItem = pItem;
    refresh();
}

CGraphicsItem *CItemAttriWidget::graphicItem()
{
    return _pItem;
}

void CItemAttriWidget::refresh()
{
}

CItemAttriWidget::CCmdBlock::CCmdBlock(CDrawScene *pScene)
    : _pScene(pScene)
{
    //记录undo
    QList<QVariant> vars;
    vars << reinterpret_cast<long long>(pScene);
    vars << pScene->sceneRect();
    CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                        CSceneUndoRedoCommand::ESizeChanged, vars, true);
}

CItemAttriWidget::CCmdBlock::CCmdBlock(CGraphicsItem *pItem, EChangedPhase phase, bool doRedo)
    : _pItem(pItem)
    , _phase(phase)
    , _doRedo(doRedo)
{
    if (_pItem == nullptr)
        return;

    if (_phase == EChangedUpdate || _phase == EChangedFinished)
        return;

    if (_pItem->type() == CutType) {
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pItem->drawScene());
        vars << _pItem->drawScene()->sceneRect();
        CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::ESizeChanged, vars, true);
    }

    QList<CGraphicsItem *> items;
    if (_pItem->type() == MgrType) {
        items = dynamic_cast<CGraphicsItemSelectedMgr *>(_pItem)->getItems();

    } else {
        items.append(pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        CGraphicsItem *pItem = items[i];

        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(pItem);
        QVariant varInfo;
        varInfo.setValue(pItem->getGraphicsUnit(false));
        vars << varInfo;

        if (_phase == EChangedBegin || _phase == EChanged) {
            CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::EItemChangedCmd,
                                                CItemUndoRedoCommand::EAllChanged, vars, i == 0);
        }
    }
}

CItemAttriWidget::CCmdBlock::~CCmdBlock()
{
    if (_pScene != nullptr) {
        //记录undo
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pScene);
        vars << _pScene->sceneRect();
        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::ESizeChanged, vars);

        CUndoRedoCommand::finishRecord(false);
        return;
    }

    if (_pItem == nullptr)
        return;

    if (_phase != EChangedFinished && _phase != EChanged)
        return;

    if (_pItem->type() == CutType) {
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pItem->drawScene());
        vars << _pItem->drawScene()->sceneRect();
        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::ESizeChanged, vars);
    }

    QList<CGraphicsItem *> items;
    if (_pItem->type() == MgrType) {
        items = dynamic_cast<CGraphicsItemSelectedMgr *>(_pItem)->getItems();

    } else {
        items.append(_pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        CGraphicsItem *pItem = items[i];

        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(pItem);
        QVariant varInfo;
        varInfo.setValue(pItem->getGraphicsUnit(false));
        vars << varInfo;

        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::EItemChangedCmd,
                                            CItemUndoRedoCommand::EAllChanged, vars);
    }

    if (_pItem->drawScene() != nullptr) {
        _pItem->drawScene()->finishRecord(_doRedo);
    }
}

CComAttrWidget::CComAttrWidget(QWidget *parent)
    : CItemAttriWidget(parent)
{
    setAttribute(Qt::WA_NoMousePropagation, true);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addStretch();
    lay->setMargin(0);
    lay->addLayout(getLayout());
    lay->addStretch();
    setLayout(lay);
}

void CComAttrWidget::showByType(CComAttrWidget::EAttriSourceItemType tp, CGraphicsItem *pItem)
{
    _pItem = pItem;
    clearUi();
    refreshHelper(tp);
    refreshDataHelper(tp);
}

QList<CGraphicsItem *> CComAttrWidget::graphicItems()
{
    QList<CGraphicsItem *> list;
    if (_pItem != nullptr) {
        if (_pItem->type() == MgrType) {
            return dynamic_cast<CGraphicsItemSelectedMgr *>(_pItem)->getItems();
        }
        list << _pItem;
        return list;
    }
    return QList<CGraphicsItem *>();
}

CSceneDefaultData &CComAttrWidget::defualtData()
{
    return m_defualDatas;
}

SComDefualData CComAttrWidget::defualtSceneData(CDrawScene *pScene)
{
    CDrawScene *pScen = (pScene == nullptr ? CManageViewSigleton::GetInstance()->getCurView()->drawScene() : pScene);
    assert(pScen != nullptr);
    return m_defualDatas[pScen];
}

void CComAttrWidget::setWindowTittle(QString tittle)
{
    getTitleLabel()->setText(tittle);
}

void CComAttrWidget::refresh()
{
    clearUi();

    int tp = getSourceTpByItem(this->graphicItem());
    if (tp < 0)
        return;

    refreshHelper(tp);

    refreshDataHelper(tp);

    // 解决焦点被移动到菜单位置的问题
    this->setFocus();
}

void CComAttrWidget::initUiWhenShow()
{
    refreshHelper(ShowTitle);
}

void CComAttrWidget::clearUi()
{
    //1.隐藏所有控件
    getTitleLabel()->hide();
    getPenColorBtn()->hide();
    getBrushColorBtn()->hide();
    getBorderWidthWidget()->hide();
    getSpLine()->hide();
    getSpinBoxForRectRadius()->hide();
    getSpinBoxForRectRadiusLabel()->hide();
    getSpinBoxForStarAnchor()->hide();
    getSpinBoxForStarinterRadius()->hide();
    getSpinBoxForStarAnchorLabel()->hide();
    getSpinBoxForStarinterRadiusLabel()->hide();
    getSpinBoxForPolgonSideNum()->hide();
    getSpinBoxForPolgonSideNumLabel()->hide();
    getComboxForLineStartStyle()->hide();
    getComboxForLineEndStyle()->hide();
    getLabelForLineStartStyle()->hide();
    getLabelForLineEndStyle()->hide();
    getMaskLabForLineStartStyle()->hide();
    getMaskLabForLineEndStyle()->hide();
    getPenColorBtn()->hide();

    getTextWidgetForText()->hide();
    getCutWidget()->hide();
    getBlurWidget()->hide();
    getPictureWidget()->hide();

    //2.清理原先的布局内的控件
    QHBoxLayout *pLay = getLayout();
    for (int i = 0; i < pLay->count();) {
        pLay->takeAt(i);
    }
}

int CComAttrWidget::getSourceTpByItem(CGraphicsItem *pItem)
{
    int retTp = 0;

    if (pItem == nullptr) {
        retTp = ShowTitle;
        return retTp;
    }

    if (MgrType == pItem->type()) {
        QList<CGraphicsItem *> bzItems = dynamic_cast<CGraphicsItemSelectedMgr *>(pItem)->getItems();
        for (CGraphicsItem *p : bzItems) {
            if (p != nullptr)
                retTp |= getSourceTpByItem(p);
        }

        if (this->isNeededNothing(retTp)) {
            retTp = ShowTitle;
        }
        return retTp;
    }

    retTp = getSourceTpByItemType(pItem->type());

    return retTp;
}

int CComAttrWidget::getSourceTpByItemType(int itemType)
{
    int retTp = 0;

    switch (itemType) {
    case RectType: {
        retTp = Rect;
        break;
    }
    case EllipseType: {
        retTp = Ellipse;
        break;
    }
    case TriangleType: {
        retTp = Triangle;
        break;
    }
    case PolygonalStarType: {
        retTp = Star;
        break;
    }
    case PolygonType: {
        retTp = Polygon;
        break;
    }
    case LineType: {
        retTp = Line;
        break;
    }
    case PenType: {
        retTp = Pen;
        break;
    }
    case TextType: {
        retTp = Text;
        break;
    }
    case PictureType: {
        retTp = Image;
        break;
    }
    case CutType: {
        retTp = Cut;
        break;
    }
    case BlurType: {
        retTp = MasicPen;
        break;
    }
    default:
        retTp = -100;
    }
    return retTp;
}

SComDefualData CComAttrWidget::getGraphicItemsDefualData(int tp)
{
    SComDefualData data;
    CGraphicsUnit unitData = graphicItem()->getGraphicsUnit(false);
    data.penColor = unitData.head.pen.color();
    data.penWidth = unitData.head.pen.width();
    data.bursh = unitData.head.brush;

    data.rectRadius = (tp == Rect ? unitData.data.pRect->xRedius : data.rectRadius);
    data.polySideCount = (tp == Polygon ? unitData.data.pPolygon->pointNum : data.polySideCount);
    data.starAnCount = (tp == Star ? unitData.data.pPolygonStar->anchorNum : data.polySideCount);
    data.starInRadiusRadio = (tp == Star ? unitData.data.pPolygonStar->radius : data.polySideCount);

    data.lineStartType = (tp == Pen ? unitData.data.pPen->start_type : data.lineStartType);
    data.lineEndType = (tp == Pen ? unitData.data.pPen->end_type : data.lineEndType);

    data.lineStartType = (tp == Line ? unitData.data.pLine->start_type : data.lineStartType);
    data.lineEndType = (tp == Line ? unitData.data.pLine->end_type : data.lineEndType);

    data.blurType = (tp == MasicPen ? static_cast<EBlurEffect>(unitData.data.pBlur->effect) : data.blurType);
    data.blurWidth = (tp == MasicPen ? data.penWidth : data.blurWidth);

    if (tp == Text) {
        data.textColor = unitData.data.pText->color;
        data.textFontSize = int(unitData.data.pText->font.pointSizeF());
        data.textFontFamily = unitData.data.pText->font.family();
    } else if (tp == Image) {
        if (graphicItem()->sceneBoundingRect() != graphicItem()->drawScene()->sceneRect()) {
            data.comVaild[PropertyImageAdjustScence] = true;
        } else {
            data.comVaild[PropertyImageAdjustScence] = false;
        }
        data.comVaild[PropertyImageFlipType] = true;
    }

    unitData.data.release();

    QList<CGraphicsItem *> lists = graphicItems();

    if (lists.count() > 1) {
        //多选时 判断值不同时，应该设置为无效
        for (CGraphicsItem *pItem : lists) {
            if (pItem->pen().color() != data.penColor) {
                data.comVaild[LineColor] = false;
            }
            if (pItem->pen().width() != data.penWidth) {
                data.comVaild[LineWidth] = false;
            }
            if (pItem->brush().color() != data.bursh.color()) {
                data.comVaild[FillColor] = false;
            }
            //都是矩形
            if (tp == Rect) {
                CGraphicsRectItem *pRect = dynamic_cast<CGraphicsRectItem *>(pItem);
                if (pRect->getXRedius() != data.rectRadius) {
                    data.comVaild[RectRadius] = false;
                }
            } else if (tp == Star) {
                CGraphicsPolygonalStarItem *pStar = dynamic_cast<CGraphicsPolygonalStarItem *>(pItem);
                if (pStar->anchorNum() != data.starAnCount) {
                    data.comVaild[Anchors] = false;
                }
                if (pStar->innerRadius() != data.starInRadiusRadio) {
                    data.comVaild[StarRadius] = false;
                }
            } else if (tp == Polygon) {
                CGraphicsPolygonItem *polygon = dynamic_cast<CGraphicsPolygonItem *>(pItem);
                if (polygon->nPointsCount() != data.polySideCount) {
                    data.comVaild[SideNumber] = false;
                }
            } else if (tp == Pen) {
                CGraphicsPenItem *pPen = dynamic_cast<CGraphicsPenItem *>(pItem);
                if (pPen->getPenStartType() != data.lineStartType) {
                    data.comVaild[LineAndPenStartType] = false;
                }
                if (pPen->getPenEndType() != data.lineEndType) {
                    data.comVaild[LineAndPenEndType] = false;
                }
            } else if (tp == Line) {
                CGraphicsLineItem *pLIne = dynamic_cast<CGraphicsLineItem *>(pItem);
                if (pLIne->getLineStartType() != data.lineStartType) {
                    data.comVaild[LineAndPenStartType] = false;
                }
                if (pLIne->getLineEndType() != data.lineEndType) {
                    data.comVaild[LineAndPenEndType] = false;
                }
            } else if (tp == Text) {
                CGraphicsTextItem *pText = dynamic_cast<CGraphicsTextItem *>(pItem);
                if (int(pText->getFontSize()) != data.textFontSize) {
                    data.comVaild[TextSize] = false;
                }
                if (pText->getFontFamily() != data.textFontFamily) {
                    data.comVaild[TextFont] = false;
                }
                if (pText->getTextFontStyle() != data.textFontHeavy) {
                    data.comVaild[TextHeavy] = false;
                }
                if (pText->getTextColor() != data.textColor) {
                    data.comVaild[TextColor] = false;
                }
            } else if (tp == MasicPen) {
                CGraphicsMasicoItem *pBlur = dynamic_cast<CGraphicsMasicoItem *>(pItem);
                if (pBlur->getBlurWidth() != data.blurWidth) {
                    data.comVaild[BlurWidth] = false;
                }
                if (pBlur->getBlurEffect() != data.blurType) {
                    data.comVaild[Blurtype] = false;
                }
            } else if (tp == Image) {
                if (graphicItem()->sceneBoundingRect() != graphicItem()->drawScene()->sceneRect()) {
                    data.comVaild[PropertyImageAdjustScence] = true;
                } else {
                    data.comVaild[PropertyImageAdjustScence] = false;
                }
                data.comVaild[PropertyImageFlipType] = false;
            }
        }
    }

    return data;
}

bool CComAttrWidget::isNeededNothing(int tp)
{
    int count = graphicItems().count();
    if (count > 1) {
        if ((tp & Text || tp & MasicPen || tp & Image) &&
                (tp != Text && tp != MasicPen && tp != Image)) {
            return true;
        }
    }
    return false;
}

bool CComAttrWidget::isBrushColorNeeded(int tp)
{
    if (isNeededNothing(tp)) {
        return false;
    }

    if (tp & Rect || tp & Ellipse || tp & Triangle || tp & Polygon || tp & Star) {
        return true;
    }
    return false;
}

bool CComAttrWidget::isBorderNeeded(int tp)
{
    if (isNeededNothing(tp)) {
        return false;
    }

    return true;
}

bool CComAttrWidget::isSpecialItem(int tp)
{
    if (tp == Text || tp == MasicPen || tp == Image || tp == Cut) {
        return true;
    }
    return false;
}

void CComAttrWidget::refreshHelper(int tp)
{
    QHBoxLayout *layout = getLayout();

    if (isSpecialItem(tp)) {
        if (tp == Text) {
            layout->addWidget(getTextWidgetForText());
            getTextWidgetForText()->show();
        } else if (tp == Image) {
            layout->addWidget(getPictureWidget());
            getPictureWidget()->show();
        } else if (tp == Cut) {
            layout->addWidget(getCutWidget());
            getCutWidget()->show();
        } else if (tp == MasicPen) {
            layout->addWidget(getBlurWidget());
            getBlurWidget()->show();
        }
        return;
    }


    if (tp == ShowTitle) {
        layout->addWidget(getTitleLabel());
        getTitleLabel()->show();
        return;
    }
    if (isBrushColorNeeded(tp)) {
        layout->addWidget(getBrushColorBtn());
        getBrushColorBtn()->show();
    }

    if (isBorderNeeded(tp)) {
        layout->addWidget(getPenColorBtn());
        layout->addWidget(getBorderWidthWidget());
        getPenColorBtn()->show();
        getBorderWidthWidget()->show();
    }

    switch (tp) {
    case Rect: {
        layout->addWidget(getSpLine());
        layout->addWidget(getSpinBoxForRectRadiusLabel());
        layout->addWidget(getSpinBoxForRectRadius());
        getSpLine()->show();
        getSpinBoxForRectRadiusLabel()->show();
        getSpinBoxForRectRadius()->show();
        break;
    }
    case Polygon: {
        layout->addWidget(getSpLine());
        layout->addWidget(getSpinBoxForPolgonSideNumLabel());
        layout->addWidget(getSpinBoxForPolgonSideNum());
        getSpLine()->show();
        getSpinBoxForPolgonSideNumLabel()->show();
        getSpinBoxForPolgonSideNum()->show();
        break;
    }
    case Star: {
        layout->addWidget(getSpLine());
        layout->addWidget(getSpinBoxForStarAnchorLabel());
        layout->addWidget(getSpinBoxForStarAnchor());
        getSpLine()->show();
        getSpinBoxForStarAnchorLabel()->show();
        getSpinBoxForStarAnchor()->show();

        layout->addWidget(getSpinBoxForStarinterRadiusLabel());
        layout->addWidget(getSpinBoxForStarinterRadius());
        getSpinBoxForStarinterRadiusLabel()->show();
        getSpinBoxForStarinterRadius()->show();
        break;
    }
    case Pen:
    case Line: {
        layout->addWidget(getSpLine());
        layout->addWidget(getLabelForLineStartStyle());
        layout->addWidget(getComboxForLineStartStyle());
        getSpLine()->show();
        getLabelForLineStartStyle()->show();
        getComboxForLineStartStyle()->show();

        layout->addSpacing(12);
        layout->addWidget(getLabelForLineEndStyle());
        layout->addWidget(getComboxForLineEndStyle());
        getLabelForLineEndStyle()->show();
        getComboxForLineEndStyle()->show();
        break;
    }
    default:
        break;
    }
}

void CComAttrWidget::refreshDataHelper(int tp)
{
    if (tp == ShowTitle) {
        return;
    }

    SComDefualData data = defualtSceneData();
    if (graphicItem() != nullptr) {
        data = getGraphicItemsDefualData(tp);
    }

    if (isSpecialItem(tp)) {
        if (tp == Text) {
            getTextWidgetForText()->setFontSize(data.textFontSize);
            getTextWidgetForText()->setTextColor(data.textColor);
            getTextWidgetForText()->setTextFamilyStyle(data.textFontFamily);
            getTextWidgetForText()->setVaild(data.comVaild[TextColor], data.comVaild[TextSize],
                                             data.comVaild[TextFont], data.comVaild[TextHeavy]);
        } else if (tp == Image) {
            getPictureWidget()->setAdjustmentIsEnable(data.comVaild[PropertyImageAdjustScence], false);
            getPictureWidget()->setRotationEnable(data.comVaild[PropertyImageFlipType]);
        } else if (tp == Cut) {
            getCutWidget()->setCutSize(data.cutSize, false);
            getCutWidget()->setCutType(data.cutType, false);
        } else if (tp == MasicPen) {
            getBlurWidget()->setBlurWidth(data.blurWidth, false);
            getBlurWidget()->setBlurType(data.blurType, false);
            getBlurWidget()->setSameProperty(data.comVaild[Blurtype], data.comVaild[BlurWidth]);
        }
        return;
    }

    if (isBrushColorNeeded(tp)) {
        CBlockObjectSig sig(getBrushColorBtn());
        getBrushColorBtn()->setColor(data.bursh.color());
        getBrushColorBtn()->setIsMultColorSame(data.comVaild[FillColor]);
    }

    if (isBorderNeeded(tp)) {
        CBlockObjectSig sig(getPenColorBtn());
        CBlockObjectSig sig1(getBorderWidthWidget());
        getPenColorBtn()->setColor(data.penColor);
        getPenColorBtn()->setIsMultColorSame(data.comVaild[LineColor]);
        getBorderWidthWidget()->setSideWidth(data.penWidth);
        getBorderWidthWidget()->setMenuNoSelected(!data.comVaild[LineWidth]);
    }
    switch (tp) {
    case Rect: {
        CBlockObjectSig sig(getSpinBoxForRectRadius());
        getSpinBoxForRectRadius()->setValue(data.rectRadius);
        if (!data.comVaild[RectRadius])
            getSpinBoxForRectRadius()->setValue(-1);
        break;
    }
    case Polygon: {
        CBlockObjectSig sig(getSpinBoxForPolgonSideNum());
        getSpinBoxForPolgonSideNum()->setValue(data.polySideCount);
        if (!data.comVaild[SideNumber])
            getSpinBoxForPolgonSideNum()->setValue(-1);
        break;
    }
    case Star: {
        CBlockObjectSig sig(getSpinBoxForStarAnchor());
        CBlockObjectSig sig1(getSpinBoxForStarinterRadius());

        getSpinBoxForStarAnchor()->setValue(data.starAnCount);
        getSpinBoxForStarinterRadius()->setValue(data.starInRadiusRadio);

        if (!data.comVaild[Anchors])
            getSpinBoxForStarAnchor()->setValue(-1);
        if (!data.comVaild[StarRadius])
            getSpinBoxForStarinterRadius()->setValue(-1);

        break;
    }
    case Pen:
    case Line: {
        CBlockObjectSig sig(getComboxForLineStartStyle());
        CBlockObjectSig sig1(getComboxForLineEndStyle());

        getComboxForLineStartStyle()->setCurrentIndex(data.lineStartType);
        getComboxForLineEndStyle()->setCurrentIndex(data.lineEndType);
        if (!data.comVaild[LineAndPenStartType])
            getComboxForLineStartStyle()->setCurrentIndex(noneLine);
        if (!data.comVaild[LineAndPenEndType])
            getComboxForLineEndStyle()->setCurrentIndex(noneLine);
        break;
    }
    default:
        break;
    }
}

QHBoxLayout *CComAttrWidget::getLayout()
{
    if (m_contrlLay == nullptr) {
        m_contrlLay = new QHBoxLayout;
        m_contrlLay->setMargin(0);
        m_contrlLay->setContentsMargins(0, 0, 0, 0);
    }
    return m_contrlLay;
}

DLabel *CComAttrWidget::getTitleLabel()
{
    if (m_title == nullptr) {
        m_title = new DLabel(this);
    }
    return m_title;
}

CPenColorBtn *CComAttrWidget::getPenColorBtn()
{
    if (m_strokeBtn == nullptr) {
        m_strokeBtn = new CPenColorBtn(this);
        connect(m_strokeBtn, &CPenColorBtn::colorChanged, this, [ = ](const QColor & color, EChangedPhase phase) {
            QList<CGraphicsItem *> lists = this->graphicItems();
            if (!lists.isEmpty()) {
                CCmdBlock block(this->graphicItem(), phase);

                for (CGraphicsItem *pItem : lists) {
                    QPen p = pItem->pen();
                    pItem->setPenColor(color, phase == EChangedUpdate || phase == EChangedBegin);
                }
            }
            this->updateDefualData(LineColor, color);
        });
    }
    return m_strokeBtn;
}

CBrushColorBtn *CComAttrWidget::getBrushColorBtn()
{
    if (m_fillBtn == nullptr) {
        m_fillBtn = new CBrushColorBtn(this);
        connect(m_fillBtn, &CBrushColorBtn::colorChanged, this, [ = ](const QColor & color, EChangedPhase phase) {
            qDebug() << "color = " << color << "phase = " << phase;
            QList<CGraphicsItem *> lists = this->graphicItems();
            if (!lists.isEmpty()) {
                CCmdBlock block(this->graphicItem(), phase);
                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *pItem : lists) {
                    pItem->setBrushColor(color, phase == EChangedUpdate || phase == EChangedBegin);
                }
            }
            this->updateDefualData(FillColor, QBrush(color, Qt::SolidPattern));
        });
    }
    return m_fillBtn;
}

CSideWidthWidget *CComAttrWidget::getBorderWidthWidget()
{
    if (m_sideWidthWidget == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_sideWidthWidget = new CSideWidthWidget(this);
        m_sideWidthWidget->setFixedWidth(100);
        m_sideWidthWidget->setFont(ft);

        connect(m_sideWidthWidget, &CSideWidthWidget::sideWidthChanged, this,
        [ = ](int lineWidth, bool preview) {
            Q_UNUSED(preview)
            QList<CGraphicsItem *> lists = this->graphicItems();
            if (!lists.isEmpty()) {
                CCmdBlock block(this->graphicItem());

                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *pItem : lists) {
                    QPen p = pItem->pen();
                    p.setWidthF(lineWidth);
                    p.setJoinStyle(Qt::MiterJoin);
                    p.setStyle(Qt::SolidLine);
                    p.setCapStyle(Qt::RoundCap);
                    pItem->setPen(p);
                    pItem->updateShape();
                }
            }
            this->updateDefualData(LineWidth, lineWidth);
        });
    }
    return m_sideWidthWidget;
}

SeperatorLine *CComAttrWidget::getSpLine()
{
    if (m_sepLine == nullptr) {
        m_sepLine = new SeperatorLine(this);
    }

    return m_sepLine;
}

CSpinBox *CComAttrWidget::getSpinBoxForRectRadius()
{
    if (m_rediusSpinbox == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_rediusSpinbox = new CSpinBox(this);
        m_rediusSpinbox->setKeyboardTracking(false);
        m_rediusSpinbox->setRange(-1, 1000);
        m_rediusSpinbox->setFixedSize(QSize(85, 36));
        m_rediusSpinbox->setFont(ft);
        m_rediusSpinbox->setSpecialValueText("— —");
        m_rediusSpinbox->setEnabledEmbedStyle(true);
        m_rediusSpinbox->lineEdit()->setClearButtonEnabled(false);
        m_rediusSpinbox->setProperty("preValue", 5);
        m_rediusSpinbox->setValue(5);

        connect(m_rediusSpinbox, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对Rect图元的
                if (getSourceTpByItem(graphicItem()) == Rect) {

                    if (value == -1) {
                        CBlockObjectSig sig(m_rediusSpinbox);
                        m_rediusSpinbox->setValue(0);
                        return;
                    }

                    //记录undo
                    CCmdBlock block(this->graphicItem(), phase);

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        CGraphicsRectItem *pItem = dynamic_cast<CGraphicsRectItem *>(p);
                        pItem->setXYRedius(value, value);
                        pItem->updateShape();
                    }
                    this->updateDefualData(RectRadius, value);
                }
            }
        });
    }
    return m_rediusSpinbox;
}

DLabel *CComAttrWidget::getSpinBoxForRectRadiusLabel()
{
    if (m_rediusLable == nullptr) {
        const int TEXT_SIZE = 13;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_rediusLable = new DLabel(this);
        m_rediusLable->setText(tr("Radius"));
        m_rediusLable->setFont(ft);
    }
    return m_rediusLable;
}

CSpinBox *CComAttrWidget::getSpinBoxForStarAnchor()
{
    if (m_anchorNumber == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_anchorNumber = new CSpinBox(this);
        m_anchorNumber->setKeyboardTracking(false);
        m_anchorNumber->setFixedSize(QSize(85, 36));
        m_anchorNumber->setRange(2, 50);
        m_anchorNumber->setFont(ft);
        m_anchorNumber->setSpecialValueText("— —");
        m_anchorNumber->setEnabledEmbedStyle(true);
        m_anchorNumber->lineEdit()->setClearButtonEnabled(false);

        connect(m_anchorNumber, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对Star图元的
                if (getSourceTpByItem(graphicItem()) == Star) {

                    if (value == 2) {
                        CBlockObjectSig sig(m_anchorNumber);
                        m_anchorNumber->setValue(3);
                        return;
                    }

                    //记录undo
                    CCmdBlock block(this->graphicItem(), phase);

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        CGraphicsPolygonalStarItem *pItem = dynamic_cast<CGraphicsPolygonalStarItem *>(p);
                        pItem->updatePolygonalStar(value, pItem->innerRadius());
                        pItem->updateShape();
                    }

                    this->updateDefualData(Anchors, value);
                }
            }
        });
    }
    return m_anchorNumber;
}

CSpinBox *CComAttrWidget::getSpinBoxForStarinterRadius()
{
    if (m_radiusNumber == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_radiusNumber = new CSpinBox(this);
        m_radiusNumber->setKeyboardTracking(false);
        m_radiusNumber->setRange(-1, 100);
        m_radiusNumber->setFixedSize(QSize(85, 36));
        m_radiusNumber->setSuffix("%");
        m_radiusNumber->setFont(ft);
        m_radiusNumber->setSpecialValueText("— —");
        m_radiusNumber->setEnabledEmbedStyle(true);
        m_radiusNumber->lineEdit()->setClearButtonEnabled(false);

        connect(m_radiusNumber, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对Star图元的
                if (getSourceTpByItem(graphicItem()) == Star) {

                    if (value == -1) {
                        CBlockObjectSig sig(m_radiusNumber);
                        m_radiusNumber->setValue(0);
                        return;
                    }

                    //记录undo
                    CCmdBlock block(this->graphicItem(), phase);

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        CGraphicsPolygonalStarItem *pItem = dynamic_cast<CGraphicsPolygonalStarItem *>(p);
                        pItem->updatePolygonalStar(pItem->anchorNum(), value);
                    }
                    this->updateDefualData(StarRadius, value);
                }
            }
        });
    }
    return m_radiusNumber;
}

DLabel *CComAttrWidget::getSpinBoxForStarAnchorLabel()
{
    if (m_anchorNumLabel == nullptr) {
        const int TEXT_SIZE = 13;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_anchorNumLabel = new DLabel(this);
        m_anchorNumLabel->setText(tr("Points"));
        m_anchorNumLabel->setFont(ft);
    }
    return m_anchorNumLabel;
}

DLabel *CComAttrWidget::getSpinBoxForStarinterRadiusLabel()
{
    if (m_radiusLabel == nullptr) {
        const int TEXT_SIZE = 13;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_radiusLabel = new DLabel(this);
        m_radiusLabel->setText(tr("Diameter"));
        m_radiusLabel->setFont(ft);
    }
    return m_radiusLabel;
}

CSpinBox *CComAttrWidget::getSpinBoxForPolgonSideNum()
{
    if (m_sideNumSpinBox == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_sideNumSpinBox = new CSpinBox(this);
        m_sideNumSpinBox->setKeyboardTracking(false);
        m_sideNumSpinBox->setFixedSize(QSize(85, 36));
        m_sideNumSpinBox->setRange(3, 10);
        m_sideNumSpinBox->setFont(ft);
        m_sideNumSpinBox->setSpecialValueText("— —");
        m_sideNumSpinBox->setEnabledEmbedStyle(true);
        m_sideNumSpinBox->lineEdit()->setClearButtonEnabled(false);

        connect(m_sideNumSpinBox, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对polgon图元的
                if (getSourceTpByItem(graphicItem()) == Polygon) {
                    if (value == 3) {
                        CBlockObjectSig sig(m_sideNumSpinBox);
                        m_sideNumSpinBox->setValue(4);
                        return;
                    }

                    //记录undo
                    CCmdBlock block(this->graphicItem(), phase);

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        CGraphicsPolygonItem *pItem = dynamic_cast<CGraphicsPolygonItem *>(p);
                        pItem->setPointCount(value);
                    }
                    this->updateDefualData(SideNumber, value);
                }
            }
        });
    }
    return m_sideNumSpinBox;
}

DLabel *CComAttrWidget::getSpinBoxForPolgonSideNumLabel()
{
    if (m_sideNumLabel == nullptr) {
        const int TEXT_SIZE = 13;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_sideNumLabel = new DLabel(this);
        m_sideNumLabel->setText(tr("Sides"));
        m_sideNumLabel->setFont(ft);
    }
    return m_sideNumLabel;
}

DComboBox *CComAttrWidget::getComboxForLineStartStyle()
{
    if (m_lineStartComboBox == nullptr) {
        m_lineStartComboBox = new DComboBox(this);
        m_lineStartComboBox->setFixedSize(QSize(90, 36));
        m_lineStartComboBox->setIconSize(QSize(34, 20));

        m_lineStartComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
        m_lineStartComboBox->addItem(QIcon::fromTheme("ddc_right_circle"), "");
        m_lineStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_circle"), "");
        m_lineStartComboBox->addItem(QIcon::fromTheme("ddc_right_arrow"), "");
        m_lineStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_arrow"), "");

        connect(m_lineStartComboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, [ = ](int index) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对pen 或者 line图元的
                int srTp = getSourceTpByItem(graphicItem());
                if (srTp == Pen || srTp == Line || srTp == (Pen | Line)) {
                    //记录undo
                    CCmdBlock block(this->graphicItem());

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        if (p->type() == PenType) {
                            CGraphicsPenItem *pItem = dynamic_cast<CGraphicsPenItem *>(p);
                            pItem->setPenStartType(ELineType(index));
                        } else {
                            CGraphicsLineItem *pItem = dynamic_cast<CGraphicsLineItem *>(p);
                            pItem->setLineStartType(ELineType(index));
                        }
                    }
                }
            }
            this->updateDefualData(LineAndPenStartType, index);
        });
    }
    return m_lineStartComboBox;
}

DComboBox *CComAttrWidget::getComboxForLineEndStyle()
{
    if (m_lineEndComboBox == nullptr) {
        m_lineEndComboBox = new DComboBox(this);
        m_lineEndComboBox->setFixedSize(QSize(90, 36));
        m_lineEndComboBox->setIconSize(QSize(34, 20));

        m_lineEndComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
        m_lineEndComboBox->addItem(QIcon::fromTheme("ddc_left_circle"), "");
        m_lineEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_circle"), "");
        m_lineEndComboBox->addItem(QIcon::fromTheme("ddc_left_arrow"), "");
        m_lineEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_arrow"), "");

        connect(m_lineEndComboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, [ = ](int index) {
            if (this->graphicItem() != nullptr) {
                //要知道这个控件是针对pen 或者 line图元的
                int srTp = getSourceTpByItem(graphicItem());
                if (srTp == Pen || srTp == Line || srTp == (Pen | Line)) {
                    //记录undo
                    CCmdBlock block(this->graphicItem());

                    QList<CGraphicsItem *> lists = this->graphicItems();
                    for (CGraphicsItem *p : lists) {
                        if (p->type() == PenType) {
                            CGraphicsPenItem *pItem = dynamic_cast<CGraphicsPenItem *>(p);
                            pItem->setPenEndType(ELineType(index));
                        } else {
                            CGraphicsLineItem *pItem = dynamic_cast<CGraphicsLineItem *>(p);
                            pItem->setLineEndType(ELineType(index));
                        }
                    }
                }
            }
            this->updateDefualData(LineAndPenEndType, index);
        });
    }
    return m_lineEndComboBox;
}

DLabel *CComAttrWidget::getLabelForLineStartStyle()
{
    if (m_startLabel == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);

        m_startLabel = new DLabel(this);
        m_startLabel->setText(tr("Start"));
        m_startLabel->setFont(ft);
    }
    return m_startLabel;
}

DLabel *CComAttrWidget::getLabelForLineEndStyle()
{
    if (m_endLabel == nullptr) {
        const int TEXT_SIZE = 14;
        QFont ft;
        ft.setPixelSize(TEXT_SIZE);
        m_endLabel = new DLabel(this);
        m_endLabel->setText(tr("End"));
        m_endLabel->setFont(ft);
    }
    return m_endLabel;
}

DLabel *CComAttrWidget::getMaskLabForLineStartStyle()
{
    if (m_maskLableStart == nullptr) {
        m_maskLableStart = new DLabel(getComboxForLineStartStyle());
        m_maskLableStart->setText("— —");
        m_maskLableStart->move(6, 6);
        m_maskLableStart->setFixedSize(35, 20);
        m_maskLableStart->setVisible(false);
        m_maskLableStart->setFont(m_maskLableStart->font());
    }
    return m_maskLableStart;
}

DLabel *CComAttrWidget::getMaskLabForLineEndStyle()
{
    if (m_maskLableEnd == nullptr) {
        m_maskLableEnd = new DLabel(getComboxForLineEndStyle());
        m_maskLableEnd->setText("— —");
        m_maskLableEnd->move(6, 6);
        m_maskLableEnd->setFixedSize(35, 20);
        m_maskLableEnd->setVisible(false);
        m_maskLableEnd->setFont(m_maskLableEnd->font());
    }
    return m_maskLableEnd;
}

TextWidget *CComAttrWidget::getTextWidgetForText()
{
    if (m_TextWidget == nullptr) {
        m_TextWidget = new TextWidget(this);
        m_TextWidget->setAttribute(Qt::WA_NoMousePropagation, true);
        connect(m_TextWidget, &TextWidget::fontSizeChanged, this, [ = ](int size) {
            qDebug() << "fontSizeChanged = " << size;
            if (this->getSourceTpByItem(this->graphicItem()) == Text) {
                //记录undo
                CCmdBlock block(this->graphicItem());

                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *p : lists) {
                    CGraphicsTextItem *pItem = dynamic_cast<CGraphicsTextItem *>(p);
                    pItem->setFontSize(size);
                }
            }
            this->updateDefualData(TextSize, size);
        });
        connect(m_TextWidget, &TextWidget::fontFamilyChanged, this, [ = ](const QString & family, bool preview) {
            qDebug() << "fontFamilyChanged = " << family << "preview = " << preview;
            if (this->getSourceTpByItem(this->graphicItem()) == Text) {
                //记录undo
                CCmdBlock block(preview ? nullptr : this->graphicItem());

                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *p : lists) {
                    CGraphicsTextItem *pItem = dynamic_cast<CGraphicsTextItem *>(p);
                    pItem->setFontFamily(family);
                }
            }
            this->updateDefualData(TextFont, family);
        });
        connect(m_TextWidget, &TextWidget::fontStyleChanged, this, [ = ](const QString & style) {
            qDebug() << "fontStyleChanged = " << style;
            if (this->getSourceTpByItem(this->graphicItem()) == Text) {
                //记录undo
                CCmdBlock block(this->graphicItem());

                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *p : lists) {
                    CGraphicsTextItem *pItem = dynamic_cast<CGraphicsTextItem *>(p);
                    pItem->setTextFontStyle(style);
                }
            }
            this->updateDefualData(TextHeavy, style);
        });
        connect(m_TextWidget, &TextWidget::colorChanged, this, [ = ](const QColor & color, EChangedPhase phase) {
            if (this->getSourceTpByItem(this->graphicItem()) == Text) {
                //记录undo
                CCmdBlock block(this->graphicItem(), phase);

                QList<CGraphicsItem *> lists = this->graphicItems();
                for (CGraphicsItem *p : lists) {
                    CGraphicsTextItem *pItem = dynamic_cast<CGraphicsTextItem *>(p);
                    pItem->setTextColor(color);
                }
            }
            this->updateDefualData(TextColor, color);
        });
    }
    return m_TextWidget;
}

BlurWidget *CComAttrWidget::getBlurWidget()
{
    if (m_blurWidget == nullptr) {
        m_blurWidget = new BlurWidget(this);

        connect(m_blurWidget, &BlurWidget::blurWidthChanged, this, [ = ](int width) {
            this->updateDefualData(BlurWidth, width);
            CCmdBlock block(this->graphicItem());
            QList<CGraphicsItem *> lists = this->graphicItems();
            for (CGraphicsItem *p : lists) {
                CGraphicsMasicoItem *pItem = dynamic_cast<CGraphicsMasicoItem *>(p);
                pItem->setBlurWidth(width);
            }
        });

        connect(m_blurWidget, &BlurWidget::blurTypeChanged, this, [ = ](EBlurEffect type) {
            this->updateDefualData(Blurtype, type);
            CCmdBlock block(this->graphicItem());
            QList<CGraphicsItem *> lists = this->graphicItems();
            for (CGraphicsItem *p : lists) {
                CGraphicsMasicoItem *pItem = dynamic_cast<CGraphicsMasicoItem *>(p);
                pItem->setBlurEffect(type);
            }
        });
    }
    return m_blurWidget;
}

CCutWidget *CComAttrWidget::getCutWidget()
{
    if (m_cutWidget == nullptr) {
        m_cutWidget = new CCutWidget(this);
        m_cutWidget->setAttribute(Qt::WA_NoMousePropagation, true);
        connect(m_cutWidget, &CCutWidget::cutSizeChanged, this, [ = ](const QSize & sz) {
            EDrawToolMode model = CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->getCurrentDrawToolMode();
            CCutTool *pTool = dynamic_cast<CCutTool *>(CDrawToolManagerSigleton::GetInstance()->getDrawTool(model));
            if (pTool != nullptr) {
                pTool->changeCutSize(CManageViewSigleton::GetInstance()->getCurView()->drawScene(), sz);
                this->updateDefualData(PropertyCutSize, sz);
            }
        });
        connect(m_cutWidget, &CCutWidget::cutTypeChanged, this, [ = ](ECutType tp) {
            EDrawToolMode model = CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->getCurrentDrawToolMode();
            CCutTool *pTool = dynamic_cast<CCutTool *>(CDrawToolManagerSigleton::GetInstance()->getDrawTool(model));
            if (pTool != nullptr) {
                pTool->changeCutType(tp, CManageViewSigleton::GetInstance()->getCurView()->drawScene());
                this->updateDefualData(PropertyCutType, tp);
            }
        });
        connect(m_cutWidget, &CCutWidget::finshed, this, [ = ](bool accept) {
            EDrawToolMode model = CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->getCurrentDrawToolMode();
            CCutTool *pTool = dynamic_cast<CCutTool *>(CDrawToolManagerSigleton::GetInstance()->getDrawTool(model));
            if (pTool != nullptr) {
                pTool->doFinished(accept);
                if (accept) {
                    QSize sz = m_cutWidget->cutSize();
                    ECutType tp = m_cutWidget->cutType();
                    this->updateDefualData(PropertyCutSize, sz);
                    this->updateDefualData(PropertyCutType, tp);
                }
            }
        });
    }
    return m_cutWidget;
}

CPictureWidget *CComAttrWidget::getPictureWidget()
{
    if (m_pictureWidget == nullptr) {
        m_pictureWidget = new CPictureWidget(this);
        connect(m_pictureWidget, &CPictureWidget::imageAdjustScence, this, [ = ]() {
            this->updateDefualData(PropertyImageAdjustScence, true);
            CCmdBlock block(this->graphicItem()->drawScene());
            QList<CGraphicsItem *> lists = this->graphicItems();
            this->graphicItem()->scene()->setSceneRect(
                this->graphicItem()->mapRectToScene(this->graphicItem()->boundingRect()));
        });

        connect(m_pictureWidget, &CPictureWidget::imageFlipChanged, this, [ = ](ERotationType type) {
            this->updateDefualData(PropertyImageFlipType, type);
            CCmdBlock block(this->graphicItem());
            QList<CGraphicsItem *> lists = this->graphicItems();
            for (CGraphicsItem *p : lists) {
                CPictureItem *pItem = dynamic_cast<CPictureItem *>(p);
                if (type == ERotationType::FlipHorizontal) {
                    pItem->setMirror(true, false);
                } else if (type == ERotationType::FlipVertical) {
                    pItem->setMirror(false, true);
                }
            }
        });

        connect(m_pictureWidget, &CPictureWidget::imageRotationChanged, this, [ = ](ERotationType type) {
            CCmdBlock block(this->graphicItem());
            QList<CGraphicsItem *> lists = this->graphicItems();
            for (CGraphicsItem *p : lists) {
                CPictureItem *pItem = dynamic_cast<CPictureItem *>(p);
                if (type == ERotationType::LeftRotate_90) {
                    pItem->setRotation90(true);
                } else if (type == ERotationType::RightRotate_90) {
                    pItem->setRotation90(false);
                }
            }
        });
    }
    return m_pictureWidget;
}

template<class T>
void CComAttrWidget::updateDefualData(EDrawProperty id, const T &var)
{
    CDrawScene *pCurScen = CManageViewSigleton::GetInstance()->getCurView()->drawScene();
    SComDefualData &scDefual = m_defualDatas[pCurScen];
    scDefual.save(id, var);
}

void SComDefualData::save(EDrawProperty property, const QVariant &var)
{
    switch (property) {
    case LineColor:
        penColor = var.value<QColor>();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineColor(penColor);
        break;
    case LineWidth:
        penWidth = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineWidth(penWidth);
        break;
    case FillColor:
        bursh = var.value<QBrush>();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setBrush(bursh);
        break;
    case RectRadius:
        rectRadius = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setRectXRedius(rectRadius);
        break;
    case Anchors:
        starAnCount = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setAnchorNum(starAnCount);
        break;
    case StarRadius:
        starInRadiusRadio = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setRadiusNum(starInRadiusRadio);
        break;
    case SideNumber:
        polySideCount = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setSideNum(polySideCount);
        break;
    case LineAndPenStartType:
        lineStartType = ELineType(var.toInt());
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineStartType(lineStartType);
        break;
    case LineAndPenEndType:
        lineEndType = ELineType(var.toInt());
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setLineEndType(lineEndType);
        break;
    case TextColor:
        textColor = var.value<QColor>();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setTextColor(textColor);
        break;
    case TextHeavy:
        textFontHeavy = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setTextFontStyle("");
        break;
    case TextSize:
        textFontSize = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setTextSize(textFontSize);
        break;
    case Blurtype:
        masicType = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setBlurEffect(EBlurEffect(masicType));
        break;
    case BlurWidth:
        masicWidth = var.toInt();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setBlurWidth(masicWidth);
        break;
    case PropertyCutType:
        cutType = ECutType(var.toInt());
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setCutType(cutType);
        break;
    case PropertyCutSize:
        cutSize = var.toSize();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setCutSize(cutSize);
        break;
    case PropertyImageAdjustScence:
        adjustScence = var.toBool();
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setImageAdjustScence(adjustScence);
        break;
    case PropertyImageFlipType:
        FlipType = static_cast<ERotationType>(var.toUInt());
        CManageViewSigleton::GetInstance()->getCurView()->getDrawParam()->setImageFlipType(FlipType);
        break;
    default:
        break;
    }
}
