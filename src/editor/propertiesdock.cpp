/*
 * Copyright 2012, Tim Baker <treectrl@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "propertiesdock.h"

#include "celldocument.h"
#include "world.h"
#include "worldcell.h"
#include "worlddocument.h"

#include <QApplication>
#include <QDebug>
#include <QStyledItemDelegate>
#include <QFileInfo>
#include <QLabel>
#include <QPainter>
#include <QTreeView>

#include <QMenu>
#include <QToolButton>

/////

class PropertiesDelegate : public QStyledItemDelegate
{
public:
    PropertiesDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent)
        , mTrashPixmap(QLatin1String(":/images/16x16/edit-delete.png"))
    {

    }

#if 1
    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        const PropertiesModel *model = dynamic_cast<const PropertiesModel*>(index.model());

        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);

        // Don't draw mouse-over highlight or draw the selection beneath "add" buttons
        // or Templates/Properties headers.
        if (!model->toProperty(index) && !model->toTemplate(index)) {
            opt.state &= ~QStyle::State_Selected;
            opt.state &= ~QStyle::State_MouseOver;
        }

        QStyledItemDelegate::paint(p, opt, index);

        // draw the close button
        if (model->isDeletable(index) && index.column() == 0 && opt.state & QStyle::State_MouseOver) {
            QRect closeRect = closeButtonRect(opt.rect); //opt.rect.x()-40+2, opt.rect.y(), mTrashPixmap.width(), opt.rect.height());
            p->setClipRect(closeRect);
            p->drawPixmap(closeRect.x(), closeRect.y(), mTrashPixmap);
            p->setClipRect(opt.rect);
        }
    }
#else
    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        const PropertiesModel *model = dynamic_cast<const PropertiesModel*>(index.model());

        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);

        const QWidget *widget = opt.widget; //QStyledItemDelegatePrivate::widget(option);
        QStyle *style = widget ? widget->style() : QApplication::style();

        p->save();
        p->setClipRect(opt.rect);

#if 0
        QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
        QRect iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
#endif
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);

        // draw the background
        // Don't draw selection beneath "add" buttons
        {
            QStyleOptionViewItemV4 option(opt);
            if ((opt.state & QStyle::State_Selected) && model->toWidget(index))
                option.state &= ~QStyle::State_Selected;
#if 0
            option.features &= ~QStyleOptionViewItemV2::HasCheckIndicator;
            option.text.clear();
            option.state &= ~QStyle::State_HasFocus;
            style->drawControl(QStyle::CE_ItemViewItem, &option, p, widget);
#else
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, widget);
#endif
        }

#if 0
        // draw the check mark
        if (opt.features & QStyleOptionViewItemV2::HasCheckIndicator) {
            QStyleOptionViewItemV4 option(opt);
            option.rect = checkRect;
            option.state = option.state & ~QStyle::State_HasFocus;

            switch (opt.checkState) {
            case Qt::Unchecked:
                option.state |= QStyle::State_Off;
                break;
            case Qt::PartiallyChecked:
                option.state |= QStyle::State_NoChange;
                break;
            case Qt::Checked:
                option.state |= QStyle::State_On;
                break;
            }
            style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, p, widget);
        }

        // draw the icon
        QIcon::Mode mode = QIcon::Normal;
        if (!(opt.state & QStyle::State_Enabled))
            mode = QIcon::Disabled;
        else if (opt.state & QStyle::State_Selected)
            mode = QIcon::Selected;
        QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
        opt.icon.paint(p, iconRect, opt.decorationAlignment, mode, state);
#endif

        // draw the text
        if (!opt.text.isEmpty()) {
            QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
                cg = QPalette::Inactive;

            if (opt.state & QStyle::State_Selected) {
                p->setPen(opt.palette.color(cg, QPalette::HighlightedText));
            } else {
                p->setPen(opt.palette.color(cg, QPalette::Text));
            }
            if (opt.state & QStyle::State_Editing) {
                p->setPen(opt.palette.color(cg, QPalette::Text));
                p->drawRect(textRect.adjusted(0, 0, -1, -1));
            }

#if 1
            const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;
            QRect textRect2 = textRect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
            QFontMetrics fm = opt.fontMetrics;
            p->drawText(textRect2.topLeft() + QPointF(0, fm.ascent()), fm.elidedText(opt.text, Qt::ElideRight, textRect2.width()));
#else
            d->viewItemDrawText(p, vopt, textRect);
#endif
        }

        // draw the close button
        if (model->isDeletable(index) && index.column() == 0 && opt.state & QStyle::State_MouseOver) {
#if 1
            QRect closeRect = closeButtonRect(opt.rect); //opt.rect.x()-40+2, opt.rect.y(), mTrashPixmap.width(), opt.rect.height());
            p->setClipRect(closeRect);
            p->drawPixmap(closeRect.x(), closeRect.y(), mTrashPixmap);
            p->setClipRect(opt.rect);
#else
            QRect closeRect = opt.rect;
            closeRect.setLeft(closeRect.right() - 20);
            QStyleOptionViewItemV4 option(opt);
            option.rect = closeRect;
            option.state |= QStyle::State_Selected;
            style->drawPrimitive(QStyle::PE_IndicatorTabClose, &option, p, widget);
#endif
        }

#if 1
        // draw the focus rect
         if (opt.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(opt);
            o.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, widget);
            o.state |= QStyle::State_KeyboardFocusChange;
            o.state |= QStyle::State_Item;
            QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                          ? QPalette::Normal : QPalette::Disabled;
            o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                         ? QPalette::Highlight : QPalette::Window);
            style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
        }
#endif
         p->restore();
    }
#endif

    QRect closeButtonRect(const QRect &itemViewRect) const
    {
       QRect closeRect(itemViewRect.x()-40+2, itemViewRect.y(), mTrashPixmap.width(), itemViewRect.height());
       return closeRect;
    }

    QPixmap mTrashPixmap;
};

/////

PropertiesModel::PropertiesModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mWorldDoc(0)
    , mPropertyHolder(0)
    , mRootItem(0)
    , mAddTemplateWidget(0)
    , mAddPropertyWidget(0)
    , mAddTemplateMenu(new QMenu)
    , mAddPropertyMenu(new QMenu)
{
//    connect(mAddTemplateMenu, SIGNAL(aboutToShow()), SLOT(setTemplatesMenu()));
    connect(mAddTemplateMenu, SIGNAL(triggered(QAction*)), SLOT(addTemplate(QAction*)));

//    connect(mAddPropertyMenu, SIGNAL(aboutToShow()), SLOT(setPropertiesMenu()));
    connect(mAddPropertyMenu, SIGNAL(triggered(QAction*)), SLOT(addProperty(QAction*)));

    mTimer.setInterval(1);
    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), SLOT(updateMenus()));
}

PropertiesModel::~PropertiesModel()
{
    delete mRootItem;
}

QModelIndex PropertiesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!mRootItem)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row < mRootItem->children.size())
            return createIndex(row, column, mRootItem->children.at(row));
        return QModelIndex();
    }

    if (Item *item = toItem(parent)) {
        if (row < item->children.size())
            return createIndex(row, column, item->children.at(row));
        return QModelIndex();
    }

    return QModelIndex();
}

QModelIndex PropertiesModel::parent(const QModelIndex &index) const
{
    if (Item *item = toItem(index)) {
        if (item->parent != mRootItem) {
            Item *grandParent = item->parent->parent;
            return createIndex(grandParent->children.indexOf(item->parent), 0, item->parent);
        }
    }
    return QModelIndex();
}

int PropertiesModel::rowCount(const QModelIndex &parent) const
{
    if (!mRootItem)
        return 0;
    if (!parent.isValid())
        return mRootItem->children.size();
    if (Item *item = toItem(parent))
        return item->children.size();
    return 0;
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Property");
        case 1: return tr("Value");
        }
    }
    return QVariant();
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    Item *item = toItem(index);

    if (item) {
        switch (item->header) {
        case Item::HeaderTemplates:
            switch (role) {
            case Qt::BackgroundRole:
                return QBrush(Qt::gray);
            //case Qt::ForegroundRole:
            //    return is  QBrush(Qt::gray);
            case Qt::DisplayRole:
                return index.column() ? QVariant() : QLatin1String("Templates");
            case Qt::EditRole:
                return QVariant(); // no edit
            case Qt::DecorationRole:
                return QVariant();
            default:
                return QVariant();
                break;
            }
            break;
        case Item::HeaderProperties:
            switch (role) {
            case Qt::BackgroundRole:
                return QBrush(Qt::gray);
            case Qt::DisplayRole:
                return index.column() ? QVariant() : QLatin1String("Properties");
            case Qt::EditRole:
                return QVariant(); // no edit
            case Qt::DecorationRole:
                return QVariant();
            default:
                return QVariant();
                break;
            }
            break;
        default:
            break;
        }
    }

    if (PropertyTemplate *pt = toTemplate(index)) {
        if (index.column())
            return QVariant();
        switch (role) {
        case Qt::DisplayRole:
            return pt->mName;
        case Qt::EditRole:
            return QVariant(); // no edit
        case Qt::DecorationRole:
            return QVariant();
        default:
            return QVariant();
        }
    }
    if (Property *p = toProperty(index)) {
        switch (role) {
        case Qt::DisplayRole:
            return index.column() ? p->mValue : p->mDefinition->mName;
        case Qt::EditRole:
            if (isEditable(index))
                return p->mValue;
            break;
        case Qt::DecorationRole:
            return QVariant();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    Item *item = toItem(index);
    if (isEditable(index)) {
        switch (role) {
        case Qt::EditRole:
            if (value.toString() != item->p->mValue)
                mWorldDoc->setPropertyValue(mPropertyHolder, item->p, value.toString());
            return true;
        }

    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (Item *item = toItem(index)) {
        if (isPropertyOfCell(item) && (index.column() == 1))
            rc |= Qt::ItemIsEditable;
    }
    return rc;
}

PropertiesModel::Item *PropertiesModel::toItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<Item*>(index.internalPointer());
    }
    return 0;
}

void PropertiesModel::createAddTemplateWidget()
{
    mAddTemplateWidget = new QToolButton;
    mAddTemplateWidget->setText(QLatin1String("add"));
    mAddTemplateWidget->setPopupMode(QToolButton::InstantPopup);
    mAddTemplateWidget->setMenu(mAddTemplateMenu);
    mAddTemplateWidget->setAutoFillBackground(true);
    mAddTemplateWidget->setFocusPolicy(Qt::NoFocus);
}

void PropertiesModel::createAddPropertyWidget()
{
    mAddPropertyWidget = new QToolButton;
    mAddPropertyWidget->setText(QLatin1String("add"));
    mAddPropertyWidget->setPopupMode(QToolButton::InstantPopup);
    mAddPropertyWidget->setMenu(mAddPropertyMenu);
    mAddPropertyWidget->setAutoFillBackground(true);
    mAddPropertyWidget->setFocusPolicy(Qt::NoFocus);
}

void PropertiesModel::redrawProperty(PropertiesModel::Item *item, PropertyDef *pd)
{
    if (item->usesPropertyDef(pd)) {
        QModelIndex index = this->index(item);
        emit dataChanged(index, index);
    }
    foreach (Item *child, item->children)
        redrawProperty(child, pd);
}

void PropertiesModel::redrawTemplate(PropertiesModel::Item *item, PropertyTemplate *pt)
{
    if (item->pt == pt) {
        QModelIndex index = this->index(item);
        emit dataChanged(index, index);
    }
    foreach (Item *child, item->children)
        redrawTemplate(child, pt);
}

PropertyTemplate *PropertiesModel::toTemplate(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer())->pt;
    return 0;
}

Property *PropertiesModel::toProperty(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer())->p;
    return 0;
}

QWidget *PropertiesModel::toWidget(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Item*>(index.internalPointer())->widget;
    return 0;
}

void PropertiesModel::setPropertyHolder(WorldDocument *worldDoc, PropertyHolder *ph)
{
    mPropertyHolder = ph;
    setDocument(worldDoc);
}

void PropertiesModel::clearPropertyHolder()
{
    mPropertyHolder = 0;
    setDocument(0);
}

QModelIndex PropertiesModel::index(PropertiesModel::Item *item)
{
    if (item == mRootItem)
        return QModelIndex();
    return index(item->parent->children.indexOf(item), 0, index(item->parent));
}

PropertiesModel::Item *PropertiesModel::itemFor(PropertyHolder *ph)
{
    if (ph == mPropertyHolder)
        return mRootItem; // cell or template
    if (!ph->isTemplate())
        return 0;
    return mRootItem->itemFor(static_cast<PropertyTemplate*>(ph));
}

// A template may appear more than once in a Cell
QList<PropertiesModel::Item *> PropertiesModel::itemsFor(PropertyHolder *ph)
{
    return mRootItem->itemsFor(ph);
}

void PropertiesModel::setDocument(WorldDocument *worldDoc)
{
    if (mWorldDoc)
        mWorldDoc->disconnect(this);

    mWorldDoc = worldDoc;

    setModelData();

    if (mWorldDoc) {
        connect(mWorldDoc, SIGNAL(propertyDefinitionAdded(PropertyDef*,int)),
                SLOT(updateMenusLater()));
        connect(mWorldDoc, SIGNAL(propertyDefinitionChanged(PropertyDef*)),
                SLOT(propertyDefinitionChanged(PropertyDef*)));
        connect(mWorldDoc, SIGNAL(propertyDefinitionAboutToBeRemoved(int)),
                SLOT(updateMenusLater()));

        connect(mWorldDoc, SIGNAL(propertyAdded(PropertyHolder*,int)),
                SLOT(propertyAdded(PropertyHolder*,int)));
        connect(mWorldDoc, SIGNAL(propertyAboutToBeRemoved(PropertyHolder*,int)),
                SLOT(propertyAboutToBeRemoved(PropertyHolder*,int)));

        connect(mWorldDoc, SIGNAL(propertyValueChanged(PropertyHolder*,int)),
                SLOT(propertyValueChanged(PropertyHolder*,int)));

        connect(mWorldDoc, SIGNAL(templateAdded(int)),
                SLOT(updateMenusLater()));
        connect(mWorldDoc, SIGNAL(templateAboutToBeRemoved(int)),
                SLOT(templateAboutToBeRemoved(int)));
        connect(mWorldDoc, SIGNAL(templateChanged(PropertyTemplate*)),
                SLOT(templateChanged(PropertyTemplate*)));

        connect(mWorldDoc, SIGNAL(templateAdded(PropertyHolder*,int)),
                SLOT(templateAdded(PropertyHolder*,int)));
        connect(mWorldDoc, SIGNAL(templateAboutToBeRemoved(PropertyHolder*,int)),
                SLOT(templateAboutToBeRemoved(PropertyHolder*,int)));

        connect(mWorldDoc, SIGNAL(cellContentsAboutToChange(WorldCell*)),
                SLOT(cellContentsAboutToChange(WorldCell*)));
        connect(mWorldDoc, SIGNAL(cellContentsChanged(WorldCell*)),
                SLOT(cellContentsChanged(WorldCell*)));

#if 1
#else
    Item *header = new Item(mRootItem, mRootItem->children.size(), Item::HeaderTemplates);
    foreach (PropertyTemplate *pt, cell->templates()) {
        addTemplate(header, header->children.size(), pt);
    }

    createAddTemplateWidget();
//    setTemplatesMenu();
    Item *addTemplateItem = new Item(header, header->children.size(), mAddTemplateWidget);

    header = new Item(mRootItem, mRootItem->children.size(), Item::HeaderProperties);
    foreach (Property *p, cell->properties()) {
        addProperty(header, header->children.size(), p);
    }

    createAddPropertyWidget();
//    setPropertiesMenuLater();
    Item *addPropertyItem = new Item(header, header->children.size(), mAddPropertyWidget);

    reset();
#endif
    }
}

void PropertiesModel::reset()
{
    QAbstractItemModel::reset();

    // When reset() is called, selectionChanged() won't get called in the view
    emit theModelWasReset();
}

bool PropertiesModel::isEditable(const QModelIndex &index) const
{
    if (Item *item = toItem(index)) {
        if (index.column() != 1) return false;
        if (item->p == 0) return false;
        if (item->parent->parent == mRootItem) return true;
    }
    return false;
}

bool PropertiesModel::isDeletable(const QModelIndex &index) const
{
    if (Item *item = toItem(index)) {
        if (item->p && item->parent->parent == mRootItem) return true;
        if (item->pt && item->parent->parent == mRootItem) return true;
    }
    return false;
}

bool PropertiesModel::isPropertyOfCell(Item *item) const
{
    return item && item->p && (item->parent->parent == mRootItem);
}

void PropertiesModel::setTemplatesMenu()
{
    // Protect against QTimer firing late
    if (!mPropertyHolder)
        return;

    QMenu *menu = mAddTemplateMenu;
    menu->clear();
    foreach (PropertyTemplate *pt, mWorldDoc->world()->propertyTemplates()) {
        if (mPropertyHolder->canAddTemplate(pt))
            menu->addAction(pt->mName);
    }

    mAddTemplateWidget->setEnabled(!menu->actions().isEmpty());
}

void PropertiesModel::setPropertiesMenu()
{
//    qDebug() << "setPropertiesMenu()";

    // Protect against QTimer firing late
    if (!mPropertyHolder)
        return;

    QMenu *menu = mAddPropertyMenu;
    menu->clear();
    foreach (PropertyDef *pd, mWorldDoc->world()->propertyDefinitions()) {
        if (mPropertyHolder->canAddProperty(pd))
            menu->addAction(pd->mName);
    }

    mAddPropertyWidget->setEnabled(!menu->actions().isEmpty());
}

void PropertiesModel::updateMenus()
{
    setTemplatesMenu();
    setPropertiesMenu();
}

void PropertiesModel::updateMenusLater()
{
    if (!mTimer.isActive())
        mTimer.start();
}

void PropertiesModel::addTemplate(Item *parent, int index, PropertyTemplate *pt)
{
    Item *item = new Item(parent, index, pt);

    Item *header = new Item(item, item->children.size(), Item::HeaderTemplates);
    foreach (PropertyTemplate *pt, pt->templates()) {
        addTemplate(header, header->children.size(), pt);
    }

    header = new Item(item, item->children.size(), Item::HeaderProperties);
    foreach (Property *p, pt->properties()) {
        addProperty(header, header->children.size(), p);
    }
}

void PropertiesModel::addProperty(PropertiesModel::Item *parent, int index, Property *p)
{
    new Item(parent, index, p);
}

void PropertiesModel::addTemplate(QAction *action)
{
    mWorldDoc->addTemplate(mPropertyHolder, action->text());
}

void PropertiesModel::addProperty(QAction *action)
{
    mWorldDoc->addProperty(mPropertyHolder, action->text());
}

void PropertiesModel::propertyDefinitionChanged(PropertyDef *pd)
{
    // Redisplay any property items using the definition
    redrawProperty(mRootItem, pd);

    // Update the name in the popup menu
    updateMenusLater();
}

void PropertiesModel::templateChanged(PropertyTemplate *pt)
{
    redrawTemplate(mRootItem, pt);

    // Update the name in the popup menu
    updateMenusLater();
}

void PropertiesModel::templateAboutToBeRemoved(int index)
{
    PropertyTemplate *pt = mWorldDoc->world()->propertyTemplates().at(index);
    if (pt == mPropertyHolder)
        return;
    foreach (Item *item, itemsFor(pt)) {
        item = item->parent;
        QModelIndex parent = this->index(item); // Templates header
        beginRemoveRows(parent, index, index);
        Item *remove = item->children.takeAt(index);
        delete remove;
        endRemoveRows();

        updateMenusLater();
    }
}

void PropertiesModel::templateAdded(PropertyHolder *ph, int index)
{
    foreach (Item *item, itemsFor(ph)) {
        item = item->children.first(); // Templates header
        QModelIndex parent = this->index(item);
        beginInsertRows(parent, index, index);
        addTemplate(item, index, ph->templates().at(index));
        endInsertRows();

        updateMenusLater();
    }
}

void PropertiesModel::templateAboutToBeRemoved(PropertyHolder *ph, int index)
{
    foreach (Item *item, itemsFor(ph)) {
        item = item->children.first(); // Templates header
        QModelIndex parent = this->index(item);
        qDebug() << "removing template at parent" << parent << "row" << index;
        beginRemoveRows(parent, index, index);
        Item *remove = item->children.takeAt(index);
        delete remove;
        endRemoveRows();

        updateMenusLater();
    }
}

void PropertiesModel::propertyAdded(PropertyHolder *ph, int index)
{
    foreach (Item *item, itemsFor(ph)) {
        item = item->children.last(); // Properties header
        QModelIndex parent = this->index(item);
        beginInsertRows(parent, index, index);
        addProperty(item, index, ph->properties().at(index));
        endInsertRows();

        updateMenusLater();
    }
}

void PropertiesModel::propertyAboutToBeRemoved(PropertyHolder *ph, int index)
{
    foreach (Item *item, itemsFor(ph)) {
        item = item->children.last(); // Properties header
        QModelIndex parent = this->index(item);
        beginRemoveRows(parent, index, index);
        Item *remove = item->children.takeAt(index);
        delete remove;
        endRemoveRows();

        updateMenusLater();
    }
}

void PropertiesModel::propertyValueChanged(PropertyHolder *ph, int index)
{
    foreach (Item *item, itemsFor(ph)) {
        item = item->children.last(); // Properties header
        QModelIndex parent = this->index(item);
        QModelIndex child = this->index(index, 1, parent);
        emit dataChanged(child, child);
    }
}

void PropertiesModel::cellContentsAboutToChange(WorldCell *cell)
{
    if (cell == mPropertyHolder) {
        delete mRootItem;
        mRootItem = 0;
        reset();
    }
}

void PropertiesModel::cellContentsChanged(WorldCell *cell)
{
    if (cell == mPropertyHolder)
        setModelData();
}

void PropertiesModel::cellObjectAboutToBeRemoved(WorldCell *cell, int index)
{
    WorldCellObject *obj = cell->objects().at(index);
    if (obj == mPropertyHolder) {
        mPropertyHolder = 0;
        delete mRootItem;
        mRootItem = 0;
        reset();
    }
}

void PropertiesModel::setModelData()
{
    delete mRootItem;
    mRootItem = 0;

    if (!mPropertyHolder) {
        reset();
        return;
    }

    mRootItem = new Item(mPropertyHolder);

    Item *header = new Item(mRootItem, mRootItem->children.size(), Item::HeaderTemplates);
    foreach (PropertyTemplate *pt, mPropertyHolder->templates()) {
        addTemplate(header, header->children.size(), pt);
    }

    createAddTemplateWidget();
    Item *addTemplateItem = new Item(header, header->children.size(), mAddTemplateWidget);

    header = new Item(mRootItem, mRootItem->children.size(), Item::HeaderProperties);
    foreach (Property *p, mPropertyHolder->properties()) {
        addProperty(header, header->children.size(), p);
    }

    createAddPropertyWidget();
    Item *addPropertyItem = new Item(header, header->children.size(), mAddPropertyWidget);

    reset();

    updateMenusLater();

    // FIXME: shouldn't be changing the view here
    {
//        QModelIndex parent = index(0, 0);
        QModelIndex widgetIndex = index(addTemplateItem); //index(toItem(parent)->children.count() - 1, 0, parent);
        mView->setIndexWidget(widgetIndex, mAddTemplateWidget);
        mView->expand(index(0, 0));
    }

    {
//        QModelIndex parent = index(1, 0);
        QModelIndex widgetIndex = index(addPropertyItem); // index(toItem(parent)->children.count() - 1, 0, parent);
        mView->setIndexWidget(widgetIndex, mAddPropertyWidget);
        mView->expand(index(1, 0));
    }
}

/////

#include <QHeaderView>
#include <QMouseEvent>

PropertiesView::PropertiesView(QWidget *parent)
    : QTreeView(parent)
    , mModel(new PropertiesModel(this))
{
    setModel(mModel);
    mModel->setView(this); // FIXME: model shouldn't know about a specific view
    setAlternatingRowColors(false);
    setItemDelegate(new PropertiesDelegate(this));
    header()->resizeSection(0, 180);
}

void PropertiesView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if ((index.column() == 0) && mModel->isDeletable(index)) {
        QRect itemRect = visualRect(index);
        QRect closeRect = ((PropertiesDelegate*)itemDelegate())->closeButtonRect(itemRect);
        if (closeRect.contains(event->pos())) {
            emit closeItem(index);
            return;
        }
    }
    QTreeView::mousePressEvent(event);
}

void PropertiesView::setPropertyHolder(WorldDocument *worldDoc, PropertyHolder *holder)
{
    mModel->setPropertyHolder(worldDoc, holder);
}

void PropertiesView::clearPropertyHolder()
{
    mModel->clearPropertyHolder();
}

/////

#include <QVBoxLayout>
#include <QTextEdit>

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mView(new PropertiesView(this))
    , mCellDoc(0)
    , mWorldDoc(0)
    , mDescEdit(new QTextEdit(this))
    , mLabel(new QLabel(this))
{
    setObjectName(QLatin1String("PropertiesDock"));

#if 0
    mView->header()->setStretchLastSection(false);
    mView->header()->setResizeMode(0, QHeaderView::Interactive);
    mView->header()->setResizeMode(1, QHeaderView::Interactive);
#endif

    QFrame *frame = new QFrame();
    frame->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->setMargin(2);
    vlayout->setSpacing(3);

    vlayout->addWidget(mLabel);
    vlayout->addWidget(mView);

    mDescEdit->setMaximumHeight(64);
    mDescEdit->setReadOnly(true);
//    mDescEdit->setFrameShape(QFrame::StyledPanel);
//    mDescEdit->setFrameStyle(mView->frameStyle());
//    mDescEdit->setFrameShadow(QFrame::Plain);
    vlayout->addWidget(mDescEdit);

    frame->setLayout(vlayout);

    setWidget(frame);
    retranslateUi();

    connect(mView, SIGNAL(closeItem(QModelIndex)), SLOT(closeItem(QModelIndex)));

    connect(mView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));
    connect(mView->model(), SIGNAL(theModelWasReset()),
            SLOT(selectionChanged()));
}

void PropertiesDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));
}

void PropertiesDock::setDocument(Document *doc)
{
    if (mWorldDoc)
        mWorldDoc->disconnect(this);
    if (mCellDoc)
        mCellDoc->disconnect(this);

    mWorldDoc = doc ? doc->asWorldDocument() : 0;
    mCellDoc = doc ? doc->asCellDocument() : 0;

    if (mCellDoc) {
        mPropertyHolder = mCellDoc->cell();
        mView->setPropertyHolder(mCellDoc->worldDocument(), mPropertyHolder);
        connect(mCellDoc, SIGNAL(selectedObjectsChanged()), SLOT(selectedObjectsChanged()));
        connect(mCellDoc->worldDocument(), SIGNAL(cellObjectNameChanged(WorldCellObject*)),
                SLOT(objectNameChanged(WorldCellObject*)));
    } else if (mWorldDoc) {
        connect(mWorldDoc, SIGNAL(selectedCellsChanged()), SLOT(selectedCellsChanged()));
        connect(mWorldDoc, SIGNAL(selectedObjectsChanged()), SLOT(selectedObjectsChanged()));
        connect(mWorldDoc, SIGNAL(cellObjectNameChanged(WorldCellObject*)),
                SLOT(objectNameChanged(WorldCellObject*)));
        selectedCellsChanged();
    } else {
        mPropertyHolder = 0;
        mView->clearPropertyHolder();
    }
    setLabel(mPropertyHolder);
}

void PropertiesDock::selectedCellsChanged()
{
    const QList<WorldCell*> &selectedCells = mWorldDoc->selectedCells();

    if (selectedCells.size() == 1) {
        // TODO: edit common properties of multiple cells
        WorldCell *cell = selectedCells.first();
        mPropertyHolder = cell;
        mView->setPropertyHolder(mWorldDoc, mPropertyHolder);
    } else {
        mPropertyHolder = 0;
        mView->clearPropertyHolder();
    }
    setLabel(mPropertyHolder);
}

void PropertiesDock::selectedObjectsChanged()
{
    if (mCellDoc) {
        WorldCell *cell = mCellDoc->cell();
        if (mCellDoc->selectedObjectCount() == 1) {
            // TODO: edit common properties of multiple objects
            WorldCellObject *obj = mCellDoc->selectedObjects().first();
            mPropertyHolder = obj;
            mView->setPropertyHolder(mCellDoc->worldDocument(), mPropertyHolder);
        } else {
            mPropertyHolder = cell;
            mView->setPropertyHolder(mCellDoc->worldDocument(), mPropertyHolder);
        }
        setLabel(mPropertyHolder);
        return;
    }
    if (mWorldDoc->selectedObjectCount() == 1) {
        // TODO: edit common properties of multiple objects
        WorldCellObject *obj = mWorldDoc->selectedObjects().first();
        mPropertyHolder = obj;
        mView->setPropertyHolder(mWorldDoc, mPropertyHolder);
    } else if (mWorldDoc->selectedCellCount() == 1) {
        WorldCell *cell = mWorldDoc->selectedCells().first();
        mPropertyHolder = cell;
        mView->setPropertyHolder(mWorldDoc, mPropertyHolder);
    } else {
        mPropertyHolder = 0;
        mView->clearPropertyHolder();
    }
    setLabel(mPropertyHolder);
}

void PropertiesDock::setLabel(PropertyHolder *ph)
{
    if (!ph)
        mLabel->setText(tr("<no selection>"));
    else if (WorldCellObject *obj = dynamic_cast<WorldCellObject*>(ph)) {
        QString name = obj->name();
        if (name.isEmpty())
            name = QLatin1String("<unnamed>");
        else
            name = tr("'%1'").arg(name);
        WorldCell *cell = obj->cell();
        mLabel->setText(tr("Object %1 in Cell %2,%3").arg(name).arg(cell->x()).arg(cell->y()));
    } else if (WorldCell *cell = dynamic_cast<WorldCell*>(ph))
        mLabel->setText(tr("Cell %1,%2").arg(cell->x()).arg(cell->y()));
}

void PropertiesDock::objectNameChanged(WorldCellObject *obj)
{
    if (obj == mPropertyHolder)
        setLabel(obj);
}

void PropertiesDock::selectionChanged()
{
    mDescEdit->clear();
    QModelIndexList selection = mView->selectionModel()->selectedIndexes();
    if (selection.size() == 2) {
        QString desc;
        if (PropertyTemplate *pt = mView->model()->toTemplate(selection.first()))
            desc = pt->mDescription;
        if (Property *p = mView->model()->toProperty(selection.first()))
            desc = p->mDefinition->mDescription;
        mDescEdit->insertPlainText(desc);
    }
}

void PropertiesDock::closeItem(const QModelIndex &index)
{
    WorldDocument *worldDoc = mWorldDoc;
    if (mCellDoc)
        worldDoc = mCellDoc->worldDocument();
    if (mView->model()->toTemplate(index)) {
        worldDoc->removeTemplate(mPropertyHolder, index.row());
    }  else if (mView->model()->toProperty(index)) {
        worldDoc->removeProperty(mPropertyHolder, index.row());
    }
}

