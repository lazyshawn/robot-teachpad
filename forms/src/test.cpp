#include <iostream>

#include "fsai_app.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	FSAIApp fsaiApp;

	return app.exec();
}

