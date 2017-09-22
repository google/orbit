//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QWidget>
#include "orbittablemodel.h"

namespace Ui {
class OrbitDataViewPanel;
}

class OrbitDataViewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit OrbitDataViewPanel(QWidget *parent = 0);
    ~OrbitDataViewPanel();

    void Initialize(DataViewType a_Type, bool a_MainInstance = true);
    void Link( OrbitDataViewPanel* a_Panel );
    void Refresh();
    void SetDataModel( std::shared_ptr<DataViewModel> a_Model );
    void SetFilter( QString a_Filter );
    void Select( int a_Row );
    class OrbitTreeView* GetTreeView();

private slots:
    void on_FilterLineEdit_textEdited(const QString &a_Text);

private:
    Ui::OrbitDataViewPanel *ui;
};
