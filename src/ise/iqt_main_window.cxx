/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <QtGui>
#include <QTimer>
#include <QFileDialog>
#include <QDir>
#include "iqt_video_widget.h"
//#include <vtkPolyDataMapper.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkSphereSource.h>
//#include "vtkSmartPointer.h"
#include "frame.h"
#include "synthetic_source_thread.h"
#include "iqt_synth_settings.h"
#include "iqt_application.h"
#include "iqt_main_window.h"

/* Some hints on displaying video.  It may be necessary to explore 
   multiple options.  Start with the QWidget option?

   Use a QGraphicsItem, which you put in a QGraphicsScene
   http://www.qtforum.org/article/35428/display-live-camera-video-in-a-graphics-item.html
   Use glTexture
   http://www.qtforum.org/article/20311/using-opengl-to-display-dib-bmp-format-from-video-for-windows.html
   Use a QWidget
   http://sourceforge.net/projects/qtv4lcapture/
   http://qt-apps.org/content/show.php/Qt+Opencv+webcam+viewer?content=89995
   http://doc.trolltech.com/qq/qq16-fader.html
   Use phonon
   http://doc.qt.nokia.com/latest/phonon-overview.html
*/

Iqt_main_window::Iqt_main_window ()
{
    /* Sets up the GUI */
    setupUi (this);

    /* Start the timer */
    //    m_qtimer = new QTimer (this);
    //    connect (m_qtimer, SIGNAL(timeout()), this, SLOT(slot_timer()));
    //    m_qtimer->start(1000);
    
    this->playing = false;

    /* Render a sphere ?? */
//    this->render_sphere ();
}

#if defined (commentout)
void
Iqt_main_window::render_sphere ()
{
    // sphere
    vtkSmartPointer<vtkSphereSource> sphereSource = 
	vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->Update();
    vtkSmartPointer<vtkPolyDataMapper> sphereMapper =
	vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
    vtkSmartPointer<vtkActor> sphereActor = 
	vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
 
    // VTK Renderer
    vtkSmartPointer<vtkRenderer> renderer = 
	vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(sphereActor);
 
    // VTK/Qt wedded
    this->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
};
#endif

Iqt_main_window::~Iqt_main_window ()
{
    QSettings settings;
    settings.sync ();

    //delete m_qtimer;
}

void
Iqt_main_window::slot_load ()
{
    if (playing) {
        this->slot_play_pause();
    }
    playing = false;
    filename = QFileDialog::getOpenFileName(this,
    	tr("Open File"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));
    //Iqt_video_widget::load();

    if (filename.isNull()) {
        return;
    }

    statusBar()->showMessage(QString("Filename: %1")
        .arg(filename));

    vid_screen->load(filename);
    //label->setText(QString("Filename: %1").arg(filename));
    this->slot_play_pause();
}

void
Iqt_main_window::slot_save ()
{
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_save() was called"));
}

void
Iqt_main_window::slot_play_pause ()
{   
    if (playing) {
	playing = false;
        play_pause_button->setText ("|>");
        action_Play->setText ("&Play");
    } else {
	playing = true;
        play_pause_button->setText ("||");
        action_Play->setText ("&Pause");
    }
    vid_screen->play(playing);
}

void
Iqt_main_window::slot_go_back ()
{
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_go_back() was called"));
}

void
Iqt_main_window::slot_go_forward ()
{
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_go_forward() was called"));
}

void
Iqt_main_window::slot_stop ()
{
    if (playing) {
	playing = false;
        play_pause_button->setText ("|>");
        action_Play->setText ("&Play");
    } else {
		QMessageBox::information (0, QString ("Info"), QString ("This video has already been stopped."));
	}
	vid_screen->stop();
}

void
Iqt_main_window::slot_synth ()
{
    Iqt_synth_settings iqt_synth_settings (this);
    iqt_synth_settings.exec();
}

void
Iqt_main_window::slot_synth_set (int a/*, int b, int c, int d, int e*/)
{
    qDebug("Setting 1: %d",a);
    /*qDebug("Setting 2: %d",b);
    qDebug("Setting 3: %d",c);
    qDebug("Setting 4: %d",d);
    qDebug("Setting 5: %d",e);
    */
}

void
Iqt_main_window::slot_timer ()
{
#if defined (commentout)
    this->video_widget;
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_timer() was called"));
#endif
    statusBar()->showMessage(QString("Num panels = %1")
        .arg(ise_app->num_panels));
}

void
Iqt_main_window::slot_frame_ready (Frame* f, int width, int height)
{
    qDebug ("Hello world");
    qDebug("Got frame %p", f);

    this->width = width;
    this->height = height;

    uchar *data = new uchar[width * height * 4];
    for (int i = 0; i < width * height; i++) {
        uchar val = (f->img[i] - 200) * 255.0 / (800-200);
        data[4*i+0] = 0xff;
        data[4*i+1] = val;
        data[4*i+2] = val;
        data[4*i+3] = val;
    }
    QImage qimage (data, width, height, QImage::Format_RGB32);
    vid_screen->set_qimage (qimage);
}
