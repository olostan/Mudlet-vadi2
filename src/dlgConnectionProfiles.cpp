/***************************************************************************
 *   Copyright (C) 2008-2009 by Heiko Koehn                                     *
 *   KoehnHeiko@googlemail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QInputDialog>
#include <QMessageBox>
#include "dlgConnectionProfiles.h"
#include "Host.h"
#include "HostManager.h"
#include "mudlet.h"
#include "XMLimport.h"
#include <QFileDialog>
#include <QPainter>

#define _DEBUG_

dlgConnectionProfiles::dlgConnectionProfiles(QWidget * parent) : QDialog(parent)
{
    setupUi( this );

    // selection mode is important. if this is not set the selection behaviour is
    // undefined. this is an undocumented qt bug, as it only shows on certain OS
    // and certain architectures.

    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );

    QAbstractButton * abort = dialog_buttonbox->button( QDialogButtonBox::Cancel );
    abort->setIcon(QIcon(":/icons/dialog-close.png"));
    QPushButton *connect_button = dialog_buttonbox->addButton(tr("Connect"), QDialogButtonBox::AcceptRole);
    connect_button->setIcon(QIcon(":/icons/dialog-ok-apply.png"));



    //connect( browseProfileHistoryButton, SIGNAL( pressed() ), this, SLOT(slot_chose_history()));
    connect( connect_button, SIGNAL(clicked()), this, SLOT(slot_connectToServer()));
    connect( abort, SIGNAL(clicked()), this, SLOT(slot_cancel()));
    connect( new_profile_button, SIGNAL( clicked() ), this, SLOT( slot_addProfile() ) );
    connect( copy_profile_button, SIGNAL( clicked() ), this, SLOT( slot_copy_profile() ) );
    connect( remove_profile_button, SIGNAL( clicked() ), this, SLOT( slot_deleteProfile() ) );
    connect( profile_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_name(const QString)));
    connect( host_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_url(const QString)));
    connect( port_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_port(const QString)));
    connect( autologin_checkBox, SIGNAL(stateChanged( int )), this, SLOT(slot_update_autologin(int)));
    connect( login_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_login(const QString)));
    connect( character_password_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_pass(const QString)));
    connect( mud_description_textedit, SIGNAL(textChanged()), this, SLOT(slot_update_description()));
    connect( website_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_website(const QString)));
    connect( this, SIGNAL( update() ), this, SLOT( slot_update() ) );
    //connect( profiles_tree_widget, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ), SLOT( slot_item_clicked(QListWidgetItem *) ) );
    connect( profiles_tree_widget, SIGNAL( itemClicked( QListWidgetItem * ) ), this, SLOT( slot_item_clicked( QListWidgetItem * )));
    connect( profiles_tree_widget, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), this, SLOT ( slot_connectToServer() ) );
    //connect( mud_list_treewidget, SIGNAL( itemClicked(QListWidgetItem *, int) ), SLOT( slot_item_clicked(QListWidgetItem *) ) );
    //connect( mud_list_treewidget, SIGNAL( itemDoubleClicked( QListWidgetItem *, int ) ), this, SLOT ( slot_connection_dlg_finnished() ) );

    connect( this, SIGNAL (accept()), this, SLOT (slot_connection_dlg_finnished()));
    connect( this, SIGNAL (finished(int)), this, SLOT (slot_finished(int)));

    notificationArea->hide();
    notificationAreaIconLabelWarning->hide();
    notificationAreaIconLabelError->hide();
    notificationAreaIconLabelInformation->hide();
    notificationAreaMessageBox->hide();

    mRegularPalette.setColor(QPalette::Text,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mRegularPalette.setColor(QPalette::Base,QColor(255,255,255));

    mReadOnlyPalette.setColor(QPalette::Base,QColor(212,212,212));
    mReadOnlyPalette.setColor(QPalette::Text,QColor(0,0,192));
    mReadOnlyPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mReadOnlyPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));

    mOKPalette.setColor(QPalette::Text,QColor(0,0,192));
    mOKPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mOKPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mOKPalette.setColor(QPalette::Base,QColor(235,255,235));

    mErrorPalette.setColor(QPalette::Text,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mErrorPalette.setColor(QPalette::Base,QColor(255,235,235));

    resize( 698, 419 );
    profiles_tree_widget->setViewMode(QListView::IconMode);
    /*profiles_tree_widget->setModelColumn(0);
    profiles_tree_widget->setFlow(QListView::LeftToRight);
    profiles_tree_widget->setWrapping(true);*/

}

void dlgConnectionProfiles::slot_update_description()
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();

    if( pItem )
    {
        QString profile = pItem->text();
        QString desc = mud_description_textedit->toPlainText();
        writeProfileData( profile, "description", desc );
    }
}

