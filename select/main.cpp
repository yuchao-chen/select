#include "select.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	widgets::Select w;
	w.show();
	return a.exec();
}
