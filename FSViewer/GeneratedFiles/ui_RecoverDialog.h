/********************************************************************************
** Form generated from reading UI file 'RecoverDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECOVERDIALOG_H
#define UI_RECOVERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_RecoverDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *folderText;
    QHBoxLayout *horizontalLayout;
    QLineEdit *folderPathEdit;
    QPushButton *openButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *RecoverDialog)
    {
        if (RecoverDialog->objectName().isEmpty())
            RecoverDialog->setObjectName(QString::fromUtf8("RecoverDialog"));
        RecoverDialog->setWindowModality(Qt::WindowModal);
        RecoverDialog->resize(611, 148);
        RecoverDialog->setSizeGripEnabled(false);
        RecoverDialog->setModal(true);
        verticalLayout_2 = new QVBoxLayout(RecoverDialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        folderText = new QLabel(RecoverDialog);
        folderText->setObjectName(QString::fromUtf8("folderText"));

        verticalLayout->addWidget(folderText);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        folderPathEdit = new QLineEdit(RecoverDialog);
        folderPathEdit->setObjectName(QString::fromUtf8("folderPathEdit"));

        horizontalLayout->addWidget(folderPathEdit);

        openButton = new QPushButton(RecoverDialog);
        openButton->setObjectName(QString::fromUtf8("openButton"));

        horizontalLayout->addWidget(openButton);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);

        verticalSpacer = new QSpacerItem(20, 36, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(RecoverDialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(RecoverDialog);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        verticalLayout_2->addLayout(hboxLayout);


        retranslateUi(RecoverDialog);
        QObject::connect(okButton, SIGNAL(clicked()), RecoverDialog, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), RecoverDialog, SLOT(reject()));
        QObject::connect(openButton, SIGNAL(clicked()), RecoverDialog, SLOT(OpenFolderDialog()));
        QObject::connect(folderPathEdit, SIGNAL(editingFinished()), RecoverDialog, SLOT(TextChanged()));

        QMetaObject::connectSlotsByName(RecoverDialog);
    } // setupUi

    void retranslateUi(QDialog *RecoverDialog)
    {
        RecoverDialog->setWindowTitle(QApplication::translate("RecoverDialog", "\320\222\320\276\321\201\321\201\321\202\320\260\320\275\320\276\320\262\320\270\321\202\321\214", nullptr));
        folderText->setText(QApplication::translate("RecoverDialog", "\320\222\321\213\320\261\320\265\321\200\320\270\321\202\320\265 \320\277\320\260\320\277\320\272\321\203", nullptr));
        openButton->setText(QApplication::translate("RecoverDialog", "\320\236\321\202\320\272\321\200\321\213\321\202\321\214", nullptr));
        okButton->setText(QApplication::translate("RecoverDialog", "OK", nullptr));
        cancelButton->setText(QApplication::translate("RecoverDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RecoverDialog: public Ui_RecoverDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RECOVERDIALOG_H
