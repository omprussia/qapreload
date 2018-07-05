#ifndef QAKEYENGINE_HPP
#define QAKEYENGINE_HPP

#include <QObject>

class QKeyEvent;
class QAKeyEngine : public QObject
{
    Q_OBJECT
public:
    explicit QAKeyEngine(QObject *parent = nullptr);

    void pressEnter(int count);
    void pressBackspace(int count);
    void pressKeys(const QString &keys);

signals:
    void triggered(QKeyEvent *event);

public slots:

private slots:
    void sendPress(const QChar &nkey, int key = 0);
    void sendRelease(const QChar &nkey, int key = 0);
};

#endif // QAKEYENGINE_HPP
