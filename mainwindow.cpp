#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QColorDialog>
#include <QRgb>

#include "imagepreviewdialog.h"
#include "segmentseditdialog.h"
#include "imageutil.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // load and set style from resources
    QFile styleFile(":/styles/default.style");
    styleFile.open(QFile::ReadOnly | QFile::Text);
    QString style(styleFile.readAll());
    styleFile.close();

    qDebug() << style;
    qApp->setStyleSheet(style);

    // init values fo images
    QColor c(255, 255, 255, 0);
    QSize s = ui->image1_scrollArea->viewport()->size();
    image1 = new ImageWrapper(s, c);
    image2 = new ImageWrapper(s, c);
    imager = new ImageWrapper(":/images/images/monkey.png");
    image1t2 = 0;
    image2t1 = 0;

    image1_ready = image2_ready = false;

    image1_widget = new ImageHighlight(ui->image1_scrollArea, image1);
    image1_widget->setHighlight(false);

    image2_widget = new ImageHighlight(ui->image2_scrollArea, image2);
    image2_widget->setHighlight(false);

    ui->image1_scrollArea->setWidget(image1_widget);
    ui->image1_scrollArea->setWidgetResizable(false);

    ui->image2_scrollArea->setWidget(image2_widget);
    ui->image2_scrollArea->setWidgetResizable(false);

    // signals connect
    connect(imager, SIGNAL(update()), this, SLOT(slot_imager_redraw()) );
    connect( ui->actionClear_all, SIGNAL(triggered()), this, SLOT(slot_clear_all()) );

    connect( image1, SIGNAL(newSegment(QPair<QPointF,QPointF>&)), this, SLOT(slot_image1_newSegment(QPair<QPointF,QPointF>&)) );
    connect( image2, SIGNAL(newSegment(QPair<QPointF,QPointF>&)), this, SLOT(slot_image2_newSegment(QPair<QPointF,QPointF>&)) );

    connect( image1, SIGNAL(update()), this, SLOT(slot_image1_update()) );
    connect( image2, SIGNAL(update()), this, SLOT(slot_image2_update()) );

    slot_imager_redraw();

    progress = new QProgressBar(this);
    progress->setMaximum(100);
    progress->setValue(0);
    progress->setVisible(false);
    ui->statusBar->addWidget(progress, 1);

    play_timer = new QTimer();
    connect( play_timer, SIGNAL(timeout()), this, SLOT(slot_play_timer_timeout()) );

    isRecording = false;
    isRepeating = false;
    this->rec_output = 0;

    this->outside_color = QColor(255,255,255);
    ui->outside_color_button->setStyleSheet("background-color: #fff;");

}

MainWindow::~MainWindow()
{
    delete ui;

//    delete image1_widget;
//    delete image2_widget;

    delete image1;
    delete image2;
    delete imager;

    if(image1t2)
        delete image1t2;
    if(image2t1)
        delete image2t1;

    delete play_timer;

    if(rec_output)
        delete rec_output;

    delete progress;
}

void MainWindow::slot_clear_all(QAction *)
{
    QColor c(255, 255, 255, 0);
    image1->fill(c);
    image1->getSegments().clear();

    image2->fill(c);
    image2->getSegments().clear();

    imager->load(":/images/images/monkey.png");

    image1_ready = image2_ready = false;

    image1_widget->setHighlight(false);
    image2_widget->setHighlight(false);

    ui->image1_load_button->setEnabled(true);
    ui->image2_load_button->setEnabled(true);

    ui->image1_clearSeg_button->setEnabled(false);
    ui->image2_clearSeg_button->setEnabled(false);

    ui->image1_save_button->setEnabled(false);
    ui->image2_save_button->setEnabled(false);

    ui->image1_info_size_label->setText(tr("Size: %1x%2").arg(0).arg(0));
    ui->image2_info_size_label->setText(tr("Size: %1x%2").arg(0).arg(0));
}

void MainWindow::on_image1_load_button_clicked()
{
    if(this->_dm.isRunning())
        return;

    ImagePreviewDialog dialog(QDir::current());
    int res = dialog.exec();
    if(res)
    {
        QString filename = dialog.getSelectedImage();
        image1->load(filename);
        updateImage1Segments();

        // check if has to resize
        if(image2_ready)
        {
            if(image2->getSize().width()!=image1->getSize().width() || image2->getSize().height()!=image1->getSize().height())
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Warning"));
                msgBox.setInformativeText(tr("Image 2 has a different size."));
                QAbstractButton *img1Button = msgBox.addButton(tr("Resize image 1"), QMessageBox::YesRole);
                /*QAbstractButton *img2Button = */msgBox.addButton(tr("Resize image 2"), QMessageBox::NoRole);
                msgBox.setIcon(QMessageBox::Question);
                msgBox.exec();

                if(msgBox.clickedButton()==img1Button)
                {
                    image1->resize(image2->getSize().width(), image2->getSize().height());
                } else {
                    image2->resize(image1->getSize().width(), image1->getSize().height());
                }

            }
            image1_widget->setHighlight(true);
            image2_widget->setHighlight(true);

            doMorph();
        }
        image1_ready = true;
        ui->image1_load_button->setEnabled(false);
        ui->image1_clearSeg_button->setEnabled(true);
        ui->image1_save_button->setEnabled(true);

        ui->image1_info_size_label->setText(tr("Size: %1x%2").arg(image1->getSize().width()).arg(image1->getSize().height()));
    }
}

