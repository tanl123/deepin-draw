/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Ji XiangLong <jixianglong@uniontech.com>
 *
 * Maintainer: ZhangYong <zhangyong@uniontech.com>
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
#ifndef CFILLTOOL_H
#define CFILLTOOL_H
#include "idrawtool.h"


#ifdef __cplusplus
extern "C" {

#endif
__attribute__((visibility("default"))) IDrawTool *creatTool();

#ifdef __cplusplus
}
#endif

class JDynamicLayer;
class CFillTool: public IDrawTool
{
    Q_OBJECT
public:
    CFillTool();

    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;

    DrawAttribution::SAttrisList attributions() override;

    CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo) override {return nullptr;}

    int  minMoveUpdateDistance() override;

    int  decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    int  allowedMaxTouchPointCount() override {return 1;}

    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;
    JDynamicLayer *currentLayer(CDrawScene *scene);
};


#endif // CFILLTOOL_H
