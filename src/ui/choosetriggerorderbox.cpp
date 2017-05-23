/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#include "choosetriggerorderbox.h"
#include "engine.h"
#include "button.h"
#include "skinbank.h"
#include "client.h"
#include "clientplayer.h"
#include "timedprogressbar.h"
#include "stylehelper.h"

#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsProxyWidget>

static qreal initialOpacity = 0.8;
static int optionButtonHeight = 40;
static QSize generalButtonSize;

static const QString arrayString = "GameRule_AskForArraySummon";
static const QString generalShowStringHead = "GameRule_AskForGeneralShow";
static const QString headString = generalShowStringHead + "Head";
static const QString deputyString = generalShowStringHead + "Deputy";

const int ChooseTriggerOrderBox::top_dark_bar = 27;
const int ChooseTriggerOrderBox::m_topBlankWidth = 42;
const int ChooseTriggerOrderBox::bottom_blank_width = 25;
const int ChooseTriggerOrderBox::interval = 15;
const int ChooseTriggerOrderBox::m_leftBlankWidth = 37;

int TriggerOptionButton::getSkinId(const QString &playerName, const QString &generalName) const
{
    const ClientPlayer *player = ClientInstance->getPlayer(playerName);
    bool head = player->getGeneralName() == generalName;
    if (!position.isEmpty())
        head = position == "head";
    if (head)
        return player->getHeadSkinId();
    else
        return player->getDeputySkinId();
}
/*
TriggerOptionButton::TriggerOptionButton(QGraphicsObject *parent, const QString &player, const QString &skillStr, const int width)
    : QGraphicsObject(parent),
    m_skillStr(skillStr), m_text(displayedTextOf(skillStr)),
    playerName(player.split("?").first()), position(player.contains("?") ? player.split("?").last() : QString()), width(width)
{
    QString realSkill = skillStr;
    if (realSkill.contains("'")) // "sgs1'songwei"
        realSkill = realSkill.split("'").last();
    else if (realSkill.contains("->")) // "tieqi->sgs4&1"
        realSkill = realSkill.split("->").first();
    const Skill *skill = Sanguosha->getSkill(realSkill);
    if (skill)
        setToolTip(skill->getDescription());

    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setOpacity(initialOpacity);
}
*/
TriggerOptionButton::TriggerOptionButton(QGraphicsObject *parent, const QString &skill_owner, const QString &skill_name, int index,
                                         const QString &target, const QString &position, int time, const int width)
    : QGraphicsObject(parent), m_skillStr(skill_name), playerName(skill_owner), position(position), index(index), width(width)
{
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill) {
        m_text = displayedTextOf(skill_name, target, time);
    } else {
        m_text = displayedTextOf(skill_name);
        QString realSkill = skill_name;
        if (realSkill.contains("'")) // "sgs1'songwei"
            realSkill = realSkill.split("'").last();
        else if (realSkill.contains("->")) { // "tieqi->sgs4&1"
            realSkill = realSkill.split("->").first();
        }

        skill = Sanguosha->getSkill(realSkill);
    }

    if (skill)
        setToolTip(skill->getDescription());

    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setOpacity(initialOpacity);
}

QString TriggerOptionButton::getGeneralNameBySkill() const
{
    QString generalName;
    const ClientPlayer *player = ClientInstance->getPlayer(playerName);
    if (!position.isEmpty())
        return position == "head" ? player->getActualGeneral1Name() : player->getActualGeneral2Name();
    QString skillName = m_skillStr;
    if (m_skillStr.contains("*"))
        skillName = m_skillStr.split("*").first();
    if (skillName == arrayString) {
        foreach (const Skill *skill, player->getVisibleSkillList()) {
            if (!skill->inherits("BattleArraySkill")) continue;
            if (player->inHeadSkills(skill))
                generalName = player->getGeneralName();
            else
                generalName = player->getGeneral2Name();
        }
    } else {
        QString realSkillName = skillName;
        if (realSkillName.contains("'")) // "sgs1'songwei"
            realSkillName = realSkillName.split("'").last();
        else if (realSkillName.contains("->")) // "tieqi->sgs4&1"
            realSkillName = realSkillName.split("->").first();
        if (player->inHeadSkills(realSkillName))
            generalName = player->getGeneralName();
        else if (player->inDeputySkills(realSkillName))
            generalName = player->getGeneral2Name();
        else
            generalName = player->getGeneralName();
    }
    return generalName;
}

QFont TriggerOptionButton::defaultFont()
{
    QFont font = StyleHelper::getFontByFileName("wqy-microhei.ttc");
    font.setPixelSize(Config.TinyFont.pixelSize());
    return font;
}

void TriggerOptionButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->save();
    painter->setBrush(Qt::black);
    ClientPlayer *player = ClientInstance->getPlayer(playerName);
    painter->setPen(Sanguosha->getKingdomColor(player->getGeneral()->getKingdom()));
    QRectF rect = boundingRect();
    painter->drawRoundedRect(rect, 5, 5);
    painter->restore();

    QString generalName = getGeneralNameBySkill();
    QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY, getSkinId(playerName, generalName));
    pixmap = pixmap.scaledToHeight(optionButtonHeight, Qt::SmoothTransformation);
    QRect pixmapRect(QPoint(0, (rect.height() - pixmap.height()) / 2), pixmap.size());
    painter->setBrush(pixmap);
    painter->drawRoundedRect(pixmapRect, 5, 5);

    QRect textArea(optionButtonHeight, 0, width - optionButtonHeight,
        optionButtonHeight);

    G_COMMON_LAYOUT.optionButtonText.paintText(painter, textArea,
        Qt::AlignCenter,
        m_text);
}

QRectF TriggerOptionButton::boundingRect() const
{
    return QRectF(0, 0, width, optionButtonHeight);
}

void TriggerOptionButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void TriggerOptionButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

void TriggerOptionButton::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(1.0);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    emit hovered(true);
}

void TriggerOptionButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(initialOpacity);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    emit hovered(false);
}

QString TriggerOptionButton::displayedTextOf(const QString &str)
{
    int time = 1;
    QString skillName = str;
    if (str.contains("*")) {
        time = str.split("*").last().toInt();
        skillName = str.split("*").first();
    }
    QString text = Sanguosha->translate(skillName);
    if (skillName.contains("->")) { // "tieqi->sgs4&1"
        QString realSkill = skillName.split("->").first(); // "tieqi"
        QString targetObj = skillName.split("->").last().split("&").first(); // "sgs4"
        QString targetName = ClientInstance->getPlayer(targetObj)->getFootnoteName();
        text = tr("%1 (use upon %2)").arg(Sanguosha->translate(realSkill))
            .arg(Sanguosha->translate(targetName));
    }
    if (skillName.contains("'")) {
        QString realSkill = skillName.split("'").last();
        QString targetName = ClientInstance->getPlayer(skillName.split("'").first())->getFootnoteName();
        text = tr("%1 (use upon %2)").arg(Sanguosha->translate(realSkill))
                .arg(Sanguosha->translate(targetName));
    }
    if (time > 1)
        //text += " " + tr("*") + time;
        text += QString(" %1 %2").arg(tr("*")).arg(time);

    return text;
}

QString TriggerOptionButton::displayedTextOf(const QString &skillName, const QString &target, int time)
{
    QString text = Sanguosha->translate(skillName);
    if (!target.isEmpty()) {
        QString targetName = ClientInstance->getPlayer(target)->getFootnoteName();
        text = tr("%1 (use upon %2)").arg(text)
            .arg(Sanguosha->translate(targetName));
    }
    if (time > 1)
        //text += " " + tr("*") + time;
        text += QString(" %1 %2").arg(tr("*")).arg(time);

    return text;
}

bool TriggerOptionButton::isPreferentialSkillOf(const TriggerOptionButton *other) const
{
    if (this == other)
        return true;

    return index < other->index;
}

void TriggerOptionButton::needDisabled(bool disabled)
{
    if (disabled) {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
        animation->setEndValue(0.2);
        animation->setDuration(100);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
        animation->setEndValue(initialOpacity);
        animation->setDuration(100);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

GeneralButton::GeneralButton(QGraphicsObject *parent, const QString &general, const bool isHead, const Player *player)
    : QGraphicsObject(parent), generalName(general), isHead(isHead), m_player(player)
{
    setToolTip(Sanguosha->getGeneral(general)->getSkillDescription(true));
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemIsFocusable);

    setAcceptHoverEvents(true);

    setOpacity(initialOpacity);
}

void GeneralButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    QPixmap generalImage = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, isHead ? m_player->getHeadSkinId() : m_player->getDeputySkinId());
    generalImage = generalImage.scaled(generalButtonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->setBrush(generalImage);
    painter->drawRoundedRect(boundingRect(), 5, 5, Qt::RelativeSize);

    const General *general = Sanguosha->getGeneral(generalName);
    Q_ASSERT(general);

    QPixmap nameBg = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, general->getKingdom());
    painter->drawPixmap(0, 5, nameBg);

    if (m_player->getGeneral() == general || m_player->getGeneral2() == general) {
        QString key = (m_player->getGeneral() == general) ? QSanRoomSkin::S_SKIN_KEY_HEAD_ICON : QSanRoomSkin::S_SKIN_KEY_DEPUTY_ICON;
        QPixmap positionIcon = G_ROOM_SKIN.getPixmap(key);
        painter->drawPixmap(G_COMMON_LAYOUT.generalButtonPositionIconRegion, positionIcon);
    }

    QString name = Sanguosha->translate("&" + general->objectName());
    if (name.startsWith("&"))
        name = Sanguosha->translate(general->objectName());
    G_DASHBOARD_LAYOUT.m_avatarNameFont.paintText(painter,
        G_COMMON_LAYOUT.generalButtonNameRegion,
        Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignJustify, name);
}

