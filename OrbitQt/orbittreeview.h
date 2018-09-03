//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QTreeView>
#include <QTimer>
#include "orbittablemodel.h"
#include "orbitglwidget.h"

class OrbitTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit OrbitTreeView(QWidget *parent = 0);
    virtual ~OrbitTreeView();
    void Initialize(DataViewType a_Type);
    void SetDataModel(std::shared_ptr<DataView> a_Model);
    void OnFilter( const QString & a_Filter );
    void Select( int a_Row );
    void Refresh();
    void Link( OrbitTreeView* a_Link );
    void SetGlWidget( OrbitGLWidget* a_Link );
    void resizeEvent(QResizeEvent * event);
    void keyPressEvent(QKeyEvent *event);
    OrbitTableModel* GetModel() { return m_Model; }
    std::wstring GetLabel();

protected:
    virtual void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    void SetLabel();

signals:

public slots:
    void columnResized(int column, int oldSize, int newSize);

private slots:
    void OnSort( int a_Section, Qt::SortOrder a_Order );
    void OnTimer();
    void OnClicked(const QModelIndex &index);
    void ShowContextMenu(const QPoint &pos);
    void OnMenuClicked(int a_Index);
    void OnRangeChanged(int a_Min, int a_Max);

private:
    OrbitTableModel* m_Model;
    QTimer*          m_Timer;
    std::vector< OrbitTreeView* > m_Links;
    bool             m_AutoResize;
};

