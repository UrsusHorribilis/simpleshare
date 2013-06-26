#ifndef QAPPLICATIONEX_H
#define QAPPLICATIONEX_H

#include "qtsingleapplication.h"

class ApplicationEx : public QtSingleApplication
{
    Q_OBJECT
public:
    explicit ApplicationEx(int &argc, char **argv);
    virtual ~ApplicationEx();

#ifdef Q_OS_LINUX
    virtual bool x11EventFilter(XEvent* event);
    void setMouseListener(QWidget* widget);
    QWidget* mouseListener() const;
    void resetMouseListener();
#endif

    static void enableLogging(bool enable = true);
    static bool isLoggingEnabled();
    static QString logFileName();

#ifdef Q_OS_WIN
    void setSysTrayWindowTransparent(WId id);
#endif

protected:
#ifdef Q_OS_WIN
    virtual bool winEventFilter(MSG *msg, long *result);
#endif
    void initLocale();

signals:
    void powerResume();
    void powerSleep();

public slots:

private:
    QPoint      m_currentPos;
    QWidget*    m_mouseListener;

#ifdef Q_OS_WIN
    WId         m_transparentWId;
#endif
};

#endif // QAPPLICATIONEX_H