void dlgConnectionProfiles::slot_update_website( const QString url )
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text();
        writeProfileData( profile, "website", url );
    }
}

void dlgConnectionProfiles::slot_update_pass( const QString pass )
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text();
        writeProfileData( profile, "password", pass );
    }
}

void dlgConnectionProfiles::slot_update_login( const QString login )
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text();
        writeProfileData( profile, "login", login );
    }
}

void dlgConnectionProfiles::slot_update_url( const QString url )
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();

    if( pItem )
    {
        QString profile = pItem->text();
        QUrl check;
        check.setHost( url );
        if( check.isValid() )
        {
            host_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "url", url );
        }
        else
        {
            host_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText( QString("Please enter the URL or IP address of the MUD server.\n\n")+check.errorString() );
        }
    }
}

void dlgConnectionProfiles::slot_update_autologin( int state )
{
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( ! pItem )
        return;
    QString profile = pItem->text();
    writeProfileData( profile, "autologin", QString::number( state ) );
}

void dlgConnectionProfiles::slot_update_port( const QString port )
{
    const QString zahlen = "0123456789";
    if( ! zahlen.contains( port.right( 1 ) ) )
    {
        QString val = port;
        val.chop( 1 );
        port_entry->setText( val );
        notificationArea->show();
        notificationAreaIconLabelWarning->show();
        notificationAreaIconLabelError->hide();
        notificationAreaIconLabelInformation->hide();
        notificationAreaMessageBox->show();
        notificationAreaMessageBox->setText( tr("You have to enter a number. Other characters are not permitted.") );
        return;
    }
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();

    if( pItem )
    {
        QString profile = pItem->text();
        int num = port.trimmed().toInt();
        if( num < 65536 )
        {
            port_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "port", port );
        }
        else
        {
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            port_entry->setPalette( mErrorPalette );
        }
    }
}

void dlgConnectionProfiles::slot_update_name( const QString _n )
{
    QString _ignore = _n;
    QListWidgetItem * pItem = profiles_tree_widget->currentItem();

    QString name = profile_name_entry->text();

    const QString allowedChars = " _0123456789-#aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
    bool __error = false;
    for( int __i=0; __i<name.size(); __i++ )
    {
        if( ! allowedChars.contains( name[__i] ) )
        {
            name.replace( name[__i], "" );
            __i=-1;
            __error = true;
        }
    }
    if( __error )
    {
        profile_name_entry->setText( name );
        notificationArea->show();
        notificationAreaIconLabelWarning->show();
        notificationAreaIconLabelError->hide();
        notificationAreaIconLabelInformation->hide();
        notificationAreaMessageBox->show();
        notificationAreaMessageBox->setText( tr("This character is not permitted. Use one of the following: %1\n").arg(allowedChars) );
        return;
    }

    /*const QString allowedChars = " _0123456789-#aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
    if( ! allowedChars.contains( name.right( 1 ) ) )
    {
        QString val = name;
        val.chop( 1 );
        profile_name_entry->setText( val );
        notificationArea->show();
        notificationAreaIconLabelWarning->show();
        notificationAreaIconLabelError->hide();
        notificationAreaIconLabelInformation->hide();
        notificationAreaMessageBox->show();
        notificationAreaMessageBox->setText( tr("This character is not permitted. Use one of the following: %1\n").arg(allowedChars) );
        return;
    }*/

    if( pItem )
    {
        if( ! mProfileList.contains( name ) )
            mEditOK = true;
        else
            mEditOK = false;


        if( ! mSavedNewName )
        {
            // keep track of the new profile name that isnt yet valid
            // and thus hasnt been written to disc yet
            mUnsavedProfileName = name;
            pItem->setText( name );
        }
        else
        {
            mCurrentProfileEditName = pItem->text();
            int row = mProfileList.indexOf( mCurrentProfileEditName );
            if( ( row >= 0 ) && ( row < mProfileList.size() ) )
            {
                mProfileList[row] = name;
                pItem->setText( name );
            }
        }

        if( mEditOK )
        {

            if( name.size() < 1 || name == " " ) return;
            QDir dir(QDir::homePath()+"/.config/mudlet/profiles");
            if( ! mSavedNewName )
            {
                dir.mkpath( QDir::homePath()+"/.config/mudlet/profiles/"+mUnsavedProfileName );
                mProfileList << name;
                pItem->setText( name );

                if( ( ! mOrigin.isEmpty() ) && ( ! mSavedNewName ) )
                {
                    // special case of a new profile that has to be cloned from
                    // an existing profile
                    QString what = QDir::homePath()+"/.config/mudlet/profiles/"+mOrigin;
                    QString where = QDir::homePath()+"/.config/mudlet/profiles/"+mUnsavedProfileName;
                    QDir dirFrom = QDir( what );
                    QStringList entries = dirFrom.entryList( QDir::Files );

                    for( int i=0; i<entries.size(); i++ )
                    {
                        QFile file( where + mOrigin+"/"+entries[i] );
                        file.copy( where );
                    }
                    mOrigin.clear();

                }
                mSavedNewName = true;
                mUnsavedProfileName = "";
            }
            else
            {
                dir.rename( mCurrentProfileEditName, name );
            }

            profile_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
        }
        else
        {
            profile_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText(tr("A profile with the current name already exists. Please use a longer name or a different name."));
        }
    }
}


