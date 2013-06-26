//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The source code for this application was originally written for the Mac platform
// It used QT which is not the most friendly environment to work on.
// The port to Windows has been done so that the application can be developed 
// on the well known visual Studio platform. However in order to maintain the original spirit of the app, QT has been retained as 
// the development language.
// The updated Box API has been used to implement OAuth2 and this is a feature which was done from scratch on Windows.
// The application has a lot of scope for improvement.
// If you find this app useful please leave me a note at http://www.samvit.com.au
//
//
// Free to modify and distribute
// May 2013
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "applicationex.h"
#include "preferenceswidget.h"
#include <QSettings>
#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
    ApplicationEx app(argc, argv);

    app.setApplicationName("SimpleShare");
    app.setOrganizationName("BoxNet");
    //app.setOrganizationDomain("box.net");
    app.setApplicationVersion("1.0.42");

    app.enableLogging();

    qDebug() << qApp->applicationName() << " " << qApp->applicationVersion();

    QStringList arguments = app.arguments();
    arguments.removeAt(0); // remove application name from arguments list;
    QString message = arguments.join("<");

    bool commandQuit    = arguments.indexOf("--quit") != -1;
    bool commandLogout  = arguments.indexOf("--logout") != -1;

    if(app.isRunning())
    {
        if (app.sendMessage(message))
        {
            qDebug() << "can't sendMessage to running copy of application";
        }
        return 0;
    }
    else
    {
        if (commandQuit)
        {
            qDebug() << "want to quit, but no running instances found";
            return 0;
        }
    }

    // migrate settings: /////////////////////////////////////////
    // in previous version.
    // ApplicationName was "simplescreenshot"
    // OrganizationName was "BoxNet"
    // OrganizationDomain "box.net"
    // try to migrate settings to new registry key:
    // ApplicationName "SimpleShare"

    QString defaultuser;
    bool needMigrate = false;
    {
        app.setApplicationName("simplescreenshot");
        QSettings registry;
        needMigrate = registry.contains("defaltuser")
                || registry.contains("data/recentUpload0");
        defaultuser = registry.value("defaltuser", "").toString();
    }
    if (needMigrate)
    {
        Settings settings; // read
        app.setApplicationName("SimpleShare");
        settings.writeSettings(); // write
        {
            QSettings registry;
            registry.setValue("defaltuser", defaultuser);
        }

        app.setApplicationName("simplescreenshot");
        {
            QSettings registry;
            registry.remove("defaltuser");
            registry.remove("data");
            registry.remove("options");
        }
        app.setApplicationName("SimpleShare");
    }
    //////////////////////////////////////////////////////////////

    app.setApplicationName("SimpleShare");

    bool firstRun = false;
    {
        QSettings registry;
        firstRun = !registry.contains("options/automaticUpload");
    }
	BxNet* bxnet  = new BxNet();
    if (commandLogout)
    {
    
		
        bxnet->setApiKey(BOXNET_API_KEY);
        bxnet->logout();
        delete bxnet;

        return 0;
    }	
	

	bxnet->setApiKey(BOXNET_API_KEY);
	//initiate the OAuth2 handshake with the Box Server
	//bxnet->takeUsrToAuthorizePage();	

    //create the window
    PreferencesWidget preferencesWidget;

#ifndef Q_OS_WIN
    if (firstRun)
#endif
    {
        preferencesWidget.show(); // need to show window for correct menu handling!
        preferencesWidget.raise();
        preferencesWidget.activateWindow();
        
     /*   if (!firstRun && PreferencesWidget.boxnet()->authentificated())
        {
            preferencesWidget.close(); // but don't need that window
        }*/
        
    }

    if (!message.isEmpty())
    {
        preferencesWidget.handleMessage(message);
    }

    app.setActiveWindow(&preferencesWidget);

    app.setQuitOnLastWindowClosed(false);
    return app.exec();

}