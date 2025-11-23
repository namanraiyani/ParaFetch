#include <QApplication>
#include "myform.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ParaFetch");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("ParaFetch");

    MyForm form;
    form.show();

    return app.exec();
}