void dlgConnectionProfiles::slot_addProfile()
{
    fillout_form();
    welcome_message->hide();

    requiredArea->show();
    informationalArea->show();
    optionalArea->show();

    QStringList newname;
    mUnsavedProfileName = tr("new profile name");
    //newname << mUnsavedProfileName;

    QListWidgetItem * pItem = new QListWidgetItem( mUnsavedProfileName);
    if( ! pItem )
    {
        return;
    }
    mSavedNewName = false;

    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    profiles_tree_widget->addItem( pItem );

    // insert newest entry on top of the list as the general sorting
    // is always newest item first -> fillout->form() filters
    // this is more practical for the user as they use the same profile most of the time

    profiles_tree_widget->setItemSelected(profiles_tree_widget->currentItem(), false); // Unselect previous item
    profiles_tree_widget->setCurrentItem( pItem );
    profiles_tree_widget->setItemSelected( pItem, true );

    profile_name_entry->setText( mUnsavedProfileName );
    profile_name_entry->setFocus();
    profile_name_entry->selectAll();
    profile_name_entry->setReadOnly( false );
    host_name_entry->setReadOnly( false );
    port_entry->setReadOnly( false );

}

// if the user has chosen to connect to an already loaded profile
// the old profile will be copied and he has to give a new name for it
void dlgConnectionProfiles::copy_profile( QString oldProfile )
{

    if( ! mSavedNewName )
    {
        mOrigin = oldProfile; // remember the original profile
    }

    profile_name_entry->setPalette( mErrorPalette );
    notificationArea->show();
    notificationAreaIconLabelWarning->show();
    notificationAreaIconLabelError->hide();
    notificationAreaIconLabelInformation->hide();
    notificationAreaMessageBox->show();
    notificationAreaMessageBox->setText(tr("A profile with the current name has already been loaded. The profile you have chosen will be copied, but you have to find a new name for it. Then press on connect again."));

    QStringList newname;
    mUnsavedProfileName = oldProfile;

    QListWidgetItem * pItem_old = profiles_tree_widget->currentItem();

    if( ! pItem_old ) return;

#ifdef _DEBUG_
    QString oldName = pItem_old->text();
    Q_ASSERT( oldProfile == oldName );
#endif

    slot_item_clicked( pItem_old ); //fillout the form with the data of the original profile

    QString oldUrl = host_name_entry->text();
    QString oldPort = port_entry->text();
    QString oldPassword = character_password_entry->text();
    QString oldLogin = login_entry->text();
    QString oldWebsite = website_entry->text();
    QString oldDescription = mud_description_textedit->toPlainText();

    if (mUnsavedProfileName[mUnsavedProfileName.size()-1].isDigit())
    {
        int i=1;
        do {
            mUnsavedProfileName = mUnsavedProfileName.left(mUnsavedProfileName.size()-1) + QString::number(mUnsavedProfileName[mUnsavedProfileName.size()-1].digitValue() + i++);
        } while (mProfileList.contains(mUnsavedProfileName));
    } else {
        int i=1;
        QString mUnsavedProfileName2;
        do {
            mUnsavedProfileName2 = mUnsavedProfileName + '_' + QString::number(i++);
        } while (mProfileList.contains(mUnsavedProfileName2));
        mUnsavedProfileName = mUnsavedProfileName2;
    }

    newname << mUnsavedProfileName;

    QListWidgetItem * pItem = new QListWidgetItem( mUnsavedProfileName );
    if( ! pItem )
    {
        return;
    }
    mSavedNewName = false;

    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    profiles_tree_widget->addItem( pItem );

    // fill out the form entries with the values of the original profile
    profiles_tree_widget->setCurrentItem( pItem );
    slot_update_name( mUnsavedProfileName );
    //profile_name_entry->setText( mUnsavedProfileName );

    host_name_entry->setText( oldUrl );
    slot_update_url( oldUrl );

    port_entry->setText( oldPort );
    slot_update_port( oldPort );

    character_password_entry->setText( oldPassword );
    slot_update_pass( oldPassword );

    login_entry->setText( oldLogin );
    slot_update_login( oldLogin );

    website_entry->setText( oldWebsite );
    slot_update_website( oldWebsite );

    mud_description_textedit->clear();
    mud_description_textedit->insertPlainText( oldDescription );
    slot_update_description();


    // insert newest entry on top of the list as the general sorting
    // is always newest item first -> fillout->form() filters
    // this is more practical for the user as they use the same profile most of the time

    //profiles_tree_widget->setItemSelected(profiles_tree_widget->currentItem(), false); // Unselect previous item

    profiles_tree_widget->setItemSelected( pItem, true );

    profile_name_entry->setReadOnly( false );
    host_name_entry->setReadOnly( false );
    port_entry->setReadOnly( false );

    profile_name_entry->setFocusPolicy( Qt::StrongFocus );
    host_name_entry->setFocusPolicy( Qt::StrongFocus );
    port_entry->setFocusPolicy( Qt::StrongFocus );

    profile_name_entry->setPalette( mErrorPalette );
    host_name_entry->setPalette( mRegularPalette );
    port_entry->setPalette( mRegularPalette );
    profile_name_entry->setFocus();
    profile_name_entry->selectAll();

}

