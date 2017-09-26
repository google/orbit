#include "processlauncherwidget.h"
#include "ui_processlauncherwidget.h"
#include <QLineEdit>
#include <QFileDialog>

#include "../OrbitCore/Params.h"
#include "../OrbitGl/App.h"


ProcessLauncherWidget::ProcessLauncherWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProcessLauncherWidget)
{
    ui->setupUi(this);
    ui->ProcessComboBox->lineEdit()->setPlaceholderText( "Process" );
    ui->WorkingDirComboBox->lineEdit()->setPlaceholderText( "Working Directory" );
    ui->ArgumentsComboBox->lineEdit()->setPlaceholderText("Arguments");
    ui->LiveProcessList->Initialize( DataViewType::PROCESSES );

    ui->gridLayout_2->setColumnStretch(0, 90);
    ui->checkBoxPause->setChecked( GParams.m_StartPaused );

    ui->ProcessComboBox->lineEdit()->setText( "" );
}

ProcessLauncherWidget::~ProcessLauncherWidget()
{
    delete ui;
}

void ProcessLauncherWidget::on_BrowseButton_clicked()
{
    QStringList list = QFileDialog::getOpenFileNames( this, "Select an executable file...", "", "*.exe" );
    for( auto & file : list )
    {
        ui->ProcessComboBox->lineEdit()->setText( file );
        break;
    }
}

void ProcessLauncherWidget::on_LaunchButton_clicked()
{
    QString process = ui->ProcessComboBox->lineEdit()->text();
    QString workingDir = ui->WorkingDirComboBox->lineEdit()->text();
    QString args = ui->ArgumentsComboBox->lineEdit()->text();
    GOrbitApp->OnLaunchProcess( process.toStdWString(), workingDir.toStdWString(), args.toStdWString() );
}

void ProcessLauncherWidget::on_checkBoxPause_clicked(bool checked)
{
    GParams.m_StartPaused = checked;
    GParams.Save();
}

void ProcessLauncherWidget::on_BrowseWorkingDirButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    ui->WorkingDirComboBox->lineEdit()->setText( dir );
}
