#ifndef LIPSTICKTESTHELPER_HPP
#define LIPSTICKTESTHELPER_HPP

#include <QObject>

class QQuickItem;
class LipstickTestHelper : public QObject
{
    Q_OBJECT
public:
    explicit LipstickTestHelper(QObject *parent = nullptr);

private:
    QQuickItem* m_pannable = nullptr;

signals:

public slots:
    void init();
};

#endif // LIPSTICKTESTHELPER_HPP