void dlgConnectionProfiles::deleteAllFiles( QString path )
{
    QStringList filters;
    QDir dir( path );
    QStringList deleteList = dir.entryList();
    for( int i=0; i<deleteList.size(); i++ )
    {
        dir.remove( deleteList[i] );
        qDebug()<<"removing:"<<deleteList[i];
    }
    dir.rmpath( dir.path());
}

void dlgConnectionProfiles::deleteDirectory( QString path )
{
    qDebug()<<"deleteDirectory path="<<path;
    QStringList filters;
    QDir dir( path );
    QStringList deleteList;
    deleteList = dir.entryList( filters, QDir::Dirs | QDir::NoDotAndDotDot );

    for( int i=0; i<deleteList.size(); i++ )
    {
        deleteAllFiles( path + QString("/") + deleteList[i] );
        dir.remove( deleteList[i] );
        qDebug()<<"removing:"<<deleteList[i];
    }
    deleteList = dir.entryList();
    for( int i=0; i<deleteList.size(); i++ )
    {
        deleteAllFiles( path + QString("/") + deleteList[i] );
        dir.remove( deleteList[i] );
        qDebug()<<"removing:"<<deleteList[i];
    }

    dir.rmpath( dir.path());
}

void dlgConnectionProfiles::slot_deleteProfile()
{
    if( ! profiles_tree_widget->currentItem() )
        return;

    QString profile = profiles_tree_widget->currentItem()->text();
    if( QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to delete %1 ?").arg( profile ), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) != QMessageBox::Yes ) return;

    profiles_tree_widget->takeItem( profiles_tree_widget->currentIndex().row() );
    QDir dir( QDir::homePath()+"/.config/mudlet/profiles/"+profile );
    deleteDirectory( dir.path() );
    dir.rmpath( dir.path());

    if( ! mProfileList.size() )
    {
        welcome_message->show();
    }
    fillout_form();
    profiles_tree_widget->setFocus();
}

QString dlgConnectionProfiles::readProfileData( QString profile, QString item )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::ReadOnly );
    QDataStream ifs( & file );
    QString ret;
    ifs >> ret;
    file.close();
    return ret;
}

QStringList dlgConnectionProfiles::readProfileHistory( QString profile, QString item )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::ReadOnly );
    QDataStream ifs( & file );
    QString ret;
    QStringList historyList;
    while( ifs.status() == QDataStream::Ok )
    {
        ifs >> ret;
        historyList << ret;
    }
    file.close();
    return historyList;
}

void dlgConnectionProfiles::writeProfileData( QString profile, QString item, QString what )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::WriteOnly | QIODevice::Unbuffered );
    QDataStream ofs( & file );
    ofs << what;
    file.close();
}

