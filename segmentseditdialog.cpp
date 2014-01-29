#include "segmentseditdialog.h"
#include "ui_segmentseditdialog.h"

SegmentsEditDialog::SegmentsEditDialog(ImageWrapper *image, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SegmentsEditDialog),
    image(image),
    multiSelect(false)
{
    ui->setupUi(this);

    init();

}

SegmentsEditDialog::~SegmentsEditDialog()
{
    delete ui;
}

int SegmentsEditDialog::selected()
{
    int res = -1;
    if(ui->listWidget->selectedItems().size()>0)
        res = ui->listWidget->currentIndex().row();
    return res;
}

QVector<int> SegmentsEditDialog::selecteds()
{
    QVector<int> res;
    return res;
}

bool SegmentsEditDialog::getMultiSelect() const
{
    return multiSelect;
}

void SegmentsEditDialog::setMultiSelect(bool value)
{
    multiSelect = value;
}

void SegmentsEditDialog::init()
{
    QPair<QPointF,QPointF> pair;
    for(int i=0;i<image->getSegments().size();i++)
    {
        pair = image->getSegments().at(i);
        ui->listWidget->addItem( QString("%1 - ( %2,%3 ) -> ( %4,%5 )")
                                 .arg(i+1)
                                 .arg(pair.first.x())
                                 .arg(pair.first.y())
                                 .arg(pair.second.x())
                                 .arg(pair.second.y())
                                 );
    }

    ui->listWidget->setCurrentRow(0);

}

