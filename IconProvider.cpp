#include "IconProvider.h"
#include <QDebug>
#include <QIcon>

IconProvider::IconProvider() :
    QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

QPixmap IconProvider::requestPixmap(const QString &id, QSize *actualSize, const QSize &requestedSize)
{
    QSize size(requestedSize);
    if (size.height() <= 0) {
        size.setHeight(128);
    }
    if (size.width() <= 0) {
        size.setWidth(128);
    }

    QIcon icon(QIcon::fromTheme(id));
    QPixmap pixmap(icon.pixmap(qMin(size.height(), size.width())));

    if (actualSize) {
        *actualSize = pixmap.size();
    }

    return pixmap;
}


QQmlImageProviderBase::Flags IconProvider::flags() const
{
    return QQmlImageProviderBase::ForceAsynchronousImageLoading;
}
