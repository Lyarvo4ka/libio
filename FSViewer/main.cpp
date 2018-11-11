#include "FSViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FSViewer w;
	w.show();
	return a.exec();
}
