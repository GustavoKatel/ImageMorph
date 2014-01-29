#include "imagepreviewdialog.h"
#include "ui_imagepreviewdialog.h"

#include <QFile>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

#include "imagewrapper.h"

ImagePreviewDialog::ImagePreviewDialog(QDir initDir) :
    QDialog(0),
    ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);

    QColor c(255,255,255,0);
    QSize s(20,20);
    image = new ImageWrapper(s, c);
    preview_widget = new ImageHighlight(this, image);
    preview_widget->setMaximumHeight(200);

    ui->preview_layout->addWidget(preview_widget);

    curDir = QDir(initDir);
    ui->current_dir_text->setText(curDir.absolutePath());

    doList();
}

ImagePreviewDialog::~ImagePreviewDialog()
{
    delete ui;
    delete image;
    delete preview_widget;
}

void ImagePreviewDialog::doList()
{
    ui->listWidget->clear();
    QStringList list = curDir.entryList();
    for(int i=0;i<list.size();i++)
    {
        ui->listWidget->addItem(list.at(i));
    }
    ui->current_dir_text->setText(curDir.absolutePath());
}

void ImagePreviewDialog::on_listWidget_doubleClicked(const QModelIndex &index)
{
    QString item = index.data().toString();
    QDir dir = curDir;
    if(dir.cd(item))
    {
        curDir = dir;
        doList();
    } else {
        accept();
    }
}

void ImagePreviewDialog::on_up_button_clicked()
{
    if(curDir.cd(".."))
        doList();
}

void ImagePreviewDialog::on_go_button_clicked()
{
    if(ui->listWidget->hasFocus() && ui->listWidget->selectedItems().size()>0)
    {
        QString item = ui->listWidget->selectedItems().at(0)->text();      
        QFileInfo info(curDir.absoluteFilePath(item));
        if( info.exists() && info.isFile() )
        {
            accept();
            return;
        }
        ui->current_dir_text->setText( curDir.absoluteFilePath( item ) );
    }

    QDir d(ui->current_dir_text->text());
    if(d.exists())
        curDir = d;
    doList();
}

void ImagePreviewDialog::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if(!current) return;
    QString item = current->text();
    QFile file( curDir.absoluteFilePath(item) );
    if(checkValidFile(file))
    {
        doPreview(file);
    } else {
        QColor c(255,255,255,0);
        image->fill(c);
//        QPixmap pix(1,1);
//        pix.fill(c);
//        ui->info_image_preview_label->setPixmap(pix);
        ui->info_bitspixel_value_label->setText("0 bits");
        ui->info_dim_value_label->setText("0 x 0 px");
    }
}

void ImagePreviewDialog::doPreview(QFile &file)
{
    QString filename = file.fileName();
    image->load(filename);
    QSize size = image->getSize();
    ui->info_dim_value_label->setText( QString::number(size.width()) + " x " + QString::number(size.height()) + " px" );
    ui->info_bitspixel_value_label->setText( QString::number( image->getQImage()->depth()) + " bits" );
}

void ImagePreviewDialog::accept()
{
    if(!ui->listWidget->currentItem())
    {
        QMessageBox::information(0, tr("Alert"), tr("Please, select a file!"));
        return;
    }
    QString item = ui->listWidget->currentItem()->text();
    QFile file( curDir.absoluteFilePath(item) );
    if(checkValidFile(file))
    {
        selectedFile = file.fileName();
        QDialog::accept();
    } else {
        QMessageBox::information(0, tr("Alert"), tr("No suitable file selected!"));
    }
}

bool ImagePreviewDialog::checkValidFile(QFile &file)
{
    bool fileExtValid = file.fileName().endsWith(".jpg") ||
            file.fileName().endsWith(".png") ||
            file.fileName().endsWith(".bmp") ||
            file.fileName().endsWith(".ims");

    return file.exists() && fileExtValid;
}

QString ImagePreviewDialog::getSelectedImage()
{
    return selectedFile;
}

void ImagePreviewDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return && ( ui->current_dir_text->hasFocus() || ui->listWidget->hasFocus() ) )
        on_go_button_clicked();
    else
        QDialog::keyPressEvent(event);
}