void dlgConnectionProfiles::slot_item_clicked(QListWidgetItem *pItem)
{
    if( pItem )
    {
        QString profile_name = pItem->text();
        QStringList loadedProfiles = HostManager::self()->getHostList();
        if( loadedProfiles.contains( profile_name ) )
        {
            profile_name_entry->setReadOnly( true );
            host_name_entry->setReadOnly( true );
            port_entry->setReadOnly( true );

            profile_name_entry->setFocusPolicy( Qt::NoFocus );
            host_name_entry->setFocusPolicy( Qt::NoFocus );
            port_entry->setFocusPolicy( Qt::NoFocus );

            profile_name_entry->setPalette( mReadOnlyPalette );
            host_name_entry->setPalette( mReadOnlyPalette );
            port_entry->setPalette( mReadOnlyPalette );

            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->show();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText(tr("This profile is currently loaded. You cant change all parameters on loaded profiles. Disconnect the profile and then do the changes."));
        }
        else
        {
            if( profile_name == "new profile name" )
                profile_name_entry->setReadOnly( false );
            else
                profile_name_entry->setReadOnly( true );   //profile name changing disabled until we have a better login dialog
            host_name_entry->setReadOnly( false );
            port_entry->setReadOnly( false );

            profile_name_entry->setFocusPolicy( Qt::StrongFocus );
            host_name_entry->setFocusPolicy( Qt::StrongFocus );
            port_entry->setFocusPolicy( Qt::StrongFocus );

            profile_name_entry->setPalette( mRegularPalette );
            host_name_entry->setPalette( mRegularPalette );
            port_entry->setPalette( mRegularPalette );

            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            notificationAreaMessageBox->setText(tr(""));

        }

        profile_name_entry->setText( profile_name );

        QString profile = profile_name;

        QString item = "url";
        QString val = readProfileData( profile, item );
        if( val.size() < 1 )
        {
            if( profile_name == "Avalon.de" )
                val = "avalon.mud.de";
            if( profile_name == "God Wars II" )
                val = "godwars2.org";
            if( profile_name == "BatMUD" )
                val = "batmud.bat.org";
            if( profile_name == "Aardwolf" )
                val = "aardmud.org";
            if( profile_name == "Achaea" )
                val = "achaea.com";
            if( profile_name == "Aetolia" )
                val = "aetolia.com";
            if( profile_name == "Midkemia" )
                val = "midkemiaonline.com";
            if( profile_name == "Lusternia" )
                val = "lusternia.com";
            if( profile_name == "Imperian" )
                val = "imperian.com";
            if( profile_name == "Realms of Despair" )
                val = "realmsofdespair.com";
            if( profile_name == "ZombieMUD" )
                val = "zombiemud.org";
        }
        host_name_entry->setText( val );
        item = "port";
        val = readProfileData( profile, item );
        if( val.size() < 1 )
        {
            if( profile_name == "Avalon.de" )
                val = "23";
            if( profile_name == "God Wars II" )
                val = "3000";
            if( profile_name == "BatMUD" )
                val = "23";
            if( profile_name == "Aardwolf" )
                val = "4000";
            if( profile_name == "Achaea" )
                val = "23";
            if( profile_name == "Aetolia" )
                val = "23";
            if( profile_name == "Midkemia" )
                val = "23";
            if( profile_name == "Lusternia" )
                val = "23";
            if( profile_name == "Imperian" )
                val = "23";
            if( profile_name == "Realms of Despair" )
                val = "4000";
            if( profile_name == "ZombieMUD" )
                val = "23";
        }
        port_entry->setText( val );
        item = "password";
        val = readProfileData( profile, item );
        character_password_entry->setText( val );
        item = "login";
        val = readProfileData( profile, item );
        login_entry->setText( val );
        item = "autologin";
        val = readProfileData( profile, item );
        if( val.toInt() == Qt::Checked )
        {
            autologin_checkBox->setChecked( true );
        }
        else
        {
            autologin_checkBox->setChecked( false );
        }
        item = "description";
        if( profile_name == "Realms of Despair" )
            val = "The Realms of Despair is the original SMAUG MUD and is FREE to play. We have an active Roleplaying community, an active player-killing (deadly) community, and a very active peaceful community. Players can choose from 13 classes (including a deadly-only class) and 13 races. Character appearances are customizable on creation and we have a vast collection of equipment that is level, gender, class, race and alignment specific. We boast well over 150 original, exclusive areas, with a total of over 20,000 rooms. Mob killing, or 'running' is one of our most popular activities, with monster difficulties varying from easy one-player kills to difficult group kills. We have four deadly-only Clans, twelve peaceful-only Guilds, eight Orders, and fourteen Role-playing Nations that players can join to interact more closely with other players. We have two mortal councils that actively work toward helping players: The Symposium hears ideas for changes, and the Newbie Council assists new players. Our team of Immortals are always willing to answer questions and to help out however necessary. Best of all, playing the Realms of Despair is totally FREE!";
        else if( profile_name == "ZombieMUD" )
            val = "Since 1994, ZombieMUD has been on-line and bringing orc-butchering fun to the masses from our home base in Oulu, Finland. We're a pretty friendly bunch, with players logging in from all over the globe to test their skill in our medieval role-playing environment. With 15 separate guilds and 41 races to choose from, as a player the only limitation to your achievements on the game is your own imagination and will to succeed.";
        else if( profile_name == "God Wars II" )
            val = "God Wars II is a fast and furious combat mud, designed to test player skill in terms of pre-battle preparation and on-the-spot reflexes, as well as the ability to adapt quickly to new situations. Take on the role of a godlike supernatural being in a fight for supremacy.\n\nRoomless world. Manual combat. Endless possibilities.";
        else
            val = readProfileData( profile, item );
        mud_description_textedit->clear();
        mud_description_textedit->insertPlainText( val );
        item = "website";
        val = readProfileData( profile, item );
        if( val.size() < 1 )
        {
            if( profile_name == "Avalon.de" )
                val = "<center><a href='http://avalon.mud.de'>http://avalon.mud.de</a></center>";
            if( profile_name == "God Wars II" )
                val = "<center><a href='http://www.godwars2.org'>http://www.godwars2.org</a></center>";
            if( profile_name == "BatMUD" )
                val = val = "<center><a href='http://www.bat.org'>http://www.bat.org</a></center>";
            if( profile_name == "Aardwolf" )
                val = "<center><a href='http://www.aardwolf.com/'>http://www.aardwolf.com</a></center>";;
            if( profile_name == "Achaea" )
                val = "<center><a href='http://www.achaea.com/'>http://www.achaea.com</a></center>";
            if( profile_name == "Realms of Despair" )
                val = "<center><a href='http://www.realmsofdespair.com/'>http://www.realmsofdespair.com</a></center>";
            if( profile_name == "ZombieMUD" )
                val = "<center><a href='http://www.zombiemud.org/'>http://www.zombiemud.org</a></center>";
            if( profile_name == "Aetolia" )
                val = "<center><a href='http://www.aetolia.com/'>http://www.aetolia.com</a></center>";;
            if( profile_name == "Midkemia" )
                val = "<center><a href='http://www.midkemiaonline.com/'>http://www.midkemiaonline.com</a></center>";;
            if( profile_name == "Lusternia" )
                val = "<center><a href='http://www.lusternia.com/'>http://www.lusternia.com</a></center>";;
            if( profile_name == "Imperian" )
                val = "<center><a href='http://www.imperian.com/'>http://www.imperian.com</a></center>";;
        }
        website_entry->setText( val );

        profile_history->clear();
        //item = "history_version";
        //QStringList historyList;
        /*historyList = readProfileHistory( profile, item );
        QStringList versionList;
        versionList << "Newest Profile";
        for( int i=historyList.size()-1; i>-1; i-- )
        {
            versionList << historyList[i];
        }
        versionList << "Oldest Profile";*/
        QString folder = QDir::homePath()+"/.config/mudlet/profiles/"+profile_name+"/current/";
        QDir dir( folder );
        dir.setSorting(QDir::Time);
        QStringList entries = dir.entryList( QDir::Files, QDir::Time );

        profile_history->insertItems( 1, entries );

    }
}

