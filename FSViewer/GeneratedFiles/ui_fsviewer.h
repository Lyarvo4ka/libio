/********************************************************************************
** Form generated from reading UI file 'FSViewer.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FSVIEWER_H
#define UI_FSVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FSViewerClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTreeView *treeView;
    QWidget *widget1;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QTableView *tableView;
    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FSViewerClass)
    {
        if (FSViewerClass->objectName().isEmpty())
            FSViewerClass->setObjectName(QStringLiteral("FSViewerClass"));
        FSViewerClass->resize(1039, 809);
        centralWidget = new QWidget(FSViewerClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setOrientation(Qt::Horizontal);
        widget = new QWidget(splitter);
        widget->setObjectName(QStringLiteral("widget"));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setMaximumSize(QSize(16777215, 16777215));
        label->setBaseSize(QSize(0, 0));

        verticalLayout->addWidget(label);

        treeView = new QTreeView(widget);
        treeView->setObjectName(QStringLiteral("treeView"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(treeView->sizePolicy().hasHeightForWidth());
        treeView->setSizePolicy(sizePolicy2);
        treeView->setMinimumSize(QSize(100, 0));
        treeView->setMaximumSize(QSize(16777215, 16777215));
        treeView->setBaseSize(QSize(100, 0));
        QFont font;
        font.setPointSize(8);
        treeView->setFont(font);
        treeView->setLineWidth(1);
        treeView->setRootIsDecorated(true);
        treeView->setAnimated(true);
        treeView->header()->setMinimumSectionSize(25);

        verticalLayout->addWidget(treeView);

        splitter->addWidget(widget);
        widget1 = new QWidget(splitter);
        widget1->setObjectName(QStringLiteral("widget1"));
        verticalLayout_2 = new QVBoxLayout(widget1);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(widget1);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy3);

        verticalLayout_2->addWidget(label_2);

        tableView = new QTableView(widget1);
        tableView->setObjectName(QStringLiteral("tableView"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(tableView->sizePolicy().hasHeightForWidth());
        tableView->setSizePolicy(sizePolicy4);
        tableView->setFont(font);
        tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        tableView->setAutoScrollMargin(16);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setShowGrid(true);
        tableView->setGridStyle(Qt::DotLine);
        tableView->setSortingEnabled(true);
        tableView->setCornerButtonEnabled(true);
        tableView->horizontalHeader()->setVisible(true);
        tableView->horizontalHeader()->setMinimumSectionSize(25);
        tableView->verticalHeader()->setVisible(false);
        tableView->verticalHeader()->setDefaultSectionSize(15);
        tableView->verticalHeader()->setHighlightSections(false);

        verticalLayout_2->addWidget(tableView);

        splitter->addWidget(widget1);

        horizontalLayout->addWidget(splitter);

        FSViewerClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(FSViewerClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1039, 20));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        FSViewerClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FSViewerClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        FSViewerClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FSViewerClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        FSViewerClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());

        retranslateUi(FSViewerClass);
        QObject::connect(treeView, SIGNAL(clicked(QModelIndex)), FSViewerClass, SLOT(OnTreeViewClicked(QModelIndex)));
        QObject::connect(tableView, SIGNAL(doubleClicked(QModelIndex)), FSViewerClass, SLOT(DoubleClickedTableView(QModelIndex)));

        QMetaObject::connectSlotsByName(FSViewerClass);
    } // setupUi

    void retranslateUi(QMainWindow *FSViewerClass)
    {
        FSViewerClass->setWindowTitle(QApplication::translate("FSViewerClass", "FSViewer", nullptr));
        label->setText(QApplication::translate("FSViewerClass", "\320\237\320\260\320\277\320\272\320\270", nullptr));
        label_2->setText(QApplication::translate("FSViewerClass", "\320\241\320\276\320\264\320\265\321\200\320\266\320\270\320\274\320\276\320\265", nullptr));
        menu->setTitle(QApplication::translate("FSViewerClass", "\320\244\320\260\320\271\320\273", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FSViewerClass: public Ui_FSViewerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FSVIEWER_H
