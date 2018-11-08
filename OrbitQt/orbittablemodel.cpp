//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbittablemodel.h"
#include <QColor>
#include <memory>

#define UNUSED(x) (void)(x)

//-----------------------------------------------------------------------------
OrbitTableModel::OrbitTableModel(DataViewType a_Type, QObject* parent) 
    : QAbstractTableModel(parent)
    , m_DataView(nullptr)
    , m_AlternateRowColor(true)
{
    a_Type;
    m_DataView = std::shared_ptr<DataView>(DataView::Create(a_Type));
    
    if( a_Type == DataViewType::LOG )
    {
        m_AlternateRowColor = false;
    }
}

//-----------------------------------------------------------------------------
OrbitTableModel::OrbitTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_DataView(nullptr)
{
}

//-----------------------------------------------------------------------------
OrbitTableModel::~OrbitTableModel()
{
}

//-----------------------------------------------------------------------------
int OrbitTableModel::columnCount(const QModelIndex &parent) const
{
    UNUSED(parent);
    return (int)m_DataView->GetColumnHeaders().size();
}

//-----------------------------------------------------------------------------
int OrbitTableModel::rowCount(const QModelIndex &parent) const
{
    UNUSED(parent);
    return (int)m_DataView->GetNumElements();
}

//-----------------------------------------------------------------------------
QVariant OrbitTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{        
    switch (role)
    {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal && section < (int)m_DataView->GetColumnHeaders().size())
        {
            std::wstring str = m_DataView->GetColumnHeaders()[section];
            return QString::fromStdWString(str);
        }
        else if (orientation == Qt::Vertical)
            return section;
        else
            return QVariant();
    default: break;
    }

    return QVariant();
}

//-----------------------------------------------------------------------------
QVariant OrbitTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return QVariant( QString::fromStdWString( m_DataView->GetValue(index.row(), index.column()).c_str() ) );
    }
    else if (role == Qt::BackgroundRole)
    {
        if( m_AlternateRowColor )
        {
            if( index.row() & 1 )
                return QColor( 60, 60, 60 );
            else
                return QColor( 45, 45, 48 );
        }
        else
        {
            return QColor( 37, 37, 38 );
        }
    }
    else if (role == Qt::ForegroundRole)
    {
        if (m_DataView->WantsDisplayColor())
        {
            unsigned char r, g, b;
            if( m_DataView->GetDisplayColor( index.row(), index.column(), r, g, b ) )
            {
                return QColor( r, g, b );
            }
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        return QString::fromStdWString( m_DataView->GetToolTip( index.row(), index.column() ) );
    }
    
    return QVariant();
}

//-----------------------------------------------------------------------------
void OrbitTableModel::sort( int column, Qt::SortOrder /*order*/ )
{
    m_DataView->OnSort(column, true);
}

//-----------------------------------------------------------------------------
void OrbitTableModel::OnTimer()
{
    m_DataView->OnTimer(); 
}

//-----------------------------------------------------------------------------
void OrbitTableModel::OnFilter( const QString & a_Filter )
{
    m_DataView->SetFilter( a_Filter.toStdWString() );
}

//-----------------------------------------------------------------------------
void OrbitTableModel::OnClicked( const QModelIndex & index )
{
    if( m_DataView->GetNumElements() > index.row() )
    {
        m_DataView->OnSelect( index.row() );
    }
}