void MainWindow::on_image2_load_button_clicked()
{
    if(this->_dm.isRunning())
        return;

    ImagePreviewDialog dialog(QDir::current());
    int res = dialog.exec();
    if(res)
    {
        QString filename = dialog.getSelectedImage();
        image2->load(filename);
        updateImage2Segments();

        // check if has to resize
        if(image1_ready)
        {
            if(image2->getSize().width()!=image1->getSize().width() || image2->getSize().height()!=image1->getSize().height())
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Warning"));
                msgBox.setInformativeText(tr("Image 1 has a different size."));
                QAbstractButton *img1Button = msgBox.addButton(tr("Resize image 1"), QMessageBox::YesRole);
                /*QAbstractButton *img2Button = */msgBox.addButton(tr("Resize image 2"), QMessageBox::NoRole);
                msgBox.setIcon(QMessageBox::Question);
                msgBox.exec();

                if(msgBox.clickedButton()==img1Button)
                {
                    image1->resize(image2->getSize().width(), image2->getSize().height());
                } else {
                    image2->resize(image1->getSize().width(), image1->getSize().height());
                }
            }
            image1_widget->setHighlight(true);
            image2_widget->setHighlight(true);

            doMorph();
        }
        image2_ready = true;
        ui->image2_load_button->setEnabled(false);
        ui->image2_clearSeg_button->setEnabled(true);
        ui->image2_save_button->setEnabled(true);

        ui->image2_info_size_label->setText(tr("Size: %1x%2").arg(image2->getSize().width()).arg(image2->getSize().height()));
    }
}

void MainWindow::slot_imager_redraw()
{
    ui->imager_label->setPixmap(QPixmap::fromImage(*imager->getQImage()).scaled(ui->imager_label->size(), Qt::KeepAspectRatio));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    slot_imager_redraw();
}

void MainWindow::slot_image1_newSegment(QPair<QPointF, QPointF> &)
{
    updateImage1Segments();

    doMorph();
}

void MainWindow::slot_image2_newSegment(QPair<QPointF, QPointF> &)
{
    updateImage2Segments();

    doMorph();
}

void MainWindow::doMorph()
{
    // TODO thread here
//    _dm = QtConcurrent::run(this, &MainWindow::_doMorph );
    _doMorph();
}

void MainWindow::_doMorph()
{
    // check num of segments
    if(!image1->getSegments().size() || !image2->getSegments().size()) return;
    if(image1->getSegments().size()!=image2->getSegments().size()) return;

    ui->image1_morph_check->setEnabled(false);
    ui->image2_morph_check->setEnabled(false);

    image1_widget->setEnabled(false);
    image2_widget->setEnabled(false);

    double a = ui->a_value_spin->value(),
            b = ui->b_value_spin->value(),
            p = ui->p_value_spin->value();

//    imager->copy(image1);
    if(imager->getSize()!=image1->getSize())
        imager->resize(image1->getSize().width(), image1->getSize().height());

    this->progress->setVisible(true);

    ImageUtil iutil(outside_color);
    connect( &iutil, SIGNAL(warp_progress(int,int)), this, SLOT(slot_progress(int,int)) );
    connect( &iutil, SIGNAL(morph_progress(int,int)), this, SLOT(slot_progress(int,int)) );

    double t = ui->image_weight_slider->value() / 100.0;

    if(image1t2)
        delete image1t2;

    if(image2t1)
        delete image2t1;

    QColor c(0,0,0);
    QSize size = image1->getSize();
    image1t2 = new ImageWrapper( size, c );
    image2t1 = new ImageWrapper( size, c );

    if(ui->image1_morph_check->isChecked())
    {
        this->progress->setFormat("Warp image1 -> 2 (%p%)");
        iutil.warp(image1, image2, image1t2, a, b, p, t);
    }

    if(ui->image2_morph_check->isChecked())
    {
        this->progress->setFormat("Warp image2 -> 1 (%p%)");
        iutil.warp(image2, image1, image2t1, a, b, p, 1-t);
    }

    if(!ui->image1_morph_check->isChecked())
        t = 1;
    if(!ui->image2_morph_check->isChecked())
        t = 0;

    this->progress->setFormat("Morph (%p%)");
    iutil.morph(image2t1, image1t2, imager, t);

    delete image1t2;
    image1t2 = 0;

    delete image2t1;
    image2t1 = 0;

//    ImageWrapper tmp(*imager);
//    qDebug()<<"interpolation";
//    iutil.bilinearInterpolation(&tmp, imager, 2);

    this->progress->setVisible(false);
    ui->image1_morph_check->setEnabled(true);
    ui->image2_morph_check->setEnabled(true);
    image1_widget->setEnabled(true);
    image2_widget->setEnabled(true);

}

