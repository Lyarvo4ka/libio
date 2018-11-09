#include "fsviewer.h"
#include <QtGui/QApplication>
#include <QtCore>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FSViewer w;
	w.show();

	QTextCodec *codec = QTextCodec::codecForName("CP1251");
	QTextCodec::setCodecForTr(codec);
	return a.exec();
}
