//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitmainwindow.h"
#include "orbitsamplingreport.h"
#include "licensedialog.h"
#include "outputdialog.h"
#include "showincludesdialog.h"
#include "orbitdiffdialog.h"
#include "orbitvisualizer.h"
#include "orbitdisassemblydialog.h"
#include "ui_orbitmainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QBuffer>
#include <QMouseEvent>
#include <QToolTip>

#include "../OrbitGl/SamplingReport.h"
#include "../OrbitGl/App.h"
#include "../OrbitGl/PluginManager.h"
#include "../OrbitCore/Path.h"
#include "../OrbitCore/Version.h"
#include "../OrbitCore/PrintVar.h"
#include "../OrbitCore/Utils.h"
#include "../OrbitPlugin/OrbitSDK.h"

#include "../external/concurrentqueue/concurrentqueue.h"

//-----------------------------------------------------------------------------
OrbitMainWindow* GMainWindow;
extern QMenu* GContextMenu;

//-----------------------------------------------------------------------------
OrbitMainWindow::OrbitMainWindow(QApplication* a_App, QWidget *parent)
    : m_App( a_App )
    , QMainWindow(parent)
    , ui(new Ui::OrbitMainWindow)
    , m_Headless( false )
    , m_IsDev( false )
{
    OrbitApp::Init();

    ui->setupUi(this);
    ui->ProcessesList->SetProcessParams();

    QList<int> sizes;
    sizes.append(5000);
    sizes.append(5000);
    ui->HomeVerticalSplitter->setSizes(sizes);
    ui->HomeHorizontalSplitter->setSizes(sizes);
    ui->splitter_2->setSizes(sizes);

    GOrbitApp->AddRefreshCallback([this](DataViewType a_Type) { this->OnRefreshDataViewPanels(a_Type); });
    GOrbitApp->AddSamplingReoprtCallback( [this]( std::shared_ptr<SamplingReport> a_Report ) { this->OnNewSamplingReport( a_Report ); } );
    GOrbitApp->AddSelectionReportCallback( [this]( std::shared_ptr<SamplingReport> a_Report ) { this->OnNewSelection( a_Report ); } );
    GOrbitApp->AddUiMessageCallback( [this]( const std::wstring & a_Message ) { this->OnReceiveMessage( a_Message ); } );
    GOrbitApp->SetFindFileCallback( [this]( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter ){ return this->FindFile( a_Caption, a_Dir, a_Filter ); } );
    GOrbitApp->AddWatchCallback( [this]( const Variable* a_Variable ) { this->OnAddToWatch( a_Variable ); } );

    ParseCommandlineArguments();

    ui->DebugGLWidget     ->Initialize( GlPanel::DEBUG    , this );
    ui->CaptureGLWidget   ->Initialize( GlPanel::CAPTURE  , this );
    ui->VisualizeGLWidget ->Initialize( GlPanel::VISUALIZE, this );

    ui->ModulesList       ->Initialize( DataViewType::MODULES   );
    ui->FunctionsList     ->Initialize( DataViewType::FUNCTIONS );
    ui->LiveFunctionsList ->Initialize( DataViewType::LIVEFUNCTIONS );
    ui->CallStackView     ->Initialize( DataViewType::CALLSTACK );
    ui->TypesList         ->Initialize( DataViewType::TYPES     );
    ui->GlobalsList       ->Initialize( DataViewType::GLOBALS   );
    ui->SessionList       ->Initialize( DataViewType::SESSIONS  );
    ui->OutputView        ->Initialize( DataViewType::LOG  );

    SetupCodeView();
    SetupRuleEditor();

    if( !m_IsDev )
    {
        HideTab(ui->MainTabWidget, "debug");
        HideTab(ui->MainTabWidget, "visualize");
    }

    // Output window
    this->ui->plainTextEdit->SetIsOutputWindow();
    
    StartMainTimer();

    m_OutputDialog = new OutputDialog(this);
    m_OutputDialog->setWindowTitle("Orbit - Loading Pdb...");

    ui->actionEnable_Context_Switches->setChecked( GOrbitApp->GetTrackContextSwitches() );
    ui->actionEnable_Unreal_Support->setChecked( GOrbitApp->GetUnrealSupportEnabled() );
    ui->actionAllow_Unsafe_Hooking->setChecked( GOrbitApp->GetUnsafeHookingEnabled() );
    ui->actionEnable_Sampling->setChecked( GOrbitApp->GetSamplingEnabled() );
    ui->actionOutputDebugString->setChecked( GOrbitApp->GetOutputDebugStringEnabled() );

    CreateSamplingTab();
    CreateSelectionTab();
    CreatePluginTabs();

    this->setWindowTitle("Orbit Profiler");
    this->setWindowIcon(QIcon("orbit.ico"));

    GMainWindow = this;

    OrbitApp::PostInit();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetupCodeView()
{
    ui->CodeTextEdit->SetEditorType( OrbitCodeEditor::CODE_VIEW );
    ui->FileMappingTextEdit->SetEditorType( OrbitCodeEditor::FILE_MAPPING );
    ui->FileMappingTextEdit->SetSaveButton( ui->SaveFileMapping );
    ui->CodeTextEdit->SetFindLineEdit( ui->lineEdit );
    ui->FileMappingWidget->hide();
    OrbitCodeEditor::setFileMappingWidget( ui->FileMappingWidget );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetupRuleEditor()
{
    m_RuleEditor = new OrbitVisualizer(this);
    m_RuleEditor->Initialize(this);
    m_RuleEditor->setWindowFlags( m_RuleEditor->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint );
    m_RuleEditor->hide();
}

//-----------------------------------------------------------------------------
OrbitMainWindow::~OrbitMainWindow()
{
    delete m_OutputDialog;
    delete ui;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::ParseCommandlineArguments()
{
    std::vector< std::string > args;
    for (QString arg : QCoreApplication::arguments())
    {
        args.push_back(arg.toStdString());

        if( arg == "remote" )
        {
            m_Headless = true;
            this->menuBar()->hide();
        }
        else if( Contains(arg.toStdString(), "inject:") )
        {
            m_Headless = true;
        }
        else if( arg == "dev" )
        {
            m_IsDev = true;
        }
    }
    GOrbitApp->SetCommandLineArguments(args);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::PostInit()
{
}

//-----------------------------------------------------------------------------
bool OrbitMainWindow::HideTab( QTabWidget* a_TabWidget, const char* a_TabName )
{
    QTabWidget* tab = a_TabWidget;
    
    for( int i = 0; i < tab->count(); ++i )
    {
        std::string tabName = tab->tabText(i).toStdString();

        if( tabName == a_TabName )
        {
            tab->removeTab( i );
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
std::wstring OrbitMainWindow::FindFile( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter )
{
    QStringList list = QFileDialog::getOpenFileNames( this, ws2s(a_Caption).c_str(), ws2s(a_Dir).c_str(), ws2s(a_Filter).c_str() );
    for( auto & file : list )
    {
        return file.toStdWString();
    }

    return L"";
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnRefreshDataViewPanels( DataViewType a_Type )
{
    if( a_Type == DataViewType::ALL )
    {
        for( int i = 0; i < DataViewType::ALL; ++i )
        {
            UpdatePanel( (DataViewType)i );
        }
    }
    else
    {
        UpdatePanel( a_Type );
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::UpdatePanel( DataViewType a_Type )
{
    switch( a_Type )
    {
    case DataViewType::CALLSTACK:
        ui->CallStackView->Refresh();
        break;
    case DataViewType::FUNCTIONS:
        ui->FunctionsList->Refresh();
        break;
    case DataViewType::LIVEFUNCTIONS:
        ui->LiveFunctionsList->Refresh();
        break;
    case DataViewType::TYPES:
        ui->TypesList->Refresh();
        break;
    case DataViewType::GLOBALS:
        ui->GlobalsList->Refresh();
        break;
    case DataViewType::MODULES:
        ui->ModulesList->Refresh();
        break;
    case DataViewType::LOG:
        ui->OutputView->Refresh();
        break;
    case DataViewType::SESSIONS:
        ui->SessionList->Refresh();
        break;
    default: break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreateSamplingTab()
{
    m_SamplingTab = new QWidget();
    m_SamplingLayout = new QGridLayout( m_SamplingTab );
    m_SamplingLayout->setSpacing( 6 );
    m_SamplingLayout->setContentsMargins( 11, 11, 11, 11 );
    m_OrbitSamplingReport = new OrbitSamplingReport( m_SamplingTab );
    m_SamplingLayout->addWidget( m_OrbitSamplingReport, 0, 0, 1, 1 );

    ui->RightTabWidget->addTab( m_SamplingTab, QString( "sampling" ) );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreatePluginTabs()
{
    for( Orbit::Plugin* plugin : GPluginManager.m_Plugins )
    {
        QWidget* widget = new QWidget();
        QGridLayout* layout = new QGridLayout( widget );
        layout->setSpacing( 6 );
        layout->setContentsMargins( 11, 11, 11, 11 );
        OrbitGLWidget* glWidget = new OrbitGLWidget( widget );
        layout->addWidget( glWidget, 0, 0, 1, 1 );
        ui->RightTabWidget->addTab( widget, plugin->GetName() );

        glWidget->Initialize( GlPanel::PLUGIN, this, (void*)plugin );
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSamplingReport( std::shared_ptr<SamplingReport> a_SamplingReport )
{
    m_SamplingLayout->removeWidget( m_OrbitSamplingReport );
    delete m_OrbitSamplingReport;
    
    m_OrbitSamplingReport = new OrbitSamplingReport( m_SamplingTab );
    m_OrbitSamplingReport->Initialize( a_SamplingReport );
    m_SamplingLayout->addWidget( m_OrbitSamplingReport, 0, 0, 1, 1 );
    ui->RightTabWidget->setCurrentWidget( m_SamplingTab );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreateSelectionTab()
{
    m_SelectionTab = new QWidget();
    m_SelectionLayout = new QGridLayout( m_SelectionTab );
    m_SelectionLayout->setSpacing( 6 );
    m_SelectionLayout->setContentsMargins( 11, 11, 11, 11 );
    m_SelectionReport = new OrbitSamplingReport( m_SelectionTab );
    m_SelectionLayout->addWidget( m_SelectionReport, 0, 0, 1, 1 );
    ui->RightTabWidget->addTab( m_SelectionTab, QString( "selection" ) );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSelection( std::shared_ptr<class SamplingReport> a_SamplingReport )
{
    m_SelectionLayout->removeWidget( m_SelectionReport );
    delete m_SelectionReport;

    m_SelectionReport = new OrbitSamplingReport( m_SelectionTab );
    m_SelectionReport->Initialize( a_SamplingReport );
    m_SelectionLayout->addWidget( m_SelectionReport, 0, 0, 1, 1 );

    ui->RightTabWidget->setCurrentWidget( m_SelectionTab );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnReceiveMessage( const std::wstring & a_Message )
{
    if( a_Message == L"ScreenShot" )
    {
        QFile file( "sshot.png" );
        file.open( QIODevice::WriteOnly );
        QPixmap pixMap = this->grab();
        pixMap.save( &file, "PNG" );

        std::wstring fileName = file.fileName().toStdWString();
        ShellExecute(0, 0, fileName.c_str(), 0, 0 , SW_SHOW );
    }
    else if( StartsWith( a_Message, L"code" ) )
    {
        ui->FileMappingWidget->hide();
        
        bool success = ui->CodeTextEdit->loadCode( ws2s(a_Message) );
        
        if (!success)
        {
            ui->FileMappingTextEdit->loadFileMap();
            ui->FileMappingWidget->show();
        }
    }
    else if( StartsWith( a_Message, L"tooltip:" ) )
    {
        QToolTip::showText( QCursor::pos(), Replace( ws2s( a_Message), "tooltip:", "" ).c_str(), this );
    }
    else if ( StartsWith(a_Message, L"output") )
    {
        ui->MainTabWidget->setCurrentWidget( ui->OutputTab );
    }
    else if( StartsWith( a_Message, L"gotocode" ) )
    {
        ui->RightTabWidget->setCurrentWidget( ui->CodeTab );
    }
    else if( StartsWith( a_Message, L"startcapture") )
    {
        ui->RightTabWidget->setCurrentWidget( ui->LiveTab );
    }
    else if( StartsWith(a_Message, L"license") )
    {
        GetLicense();
    }
    else if( StartsWith(a_Message, L"pdb:") )
    {
        if( GOrbitApp->IsLoading() )
        {
            m_CurrentPdbName = Replace( ws2s(a_Message), "pdb:", "" );
            m_OutputDialog->Reset();
            m_OutputDialog->SetStatus( m_CurrentPdbName );
            m_OutputDialog->show();
            this->setDisabled(true);
        }
    }
    else if( StartsWith( a_Message, L"pdbloaded" ) )
    {
        this->setDisabled(false);
        m_OutputDialog->hide();
    }
    else if( StartsWith( a_Message, L"status:" ) )
    {
        m_OutputDialog->SetStatus( Replace( ws2s(a_Message), "status:", "" ) );
    }
    else if( StartsWith( a_Message, L"log:" ) )
    {
        m_OutputDialog->AddLog( Replace( a_Message, L"log:", L"" ) );
    }
    else if( a_Message == L"Update" )
    {
        std::string title = "Orbit Profiler";

        title += " | Version " + GOrbitApp->GetVersion();
        std::string msg = Format("A new version (%s) is available at <a href='www.telescopp.com/update'>telescopp.com/update</a>", OrbitVersion::s_LatestVersion.c_str() );
        QMessageBox::about( this, title.c_str(), msg.c_str() );
    }
    else if( StartsWith( a_Message, L"asm:" ) )
    {
        OpenDisassembly( a_Message );
    }
    else if( StartsWith( a_Message, L"RuleEditor" ) )
    {
        m_RuleEditor->show();
    }
    else if (StartsWith(a_Message, L"UpdateProcessParams"))
    {
        ui->ProcessesList->UpdateProcessParams();
    }
    else if (StartsWith(a_Message, L"SetProcessParams"))
    {
        ui->ProcessesList->SetProcessParams();
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnAddToWatch( const class Variable* a_Variable )
{
    ui->WatchWidget->AddToWatch( a_Variable );
    ui->MainTabWidget->setCurrentWidget( ui->WatchTab );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::GetLicense()
{
    LicenseDialog dialog(this);
    dialog.setWindowTitle("Orbit Profiler");
    if( dialog.exec() == QDialog::Accepted )
    {
        GOrbitApp->SetLicense( dialog.GetLicense() );
    }
    else
    {
        exit(0);
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionAbout_triggered()
{
    std::string title = "Orbit Profiler";

    title += " | Version " + GOrbitApp->GetVersion();

    QMessageBox::about(this, title.c_str(), "Copyright (c) 2013-2018 - Pierric Gimmig");
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::StartMainTimer()
{
    m_MainTimer = new QTimer(this);
    connect(m_MainTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));

    // Update period set to 16ms (~60FPS)
    int msec = 16;
    m_MainTimer->start(msec);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnTimer()
{
    OrbitApp::MainTick();

    for (OrbitGLWidget* glWidget : m_GlWidgets)
    {
        glWidget->update();
    }

    // Output window
    this->ui->plainTextEdit->OnTimer();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnHideSearch()
{
    ui->lineEdit->hide();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Capture_triggered()
{
    QStringList list = QFileDialog::getOpenFileNames(this, "Select a file to open...", ws2s(Path::GetCapturePath()).c_str(), "*.hdb" );
    for( auto & file : list )
    {
        GOrbitApp->OnOpenCapture( file.toStdWString() );
        break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Session_triggered()
{
    std::wstring sessionName = GOrbitApp->GetSessionFileName();
    if( sessionName != L"" )
    {
        GOrbitApp->OnSaveSession( sessionName );
    }
    else
    {
        on_actionSave_Session_As_triggered();
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Session_triggered()
{
    QStringList list = QFileDialog::getOpenFileNames(this, "Select a file to open...", ws2s(Path::GetPresetPath()).c_str(), "*.opr");
    for (auto & file : list)
    {
        GOrbitApp->OnLoadSession( file.toStdWString() );
        break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_PDB_triggered()
{
    QStringList list = QFileDialog::getOpenFileNames(this, "Select a pdb file to open...", "", "*.pdb");
    for (auto & file : list)
    {
        GOrbitApp->OnOpenPdb(file.toStdWString());
        break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionDisconnect_triggered()
{
    GOrbitApp->OnDisconnect();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

//-----------------------------------------------------------------------------
__declspec(noinline) QPixmap QtGrab( OrbitMainWindow* a_Window )
{
    QPixmap pixMap = a_Window->grab();
    if (GContextMenu)
    {
        QPixmap menuPixMap = GContextMenu->grab();
        pixMap.copy();
    }
    return pixMap;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionToogleDevMode_toggled(bool a_Toggle)
{
    a_Toggle;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Session_As_triggered()
{
    QString file = QFileDialog::getSaveFileName( this, "Specify a file to save...", ws2s(Path::GetPresetPath()).c_str(), "*.opr" );
    GOrbitApp->OnSaveSession( file.toStdWString() );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Context_Switches_triggered()
{
   
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Context_Switches_triggered(bool checked)
{
    GOrbitApp->SetTrackContextSwitches(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionLaunch_Process_triggered()
{
    QStringList list = QFileDialog::getOpenFileNames( this, "Select an executable to launch...", "", "*.exe" );
    for( auto & file : list )
    {
        GOrbitApp->OnLaunchProcess( file.toStdWString(), TEXT(""), TEXT("") );
        break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Unreal_Support_triggered(bool checked)
{
    GOrbitApp->EnableUnrealSupport(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionAllow_Unsafe_Hooking_triggered(bool checked)
{
    GOrbitApp->EnableUnsafeHooking( checked );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Sampling_triggered( bool checked )
{
    GOrbitApp->EnableSampling( checked );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Sampling_toggled(bool arg1)
{
    arg1;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Capture_triggered()
{
    QString file = QFileDialog::getSaveFileName( this, "Save capture...", ws2s( Path::GetCapturePath() + GOrbitApp->GetCaptureFileName() ).c_str() , "*.orbit" );
    GOrbitApp->OnSaveCapture( file.toStdWString() );
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Capture_2_triggered()
{
    QStringList list = QFileDialog::getOpenFileNames( this, "Open capture...", ws2s( Path::GetCapturePath() ).c_str(), "*.orbit" );
    for( auto & file : list )
    {
        GOrbitApp->OnLoadCapture( file.toStdWString() );
        break;
    }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionShow_Includes_Util_triggered()
{
    ShowIncludesDialog* dialog = new ShowIncludesDialog(this);
    dialog->setWindowTitle("Orbit Show Includes Utility");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionDiff_triggered()
{
    OrbitDiffDialog* dialog = new OrbitDiffDialog(this);
    dialog->setWindowTitle("Orbit Diff Utility");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OpenDisassembly( const std::wstring & a_String )
{
    OrbitDisassemblyDialog* dialog = new OrbitDisassemblyDialog( this );
    dialog->SetText( a_String );
    dialog->setWindowTitle( "Orbit Disassembly" );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setWindowFlags( dialog->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint );
    dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOutputDebugString_triggered(bool checked)
{
    GOrbitApp->EnableOutputDebugString(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionRule_Editor_triggered()
{    
    m_RuleEditor->show();
}