void MainWindow::on_image1_clearSeg_button_clicked()
{
    if(this->_dm.isRunning())
        return;

    image1->getSegments().clear();
    image1_widget->repaint();
    updateImage1Segments();
}

void MainWindow::on_image2_clearSeg_button_clicked()
{
    if(this->_dm.isRunning())
        return;

    image2->getSegments().clear();
    image2_widget->repaint();
    updateImage2Segments();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(0);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(0, tr("About"), tr("Developed by Gustavo B. Sampaio as a part his studies during the Digital Image Processing Course at Federal University of Paraiba."));
}

void MainWindow::slot_image1_update()
{
    ui->image1_info_size_label->setText(tr("Size: %1x%2").arg(image1->getSize().width()).arg(image1->getSize().height()));
}

void MainWindow::slot_image2_update()
{
    ui->image2_info_size_label->setText(tr("Size: %1x%2").arg(image2->getSize().width()).arg(image2->getSize().height()));
}

void MainWindow::on_a_value_spin_editingFinished()
{
    doMorph();
}

void MainWindow::on_b_value_spin_editingFinished()
{
    doMorph();
}

void MainWindow::on_p_value_spin_editingFinished()
{
    doMorph();
}

void MainWindow::slot_progress(int progress, int total)
{
    double percent = ((double)progress) / total;
    percent *= 100;
    this->progress->setValue(percent);
}

void MainWindow::updateImage1Segments()
{
    ui->image1_info_label->setText( QString( tr("Segments: %1") ).arg(image1->getSegments().size()) );
}

void MainWindow::updateImage2Segments()
{
    ui->image2_info_label->setText( QString( tr("Segments: %1") ).arg(image2->getSegments().size()) );
}

void MainWindow::on_image1_save_button_clicked()
{
    QFileDialog fd(this, tr("Save image"), QDir::homePath(), "ImageMorph Structure (*.ims)" );
    fd.setAcceptMode(QFileDialog::AcceptSave);
    if(fd.exec())
    {
        QString filename = fd.selectedFiles().at(0);
        image1->save(filename);
    }
}

void MainWindow::on_image2_save_button_clicked()
{
    QFileDialog fd(this, tr("Save image"), QDir::homePath(), "ImageMorph Structure (*.ims)" );
    fd.setAcceptMode(QFileDialog::AcceptSave);
    if(fd.exec())
    {
        QString filename = fd.selectedFiles().at(0);
        image2->save(filename);
    }
}

void MainWindow::on_image_weight_slider_sliderReleased()
{
    doMorph();
}

void MainWindow::on_image2_morph_check_toggled(bool checked)
{
    if(!checked && !ui->image1_morph_check->isChecked())
        ui->image1_morph_check->setChecked(true);
    doMorph();
}

void MainWindow::on_image1_morph_check_toggled(bool checked)
{
    if(!checked && !ui->image2_morph_check->isChecked())
        ui->image2_morph_check->setChecked(true);
    doMorph();
}

void MainWindow::on_play_button_clicked()
{
    play();
}

void MainWindow::on_stop_button_clicked()
{
    stop();
    isRecording = false;
    ui->record_button->setChecked(false);
}

void MainWindow::on_record_button_toggled(bool checked)
{
    isRecording = checked;
    ui->repeat_button->setEnabled(!checked);
    if(checked)
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("Select output file"), "", "MKV Video (*.mkv)");

        if(filename.isEmpty())
        {
            ui->record_button->blockSignals(true);
            ui->record_button->setChecked(false);
            ui->record_button->blockSignals(false);
            return;
        }

        this->rec_output = new QFile(filename);

        stop();
        ui->repeat_button->setChecked(false);
        isRepeating = false;
        ui->image_weight_slider->setValue(0);
        doMorph();

        recordCount=0;
        rec_temp_dir = QDir::tempPath();
        if(!rec_temp_dir.cd("ImageMorph"))
        {
            rec_temp_dir.mkdir("ImageMorph");
            rec_temp_dir.cd("ImageMorph");
        } else {
            rec_temp_dir.removeRecursively();
            rec_temp_dir.cd("..");
            rec_temp_dir.mkdir("ImageMorph");
            rec_temp_dir.cd("ImageMorph");
        }

    }
}

