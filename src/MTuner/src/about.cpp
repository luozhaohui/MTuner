//--------------------------------------------------------------------------//
/// Copyright 2024 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <MTuner_pch.h>
#include <MTuner/src/about.h>
#include <MTuner/src/version.h>

AboutDialog::AboutDialog(QWidget* _parent, Qt::WindowFlags /*_flags*/)
    : QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    ui.setupUi(this);
    QLabel* version = findChild<QLabel*>("labelVersion");
    version->setText(tr("Version") + QString(" ") + QString(MTunerVersion));
}

void AboutDialog::changeEvent(QEvent* _event)
{
    QDialog::changeEvent(_event);
    if (_event->type() == QEvent::LanguageChange)
        ui.retranslateUi(this);
}