void dlgConnectionProfiles::fillout_form()
{
    profiles_tree_widget->clear();
    profile_name_entry->clear();
    host_name_entry->clear();
    port_entry->clear();

    mProfileList = QDir(QDir::homePath()+"/.config/mudlet/profiles").entryList(QDir::Dirs, QDir::Name);

    if( mProfileList.size() < 3 )
    {
        welcome_message->show();
        requiredArea->hide();
        informationalArea->hide();
        optionalArea->hide();
        resize( 698, 419 );
    }
    else
    {
        welcome_message->hide();

        requiredArea->show();
        informationalArea->show();
        optionalArea->show();
    }


    /*QStringList headerList;
    headerList << "Game" << "MUD name" << "Language" << "Location";*/

    //profiles_tree_widget->setHeaderLabels( headerList );*/

   /* profiles_tree_widget->setColumnWidth( 0, 130 );
    profiles_tree_widget->setColumnWidth( 1, 70 );
    profiles_tree_widget->setColumnWidth( 2, 65 );
    profiles_tree_widget->setColumnWidth( 3, 65 );
    profiles_tree_widget->setColumnCount( 4 );*/
    profiles_tree_widget->setIconSize(QSize(120,30));
    //connect( mudList, SIGNAL(itemClicked ( QListWidgetItem *, int )), this, SLOT(slot_mud_clicked()));
    //connect( mudList, SIGNAL(itemDoubleClicked ( QListWidgetItem *, int )), this, SLOT(slot_mud_connectToServer()));
    QFont font("Bitstream Vera Sans Mono", 1 );//mDisplayFont( QFont("Monospace", 10, QFont::Courier ) )
    QString muds;
    muds = "Avalon.de";
    QListWidgetItem * pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    QPixmap p(":/icons/avalon.png");
    QIcon mi( p.scaled(QSize(120,30)) );
    pM->setIcon(mi);
    muds.clear();

    muds = "God Wars II";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/gw2.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "Achaea";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/achaea_120_30.png" );
    pM->setIcon(mi);
    muds.clear();



    QString muds3;
    muds3 = "Aardwolf";
    QListWidgetItem * pM3 = new QListWidgetItem( muds3 );
    pM3->setFont(font);
    pM3->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM3 );
    QPixmap pa(":/icons/aardwolf_mud.png");
    QPixmap pa1 = pa.scaled(QSize(120,30)).copy();
    QIcon mi3( pa1 );
    pM3->setIcon(mi3);
    muds.clear();

    muds = "Realms of Despair";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/120x30RoDLogo.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "ZombieMUD";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/zombiemud.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "Aetolia";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/aetolia_120_30.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "Lusternia";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/lusternia_120_30.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "Imperian";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/imperian_120_30.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "Midkemia";
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    mi = QIcon( ":/icons/midkemia_120_30.png" );
    pM->setIcon(mi);
    muds.clear();

    muds = "BatMUD";
    QPixmap pb(":/icons/batmud_mud.png");
    QPixmap pb1 = pb.scaled(QSize(120,30)).copy();
    mi = QIcon( pb1 );
    pM = new QListWidgetItem( muds );
    pM->setFont(font);
    pM->setForeground(QColor(255,255,255));
    profiles_tree_widget->addItem( pM );
    pM->setIcon(mi);
    muds.clear();

    QDateTime test_date;
    QListWidgetItem * toselect = 0;
    for( int i=0; i<mProfileList.size(); i++ )
    {
        QString s = mProfileList[i];
        if( s.size() < 1 )
            continue;
        if( (mProfileList[i] == ".") || (mProfileList[i] == ".." ) )
            continue;

        if( mProfileList[i] == "Avalon.de" )
            continue;
        if( mProfileList[i] == "BatMUD" )
            continue;
        if( mProfileList[i] == "Aardwolf" )
            continue;
        if( mProfileList[i] == "Achaea" )
            continue;
        if( mProfileList[i] == "Aetolia" )
            continue;
        if( mProfileList[i] == "Midkemia" )
            continue;
        if( mProfileList[i] == "Lusternia" )
            continue;
        if( mProfileList[i] == "Imperian" )
            continue;
        if( mProfileList[i] == "Realms of Despair" )
            continue;
        if( mProfileList[i] == "ZombieMUD" )
            continue;
        QString sList;
        sList = mProfileList[i];
        QListWidgetItem * pItem = new QListWidgetItem( sList );
        pItem->setFont(font);
        pItem->setForeground(QColor(255,255,255,255));
        profiles_tree_widget->addItem( pItem );
        QPixmap pb( 120, 30 );
        pb.fill(QColor(0,0,0,0));
        int hash = qHash( sList );
        QLinearGradient shade(0, 0, 120, 30);
        int i1 = hash%255;
        int i2 = (hash+i)%255;
        int i3 = (abs(i*hash))%255;
        int i4 = (3*hash)%255;
        int i5 = (hash)%255;
        int i6 = abs((hash/i))%255;
        shade.setColorAt( 1, QColor(i1, i2, i3,255) );
        shade.setColorAt( 0, QColor(i4, i5, i6,255) );
        QBrush br( shade );
        QPainter pt(&pb);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pt.fillRect(QRect(0,0,120,30), shade);
        QPixmap pg( ":/icons/mudlet_main_32px.png");
        pt.drawPixmap( QRect(5,5, 20, 20 ), pg );

        QFont _font;
        QImage _pm( 90, 30, QImage::Format_ARGB32_Premultiplied	);
        QPainter _pt( &_pm );
        _pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        int fs=30;
        for( ; fs>1; fs-- )
        {
            _pt.eraseRect( QRect( 0, 0, 90, 30 ) );
            _pt.fillRect(QRect(0,0,90,30), QColor(255,0,0,10));
            _font = QFont("DejaVu Sans", fs, QFont::Helvetica);
            _pt.setFont( _font );
            QRect _r;
            if( (i1+i2+i3+i4+i5+i6)/6 < 100 )
                _pt.setPen( QColor(255,255,255,255) );
            else
                _pt.setPen( QColor(0,0,0,255));
            _pt.drawText(QRect(0,0, 90, 30), Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap, s, &_r );
            /*if( QFontMetrics( _font ).boundingRect( s ).width() <= 80
            && QFontMetrics( _font ).boundingRect( s ).height() <= 30 )*/
            if( _r.width() <= 90 && _r.height() <= 30 )
            {
                break;
            }

        }
        pt.setFont( _font );
        QRect _r;
        if( (i1+i2+i3+i4+i5+i6)/6 < 100 )
            pt.setPen( QColor(255,255,255,255) );
        else
            pt.setPen( QColor(0,0,0,255));
        pt.drawText( QRect(30,0, 90, 30), Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap, s, &_r );
        mi = QIcon( pb );
        pItem->setIcon( mi );
        QDateTime profile_lastRead = QFileInfo(QDir::homePath()+"/.config/mudlet/profiles/"+mProfileList[i]+"/Host.dat").lastRead();
        if (profile_lastRead > test_date)
        {
            test_date = profile_lastRead;
            toselect = pItem;
        }
    }

    if( toselect )
        profiles_tree_widget->setCurrentItem( toselect );
}

