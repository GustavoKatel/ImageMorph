#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imagewrapper.h"
#include "imagehighlight.h"

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slot_clear_all(QAction *action=0);
    void slot_imager_redraw();

    void slot_image1_newSegment(QPair<QPointF, QPointF> &segment);
    void slot_image2_newSegment(QPair<QPointF, QPointF> &segment);

    void slot_image1_update();
    void slot_image2_update();

    void slot_play_timer_timeout();

private slots:
    void on_image1_load_button_clicked();

    void on_image2_load_button_clicked();

    void on_image1_clearSeg_button_clicked();

    void on_image2_clearSeg_button_clicked();

    void on_actionAbout_Qt_triggered();

    void on_actionAbout_triggered();

    void on_a_value_spin_editingFinished();

    void on_b_value_spin_editingFinished();

    void on_p_value_spin_editingFinished();

    void slot_progress(int progress, int total);

    void on_image1_save_button_clicked();

    void on_image2_save_button_clicked();

    void on_image_weight_slider_sliderReleased();

    void on_image2_morph_check_toggled(bool checked);

    void on_image1_morph_check_toggled(bool checked);

    void on_play_button_clicked();

    void on_stop_button_clicked();

    void on_record_button_toggled(bool checked);

    void on_repeat_button_toggled(bool checked);

    void on_duration_spin_editingFinished();

    void on_trasitions_spin_editingFinished();

private:
    Ui::MainWindow *ui;
    //
    QProgressBar *progress;
    //
    ImageWrapper *image1, *image2, *imager, *image1t2, *image2t1;
    ImageHighlight *image1_widget, *image2_widget;
    bool image1_ready, image2_ready;

    QTimer *play_timer;
    bool isRecording, isRepeating;
    QDir rec_temp_dir;
    QFile *rec_output;
    int recordCount;
    int fps;

    virtual void resizeEvent(QResizeEvent *event);

    QFuture<void> _dm;
    void doMorph();
    void _doMorph();

    void updateImage1Segments();
    void updateImage2Segments();

    void play();
    void stop();
    void record_tick();
    void record_finish();
    void doFPS();
    void doFFMPEG();

};

#endif // MAINWINDOW_H