QRectF GeneralButton::boundingRect() const
{
    return QRectF(QPoint(0, 0), generalButtonSize);
}

void GeneralButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void GeneralButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

void GeneralButton::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(1.0);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GeneralButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(initialOpacity);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

ChooseTriggerOrderBox::ChooseTriggerOrderBox()
    : optional(true), m_minimumWidth(0),
    cancel(new Button(tr("cancel"), 0.6)), progressBar(NULL), m_player(Self)
{
    cancel->hide();
    cancel->setParentItem(this);
    cancel->setObjectName("cancel");
    connect(cancel, &Button::clicked, this, &ChooseTriggerOrderBox::reply);

    generalButtonSize = G_ROOM_SKIN.getGeneralPixmap("caocao", QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE).size() * 0.6;
}

int ChooseTriggerOrderBox::getGeneralNum() const
{
    int count = 0;
    if (options.contains(QString("%1:%2").arg(m_player->objectName()).arg(headString)))
        ++count;
    if (options.contains(QString("%1:%2").arg(m_player->objectName()).arg(deputyString)))
        ++count;

    foreach (const TriggerStruct &skill, skills) {
        if (skill.skill_name.contains(headString))
            ++count;
        if (skill.skill_name.contains(deputyString))
            ++count;
    }

    return count;
}

void ChooseTriggerOrderBox::storeMinimumWidth()
{
    int width = 0;
    static QFontMetrics fontMetrics(TriggerOptionButton::defaultFont());
    foreach (const QString &option, options) {
        const QString skill = option.split(":").last();
        if (skill.startsWith(generalShowStringHead))
            continue;

        const int w = fontMetrics.width(TriggerOptionButton::displayedTextOf(skill));
        if (w > width)
            width = w;
    }
    foreach (const TriggerStruct &skill, skills) {
        if (skill.skill_name.startsWith(generalShowStringHead))
            continue;

        const int w = fontMetrics.width(TriggerOptionButton::displayedTextOf(skill.skill_name, skill.targets.isEmpty() ? QString() : skill.result_target, skill.times));
        if (w > width)
            width = w;
    }
    m_minimumWidth = width + optionButtonHeight + 20;
}

QRectF ChooseTriggerOrderBox::boundingRect() const
{
    const int generalNum = getGeneralNum();
    int width = generalButtonSize.width();
    if (generalNum == 2)
        width += generalButtonSize.width() + interval;

    width = qMax(m_minimumWidth, width) + m_leftBlankWidth * 2;

    int height = m_topBlankWidth + bottom_blank_width;
    if (!options.isEmpty())
        height = height + (options.size() - generalNum) * optionButtonHeight + (options.size() - generalNum - 1) * interval;
    if (!skills.isEmpty())
        height = height + (skills.size() - generalNum) * optionButtonHeight + (skills.size() - generalNum - 1) * interval;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    if (generalNum > 0)
        height += generalButtonSize.height() + interval;

    if (optional)
        height += cancel->boundingRect().height() + interval;

    return QRectF(0, 0, width, height);
}

