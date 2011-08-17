/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

/* JAS - 2011.08.14
 *   This is CrystalView... which is currently more or less just a 
 *   testbed for my PortalWidget Qt4 class.  All of this is in
 *   very early stages of development.
 */

#include "plm_config.h"
#include <iostream>
#include <QtGui>
#include "volume.h"
#include "mha_io.h"
#include "cview_portal.h"
#include "cview_main.h"

#define VERSION "0.04a"

/////////////////////////////////////////////////////////
// PortalGrid : public
//

PortalGrid::PortalGrid (Volume* input_vol, QWidget *parent)
    :QWidget (parent)
{
    /* Create a grid layout with splitters */
    QGridLayout *grid = new QGridLayout (this);

    QSplitter *splitterT = new QSplitter (Qt::Horizontal);
    QSplitter *splitterB = new QSplitter (Qt::Horizontal);
    QSplitter *splitterV = new QSplitter (Qt::Vertical);

    /* Create some portals */
    portal0 = new PortalWidget;
    portal1 = new PortalWidget;
    portal2 = new PortalWidget;
    portal3 = new PortalWidget;

    /* place portals inside splitters */
    splitterT->addWidget (portal0);
    splitterT->addWidget (portal1);
    splitterB->addWidget (portal2);
    splitterB->addWidget (portal3);
    splitterV->addWidget (splitterT);
    splitterV->addWidget (splitterB);


    /* Let's make a slider and connect it to portal1 */
    QSlider *slider1 = new QSlider (Qt::Vertical);
    slider1->setRange (0, 512);

    connect (slider1, SIGNAL(valueChanged(int)),
             portal1, SLOT(renderSlice(int)));

    connect (portal1, SIGNAL(sliceChanged(int)),
             slider1, SLOT(setValue(int)));

    /* Set the layout */
    grid->addWidget (splitterV, 0, 0);
    grid->addWidget (slider1, 0, 1);
    setLayout (grid);
}


/////////////////////////////////////////////////////////
// CrystalWindow : private
//

bool
CrystalWindow::openVol (const char* fn)
{
    if (input_vol) {
        delete input_vol;
    }
    input_vol = read_mha (fn);

    if (!input_vol) {
        return false;
    }

    volume_convert_to_float (input_vol);

    portalGrid->portal0->setVolume (input_vol);
    portalGrid->portal1->setVolume (input_vol);
    portalGrid->portal2->setVolume (input_vol);
    portalGrid->portal3->setVolume (input_vol);

    portalGrid->portal0->setView (PortalWidget::Axial);
    portalGrid->portal1->setView (PortalWidget::Coronal);
    portalGrid->portal2->setView (PortalWidget::Sagittal);
    portalGrid->portal3->setView (PortalWidget::Axial);

    return true;
}

void
CrystalWindow::createActions ()
{
    actionOpen = new QAction (tr("&Open"), this);
    actionExit = new QAction (tr("E&xit"), this);
    actionAboutQt = new QAction (tr("About &Qt"), this);

    connect (actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect (actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect (actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void
CrystalWindow::createMenu ()
{
    menuFile = menuBar()->addMenu (tr("&File"));
    menuFile->addAction (actionOpen);
    menuFile->addAction (actionExit);

    menuBar()->addSeparator();  /* fancy in some environments */

    menuHelp = menuBar()->addMenu (tr("&Help"));
    menuHelp->addAction (actionAboutQt);
}

/////////////////////////////////////////////////////////
// CrystalWindow : slots
//

void
CrystalWindow::open ()
{
    QString fileName =
        QFileDialog::getOpenFileName (
                this,
                tr("Open Volume"),
                "",
                tr("MHA Volumes (*.mha)")
    );

    QByteArray ba = fileName.toLocal8Bit();
    const char *fn = ba.data();

    /* Only attempt load if user selects a file */
    if (strcmp (fn, "")) {
        openVol (fn);
    }
}

/////////////////////////////////////////////////////////
// CrystalWindow : public
//

CrystalWindow::CrystalWindow (int argc, char** argv, QWidget *parent)
    :QMainWindow (parent)
{
    input_vol = NULL;

    createActions ();
    createMenu ();

    portalGrid = new PortalGrid (input_vol);
    setCentralWidget (portalGrid);

    /* open mha from command line */
    if (argc > 1) {
        if (!openVol (argv[1])) {
            std::cout << "Failed to load: " << argv[1] << "\n";
        }
    }
}

/////////////////////////////////////////////////////////
// main ()
//

int
main (int argc, char** argv)
{
    std::cout << "CrystalView " << VERSION << "\n"
              << "Usage: cview [mha_file]\n\n"
              << "  Application Hotkeys:\n"
              << "     1 - axial view\n"
              << "     2 - coronal view\n"
              << "     3 - sagittal view\n"
              << "   -/+ - change active slice\n"
              << "     [ - zoom in\n"
              << "     ] - zoom out\n\n"
              << "  Mouse Functions:\n"
              << "     scroll wheel - change active slice\n"
              << "      right click - pan/scroll\n"
              << "       left click - report coordinate (incorrect)\n\n";

    QApplication app (argc, argv);

    CrystalWindow cview_window (argc, argv);
    cview_window.setWindowTitle ("CrystalView " VERSION);
    cview_window.setMinimumSize (640, 480);
    cview_window.show ();

    return app.exec ();
}
