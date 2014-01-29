#ifndef SEGMENTSEDITDIALOG_H
#define SEGMENTSEDITDIALOG_H

#include <QDialog>
#include <QVector>

#include "imagewrapper.h"

namespace Ui {
class SegmentsEditDialog;
}

class SegmentsEditDialog : public QDialog
{
    Q_OBJECT

public: 
    explicit SegmentsEditDialog(ImageWrapper *image, QWidget *parent = 0);
    ~SegmentsEditDialog();

    int selected();
    QVector<int> selecteds();

    bool getMultiSelect() const;
    void setMultiSelect(bool value);

private:
    Ui::SegmentsEditDialog *ui;

    bool multiSelect;
    ImageWrapper *image;

    void init();
};

#endif // SEGMENTSEDITDIALOG_H