void ChooseTriggerOrderBox::chooseOption(const Player *player, const QString &reason, const QStringList &options, const bool optional)
{
    skills.clear();
    this->options = options;
    this->optional = optional;
    title = Sanguosha->translate(reason);
    m_player = player;

    storeMinimumWidth();

    prepareGeometryChange();

    const int generalCount = getGeneralNum();

    int width = generalButtonSize.width();
    int generalHeight = 0;

    if (generalCount == 2) {
        GeneralButton *head = new GeneralButton(this, player->getGeneral()->objectName(), true, player);
        head->setObjectName(QString("%1:%2").arg(player->objectName()).arg(headString));
        generalButtons << head;

        const int generalTop = m_topBlankWidth
            + (options.size() - generalCount) * optionButtonHeight
            + (options.size() - generalCount) * interval;

        const int generalLeft = (boundingRect().width() / 2) - (interval / 2) - width;
        head->setPos(generalLeft, generalTop);

        GeneralButton *deputy = new GeneralButton(this, player->getGeneral2()->objectName(), false, player);
        deputy->setObjectName(QString("%1:%2").arg(player->objectName()).arg(deputyString));
        generalButtons << deputy;
        deputy->setPos(head->pos().x() + head->boundingRect().width() + interval,
            generalTop);

        width = deputy->pos().x() - head->pos().x() + deputy->boundingRect().width();
        generalHeight = head->boundingRect().height();
    } else if (generalCount == 1) {
        const bool isHead = options.contains(QString("%1:%2").arg(player->objectName()).arg(headString));
        const QString general = isHead ? player->getGeneralName() : player->getGeneral2Name();
        GeneralButton *generalButton = new GeneralButton(this, general, isHead, player);
        QString objectName = QString("%1:%2")
            .arg(player->objectName())
            .arg(isHead ? headString : deputyString);
        generalButton->setObjectName(objectName);
        generalButtons << generalButton;
        const int generalTop = m_topBlankWidth
            + (options.size() - generalCount) * optionButtonHeight
            + (options.size() - generalCount) * interval;
        const int generalLeft = qMax((int)(boundingRect().width() / 2) - width, m_leftBlankWidth);
        generalButton->setPos(generalLeft, generalTop);

        generalHeight = generalButton->boundingRect().height();
    }

    width = qMax(width, m_minimumWidth);

    foreach (const QString &option, options) {
        QStringList pair = option.split(":");
        if (pair.last().startsWith(generalShowStringHead))
            continue;

        QString invoker = pair.first().split("?").first();
		QString position = pair.first().contains("?") ? pair.first().split("?").last() : QString();
        QString target = option.contains("->") ? option.split("->").last().split("&").first() : QString();
        int index = option.contains("->") ? option.split("->").last().split("&").last().toInt() : -1;
        int time = option.contains("*") ? option.split("*").last().toInt() : 1;
        TriggerOptionButton *button = new TriggerOptionButton(this, invoker, pair.last(), index, target, position, time, width);
        button->setObjectName(option);
        foreach (TriggerOptionButton *otherButton, optionButtons) {
            if (otherButton->isPreferentialSkillOf(button))
                connect(button, &TriggerOptionButton::hovered, otherButton, &TriggerOptionButton::needDisabled);
        }
        optionButtons << button;
    }

    moveToCenter();
    show();

    int y = m_topBlankWidth;
    foreach (TriggerOptionButton *button, optionButtons) {
        QPointF pos;
        pos.setX(m_leftBlankWidth);
        pos.setY(y);

        button->setPos(pos);
        connect(button, &TriggerOptionButton::clicked, this, &ChooseTriggerOrderBox::reply);
        y += button->boundingRect().height() + interval;
    }

    foreach(GeneralButton *button, generalButtons)
        connect(button, &GeneralButton::clicked, this, &ChooseTriggerOrderBox::reply);


    if (optional) {
        cancel->setPos((boundingRect().width() - cancel->boundingRect().width()) / 2,
            y + generalHeight + interval);
        cancel->show();
    }


    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar;
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progress_bar_item = new QGraphicsProxyWidget(this);
            progress_bar_item->setWidget(progressBar);
            progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, &QSanCommandProgressBar::timedOut, this, &ChooseTriggerOrderBox::reply);
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_TRIGGER_ORDER);
        progressBar->show();
    }
}

