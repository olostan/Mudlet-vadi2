#ifndef DLGIRC_H
#define DLGIRC_H

#include <QMainWindow>
#include "ui_irc.h"

#include "irc/include/ircsession.h"

class dlgIRC : public QMainWindow, public Ui::irc_dlg
{
  Q_OBJECT

public:
    dlgIRC();
    Irc::Session* session;

public slots:
    void irc_gotMsg( QString, QString, QString );
    void irc_gotMsg2( QString a, QStringList c );
    void irc_gotMsg3( QString a, uint code, QStringList c );
    void anchorClicked(const QUrl& link);
    void slot_joined(QString, QString);
    void slot_parted(QString, QString, QString);
    void sendMsg();

};

#endif // DLGIRC_H
