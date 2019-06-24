/********************************************************************************
** Form generated from reading UI file 'controlgui.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLGUI_H
#define UI_CONTROLGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_ControlGUI
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_9;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_3;
    QPushButton *openFileButton;
    QCheckBox *waitForGoCheckbox;
    QCheckBox *waitForHmGXCheckBox;
    QCheckBox *startMidasCheckbox;
    QCheckBox *fillCheckCheckbox;
    QPushButton *startButton;
    QPushButton *abortButton;
    QCheckBox *stopAfterThisRunCheckBox;
    QVBoxLayout *verticalLayout_8;
    QLabel *label_7;
    QLCDNumber *cyclePeriodLCD;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_6;
    QLCDNumber *elapsedTimeLCD;
    QLabel *label_2;
    QSpinBox *listnumber_spinbox;
    QPushButton *loadfileNumber_button;
    QSpacerItem *verticalSpacer;
    QFrame *frame;
    QVBoxLayout *verticalLayout_5;
    QLabel *label_5;
    QRadioButton *beamEnableRadio;
    QRadioButton *gateValveRadio;
    QRadioButton *cleanerRadio;
    QRadioButton *butterflyRadio;
    QRadioButton *D2PumpOutradioButton;
    QRadioButton *nEDMGVButton;
    QVBoxLayout *verticalLayout_4;
    QLabel *label;
    QComboBox *trapdoorComboBox;
    QVBoxLayout *verticalLayout;
    QLabel *label_8;
    QHBoxLayout *horizontalLayout;
    QPushButton *daggerPushButton;
    QDoubleSpinBox *daggerSpinBox;
    QVBoxLayout *verticalLayout_6;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *giantCleanerPushButton;
    QDoubleSpinBox *giantCleanerSpinBox;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *filenameLineEdit;
    QLineEdit *commentLineEdit;
    QCustomPlot *plot;
    QTextBrowser *textBrowser;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ControlGUI)
    {
        if (ControlGUI->objectName().isEmpty())
            ControlGUI->setObjectName(QStringLiteral("ControlGUI"));
        ControlGUI->resize(709, 937);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ControlGUI->sizePolicy().hasHeightForWidth());
        ControlGUI->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(ControlGUI);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        verticalLayout_9 = new QVBoxLayout();
        verticalLayout_9->setSpacing(6);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        frame_2 = new QFrame(centralWidget);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy1);
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        openFileButton = new QPushButton(frame_2);
        openFileButton->setObjectName(QStringLiteral("openFileButton"));

        verticalLayout_3->addWidget(openFileButton);

        waitForGoCheckbox = new QCheckBox(frame_2);
        waitForGoCheckbox->setObjectName(QStringLiteral("waitForGoCheckbox"));
        waitForGoCheckbox->setEnabled(false);
        waitForGoCheckbox->setChecked(false);

        verticalLayout_3->addWidget(waitForGoCheckbox);

        waitForHmGXCheckBox = new QCheckBox(frame_2);
        waitForHmGXCheckBox->setObjectName(QStringLiteral("waitForHmGXCheckBox"));
        waitForHmGXCheckBox->setEnabled(true);
        waitForHmGXCheckBox->setChecked(true);

        verticalLayout_3->addWidget(waitForHmGXCheckBox);

        startMidasCheckbox = new QCheckBox(frame_2);
        startMidasCheckbox->setObjectName(QStringLiteral("startMidasCheckbox"));
        startMidasCheckbox->setEnabled(true);
        startMidasCheckbox->setChecked(true);

        verticalLayout_3->addWidget(startMidasCheckbox);

        fillCheckCheckbox = new QCheckBox(frame_2);
        fillCheckCheckbox->setObjectName(QStringLiteral("fillCheckCheckbox"));
        fillCheckCheckbox->setChecked(false);

        verticalLayout_3->addWidget(fillCheckCheckbox);

        startButton = new QPushButton(frame_2);
        startButton->setObjectName(QStringLiteral("startButton"));
        startButton->setEnabled(false);

        verticalLayout_3->addWidget(startButton);

        abortButton = new QPushButton(frame_2);
        abortButton->setObjectName(QStringLiteral("abortButton"));
        abortButton->setEnabled(false);

        verticalLayout_3->addWidget(abortButton);

        stopAfterThisRunCheckBox = new QCheckBox(frame_2);
        stopAfterThisRunCheckBox->setObjectName(QStringLiteral("stopAfterThisRunCheckBox"));

        verticalLayout_3->addWidget(stopAfterThisRunCheckBox);


        verticalLayout_9->addWidget(frame_2);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        verticalLayout_8->addWidget(label_7);

        cyclePeriodLCD = new QLCDNumber(centralWidget);
        cyclePeriodLCD->setObjectName(QStringLiteral("cyclePeriodLCD"));
        cyclePeriodLCD->setSegmentStyle(QLCDNumber::Flat);

        verticalLayout_8->addWidget(cyclePeriodLCD);


        verticalLayout_9->addLayout(verticalLayout_8);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        verticalLayout_7->addWidget(label_6);

        elapsedTimeLCD = new QLCDNumber(centralWidget);
        elapsedTimeLCD->setObjectName(QStringLiteral("elapsedTimeLCD"));
        QFont font;
        font.setPointSize(13);
        font.setBold(false);
        font.setWeight(50);
        elapsedTimeLCD->setFont(font);
        elapsedTimeLCD->setStyleSheet(QStringLiteral(""));
        elapsedTimeLCD->setLineWidth(1);
        elapsedTimeLCD->setSmallDecimalPoint(false);
        elapsedTimeLCD->setSegmentStyle(QLCDNumber::Flat);

        verticalLayout_7->addWidget(elapsedTimeLCD);


        verticalLayout_9->addLayout(verticalLayout_7);

        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        verticalLayout_9->addWidget(label_2);

        listnumber_spinbox = new QSpinBox(centralWidget);
        listnumber_spinbox->setObjectName(QStringLiteral("listnumber_spinbox"));

        verticalLayout_9->addWidget(listnumber_spinbox);

        loadfileNumber_button = new QPushButton(centralWidget);
        loadfileNumber_button->setObjectName(QStringLiteral("loadfileNumber_button"));

        verticalLayout_9->addWidget(loadfileNumber_button);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer);

        frame = new QFrame(centralWidget);
        frame->setObjectName(QStringLiteral("frame"));
        sizePolicy1.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy1);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_5 = new QVBoxLayout(frame);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        label_5 = new QLabel(frame);
        label_5->setObjectName(QStringLiteral("label_5"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        label_5->setFont(font1);

        verticalLayout_5->addWidget(label_5);

        beamEnableRadio = new QRadioButton(frame);
        beamEnableRadio->setObjectName(QStringLiteral("beamEnableRadio"));
        beamEnableRadio->setAutoExclusive(false);

        verticalLayout_5->addWidget(beamEnableRadio);

        gateValveRadio = new QRadioButton(frame);
        gateValveRadio->setObjectName(QStringLiteral("gateValveRadio"));
        gateValveRadio->setAutoExclusive(false);

        verticalLayout_5->addWidget(gateValveRadio);

        cleanerRadio = new QRadioButton(frame);
        cleanerRadio->setObjectName(QStringLiteral("cleanerRadio"));
        cleanerRadio->setAutoExclusive(false);

        verticalLayout_5->addWidget(cleanerRadio);

        butterflyRadio = new QRadioButton(frame);
        butterflyRadio->setObjectName(QStringLiteral("butterflyRadio"));
        butterflyRadio->setAutoExclusive(false);

        verticalLayout_5->addWidget(butterflyRadio);

        D2PumpOutradioButton = new QRadioButton(frame);
        D2PumpOutradioButton->setObjectName(QStringLiteral("D2PumpOutradioButton"));

        verticalLayout_5->addWidget(D2PumpOutradioButton);

        nEDMGVButton = new QRadioButton(frame);
        nEDMGVButton->setObjectName(QStringLiteral("nEDMGVButton"));
        nEDMGVButton->setAutoExclusive(false);

        verticalLayout_5->addWidget(nEDMGVButton);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout_4->addWidget(label);

        trapdoorComboBox = new QComboBox(frame);
        trapdoorComboBox->setObjectName(QStringLiteral("trapdoorComboBox"));
        trapdoorComboBox->setEditable(false);

        verticalLayout_4->addWidget(trapdoorComboBox);


        verticalLayout_5->addLayout(verticalLayout_4);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label_8 = new QLabel(frame);
        label_8->setObjectName(QStringLiteral("label_8"));

        verticalLayout->addWidget(label_8);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        daggerPushButton = new QPushButton(frame);
        daggerPushButton->setObjectName(QStringLiteral("daggerPushButton"));

        horizontalLayout->addWidget(daggerPushButton);

        daggerSpinBox = new QDoubleSpinBox(frame);
        daggerSpinBox->setObjectName(QStringLiteral("daggerSpinBox"));
        daggerSpinBox->setDecimals(0);
        daggerSpinBox->setMaximum(1e+06);
        daggerSpinBox->setSingleStep(100);
        daggerSpinBox->setValue(400000);

        horizontalLayout->addWidget(daggerSpinBox);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_5->addLayout(verticalLayout);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        label_9 = new QLabel(frame);
        label_9->setObjectName(QStringLiteral("label_9"));

        verticalLayout_6->addWidget(label_9);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        giantCleanerPushButton = new QPushButton(frame);
        giantCleanerPushButton->setObjectName(QStringLiteral("giantCleanerPushButton"));

        horizontalLayout_2->addWidget(giantCleanerPushButton);

        giantCleanerSpinBox = new QDoubleSpinBox(frame);
        giantCleanerSpinBox->setObjectName(QStringLiteral("giantCleanerSpinBox"));
        giantCleanerSpinBox->setDecimals(0);
        giantCleanerSpinBox->setMaximum(2e+07);
        giantCleanerSpinBox->setSingleStep(10000);
        giantCleanerSpinBox->setValue(400000);

        horizontalLayout_2->addWidget(giantCleanerSpinBox);


        verticalLayout_6->addLayout(horizontalLayout_2);


        verticalLayout_5->addLayout(verticalLayout_6);


        verticalLayout_9->addWidget(frame);


        gridLayout->addLayout(verticalLayout_9, 0, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        filenameLineEdit = new QLineEdit(centralWidget);
        filenameLineEdit->setObjectName(QStringLiteral("filenameLineEdit"));

        verticalLayout_2->addWidget(filenameLineEdit);

        commentLineEdit = new QLineEdit(centralWidget);
        commentLineEdit->setObjectName(QStringLiteral("commentLineEdit"));
        commentLineEdit->setReadOnly(false);

        verticalLayout_2->addWidget(commentLineEdit);

        plot = new QCustomPlot(centralWidget);
        plot->setObjectName(QStringLiteral("plot"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(plot->sizePolicy().hasHeightForWidth());
        plot->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(plot);

        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        sizePolicy2.setHeightForWidth(textBrowser->sizePolicy().hasHeightForWidth());
        textBrowser->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(textBrowser);

        verticalLayout_2->setStretch(2, 1);

        gridLayout->addLayout(verticalLayout_2, 0, 1, 1, 1);

        ControlGUI->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ControlGUI);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 709, 25));
        ControlGUI->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ControlGUI);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ControlGUI->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ControlGUI);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ControlGUI->setStatusBar(statusBar);

        retranslateUi(ControlGUI);

        QMetaObject::connectSlotsByName(ControlGUI);
    } // setupUi

    void retranslateUi(QMainWindow *ControlGUI)
    {
        ControlGUI->setWindowTitle(QApplication::translate("ControlGUI", "ControlGUI", 0));
        openFileButton->setText(QApplication::translate("ControlGUI", "Open Run File", 0));
        waitForGoCheckbox->setText(QApplication::translate("ControlGUI", "Wait for MPA3 Go", 0));
        waitForHmGXCheckBox->setText(QApplication::translate("ControlGUI", "Wait for H-GX", 0));
        startMidasCheckbox->setText(QApplication::translate("ControlGUI", "Start MIDAS", 0));
        fillCheckCheckbox->setText(QApplication::translate("ControlGUI", "Monitor Fills", 0));
        startButton->setText(QApplication::translate("ControlGUI", "Start", 0));
        abortButton->setText(QApplication::translate("ControlGUI", "Abort", 0));
        stopAfterThisRunCheckBox->setText(QApplication::translate("ControlGUI", "Stop after this run", 0));
        label_7->setText(QApplication::translate("ControlGUI", "Cycle Period (s)", 0));
        label_6->setText(QApplication::translate("ControlGUI", "Elapsed Time (s)", 0));
        label_2->setText(QApplication::translate("ControlGUI", "List number", 0));
        loadfileNumber_button->setText(QApplication::translate("ControlGUI", "Load File Number", 0));
        label_5->setText(QApplication::translate("ControlGUI", "Present State", 0));
        beamEnableRadio->setText(QApplication::translate("ControlGUI", "Beam Veto", 0));
        gateValveRadio->setText(QApplication::translate("ControlGUI", "Gate Valve Open", 0));
        cleanerRadio->setText(QApplication::translate("ControlGUI", "Cleaner Down", 0));
        butterflyRadio->setText(QApplication::translate("ControlGUI", "Butterfly Closed", 0));
        D2PumpOutradioButton->setText(QApplication::translate("ControlGUI", "D2 Pump Out Port", 0));
        nEDMGVButton->setText(QApplication::translate("ControlGUI", "Disable nEDM GV", 0));
        label->setText(QApplication::translate("ControlGUI", "Trap Door", 0));
        trapdoorComboBox->clear();
        trapdoorComboBox->insertItems(0, QStringList()
         << QApplication::translate("ControlGUI", "DONT KNOW", 0)
         << QApplication::translate("ControlGUI", "FILL", 0)
         << QApplication::translate("ControlGUI", "HOLD", 0)
         << QApplication::translate("ControlGUI", "DUMP", 0)
         << QApplication::translate("ControlGUI", "UP", 0)
        );
        label_8->setText(QApplication::translate("ControlGUI", "Dagger Position (steps)", 0));
        daggerPushButton->setText(QApplication::translate("ControlGUI", "Set", 0));
        label_9->setText(QApplication::translate("ControlGUI", "Giant Cleaner Position (steps)", 0));
        giantCleanerPushButton->setText(QApplication::translate("ControlGUI", "Set", 0));
    } // retranslateUi

};

namespace Ui {
    class ControlGUI: public Ui_ControlGUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLGUI_H
