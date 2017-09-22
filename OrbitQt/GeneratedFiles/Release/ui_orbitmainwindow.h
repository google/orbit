/********************************************************************************
** Form generated from reading UI file 'orbitmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITMAINWINDOW_H
#define UI_ORBITMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>
#include "orbitcodeeditor.h"
#include "orbitdataviewpanel.h"
#include "orbitglwidget.h"
#include "orbitwatchwidget.h"
#include "processlauncherwidget.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitMainWindow
{
public:
    QAction *actionSave_Session;
    QAction *actionAbout;
    QAction *actionOpen_Session;
    QAction *actionOpen_PDB;
    QAction *actionOpen_Capture;
    QAction *actionDisconnect;
    QAction *actionStart_Captue;
    QAction *actionStop_Capture;
    QAction *actionQuit;
    QAction *actionToogleDevMode;
    QAction *actionSave_Session_As;
    QAction *actionEnable_Context_Switches;
    QAction *actionLaunch_Process;
    QAction *actionEnable_Unreal_Support;
    QAction *actionAllow_Unsafe_Hooking;
    QAction *actionEnable_Sampling;
    QAction *actionOpen_Capture_2;
    QAction *actionSave_Capture;
    QAction *actionShow_Includes_Util;
    QAction *actionDiff;
    QAction *actionOutputDebugString;
    QAction *actionRule_Editor;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QSplitter *splitter_2;
    QTabWidget *MainTabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_7;
    QSplitter *HomeHorizontalSplitter;
    QSplitter *HomeVerticalSplitter;
    ProcessLauncherWidget *ProcessesList;
    OrbitDataViewPanel *SessionList;
    OrbitDataViewPanel *ModulesList;
    QWidget *DebugTab;
    QGridLayout *gridLayout_3;
    OrbitGLWidget *DebugGLWidget;
    QWidget *CaptureTab;
    QGridLayout *gridLayout_5;
    OrbitGLWidget *CaptureGLWidget;
    QWidget *OutputTab;
    QGridLayout *gridLayout_13;
    OrbitCodeEditor *plainTextEdit;
    QWidget *VisualizeTab;
    QGridLayout *gridLayout_6;
    OrbitGLWidget *VisualizeGLWidget;
    QWidget *WatchTab;
    QGridLayout *gridLayout_14;
    OrbitWatchWidget *WatchWidget;
    QTabWidget *RightTabWidget;
    QWidget *FunctionsTab;
    QGridLayout *gridLayout_2;
    OrbitDataViewPanel *FunctionsList;
    QWidget *TypesTab;
    QGridLayout *gridLayout_11;
    OrbitDataViewPanel *TypesList;
    QWidget *GlobalsType;
    QGridLayout *gridLayout_12;
    OrbitDataViewPanel *GlobalsList;
    QWidget *LiveTab;
    QGridLayout *gridLayout_15;
    OrbitDataViewPanel *LiveFunctionsList;
    QWidget *CallStackTab;
    QGridLayout *gridLayout_8;
    OrbitDataViewPanel *CallStackView;
    QWidget *CodeTab;
    QGridLayout *gridLayout_10;
    QLineEdit *lineEdit;
    OrbitCodeEditor *CodeTextEdit;
    QWidget *FileMappingWidget;
    QGridLayout *gridLayout_9;
    QPushButton *SaveFileMapping;
    OrbitCodeEditor *FileMappingTextEdit;
    QWidget *outputTab;
    QGridLayout *gridLayout_4;
    OrbitDataViewPanel *OutputView;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuDev;
    QMenu *menuTools;

    void setupUi(QMainWindow *OrbitMainWindow)
    {
        if (OrbitMainWindow->objectName().isEmpty())
            OrbitMainWindow->setObjectName(QStringLiteral("OrbitMainWindow"));
        OrbitMainWindow->resize(976, 594);
        actionSave_Session = new QAction(OrbitMainWindow);
        actionSave_Session->setObjectName(QStringLiteral("actionSave_Session"));
        actionAbout = new QAction(OrbitMainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionOpen_Session = new QAction(OrbitMainWindow);
        actionOpen_Session->setObjectName(QStringLiteral("actionOpen_Session"));
        actionOpen_PDB = new QAction(OrbitMainWindow);
        actionOpen_PDB->setObjectName(QStringLiteral("actionOpen_PDB"));
        actionOpen_Capture = new QAction(OrbitMainWindow);
        actionOpen_Capture->setObjectName(QStringLiteral("actionOpen_Capture"));
        actionDisconnect = new QAction(OrbitMainWindow);
        actionDisconnect->setObjectName(QStringLiteral("actionDisconnect"));
        actionStart_Captue = new QAction(OrbitMainWindow);
        actionStart_Captue->setObjectName(QStringLiteral("actionStart_Captue"));
        actionStop_Capture = new QAction(OrbitMainWindow);
        actionStop_Capture->setObjectName(QStringLiteral("actionStop_Capture"));
        actionQuit = new QAction(OrbitMainWindow);
        actionQuit->setObjectName(QStringLiteral("actionQuit"));
        actionToogleDevMode = new QAction(OrbitMainWindow);
        actionToogleDevMode->setObjectName(QStringLiteral("actionToogleDevMode"));
        actionToogleDevMode->setCheckable(true);
        actionSave_Session_As = new QAction(OrbitMainWindow);
        actionSave_Session_As->setObjectName(QStringLiteral("actionSave_Session_As"));
        actionEnable_Context_Switches = new QAction(OrbitMainWindow);
        actionEnable_Context_Switches->setObjectName(QStringLiteral("actionEnable_Context_Switches"));
        actionEnable_Context_Switches->setCheckable(true);
        actionLaunch_Process = new QAction(OrbitMainWindow);
        actionLaunch_Process->setObjectName(QStringLiteral("actionLaunch_Process"));
        actionEnable_Unreal_Support = new QAction(OrbitMainWindow);
        actionEnable_Unreal_Support->setObjectName(QStringLiteral("actionEnable_Unreal_Support"));
        actionEnable_Unreal_Support->setCheckable(true);
        actionAllow_Unsafe_Hooking = new QAction(OrbitMainWindow);
        actionAllow_Unsafe_Hooking->setObjectName(QStringLiteral("actionAllow_Unsafe_Hooking"));
        actionAllow_Unsafe_Hooking->setCheckable(true);
        actionEnable_Sampling = new QAction(OrbitMainWindow);
        actionEnable_Sampling->setObjectName(QStringLiteral("actionEnable_Sampling"));
        actionEnable_Sampling->setCheckable(true);
        actionOpen_Capture_2 = new QAction(OrbitMainWindow);
        actionOpen_Capture_2->setObjectName(QStringLiteral("actionOpen_Capture_2"));
        actionSave_Capture = new QAction(OrbitMainWindow);
        actionSave_Capture->setObjectName(QStringLiteral("actionSave_Capture"));
        actionShow_Includes_Util = new QAction(OrbitMainWindow);
        actionShow_Includes_Util->setObjectName(QStringLiteral("actionShow_Includes_Util"));
        actionDiff = new QAction(OrbitMainWindow);
        actionDiff->setObjectName(QStringLiteral("actionDiff"));
        actionOutputDebugString = new QAction(OrbitMainWindow);
        actionOutputDebugString->setObjectName(QStringLiteral("actionOutputDebugString"));
        actionOutputDebugString->setCheckable(true);
        actionRule_Editor = new QAction(OrbitMainWindow);
        actionRule_Editor->setObjectName(QStringLiteral("actionRule_Editor"));
        centralWidget = new QWidget(OrbitMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        splitter_2 = new QSplitter(centralWidget);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitter_2->sizePolicy().hasHeightForWidth());
        splitter_2->setSizePolicy(sizePolicy);
        splitter_2->setOrientation(Qt::Horizontal);
        MainTabWidget = new QTabWidget(splitter_2);
        MainTabWidget->setObjectName(QStringLiteral("MainTabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        gridLayout_7 = new QGridLayout(tab);
        gridLayout_7->setSpacing(6);
        gridLayout_7->setContentsMargins(11, 11, 11, 11);
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        HomeHorizontalSplitter = new QSplitter(tab);
        HomeHorizontalSplitter->setObjectName(QStringLiteral("HomeHorizontalSplitter"));
        sizePolicy.setHeightForWidth(HomeHorizontalSplitter->sizePolicy().hasHeightForWidth());
        HomeHorizontalSplitter->setSizePolicy(sizePolicy);
        HomeHorizontalSplitter->setOrientation(Qt::Vertical);
        HomeVerticalSplitter = new QSplitter(HomeHorizontalSplitter);
        HomeVerticalSplitter->setObjectName(QStringLiteral("HomeVerticalSplitter"));
        sizePolicy.setHeightForWidth(HomeVerticalSplitter->sizePolicy().hasHeightForWidth());
        HomeVerticalSplitter->setSizePolicy(sizePolicy);
        HomeVerticalSplitter->setBaseSize(QSize(0, 0));
        HomeVerticalSplitter->setOrientation(Qt::Horizontal);
        ProcessesList = new ProcessLauncherWidget(HomeVerticalSplitter);
        ProcessesList->setObjectName(QStringLiteral("ProcessesList"));
        HomeVerticalSplitter->addWidget(ProcessesList);
        SessionList = new OrbitDataViewPanel(HomeVerticalSplitter);
        SessionList->setObjectName(QStringLiteral("SessionList"));
        HomeVerticalSplitter->addWidget(SessionList);
        HomeHorizontalSplitter->addWidget(HomeVerticalSplitter);
        ModulesList = new OrbitDataViewPanel(HomeHorizontalSplitter);
        ModulesList->setObjectName(QStringLiteral("ModulesList"));
        HomeHorizontalSplitter->addWidget(ModulesList);

        gridLayout_7->addWidget(HomeHorizontalSplitter, 0, 1, 1, 1);

        MainTabWidget->addTab(tab, QString());
        DebugTab = new QWidget();
        DebugTab->setObjectName(QStringLiteral("DebugTab"));
        gridLayout_3 = new QGridLayout(DebugTab);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        DebugGLWidget = new OrbitGLWidget(DebugTab);
        DebugGLWidget->setObjectName(QStringLiteral("DebugGLWidget"));

        gridLayout_3->addWidget(DebugGLWidget, 0, 0, 1, 1);

        MainTabWidget->addTab(DebugTab, QString());
        CaptureTab = new QWidget();
        CaptureTab->setObjectName(QStringLiteral("CaptureTab"));
        gridLayout_5 = new QGridLayout(CaptureTab);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        CaptureGLWidget = new OrbitGLWidget(CaptureTab);
        CaptureGLWidget->setObjectName(QStringLiteral("CaptureGLWidget"));

        gridLayout_5->addWidget(CaptureGLWidget, 0, 0, 1, 1);

        MainTabWidget->addTab(CaptureTab, QString());
        OutputTab = new QWidget();
        OutputTab->setObjectName(QStringLiteral("OutputTab"));
        gridLayout_13 = new QGridLayout(OutputTab);
        gridLayout_13->setSpacing(6);
        gridLayout_13->setContentsMargins(11, 11, 11, 11);
        gridLayout_13->setObjectName(QStringLiteral("gridLayout_13"));
        plainTextEdit = new OrbitCodeEditor(OutputTab);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

        gridLayout_13->addWidget(plainTextEdit, 0, 0, 1, 1);

        MainTabWidget->addTab(OutputTab, QString());
        VisualizeTab = new QWidget();
        VisualizeTab->setObjectName(QStringLiteral("VisualizeTab"));
        gridLayout_6 = new QGridLayout(VisualizeTab);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        VisualizeGLWidget = new OrbitGLWidget(VisualizeTab);
        VisualizeGLWidget->setObjectName(QStringLiteral("VisualizeGLWidget"));

        gridLayout_6->addWidget(VisualizeGLWidget, 0, 0, 1, 1);

        MainTabWidget->addTab(VisualizeTab, QString());
        WatchTab = new QWidget();
        WatchTab->setObjectName(QStringLiteral("WatchTab"));
        gridLayout_14 = new QGridLayout(WatchTab);
        gridLayout_14->setSpacing(6);
        gridLayout_14->setContentsMargins(11, 11, 11, 11);
        gridLayout_14->setObjectName(QStringLiteral("gridLayout_14"));
        WatchWidget = new OrbitWatchWidget(WatchTab);
        WatchWidget->setObjectName(QStringLiteral("WatchWidget"));

        gridLayout_14->addWidget(WatchWidget, 0, 0, 1, 1);

        MainTabWidget->addTab(WatchTab, QString());
        splitter_2->addWidget(MainTabWidget);
        RightTabWidget = new QTabWidget(splitter_2);
        RightTabWidget->setObjectName(QStringLiteral("RightTabWidget"));
        FunctionsTab = new QWidget();
        FunctionsTab->setObjectName(QStringLiteral("FunctionsTab"));
        gridLayout_2 = new QGridLayout(FunctionsTab);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        FunctionsList = new OrbitDataViewPanel(FunctionsTab);
        FunctionsList->setObjectName(QStringLiteral("FunctionsList"));

        gridLayout_2->addWidget(FunctionsList, 0, 0, 1, 1);

        RightTabWidget->addTab(FunctionsTab, QString());
        TypesTab = new QWidget();
        TypesTab->setObjectName(QStringLiteral("TypesTab"));
        gridLayout_11 = new QGridLayout(TypesTab);
        gridLayout_11->setSpacing(6);
        gridLayout_11->setContentsMargins(11, 11, 11, 11);
        gridLayout_11->setObjectName(QStringLiteral("gridLayout_11"));
        TypesList = new OrbitDataViewPanel(TypesTab);
        TypesList->setObjectName(QStringLiteral("TypesList"));

        gridLayout_11->addWidget(TypesList, 0, 0, 1, 1);

        RightTabWidget->addTab(TypesTab, QString());
        GlobalsType = new QWidget();
        GlobalsType->setObjectName(QStringLiteral("GlobalsType"));
        gridLayout_12 = new QGridLayout(GlobalsType);
        gridLayout_12->setSpacing(6);
        gridLayout_12->setContentsMargins(11, 11, 11, 11);
        gridLayout_12->setObjectName(QStringLiteral("gridLayout_12"));
        GlobalsList = new OrbitDataViewPanel(GlobalsType);
        GlobalsList->setObjectName(QStringLiteral("GlobalsList"));

        gridLayout_12->addWidget(GlobalsList, 0, 1, 1, 1);

        RightTabWidget->addTab(GlobalsType, QString());
        LiveTab = new QWidget();
        LiveTab->setObjectName(QStringLiteral("LiveTab"));
        gridLayout_15 = new QGridLayout(LiveTab);
        gridLayout_15->setSpacing(6);
        gridLayout_15->setContentsMargins(11, 11, 11, 11);
        gridLayout_15->setObjectName(QStringLiteral("gridLayout_15"));
        LiveFunctionsList = new OrbitDataViewPanel(LiveTab);
        LiveFunctionsList->setObjectName(QStringLiteral("LiveFunctionsList"));

        gridLayout_15->addWidget(LiveFunctionsList, 0, 0, 1, 1);

        RightTabWidget->addTab(LiveTab, QString());
        CallStackTab = new QWidget();
        CallStackTab->setObjectName(QStringLiteral("CallStackTab"));
        gridLayout_8 = new QGridLayout(CallStackTab);
        gridLayout_8->setSpacing(6);
        gridLayout_8->setContentsMargins(11, 11, 11, 11);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        CallStackView = new OrbitDataViewPanel(CallStackTab);
        CallStackView->setObjectName(QStringLiteral("CallStackView"));

        gridLayout_8->addWidget(CallStackView, 0, 0, 1, 1);

        RightTabWidget->addTab(CallStackTab, QString());
        CodeTab = new QWidget();
        CodeTab->setObjectName(QStringLiteral("CodeTab"));
        gridLayout_10 = new QGridLayout(CodeTab);
        gridLayout_10->setSpacing(6);
        gridLayout_10->setContentsMargins(11, 11, 11, 11);
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));
        lineEdit = new QLineEdit(CodeTab);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        gridLayout_10->addWidget(lineEdit, 0, 0, 1, 1);

        CodeTextEdit = new OrbitCodeEditor(CodeTab);
        CodeTextEdit->setObjectName(QStringLiteral("CodeTextEdit"));

        gridLayout_10->addWidget(CodeTextEdit, 1, 0, 1, 1);

        FileMappingWidget = new QWidget(CodeTab);
        FileMappingWidget->setObjectName(QStringLiteral("FileMappingWidget"));
        gridLayout_9 = new QGridLayout(FileMappingWidget);
        gridLayout_9->setSpacing(6);
        gridLayout_9->setContentsMargins(11, 11, 11, 11);
        gridLayout_9->setObjectName(QStringLiteral("gridLayout_9"));
        gridLayout_9->setContentsMargins(0, 0, 0, 0);
        SaveFileMapping = new QPushButton(FileMappingWidget);
        SaveFileMapping->setObjectName(QStringLiteral("SaveFileMapping"));

        gridLayout_9->addWidget(SaveFileMapping, 0, 0, 1, 1);

        FileMappingTextEdit = new OrbitCodeEditor(FileMappingWidget);
        FileMappingTextEdit->setObjectName(QStringLiteral("FileMappingTextEdit"));

        gridLayout_9->addWidget(FileMappingTextEdit, 1, 0, 1, 1);


        gridLayout_10->addWidget(FileMappingWidget, 2, 0, 1, 1);

        RightTabWidget->addTab(CodeTab, QString());
        outputTab = new QWidget();
        outputTab->setObjectName(QStringLiteral("outputTab"));
        gridLayout_4 = new QGridLayout(outputTab);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        OutputView = new OrbitDataViewPanel(outputTab);
        OutputView->setObjectName(QStringLiteral("OutputView"));

        gridLayout_4->addWidget(OutputView, 0, 0, 1, 1);

        RightTabWidget->addTab(outputTab, QString());
        splitter_2->addWidget(RightTabWidget);

        gridLayout->addWidget(splitter_2, 0, 0, 1, 1);

        OrbitMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(OrbitMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 976, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuDev = new QMenu(menuBar);
        menuDev->setObjectName(QStringLiteral("menuDev"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        OrbitMainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuDev->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionSave_Capture);
        menuFile->addAction(actionOpen_Capture_2);
        menuFile->addSeparator();
        menuFile->addAction(actionSave_Session);
        menuFile->addAction(actionSave_Session_As);
        menuFile->addAction(actionOpen_Session);
        menuFile->addAction(actionOpen_PDB);
        menuFile->addSeparator();
        menuFile->addAction(actionLaunch_Process);
        menuFile->addAction(actionDisconnect);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuHelp->addAction(actionAbout);
        menuDev->addAction(actionEnable_Sampling);
        menuDev->addAction(actionEnable_Context_Switches);
        menuDev->addAction(actionEnable_Unreal_Support);
        menuDev->addAction(actionAllow_Unsafe_Hooking);
        menuDev->addAction(actionOutputDebugString);
        menuTools->addAction(actionShow_Includes_Util);
        menuTools->addAction(actionDiff);
        menuTools->addAction(actionRule_Editor);

        retranslateUi(OrbitMainWindow);

        MainTabWidget->setCurrentIndex(0);
        RightTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(OrbitMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *OrbitMainWindow)
    {
        OrbitMainWindow->setWindowTitle(QApplication::translate("OrbitMainWindow", "Orbit Profiler", Q_NULLPTR));
        actionSave_Session->setText(QApplication::translate("OrbitMainWindow", "Save Session", Q_NULLPTR));
        actionAbout->setText(QApplication::translate("OrbitMainWindow", "About", Q_NULLPTR));
        actionOpen_Session->setText(QApplication::translate("OrbitMainWindow", "Open Session", Q_NULLPTR));
        actionOpen_PDB->setText(QApplication::translate("OrbitMainWindow", "Open PDB", Q_NULLPTR));
        actionOpen_Capture->setText(QApplication::translate("OrbitMainWindow", "Open Capture", Q_NULLPTR));
        actionDisconnect->setText(QApplication::translate("OrbitMainWindow", "Disconnect", Q_NULLPTR));
        actionStart_Captue->setText(QApplication::translate("OrbitMainWindow", "Start Captue", Q_NULLPTR));
        actionStop_Capture->setText(QApplication::translate("OrbitMainWindow", "Stop Capture", Q_NULLPTR));
        actionQuit->setText(QApplication::translate("OrbitMainWindow", "Quit", Q_NULLPTR));
        actionToogleDevMode->setText(QApplication::translate("OrbitMainWindow", "Dev Mode", Q_NULLPTR));
        actionSave_Session_As->setText(QApplication::translate("OrbitMainWindow", "Save Session As...", Q_NULLPTR));
        actionEnable_Context_Switches->setText(QApplication::translate("OrbitMainWindow", "Context Switches", Q_NULLPTR));
        actionLaunch_Process->setText(QApplication::translate("OrbitMainWindow", "Launch Process", Q_NULLPTR));
        actionEnable_Unreal_Support->setText(QApplication::translate("OrbitMainWindow", "Unreal Support", Q_NULLPTR));
        actionAllow_Unsafe_Hooking->setText(QApplication::translate("OrbitMainWindow", "Unsafe Hooking", Q_NULLPTR));
        actionEnable_Sampling->setText(QApplication::translate("OrbitMainWindow", "Sampling", Q_NULLPTR));
        actionOpen_Capture_2->setText(QApplication::translate("OrbitMainWindow", "Open Capture", Q_NULLPTR));
        actionSave_Capture->setText(QApplication::translate("OrbitMainWindow", "Save Capture", Q_NULLPTR));
        actionShow_Includes_Util->setText(QApplication::translate("OrbitMainWindow", "Show Includes Util", Q_NULLPTR));
        actionDiff->setText(QApplication::translate("OrbitMainWindow", "Diff", Q_NULLPTR));
        actionOutputDebugString->setText(QApplication::translate("OrbitMainWindow", "OutputDebugString", Q_NULLPTR));
        actionRule_Editor->setText(QApplication::translate("OrbitMainWindow", "Rule Editor", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(tab), QApplication::translate("OrbitMainWindow", "home", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(DebugTab), QApplication::translate("OrbitMainWindow", "debug", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(CaptureTab), QApplication::translate("OrbitMainWindow", "capture", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(OutputTab), QApplication::translate("OrbitMainWindow", "output", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(VisualizeTab), QApplication::translate("OrbitMainWindow", "visualize", Q_NULLPTR));
        MainTabWidget->setTabText(MainTabWidget->indexOf(WatchTab), QApplication::translate("OrbitMainWindow", "watch", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(FunctionsTab), QApplication::translate("OrbitMainWindow", "functions", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(TypesTab), QApplication::translate("OrbitMainWindow", "types", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(GlobalsType), QApplication::translate("OrbitMainWindow", "globals", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(LiveTab), QApplication::translate("OrbitMainWindow", "live", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(CallStackTab), QApplication::translate("OrbitMainWindow", "callstack", Q_NULLPTR));
#ifndef QT_NO_ACCESSIBILITY
        CodeTextEdit->setAccessibleName(QApplication::translate("OrbitMainWindow", "CodeTextEdit", Q_NULLPTR));
#endif // QT_NO_ACCESSIBILITY
        SaveFileMapping->setText(QApplication::translate("OrbitMainWindow", "Save File Mapping", Q_NULLPTR));
#ifndef QT_NO_ACCESSIBILITY
        FileMappingTextEdit->setAccessibleName(QApplication::translate("OrbitMainWindow", "FileMappingTextEdit", Q_NULLPTR));
#endif // QT_NO_ACCESSIBILITY
        RightTabWidget->setTabText(RightTabWidget->indexOf(CodeTab), QApplication::translate("OrbitMainWindow", "code", Q_NULLPTR));
        RightTabWidget->setTabText(RightTabWidget->indexOf(outputTab), QApplication::translate("OrbitMainWindow", "debug", Q_NULLPTR));
        menuFile->setTitle(QApplication::translate("OrbitMainWindow", "File", Q_NULLPTR));
        menuHelp->setTitle(QApplication::translate("OrbitMainWindow", "Help", Q_NULLPTR));
        menuDev->setTitle(QApplication::translate("OrbitMainWindow", "Options", Q_NULLPTR));
        menuTools->setTitle(QApplication::translate("OrbitMainWindow", "Tools", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitMainWindow: public Ui_OrbitMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITMAINWINDOW_H
