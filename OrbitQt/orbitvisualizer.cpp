#include "orbitvisualizer.h"
#include "ui_orbitvisualizer.h"

OrbitVisualizer::OrbitVisualizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OrbitVisualizer)
{
    ui->setupUi(this);
}

OrbitVisualizer::~OrbitVisualizer()
{
    delete ui;
}

void OrbitVisualizer::Initialize( OrbitMainWindow* a_MainWindow )
{
    ui->RuleEditor->Initialize( GlPanel::RULE_EDITOR, a_MainWindow );
    ui->Visualizer->Initialize( GlPanel::VISUALIZE  , a_MainWindow );
}
