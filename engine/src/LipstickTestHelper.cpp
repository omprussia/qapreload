#include "LipstickTestHelper.hpp"
#include "QAEngine.hpp"

#include <QQuickItem>
#include <QDebug>

LipstickTestHelper::LipstickTestHelper(QObject *parent) : QObject(parent)
{
    QVariantList pannables = QAEngine::findItemsByClassName(QStringLiteral("Pannable"));
    qWarning() << Q_FUNC_INFO << pannables;
    if (pannables.count() > 0) {
        m_pannable = pannables.first().value<QQuickItem*>();
    }
}

void LipstickTestHelper::init()
{
    qWarning() << Q_FUNC_INFO;
}
