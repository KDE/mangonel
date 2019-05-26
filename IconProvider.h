#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QQuickImageProvider>

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider();

    QPixmap requestPixmap(const QString &id, QSize *actualSize, const QSize &requestedSize) override;
    Flags flags() const override;
};

#endif // ICONPROVIDER_H