void ChooseTriggerOrderBox::chooseOption(const Player *player, const QString &reason, QList<TriggerStruct> skills, const bool optional)
{
    options.clear();
    this->skills = skills;
    this->optional = optional;
    title = Sanguosha->translate(reason);

    storeMinimumWidth();

    prepareGeometryChange();

    const int generalCount = getGeneralNum();

    int width = generalButtonSize.width();
    int generalHeight = 0;

    if (generalCount == 2) {
        GeneralButton *head = new GeneralButton(this, player->getGeneral()->objectName(), true, player);
        foreach (const TriggerStruct &skill, skills) {
            if (skill.skill_name.contains(headString))
                head->setProperty("trigger", skill.toVariant());
        }
        generalButtons << head;

        const int generalTop = m_topBlankWidth
            + (skills.size() - generalCount) * optionButtonHeight
            + (skills.size() - generalCount) * interval;

        const int generalLeft = (boundingRect().width() / 2) - (interval / 2) - width;
        head->setPos(generalLeft, generalTop);

        GeneralButton *deputy = new GeneralButton(this, player->getGeneral2()->objectName(), false, player);
        foreach (const TriggerStruct &skill, skills) {
            if (skill.skill_name.contains(deputyString))
                deputy->setProperty("trigger", skill.toVariant());
        }
        generalButtons << deputy;
        deputy->setPos(head->pos().x() + head->boundingRect().width() + interval,
            generalTop);

        width = deputy->pos().x() - head->pos().x() + deputy->boundingRect().width();
        generalHeight = head->boundingRect().height();
    } else if (generalCount == 1) {
        bool isHead = false;
        QVariant q;
        foreach (const TriggerStruct &skill, skills) {
            if (skill.skill_name.contains(headString)) {
                isHead = true;
                q = skill.toVariant();
            }
            if (skill.skill_name.contains(deputyString))
                q = skill.toVariant();
        }
        const QString general = isHead ? player->getGeneralName() : player->getGeneral2Name();
        GeneralButton *generalButton = new GeneralButton(this, general, isHead, player);
        generalButton->setProperty("trigger", q);
        generalButtons << generalButton;
        const int generalTop = m_topBlankWidth
            + (skills.size() - generalCount) * optionButtonHeight
            + (skills.size() - generalCount) * interval;
        const int generalLeft = qMax((int)(boundingRect().width() / 2) - width, m_leftBlankWidth);
        generalButton->setPos(generalLeft, generalTop);

        generalHeight = generalButton->boundingRect().height();
    }

    width = qMax(width, m_minimumWidth);

    foreach (const TriggerStruct &skill, skills) {
        if (skill.skill_name.startsWith(generalShowStringHead))
            continue;

        TriggerOptionButton *button = new TriggerOptionButton(this, skill.skill_owner.isEmpty() ? skill.invoker : skill.skill_owner,
            skill.skill_name, skill.targets.isEmpty() ? -1 : skill.targets.indexOf(skill.result_target), skill.result_target, skill.skill_position, skill.times, width);
        button->setProperty("trigger", skill.toVariant());
        foreach (TriggerOptionButton *otherButton, optionButtons) {
            if (otherButton->isPreferentialSkillOf(button))
                connect(button, &TriggerOptionButton::hovered, otherButton, &TriggerOptionButton::needDisabled);
        }
        optionButtons << button;
    }

    moveToCenter();
    show();

    int y = m_topBlankWidth;
    foreach (TriggerOptionButton *button, optionButtons) {
        QPointF pos;
        pos.setX(m_leftBlankWidth);
        pos.setY(y);

        button->setPos(pos);
        connect(button, &TriggerOptionButton::clicked, this, &ChooseTriggerOrderBox::reply);
        y += button->boundingRect().height() + interval;
    }

    foreach(GeneralButton *button, generalButtons)
        connect(button, &GeneralButton::clicked, this, &ChooseTriggerOrderBox::reply);


    if (optional) {
        cancel->setPos((boundingRect().width() - cancel->boundingRect().width()) / 2,
            y + generalHeight + interval);
        cancel->show();
    }


    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar;
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progress_bar_item = new QGraphicsProxyWidget(this);
            progress_bar_item->setWidget(progressBar);
            progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, &QSanCommandProgressBar::timedOut, this, &ChooseTriggerOrderBox::reply);
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_TRIGGER_ORDER);
        progressBar->show();
    }
}

void ChooseTriggerOrderBox::clear()
{
    if (progressBar != NULL) {
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;
    }

    foreach(TriggerOptionButton *button, optionButtons)
        button->deleteLater();

    foreach(GeneralButton *button, generalButtons)
        button->deleteLater();

    optionButtons.clear();
    generalButtons.clear();

    cancel->hide();

    disappear();
}

void ChooseTriggerOrderBox::reply()
{
    if (!options.isEmpty()) {
        QString choice = sender()->objectName();
        if (choice.isEmpty()) {
            if (optional)
                choice = "cancel";
            else
                choice = options.first();
        }
        ClientInstance->onPlayerChooseTriggerOrder(choice);
    } else {
        QString choice = sender()->objectName();
        if (choice == "cancel")
            ClientInstance->onPlayerChooseTriggerOrder2(TriggerStruct().toVariant());
        else if (choice.isEmpty())
            ClientInstance->onPlayerChooseTriggerOrder2(sender()->property("trigger"));
    }
}