void dlgConnectionProfiles::slot_connection_dlg_finnished()
{
}




void dlgConnectionProfiles::slot_cancel()
{
    QDialog::done( 0 );
}

void dlgConnectionProfiles::slot_copy_profile()
{
    QString profile_name = profile_name_entry->text().trimmed();

    if( profile_name.size() < 1 )
        return;

    // copy profile as the same profile is already loaded
    // show information that he has to select a new name for this copy
    copy_profile( profile_name );
    return;
}

void dlgConnectionProfiles::slot_connectToServer()
{
    QString profile_name = profile_name_entry->text().trimmed();

    if( profile_name.size() < 1 )
        return;

    Host * pOH = HostManager::self()->getHost( profile_name );
    if( pOH )
    {
        pOH->mTelnet.connectIt( pOH->getUrl(), pOH->getPort() );
        QDialog::accept();
        return;
    }
    // load an old profile if there is any
    HostManager::self()->addHost( profile_name, port_entry->text().trimmed(), "", "" );
    Host * pHost = HostManager::self()->getHost( profile_name );

    if( ! pHost ) return;

    QString folder = QDir::homePath()+"/.config/mudlet/profiles/"+profile_name+"/current/";
    QDir dir( folder );
    dir.setSorting(QDir::Time);
    QStringList entries = dir.entryList( QDir::Files, QDir::Time );
    for( int i=0;i<entries.size(); i++ )
        qDebug()<<i<<"#"<<entries[i];
    if( entries.size() > 0 )
    {
        QFile file(folder+"/"+profile_history->currentText());   //entries[0]);
        file.open(QFile::ReadOnly | QFile::Text);
        XMLimport importer( pHost );
        qDebug()<<"[LOADING PROFILE]:"<<file.fileName();
        importer.importPackage( & file );
    }

    // overwrite the generic profile with user supplied name, url and login information
    if( pHost )
    {
        pHost->setName( profile_name );

        if( host_name_entry->text().trimmed().size() > 0 )
            pHost->setUrl( host_name_entry->text().trimmed() );
        else
            slot_update_url( pHost->getUrl() );

        if( port_entry->text().trimmed().size() > 0 )
            pHost->setPort( port_entry->text().trimmed().toInt() );
        else
            slot_update_port( QString::number( pHost->getPort() ) );

        if( character_password_entry->text().trimmed().size() > 0 )
            pHost->setPass( character_password_entry->text().trimmed() );
        else
            slot_update_pass( pHost->getPass() );

        if( login_entry->text().trimmed().size() > 0 )
            pHost->setLogin( login_entry->text().trimmed() );
        else
            slot_update_login( pHost->getLogin() );
    }

    emit signal_establish_connection( profile_name, 0 );
    QDialog::accept();
}

void dlgConnectionProfiles::slot_chose_history()
{
    QString profile_name = profile_name_entry->text().trimmed();
    if( profile_name.size() < 1 )
    {
        QMessageBox::warning(this, tr("Browse Profile History:"),
                             tr("You have not selected a profile yet.\nWhich profile history do you want to browse?\nPlease select a profile first."));
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Chose Mudlet Profile"),
        QDir::homePath()+"/.config/mudlet/profiles/"+profile_name,
        tr("*.xml"));

    if( fileName.isEmpty() ) return;

    QFile file(fileName);
    if( ! file.open(QFile::ReadOnly | QFile::Text) )
    {
        QMessageBox::warning(this, tr("Import Mudlet Package:"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    HostManager::self()->addHost( profile_name, port_entry->text().trimmed(), "", "" );
    Host * pHost = HostManager::self()->getHost( profile_name );
    if( ! pHost ) return;
    XMLimport importer( pHost );
    importer.importPackage( & file );

    emit signal_establish_connection( profile_name, -1 );
    QDialog::accept();
}

void dlgConnectionProfiles::slot_update()
{
    update();
}

