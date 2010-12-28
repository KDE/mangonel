#ifndef Config_H
#define Config_H

#include <QDialog>
#include <QVBoxLayout>

class KKeySequenceWidget;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget* parent = 0);
    ~ConfigDialog();
public slots:
    void setHotkey(QKeySequence hotkey);
private:
    QVBoxLayout* layout;
    KKeySequenceWidget* hotkeySelect;
signals:
    void hotkeyChanged(const QKeySequence& hotkey);
};

#endif //Config_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