void MainWindow::on_repeat_button_toggled(bool checked)
{
    isRepeating = checked;
}

void MainWindow::slot_play_timer_timeout()
{
    int v = ui->image_weight_slider->value();
    double step = 100.0 / ui->trasitions_spin->value();

    int nv = ((double)v) + step;

    ui->image_weight_slider->setValue( std::min(100, nv) );
    doMorph();

    if(isRepeating)
    {
        if(nv>100)
            nv = 0;
    } else {
        if(nv>100)
            this->play_timer->stop();
    }

    if(isRecording)
    {
        if(nv>100)
        {
            stop();
            record_finish();
        } else {
            record_tick();
        }
    }

}

void MainWindow::play()
{
    ui->duration_spin->setEnabled(false);
    ui->trasitions_spin->setEnabled(false);
    ui->image_weight_slider->setValue(0);

    ui->image1_segments_edit_button->setEnabled(false);
    ui->image2_segments_edit_button->setEnabled(false);

    play_timer->start();
}

void MainWindow::stop()
{
    play_timer->stop();
    ui->duration_spin->setEnabled(true);
    ui->trasitions_spin->setEnabled(true);

    ui->image1_segments_edit_button->setEnabled(true);
    ui->image2_segments_edit_button->setEnabled(true);

}

void MainWindow::record_tick()
{
    //int width = QString::number(ui->trasitions_spin->value()).size();
    QString filename = rec_temp_dir.absoluteFilePath( QString("temp%1.jpg").arg( QString::number(recordCount++) ) );
    imager->save(filename);
}

void MainWindow::record_finish()
{
    record_tick();
    doFFMPEG();
    this->recordCount=0;
    ui->record_button->setChecked(false);


    ui->image1_segments_edit_button->setEnabled(true);
    ui->image2_segments_edit_button->setEnabled(true);
}

void MainWindow::doFPS()
{
    this->fps = ui->trasitions_spin->value() / ui->duration_spin->value();
    int step = 1000/this->fps;
    play_timer->setInterval(step);
}

void MainWindow::on_duration_spin_editingFinished()
{
    doFPS();
}

void MainWindow::on_trasitions_spin_editingFinished()
{
    doFPS();
}

void MainWindow::doFFMPEG()
{
//    QString cmd = "ffmpeg -y -itsoffset 0.5 -r %1 -i \"%2\" -c:v libx264 -r %3 \"%4\"";
//    QString cmd = "ffmpeg -y -itsoffset 0.5 -r %1 -i \"%2\" -r %3 \"%4\"";
    QString cmd = "avconv -y -itsoffset 0.5 -r %1 -i \"%2\" -r %3 \"%4\"";
    QProcess ffmpeg;

    if(!this->rec_output->fileName().endsWith(".mkv"))
        this->rec_output->setFileName(this->rec_output->fileName()+".mkv");

    int fps_fix = this->recordCount / ui->duration_spin->value();

    cmd = cmd.arg(fps_fix)
            .arg( this->rec_temp_dir.absoluteFilePath("temp%d.jpg") )
            .arg(fps_fix)
            .arg(this->rec_output->fileName());

    qDebug()<<cmd;

    ffmpeg.execute(cmd);

}

void MainWindow::on_outside_color_button_clicked()
{
    QRgb rgba = QColorDialog::getRgba();
    this->outside_color = QColor(rgba);
    QString ssColor = QString("background-color: %1;").arg(QColor(rgba).name());
    ui->outside_color_button->setStyleSheet(ssColor);
}

void MainWindow::on_image1_segments_edit_button_clicked()
{
    SegmentsEditDialog dlg(image1);
    if(dlg.exec())
    {
        int toRem = dlg.selected();
        if(toRem < image1->getSegments().size() && toRem >= 0)
        {
            image1->getSegments().remove(toRem);
            updateImage1Segments();
            image1_widget->repaint();
        }
    }
}

void MainWindow::on_image2_segments_edit_button_clicked()
{
    SegmentsEditDialog dlg(image2);
    if(dlg.exec())
    {
        int toRem = dlg.selected();
        if(toRem < image2->getSegments().size() && toRem >= 0)
        {
            image2->getSegments().remove(toRem);
            updateImage2Segments();
            image2_widget->repaint();
        }
    }
}

void MainWindow::on_actionSave_result_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save result"), "", "JPEG Image (*.jpg)");
    if(!filename.isEmpty())
        imager->save(filename);
}
