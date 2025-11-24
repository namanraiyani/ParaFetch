#include <QApplication>
#include "myform.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ParaFetch");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("ParaFetch");

    // Set Global Font
    QFont font("Inter");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(10);
    app.setFont(font);

    MyForm form;
    form.show();

    return app.exec();
}