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

#ifndef PROPERTYDEFINITIONSDIALOG_H
#define PROPERTYDEFINITIONSDIALOG_H

#include <QDialog>

class PropertyDef;
class WorldDocument;

class QTreeWidgetItem;

namespace Ui {
class PropertyDefinitionsDialog;
}

class PropertyDefinitionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PropertyDefinitionsDialog(WorldDocument *worldDoc, QWidget *parent = 0);

private slots:
    void definitionSelected();
    void addDefinition();
    void updateDefinition();
    void removeDefinition();
    void clearUI();
    void synchButtons();

private:
    void setList();

private:
    Ui::PropertyDefinitionsDialog *ui;
    WorldDocument *mWorldDoc;
    PropertyDef *mDef;
    QTreeWidgetItem *mItem;
    bool mSynching;
};

#endif // PROPERTYDEFINITIONSDIALOG_H
