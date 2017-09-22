//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitsamplingreport.h"
#include "orbitdataviewpanel.h"
#include "orbittreeview.h"
#include "ui_orbitsamplingreport.h"
#include "../OrbitGl/SamplingReport.h"

//-----------------------------------------------------------------------------
OrbitSamplingReport::OrbitSamplingReport(QWidget *parent) : QWidget(parent)
                                                          , ui(new Ui::OrbitSamplingReport)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes.append( 5000 );
    sizes.append( 5000 );
    ui->splitter->setSizes( sizes );

    OrbitDataViewPanel* callStackView = ui->CallstackTreeView;
    callStackView->Initialize( DataViewType::CALLSTACK, false );
}

//-----------------------------------------------------------------------------
OrbitSamplingReport::~OrbitSamplingReport()
{
    delete ui;
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::Initialize( std::shared_ptr<SamplingReport> a_Report )
{
    m_SamplingReport = a_Report;

    if( !a_Report )
        return;

    m_SamplingReport->SetUiRefreshFunc( [&](){ this->Refresh(); } );

    for( std::shared_ptr<class DataViewModel> report : a_Report->GetThreadReports() )
    {
        //OrbitDataViewPanel *treeView = new OrbitDataViewPanel();
        QWidget* tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));

        QGridLayout* gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        OrbitDataViewPanel* treeView = new OrbitDataViewPanel(tab);
        treeView->SetDataModel( report );
        treeView->setObjectName( QStringLiteral("treeView") );
        gridLayout_2->addWidget( treeView, 0, 0, 1, 1 );
        treeView->GetTreeView()->setSelectionMode( OrbitTreeView::ExtendedSelection );
        treeView->GetTreeView()->header()->resizeSections( QHeaderView::ResizeToContents );

        treeView->Link( ui->CallstackTreeView );
        
        QString threadName = QString::fromStdWString(report->GetName());
        ui->tabWidget->addTab(tab, threadName);
    }
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::on_NextCallstackButton_clicked()
{
    m_SamplingReport->IncrementCallstackIndex();
    Refresh();
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::on_PreviousCallstackButton_clicked()
{
    m_SamplingReport->DecrementCallstackIndex();
    Refresh();
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::Refresh()
{
    std::wstring label = m_SamplingReport->GetSelectedCallstackString();
    ui->CallStackLabel->setText( QString::fromStdWString(label) );
    ui->CallstackTreeView->Refresh();
}
