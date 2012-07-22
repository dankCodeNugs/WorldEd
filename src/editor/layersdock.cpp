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

#include "layersdock.h"

#include "celldocument.h"
#include "cellscene.h"
#include "documentmanager.h"
#include "layersmodel.h"
#include "mapcomposite.h"
#include "worlddocument.h"

#include "map.h"
#include "tilelayer.h"

#include <QBoxLayout>
#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>

using namespace Tiled;

LayersDock::LayersDock(QWidget *parent)
    : QDockWidget(parent)
    , mView(new LayersView())
    , mCellDocument(0)
{
    setObjectName(QLatin1String("LevelsDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(2);
    layout->addWidget(mView);

    setWidget(widget);
    retranslateUi();

    // Workaround since a tabbed dockwidget that is not currently visible still
    // returns true for isVisible()
    connect(this, SIGNAL(visibilityChanged(bool)),
            mView, SLOT(setVisible(bool)));

    connect(DocumentManager::instance(), SIGNAL(documentAboutToClose(int,Document*)),
            SLOT(documentAboutToClose(int,Document*)));
}

void LayersDock::setCellDocument(CellDocument *doc)
{
    if (mCellDocument) {
        saveExpandedLevels(mCellDocument);
    }

    mCellDocument = doc;

    mView->setCellDocument(mCellDocument);

    if (mCellDocument) {
        // This connection won't break until the document is closed
        connect(mCellDocument->worldDocument(), SIGNAL(cellMapFileAboutToChange(WorldCell*)),
                SLOT(cellMapFileAboutToChange(WorldCell*)), Qt::UniqueConnection);
        connect(mCellDocument->worldDocument(), SIGNAL(cellContentsAboutToChange(WorldCell*)),
                SLOT(cellMapFileAboutToChange(WorldCell*)), Qt::UniqueConnection);
        restoreExpandedLevels(mCellDocument);
    }
}

void LayersDock::changeEvent(QEvent *e)
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

void LayersDock::retranslateUi()
{
    setWindowTitle(tr("Layers"));
}

void LayersDock::saveExpandedLevels(CellDocument *doc)
{
    mExpandedLevels[doc].clear();
    MapComposite *mapComposite = doc->scene()->mapComposite();
    foreach (CompositeLayerGroup *g, mapComposite->layerGroups()) {
        if (mView->isExpanded(mView->model()->index(g)))
            mExpandedLevels[doc].append(g);
    }
}

void LayersDock::restoreExpandedLevels(CellDocument *doc)
{
    if (!mExpandedLevels.contains(doc))
        mView->collapseAll();
    foreach (CompositeLayerGroup *g, mExpandedLevels[doc])
        mView->setExpanded(mView->model()->index(g), true);
    mExpandedLevels[doc].clear();
#if 0
    // Also restore the selection
    foreach (MapObject *o, mapDoc->selectedObjects()) {
        QModelIndex index = mView->model()->index(o);
        mView->selectionModel()->select(index, QItemSelectionModel::Select |  QItemSelectionModel::Rows);
    }
#endif
}

void LayersDock::documentAboutToClose(int index, Document *doc)
{
    Q_UNUSED(index)
    if (CellDocument *cellDoc = doc->asCellDocument()) {
        if (mExpandedLevels.contains(cellDoc))
            mExpandedLevels.remove(cellDoc);
    }
}

void LayersDock::cellMapFileAboutToChange(WorldCell *cell)
{
    CellDocument *doc = DocumentManager::instance()->findDocument(cell);
    if (doc && mExpandedLevels.contains(doc))
        mExpandedLevels.remove(doc);
}

///// ///// ///// ///// /////

LayersView::LayersView(QWidget *parent)
    : QTreeView(parent)
    , mCellDocument(0)
    , mSynching(false)
    , mModel(new LayersModel(this))
{
    setRootIsDecorated(true);
    setHeaderHidden(true);
    setItemsExpandable(true);
    setUniformRowHeights(true);

    setModel(mModel);

    setSelectionBehavior(QAbstractItemView::SelectRows);
//    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(onActivated(QModelIndex)));
}

QSize LayersView::sizeHint() const
{
    return QSize(130, 100);
}

void LayersView::setCellDocument(CellDocument *doc)
{
    if (doc == mCellDocument)
        return;

    if (mCellDocument) {
        mCellDocument->disconnect(this);
    }

    mCellDocument = doc;

    if (mCellDocument) {
//        setModel(mModel = mCellDocument->levelsModel());
        model()->setCellDocument(mCellDocument);
        header()->setResizeMode(0, QHeaderView::Stretch); // 2 equal-sized columns, user can't adjust

        connect(mCellDocument, SIGNAL(currentLevelChanged(int)),
                SLOT(currentLevelOrLayerIndexChanged(int)));

        mSynching = true;
        if (TileLayer *tl = mCellDocument->currentTileLayer())
            setCurrentIndex(model()->index(tl));
        else if (CompositeLayerGroup *g = mCellDocument->currentLayerGroup())
            setCurrentIndex(model()->index(g));
        mSynching = false;
    } else {
        model()->setCellDocument(0);
//        setModel(mModel = 0);
    }

}

void LayersView::onActivated(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void LayersView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    if (!mCellDocument || mSynching)
        return;

    QModelIndexList selectedRows = selectionModel()->selectedRows();
    int count = selectedRows.count();

    mSynching = true;
    if (count == 1) {
        QModelIndex index = selectedRows.first();
        if (TileLayer *tl = model()->toTileLayer(index)) {
            int layerIndex = mCellDocument->scene()->map()->layers().indexOf(tl);
            if (layerIndex != mCellDocument->currentLayerIndex())
                mCellDocument->setCurrentLayerIndex(layerIndex);
        }
        if (CompositeLayerGroup *g = model()->toTileLayerGroup(index)) {
            int level = g->level();
            if (level != mCellDocument->currentLevel())
                mCellDocument->setCurrentLevel(level);
        }
    } else if (!count) {
        mCellDocument->setCurrentLayerIndex(-1);
    }
    mSynching = false;
}

void LayersView::currentLevelOrLayerIndexChanged(int index)
{
    Q_UNUSED(index)

    if (mSynching)
        return;

    if (TileLayer *tl = mCellDocument->currentTileLayer()) {
        mSynching = true;
        setCurrentIndex(model()->index(tl));
        mSynching = false;
        return;
    }
    if (CompositeLayerGroup *g = mCellDocument->currentLayerGroup()) {
        mSynching = true;
        setCurrentIndex(model()->index(g));
        mSynching = false;
        return;
    }

    // This should never happen
    mSynching = true;
    setCurrentIndex(QModelIndex());
    mSynching = false;
}
