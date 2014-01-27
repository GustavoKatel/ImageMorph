#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QDir>
#include <QListWidgetItem>

#include "imagehighlight.h"
#include "imagewrapper.h"

namespace Ui {
class ImagePreviewDialog;
}

class ImagePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    ImagePreviewDialog(QDir initDir);
    ~ImagePreviewDialog();

    virtual void accept();

    virtual void keyPressEvent(QKeyEvent *event);

    QString getSelectedImage();

private slots:
    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_up_button_clicked();

    void on_go_button_clicked();

    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::ImagePreviewDialog *ui;
    ImageHighlight *preview_widget;

    ImageWrapper *image;

    QDir curDir;
    QString selectedFile;

    void doList();
    void doPreview(QFile &file);
    bool checkValidFile(QFile &file);
};

#endif // IMAGEPREVIEWDIALOG_H
