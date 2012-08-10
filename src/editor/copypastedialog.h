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

#ifndef COPYPASTEDIALOG_H
#define COPYPASTEDIALOG_H

#include <QDialog>
#include <QMap>

class CellDocument;
class PropertyDef;
class World;
class WorldCell;
class WorldDocument;

class QTreeWidgetItem;

namespace Ui {
class CopyPasteDialog;
}

class CopyPasteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CopyPasteDialog(WorldDocument *worldDoc, QWidget *parent = 0);
    explicit CopyPasteDialog(CellDocument *cellDoc, QWidget *parent = 0);
    ~CopyPasteDialog();

private:
    void setup();

    enum WorldCat {
        PropertyDefs = 0,
        Templates,
        ObjectTypes,
        ObjectGroups
    };

    void showPropertyDefs();
    void showTemplates();
    void showObjectTypes();
    void showObjectGroups();

private slots:
    void worldSelectionChanged(int index);
    void worldItemChanged(QTreeWidgetItem *item, int column);
    void worldCheckAll();
    void worldCheckNone();

private:
    enum CellCat {
        FirstCellCat = 0,
        Properties = FirstCellCat,
        CellTemplates,
        Lots,
        Objects,
        Map,
        MaxCellCat
    };

    void showCellProperties();
    void showCellTemplates();
    void showCellLots();
    void showCellObjects();
    void showCellMap();

private slots:
    void cellCategoryChanged(int index);
    void cellItemChanged(QTreeWidgetItem *viewItem, int column);
    void cellCheckAll();
    void cellCheckNone();

private:
    Ui::CopyPasteDialog *ui;
    WorldDocument *mWorldDoc;
    CellDocument *mCellDoc;
    World *mWorld;

    WorldCat mWorldCat;
    QMap<PropertyDef*,bool> mCheckedPropertyDefs;
    QMap<QTreeWidgetItem*,PropertyDef*> mItemToPropertyDef;

    CellCat mCellCat;
    QList<WorldCell*> mCells;

public: // public for Q_DECLARE_METATYPE
    class Item;
    class CellItem;
    class LevelItem;
    class LotItem;
    class MapTypeItem;
    class ObjectGroupItem;
    class ObjectItem;
    class PropertyItem;
    class TemplateItem;

    Item *mRootItem[MaxCellCat];

    void addToTree(Item *parent, int index, Item *item, const QString &text,
                   const QString &text2 = QString());
};

#endif // COPYPASTEDIALOG_H
