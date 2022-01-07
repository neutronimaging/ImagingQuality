//<LICENSE>

#include "niqamainwindow.h"
#include "ui_niqamainwindow.h"

#include <sstream>
#include <fstream>
#include <array>
#include <numeric>
#include <cmath>

#include <QSignalBlocker>
#include <QLineSeries>
#include <QPointF>
#include <QChart>
#include <QString>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QBoxSet>
#include <QBoxPlotSeries>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <armadillo>
#include <base/index2coord.h>

#include <fileset.h>
#include <imagereader.h>
#include <readerexception.h>
#include <base/tprofile.h>
#include <math/basicprojector.h>
#include <pixelsizedlg.h>
#include <strings/filenames.h>
#include <math/nonlinfit.h>
#include <math/sums.h>
#include <math/linfit.h>
#include <io/io_serializecontainers.h>
#include <profile/MicroTimer.h>
#include <profileextractor.h>
#include <stltools/stlvecmath.h>
#include <readerexception.h>
#include <base/KiplException.h>
#include <qglyphs.h>
#include <qmarker.h>

#include "edgefileitemdialog.h"
#include "reportmaker.h"

class EdgeFileListItem : public QListWidgetItem
{
public:
    EdgeFileListItem() {}
    EdgeFileListItem(const EdgeFileListItem &item) : QListWidgetItem(item) {}

    float distance;
    QString filename;
};

class EdgeInfoListItem : public QListWidgetItem
{
public:
    EdgeInfoListItem();
    EdgeInfoListItem(const EdgeInfoListItem &item);
    const EdgeInfoListItem & operator=(const EdgeInfoListItem &item);
    Nonlinear::Gaussian fitModel;
    std::vector<float> edge;
    std::vector<float> dedge;
    float distance;
    float FWHMpixels;
    float FWHMmetric;
};

NIQAMainWindow::NIQAMainWindow(QWidget *parent) :
    QMainWindow(parent),
    logger("NIQAMainWindow"),
    ui(new Ui::NIQAMainWindow),
    logdlg(new QtAddons::LoggingDialog(this)),
    configFileName("niqaconfig.xml")
{
    ui->setupUi(this);
    ui->groupBox_2Dreferences->hide(); // TODO implement normalization of edge images

    // Setup logging dialog
    logdlg->setModal(false);
    kipl::logging::Logger::AddLogTarget(*logdlg);

    ui->widget_roiEdge2D->setAllowUpdateImageDims(false);
    ui->widget_roiEdge2D->registerViewer(ui->viewer_edgeimages);
    ui->widget_roiEdge2D->setROIColor("red");
    ui->widget_roiEdge2D->setCheckable(true);
    ui->widget_roiEdge2D->setChecked(false);
    ui->viewer_edgeimages->hold_annotations(true);

    ui->widget_openBeamReader->setLabel("Open beam mask");
    ui->widget_darkCurrentReader->setLabel("Dark current mask");

    ui->widget_roi3DBalls->registerViewer(ui->viewer_Packing);
    ui->widget_roi3DBalls->setChecked(false);    

    ui->widget_insetrois->setViewer(ui->viewer_contrast);
    ui->widget_bundleroi->setViewer(ui->viewer_Packing);
    ui->widget_bundleroi->setLabelVisible(true);
    ui->widget_bundleroi->setRequireLabel(true);
    ui->widget_reportName->setFileOperation(false);
    loadCurrent();
    updateDialog();

}

NIQAMainWindow::~NIQAMainWindow()
{
    delete ui;
}

void NIQAMainWindow::on_button_bigball_load_clicked()
{
    saveCurrent();
    FileSet loader=ui->ImageLoader_bigball->getReaderConfig();

    ImageReader reader;

    try
    {
        m_BigBall=reader.Read(loader,
                              kipl::base::ImageFlipNone,
                              kipl::base::ImageRotateNone,
                              1.0f,
                              {});
    }
    catch (const kipl::base::KiplException & e)
    {
        QMessageBox::warning(this,"File error","Failed to load the image data. Please, check that the correct file name was provided.");
        logger.warning(e.what());
        return;
    }


    ui->slider_bigball_slice->setMinimum(0);
    ui->slider_bigball_slice->setMaximum(m_BigBall.Size(2)-1);
    ui->spin_bigball_slice->setMinimum(0);
    ui->spin_bigball_slice->setMaximum(m_BigBall.Size(2)-1);
    ui->slider_bigball_slice->setValue(m_BigBall.Size(2)/2);
    on_slider_bigball_slice_sliderMoved(m_BigBall.Size(2)/2);


}

void NIQAMainWindow::on_comboBox_bigball_plotinformation_currentIndexChanged(int index)
{
    plot3DEdgeProfiles(index);
}

void NIQAMainWindow::plot3DEdgeProfiles(int index)
{
    QLineSeries *series0 = new QLineSeries(); //Life time

    std::vector<float> profile;

    switch (index) {
    case 0: profile=m_edge3DProfile; break;
    case 1: profile=m_edge3DDprofile; break;
    }

    qDebug() << "plot 3d edge" <<index;
    for (auto dit=m_edge3DDistance.begin(), pit=profile.begin(); pit!=profile.end(); ++dit, ++pit) {
        series0->append(QPointF((*dit)*config.edgeAnalysis3D.pixelSize,*pit));
    }

    ui->chart_bigball->setCurveData(0,series0,"Global edge");
    ui->chart_bigball->hideLegend();
    ui->chart_bigball->setXLabel("Position [mm]");
}

void NIQAMainWindow::on_slider_bigball_slice_sliderMoved(int position)
{
    QSignalBlocker blocker(ui->spin_bigball_slice);

    ui->viewer_bigball->set_image(m_BigBall.GetLinePtr(0,position),m_BigBall.dims());
    ui->spin_bigball_slice->setValue(position);
}

void NIQAMainWindow::on_spin_bigball_slice_valueChanged(int arg1)
{
    QSignalBlocker blocker(ui->slider_bigball_slice);

    ui->slider_bigball_slice->setValue(arg1);
    on_slider_bigball_slice_sliderMoved(arg1);
}

void NIQAMainWindow::on_button_contrast_load_clicked()
{
    saveCurrent();
    std::ostringstream msg;

    FileSet loader=ui->ImageLoader_contrast->getReaderConfig();

    ImageReader reader;

    msg<<loader.m_sFilemask<<loader.m_nFirst<<", "<<loader.m_nLast;
    logger.message(msg.str());
    qDebug() << msg.str().c_str();
    try
    {
        m_Contrast=reader.Read(loader,
                               kipl::base::ImageFlipNone,
                               kipl::base::ImageRotateNone,
                               1.0f,
                               {});
    }
    catch (ReaderException &e)
    {
        QMessageBox::warning(this,"Load images failed",e.what());
        return ;
    }
    catch (kipl::base::KiplException &e)
    {
        QMessageBox::warning(this,"Load images failed",e.what());
        return ;
    }
    qDebug() << "Image loaded"<< m_Contrast.Size(0) << m_Contrast.Size(1)<< m_Contrast.Size(2);
    ui->slider_contrast_images->setMinimum(0);
    ui->slider_contrast_images->setMaximum(static_cast<int>(m_Contrast.Size(2)-1));
    ui->slider_contrast_images->setValue(static_cast<int>((m_Contrast.Size(2)-1)/2));

    ui->slider_contrast_images->setMinimum(0);
    ui->spin_contrast_images->setMaximum(static_cast<int>(m_Contrast.Size(2)-1));

    qDebug() << "pre update";
    on_slider_contrast_images_sliderMoved(static_cast<int>(m_Contrast.Size(2)/2));


    qDebug() << "pre analysis";
    m_ContrastSampleAnalyzer.setImage(m_Contrast);

    qDebug() << "analysis done";
    showContrastHistogram();
    ui->widget_insetrois->updateViewer();

}

void NIQAMainWindow::on_slider_contrast_images_sliderMoved(int position)
{
    QSignalBlocker blocker(ui->spin_contrast_images);

    ui->spin_contrast_images->setValue(position);

    ui->viewer_contrast->set_image(m_Contrast.GetLinePtr(0,position),m_Contrast.dims());

}

void NIQAMainWindow::on_spin_contrast_images_valueChanged(int arg1)
{
    QSignalBlocker blocker(ui->slider_contrast_images);

    ui->slider_contrast_images->setValue(arg1);

    on_slider_contrast_images_sliderMoved(arg1);
}

//void NIQAMainWindow::on_combo_contrastplots_currentIndexChanged(int index)
//{
//    switch (index) {
//        case 0: showContrastHistogram(); break;
//        case 1: showContrastBoxPlot(); break;
//    }
//}

void NIQAMainWindow::showContrastBoxPlot()
{
    std::ostringstream msg;
//    QChart *chart = new QChart(); // Life time
    std::vector<kipl::math::Statistics> stats=m_ContrastSampleAnalyzer.getStatistics();

    std::vector<QString> insetLbl;
    if (stats.empty())
    {
        logger.warning("ShowContrastBoxPlot: empty statistics list");
        return;
    }


    if (stats[1].E()/stats[0].E()<0.5)
        insetLbl = {"Ni","Al","Cu","Pb","Ti","Fe"};
    else
        insetLbl = {"Ni","Fe","Ti","Pb","Cu","Al"};

    QBoxPlotSeries *insetSeries = new QBoxPlotSeries();
    for (int i=0; i<6; ++i)
    {
        QBoxSet *set = new QBoxSet(insetLbl[i]);

        double slope=1.0;
        double intercept=0.0;

        qDebug() <<"bits per pixel"<<m_Contrast.info.nBitsPerSample;
        if (ui->groupBox_contrast_intensityMapping->isChecked()==true)
        {
            if (ui->radioButton_contrast_scaling->isChecked()==true)
            {
                slope=ui->spin_contrast_intensity0->value();
                intercept=ui->spin_contrast_intensity1->value();
            }
            if (ui->radioButton_contrast_interval->isChecked()==true)
            {
                double a=ui->spin_contrast_intensity0->value();
                double b=ui->spin_contrast_intensity1->value();
                switch (m_Contrast.info.nBitsPerSample)
                {
                case 8:
                    slope=(b-a)/255; break;
                case 16:
                    slope=(b-a)/65535; break;
                }
                intercept=a;
            }
        }
        set->setValue(QBoxSet::LowerExtreme,(stats[i].Min())*slope+intercept);
        set->setValue(QBoxSet::UpperExtreme,(stats[i].Max())*slope+intercept);
        set->setValue(QBoxSet::LowerQuartile,(stats[i].E()-stats[i].s()*1.96)*slope+intercept);
        set->setValue(QBoxSet::UpperQuartile,(stats[i].E()+stats[i].s()*1.96)*slope+intercept);
        set->setValue(QBoxSet::Median,(stats[i].E())*slope+intercept);
        insetSeries->append(set);
    }
    ui->chart_contrast->setDataSeries(0,insetSeries);
    ui->chart_contrast->setTitle("Elements");
    ui->chart_contrast->hideLegend();
}

void NIQAMainWindow::showContrastHistogram()
{
    std::vector<size_t> bins;
    std::vector<float>  axis;
    m_ContrastSampleAnalyzer.getHistogram(axis,bins);

    double slope=1.0;
    double intercept=0.0;
    qDebug() << "Hist bits per smaple"<<m_Contrast.info.nBitsPerSample;
    if (ui->groupBox_contrast_intensityMapping->isChecked()==true)
    {
        if (ui->radioButton_contrast_scaling->isChecked()==true)
        {
            slope=ui->spin_contrast_intensity0->value();
            intercept=ui->spin_contrast_intensity1->value();
        }
        if (ui->radioButton_contrast_interval->isChecked()==true)
        {
            double a=ui->spin_contrast_intensity0->value();
            double b=ui->spin_contrast_intensity1->value();
            switch (m_Contrast.info.nBitsPerSample)
            {
                case 8: slope=(b-a)/255.0; break;
                case 16: slope=(b-a)/65535.0; break;
            }
            intercept=a;
        }
        for (auto & x : axis)
            x = x*slope+intercept;
    }

    QLineSeries *series0 = new QLineSeries();

    if (axis.size() == bins.size())
    {
        auto axisItem = axis.begin();
        auto binItem  = bins.begin();

        for (; axisItem !=axis.end(); ++axisItem, ++binItem)
        {
            series0->append(QPointF(static_cast<qreal>(*axisItem),static_cast<qreal>(*binItem)));
        }

    }
    else
    {
        logger.warning("Histogram vectors were not equal size.");
        return;
    }

    ui->chart_contrastHistogram->setCurveData(0,series0);
    ui->chart_contrastHistogram->setXLabel("Image intensity");
    ui->chart_contrastHistogram->setTitle("Histogram");
    ui->chart_contrastHistogram->hideLegend();
}

void NIQAMainWindow::on_button_AnalyzeContrast_clicked()
{
    saveCurrent();

    std::list<kipl::base::RectROI> roiList=ui->widget_insetrois->getSelectedROIs();
    kipl::profile::MicroTimer timer;
    try
    {
        timer.Tic();
        m_ContrastSampleAnalyzer.saveIntermediateImages = false;
        m_ContrastSampleAnalyzer.analyzeContrast(ui->spin_contrast_pixelsize->value(),roiList);
        timer.Toc();
    }
    catch (kipl::base::KiplException &e)
    {
        QMessageBox::warning(this,"Contrast analysis failed",e.what());
    }
    std::ostringstream msg;
    msg<<timer;
    logger.message(msg.str());
    showContrastBoxPlot();
    auto insetPositions = m_ContrastSampleAnalyzer.getInsetCoordinates();
    qDebug() << "Inset positions size"<<insetPositions.size();
    int idx=0;
    for (const auto &pos : insetPositions)
    {
        ui->viewer_contrast->set_marker(QtAddons::QMarker(QtAddons::PlotGlyph_Plus,QPointF(pos.x,pos.y) , QColor("red")),idx++);
    }
    auto pos = m_ContrastSampleAnalyzer.centerCoordinate();
    ui->viewer_contrast->set_marker(QtAddons::QMarker(QtAddons::PlotGlyph_Plus,QPointF(pos.x,pos.y) , QColor("yellow")),idx++);
}

void NIQAMainWindow::on_button_addEdgeFile_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,"Select files to open",QDir::homePath());

    std::ostringstream msg;

    for (auto it=filenames.begin(); it!=filenames.end(); it++) {

        msg.str("");

        EdgeFileListItem *item = new EdgeFileListItem;

        item->distance  = 0.0f;
        item->filename  = *it;

        EdgeFileItemDialog dlg;

        dlg.setInfo(item->distance,item->filename);
        int res=dlg.exec();

        if (res==dlg.Accepted) {
            dlg.getInfo(item->distance,item->filename);
        }

        msg<<it->toStdString()<<std::endl<<"Edge distance="<<item->distance;
        item->setData(Qt::DisplayRole,QString::fromStdString(msg.str()));
        item->setData(Qt::CheckStateRole,Qt::Unchecked);

        item->setCheckState(Qt::CheckState::Checked);
        ui->listEdgeFiles->addItem(item);
    }
}



void NIQAMainWindow::on_button_deleteEdgeFile_clicked()
{
    QList<QListWidgetItem*> items = ui->listEdgeFiles->selectedItems();
    foreach(QListWidgetItem * item, items)
    {
        delete ui->listEdgeFiles->takeItem(ui->listEdgeFiles->row(item));
    }
}

void NIQAMainWindow::on_listEdgeFiles_doubleClicked(const QModelIndex &index)
{
    EdgeFileListItem *item = dynamic_cast<EdgeFileListItem *>(ui->listEdgeFiles->item(index.row()));

    EdgeFileItemDialog dlg;

    dlg.setInfo(item->distance,item->filename);
    int res=dlg.exec();

    if (res==dlg.Accepted) {
        dlg.getInfo(item->distance,item->filename);
        std::ostringstream msg;
        msg<<item->filename.toStdString()<<std::endl<<"Edge distance="<<item->distance;
        item->setData(Qt::DisplayRole,QString::fromStdString(msg.str()));
    }


}

void NIQAMainWindow::on_listEdgeFiles_clicked(const QModelIndex &index)
{
    EdgeFileListItem *item = dynamic_cast<EdgeFileListItem *>(ui->listEdgeFiles->item(index.row()));

    kipl::base::TImage<float,2> img;

    ImageReader reader;

    try
    {
        img=reader.Read(item->filename.toStdString(),
                        kipl::base::ImageFlipNone,
                        kipl::base::ImageRotateNone,
                        1.0f,
                        {});
    }
    catch (const kipl::base::KiplException & e)
    {
        QMessageBox::warning(this,"File error","Failed to load the image data. Please, check that the correct file name was provided.");
        logger.warning(e.what());
        return;
    }
    ui->viewer_edgeimages->set_image(img.GetDataPtr(),img.dims());
   // on_check_edge2dcrop_toggled(ui->check_edge2dcrop->isEnabled());

}

void NIQAMainWindow::on_button_LoadPacking_clicked()
{
    saveCurrent();
    FileSet loader=ui->imageloader_packing->getReaderConfig();

    ImageReader reader;

    std::vector<size_t> crop;

    if (ui->widget_roi3DBalls->isChecked())
    {
        ui->widget_roi3DBalls->getROI(crop);
    }

    try
    {
        m_BallAssembly=reader.Read(loader,
                                   kipl::base::ImageFlipNone,
                                   kipl::base::ImageRotateNone,
                                   1.0f,
                                   crop);
    }
    catch (const ReaderException &e)
    {
        QMessageBox::warning(this,"File error","[Reader] Failed to load the image data. Please, check that the correct file name was provided.");
        logger.warning(e.what());
        return;
    }
    catch (const kipl::base::KiplException & e) {
        QMessageBox::warning(this,"File error","[kipl] Failed to load the image data. Please, check that the correct file name was provided.");
        logger.warning(e.what());
        return;
    }

     QSignalBlocker blocker(ui->slider_PackingImages);
     ui->slider_PackingImages->setMinimum(0);
     ui->slider_PackingImages->setMaximum(static_cast<int>(m_BallAssembly.Size(2))-1);
     ui->slider_PackingImages->setValue(static_cast<int>(m_BallAssembly.Size(2)/2));
     on_slider_PackingImages_sliderMoved(static_cast<int>(m_BallAssembly.Size(2)/2));
     m_BallAssemblyProjection=kipl::math::BasicProjector<float>::project(m_BallAssembly,kipl::base::ImagePlaneXY);
     ui->widget_bundleroi->updateViewer();
}

void NIQAMainWindow::on_button_AnalyzePacking_clicked()
{
    saveCurrent();
    std::ostringstream msg;
    std::list<kipl::base::RectROI> roiList=ui->widget_bundleroi->getSelectedROIs();

    msg<<"Have "<<roiList.size()<<" ROIs";
    logger.message(msg.str());

    if (roiList.empty())
        return ;

    try
    {
        m_BallAssemblyAnalyzer.analyzeImage(m_BallAssembly,roiList);
    }
    catch (kipl::base::KiplException &e)
    {
        logger(logger.LogError, e.what());
        return;
    }

    auto roiStats=m_BallAssemblyAnalyzer.getStatistics();
    qDebug() << "ROI stats size" <<roiStats.size();

    plotPackingStatistics(roiStats);
}

void NIQAMainWindow::plotPackingStatistics(std::map<float,kipl::math::Statistics> &roiStats)
{
    std::vector<float> points;

    QLineSeries *series0 = new QLineSeries(); //Life time
    if (false)
    {
        foreach(auto stats, roiStats) {
            points.push_back(stats.second.s());
        }

        std::sort(points.begin(),points.end());

        float pos=0;
        foreach (auto point, points) {
            series0->append(QPointF(++pos,point));
        }
    }
    else {
        for (auto & points : roiStats)
        {
            series0->append(QPointF(points.first,points.second.s()));
        }
    }

    qDebug() << "Packing plot size"<<series0->count();
    ui->chart_packing->setCurveData(0,series0);
    ui->chart_packing->setXLabel("Ball diameter [mm]");
    ui->chart_packing->setYLabel("StdDev");
    ui->chart_packing->hideLegend();
}



void NIQAMainWindow::saveCurrent()
{
    ostringstream confpath;
    ostringstream msg;
    // Save current recon settings
    QDir dir;

    QString path=dir.homePath()+"/.imagingtools";

    logger.message(path.toStdString());
    if (!dir.exists(path)) {
        dir.mkdir(path);
    }
    std::string sPath=path.toStdString();
    kipl::strings::filenames::CheckPathSlashes(sPath,true);
    confpath<<sPath<<"CurrentNIQA.xml";

    try
    {
        updateConfig();
        std::ofstream confFile(confpath.str().c_str());
        if (!confFile.is_open())
        {
            msg.str("");
            msg<<"Failed to open config file: "<<confpath.str()<<" for writing.";
            logger.error(msg.str());
            return ;
        }
        std::string formattedConf = config.WriteXML();
        confFile << formattedConf;
        confFile.close();
        logger.message("Saved current recon config");
    }
    catch (kipl::base::KiplException &e) {
        msg.str("");
        msg<<"Saving current config failed with exception: "<<e.what();
        logger.error(msg.str());
        return;
    }
    catch (std::exception &e) {
        msg.str("");
        msg<<"Saving current config failed with exception: "<<e.what();
        logger.error(msg.str());
        return;
    }

}

void NIQAMainWindow::loadCurrent()
{
    std::ostringstream msg;

    QDir dir;

    std::string dname=dir.homePath().toStdString()+"/.imagingtools/CurrentNIQA.xml";
    kipl::strings::filenames::CheckPathSlashes(dname,false);

    QString defaultsname=QString::fromStdString(dname);
    msg.str("");
    msg<<"The config file "<<(dir.exists(defaultsname)==true ? "exists." : "doesn't exist.");
    logger.message(msg.str());
    bool bUseDefaults=true;
    if (dir.exists(defaultsname)==true)
    { // is there a previous recon?
        bUseDefaults=false;

        try
        {
            config.loadConfigFile(dname.c_str(),"niqa");
            msg.str("");
            msg<<config.WriteXML();
            logger.message(msg.str());
        }
        catch (kipl::base::KiplException &e)
        {
            msg.str("");
            msg<<"Loading defaults failed :\n"<<e.what();
            logger(kipl::logging::Logger::LogError,msg.str());
        }
        catch (std::exception &e)
        {
            msg.str("");
            msg<<"Loading defaults failed :\n"<<e.what();
            logger(kipl::logging::Logger::LogError,msg.str());
        }
    }
}

void NIQAMainWindow::updateConfig()
{
    // User information
    config.userInformation.userName  = ui->lineEdit_userName->text().toStdString();
    config.userInformation.institute = ui->lineEdit_institute->text().toStdString();
    config.userInformation.instrument = ui->lineEdit_instrument->text().toStdString();
    QDate date;
    date = ui->dateEdit_experimentDate->date();
    config.userInformation.experimentDate[0] = date.year();
    config.userInformation.experimentDate[1] = date.month();
    config.userInformation.experimentDate[2] = date.day();

    QString str=ui->plainTextEdit_comment->toPlainText();
    config.userInformation.comment = str.toStdString();

    date = ui->dateEdit_analysisDate->date();
    config.userInformation.analysisDate[0] = date.year();
    config.userInformation.analysisDate[1] = date.month();
    config.userInformation.analysisDate[2] = date.day();

    config.userInformation.country = ui->lineEdit_country->text().toStdString();
    config.userInformation.reportName = ui->widget_reportName->getFileName();

    // Contrast analysis
    FileSet loader;
    loader=ui->ImageLoader_contrast->getReaderConfig();
    config.contrastAnalysis.fileMask = loader.m_sFilemask;
    config.contrastAnalysis.first    = loader.m_nFirst;
    config.contrastAnalysis.last     = loader.m_nLast;
    config.contrastAnalysis.step     = loader.m_nStep;
    if (ui->radioButton_contrast_scaling) {
        config.contrastAnalysis.intensitySlope     = ui->spin_contrast_intensity0->value();
        config.contrastAnalysis.intensityIntercept = ui->spin_contrast_intensity1->value();
    }
    if (ui->radioButton_contrast_interval) {
        config.contrastAnalysis.intensityMin     = ui->spin_contrast_intensity0->value();
        config.contrastAnalysis.intensityMax     = ui->spin_contrast_intensity1->value();
    }

    config.contrastAnalysis.pixelSize          = ui->spin_contrast_pixelsize->value();
    config.contrastAnalysis.analysisROIs       = ui->widget_insetrois->getROIs();
    config.contrastAnalysis.makeReport         = ui->checkBox_reportContrast->isChecked();

    config.edgeAnalysis2D.multiImageList.clear();
    for(int i = 0; i < ui->listEdgeFiles->count(); ++i)
    {
        EdgeFileListItem *item = dynamic_cast<EdgeFileListItem *>(ui->listEdgeFiles->item(i));

        config.edgeAnalysis2D.multiImageList.insert(make_pair(item->distance,item->filename.toStdString()));
    }

    config.edgeAnalysis2D.normalize = ui->groupBox_2Dreferences->isChecked();

    loader = ui->widget_openBeamReader->getReaderConfig();
    config.edgeAnalysis2D.obMask = loader.m_sFilemask;
    config.edgeAnalysis2D.obFirst = loader.m_nFirst;
    config.edgeAnalysis2D.obLast = loader.m_nLast;
    config.edgeAnalysis2D.obStep = loader.m_nStep;

    loader = ui->widget_darkCurrentReader->getReaderConfig();
    config.edgeAnalysis2D.dcMask = loader.m_sFilemask;
    config.edgeAnalysis2D.dcFirst = loader.m_nFirst;
    config.edgeAnalysis2D.dcLast = loader.m_nLast;
    config.edgeAnalysis2D.dcStep = loader.m_nStep;
    config.edgeAnalysis2D.pixelSize = ui->doubleSpinBox_2dEdge_pixelSize->value();
    config.edgeAnalysis2D.fitFunction = ui->comboBox_edgeFitFunction->currentIndex();
    config.edgeAnalysis2D.useROI = ui->widget_roiEdge2D->isChecked();
    ui->widget_roiEdge2D->getROI(config.edgeAnalysis2D.roi);
    config.edgeAnalysis2D.makeReport = ui->checkBox_report2DEdge->isChecked();

    loader = ui->ImageLoader_bigball->getReaderConfig();
    config.edgeAnalysis3D.fileMask = loader.m_sFilemask;
    config.edgeAnalysis3D.first = loader.m_nFirst;
    config.edgeAnalysis3D.last  = loader.m_nLast;
    config.edgeAnalysis3D.step  = loader.m_nStep;
    config.edgeAnalysis3D.pixelSize = ui->dspin_bigball_pixelsize->value();
    config.edgeAnalysis3D.precision = ui->spin_bigball_precision->value();
    config.edgeAnalysis3D.profileWidth = ui->dspin_bigball_profileWidth->value();
    config.edgeAnalysis3D.makeReport =ui->checkBox_report3DEdge->isChecked();

    loader = ui->imageloader_packing->getReaderConfig();
    config.ballPackingAnalysis.fileMask = loader.m_sFilemask;
    config.ballPackingAnalysis.first    = loader.m_nFirst;
    config.ballPackingAnalysis.last     = loader.m_nLast;
    config.ballPackingAnalysis.step     = loader.m_nStep;
    config.ballPackingAnalysis.useCrop  = ui->widget_roi3DBalls->isChecked();
    config.ballPackingAnalysis.analysisROIs = ui->widget_bundleroi->getROIs();
    ui->widget_roi3DBalls->getROI(config.ballPackingAnalysis.roi);
    config.ballPackingAnalysis.makeReport = ui->checkBox_reportBallPacking->isChecked();
}

void NIQAMainWindow::updateDialog()
{
    std::ostringstream msg;
    // User information
    ui->lineEdit_userName->setText(QString::fromStdString(config.userInformation.userName));
    ui->lineEdit_institute->setText(QString::fromStdString(config.userInformation.institute));
    ui->lineEdit_instrument->setText(QString::fromStdString(config.userInformation.instrument));
    ui->dateEdit_experimentDate->setDate(QDate(config.userInformation.experimentDate[0],config.userInformation.experimentDate[1],config.userInformation.experimentDate[2]));
    ui->dateEdit_analysisDate->setDate(QDate(config.userInformation.analysisDate[0],config.userInformation.analysisDate[1],config.userInformation.analysisDate[2]));
    ui->label_version->setText(QString::fromStdString(config.userInformation.softwareVersion));
    ui->lineEdit_country->setText(QString::fromStdString(config.userInformation.country));
    ui->widget_reportName->setFileName(config.userInformation.reportName);

    // Contrast analysis
    FileSet loader;
    loader=ui->ImageLoader_contrast->getReaderConfig();
    loader.m_sFilemask = config.contrastAnalysis.fileMask;
    loader.m_nFirst    = config.contrastAnalysis.first;
    loader.m_nLast     = config.contrastAnalysis.last;
    loader.m_nStep     = config.contrastAnalysis.step;
    ui->ImageLoader_contrast->setReaderConfig(loader);

    if (ui->radioButton_contrast_scaling) {
        ui->spin_contrast_intensity0->setValue(config.contrastAnalysis.intensitySlope);
        ui->spin_contrast_intensity1->setValue(config.contrastAnalysis.intensityIntercept);
    }
    if (ui->radioButton_contrast_interval) {
        ui->spin_contrast_intensity0->setValue(config.contrastAnalysis.intensityMin);
        ui->spin_contrast_intensity1->setValue(config.contrastAnalysis.intensityMax);
    }
    ui->spin_contrast_pixelsize->setValue(config.contrastAnalysis.pixelSize);
    ui->checkBox_reportContrast->setChecked(config.contrastAnalysis.makeReport);
    ui->widget_insetrois->setROIs(config.contrastAnalysis.analysisROIs);
    ui->widget_insetrois->updateViewer();

    if (config.edgeAnalysis2D.multiImageList.empty()==false) {
        for (auto it=config.edgeAnalysis2D.multiImageList.begin(); it!=config.edgeAnalysis2D.multiImageList.end(); ++it) {
            EdgeFileListItem *item = new EdgeFileListItem;

            item->distance  = it->first;
            item->filename  = QString::fromStdString(it->second);
            msg.str("");
            msg<<it->second<<std::endl<<"Edge distance="<<it->first;
            item->setData(Qt::DisplayRole,QString::fromStdString(msg.str()));
            item->setData(Qt::CheckStateRole,Qt::Unchecked);

            item->setCheckState(Qt::CheckState::Checked);
            ui->listEdgeFiles->addItem(item);
        }
    }

    ui->groupBox_2Dreferences->setChecked(config.edgeAnalysis2D.normalize);

    loader.m_sFilemask = config.edgeAnalysis2D.obMask;
    loader.m_nFirst    = config.edgeAnalysis2D.obFirst;
    loader.m_nLast     = config.edgeAnalysis2D.obLast;
    loader.m_nStep     = config.edgeAnalysis2D.obStep;
    ui->widget_openBeamReader->setReaderConfig(loader);


    loader.m_sFilemask = config.edgeAnalysis2D.dcMask;
    loader.m_nFirst    = config.edgeAnalysis2D.dcFirst;
    loader.m_nLast     = config.edgeAnalysis2D.dcLast;
    loader.m_nStep     = config.edgeAnalysis2D.dcStep;
    ui->widget_darkCurrentReader->setReaderConfig(loader);

    ui->doubleSpinBox_2dEdge_pixelSize->setValue(config.edgeAnalysis2D.pixelSize);
    ui->widget_roiEdge2D->setChecked(config.edgeAnalysis2D.useROI);
    ui->widget_roiEdge2D->setROI(config.edgeAnalysis2D.roi);
    ui->comboBox_edgeFitFunction->setCurrentIndex(config.edgeAnalysis2D.fitFunction);
    ui->checkBox_report2DEdge->setChecked(config.edgeAnalysis2D.makeReport);

    loader.m_sFilemask = config.edgeAnalysis3D.fileMask;
    loader.m_nFirst = config.edgeAnalysis3D.first;
    loader.m_nLast = config.edgeAnalysis3D.last;
    loader.m_nStep = config.edgeAnalysis3D.step;
    ui->ImageLoader_bigball->setReaderConfig(loader);

    ui->dspin_bigball_pixelsize->setValue(config.edgeAnalysis3D.pixelSize);
    ui->spin_bigball_precision->setValue(config.edgeAnalysis3D.precision);
    ui->dspin_bigball_profileWidth->setValue(config.edgeAnalysis3D.profileWidth);
    ui->checkBox_report3DEdge->setChecked(config.edgeAnalysis3D.makeReport);

    loader.m_sFilemask = config.ballPackingAnalysis.fileMask;
    loader.m_nFirst = config.ballPackingAnalysis.first;
    loader.m_nLast = config.ballPackingAnalysis.last;
    loader.m_nStep = config.ballPackingAnalysis.step;
    ui->imageloader_packing->setReaderConfig(loader);
    ui->widget_roi3DBalls->setChecked(config.ballPackingAnalysis.useCrop);
    ui->widget_bundleroi->setROIs(config.ballPackingAnalysis.analysisROIs);
    ui->widget_roi3DBalls->setROI(config.ballPackingAnalysis.roi);
    ui->widget_bundleroi->updateViewer();
    ui->checkBox_reportBallPacking->setChecked(config.ballPackingAnalysis.makeReport);
}

void NIQAMainWindow::saveConfig(std::string fname)
{
    std::ostringstream msg;
    updateConfig();
    configFileName=fname;
    std::ofstream confFile(configFileName.c_str());

    if (!confFile.is_open())
    {
        msg.str("");
        msg<<"Failed to open config file: "<<configFileName<<" for writing.";
        logger.error(msg.str());
        return ;
    }
    confFile<<config.WriteXML();
    confFile.close();
}

void NIQAMainWindow::on_slider_PackingImages_sliderMoved(int position)
{

    switch (ui->combo_PackingImage->currentIndex()) {
        case 0: ui->slider_PackingImages->setEnabled(true);
                ui->viewer_Packing->set_image(m_BallAssembly.GetLinePtr(0,position),
                                              m_BallAssembly.dims());
                if (ui->widget_roi3DBalls->isChecked()) {
                    QRect roi;
                    ui->widget_roi3DBalls->getROI(roi);
                    ui->viewer_Packing->set_rectangle(roi,QColor("red"),0);
                }

                break;

        case 1:
                ui->slider_PackingImages->setEnabled(false);
                ui->viewer_Packing->set_image(m_BallAssemblyProjection.GetDataPtr(),
                                      m_BallAssemblyProjection.dims());

                break;
        case 2: ui->slider_PackingImages->setEnabled(true);
                ui->viewer_Packing->set_image(m_BallAssemblyAnalyzer.getMask().GetLinePtr(0,position),
                                              m_BallAssemblyAnalyzer.getMask().dims());
                break;
        case 3:
                {
                    ui->slider_PackingImages->setEnabled(true);
                    kipl::base::TImage<float,2> slice(m_BallAssembly.dims());
                    int *pSlice=m_BallAssemblyAnalyzer.getLabels().GetLinePtr(0,position);
                    std::copy_n(pSlice,slice.Size(),slice.GetDataPtr());
                    ui->viewer_Packing->set_image(slice.GetDataPtr(),slice.dims());
                    break;
                }
        case 4:
                ui->slider_PackingImages->setEnabled(true);
                ui->viewer_Packing->set_image(m_BallAssemblyAnalyzer.getDist().GetLinePtr(0,position),
                                                     m_BallAssemblyAnalyzer.getDist().dims());
                       break;

    }


}

void NIQAMainWindow::on_combo_PackingImage_currentIndexChanged(int index)
{
    (void) index;
    on_slider_PackingImages_sliderMoved(ui->slider_PackingImages->value());
}

void NIQAMainWindow::on_widget_roiEdge2D_getROIclicked()
{
    QRect rect=ui->viewer_edgeimages->get_marked_roi();

    ui->widget_roiEdge2D->setROI(rect);
    ui->viewer_edgeimages->set_rectangle(rect,QColor("red"),0);
}

void NIQAMainWindow::on_widget_roiEdge2D_valueChanged(int x0, int y0, int x1, int y1)
{
    QRect rect(x0,y0,x1-x0+1,y1-y0+1);
    ui->viewer_edgeimages->set_rectangle(rect,QColor("red"),0);
}

void NIQAMainWindow::on_button_get2Dedges_clicked()
{
    saveCurrent();
    getEdge2Dprofiles();
    plotEdgeProfiles();
    fitEdgeProfiles();
    estimateCollimation();

    ui->tabWidget_edge2D->setCurrentIndex(1);
}

void NIQAMainWindow::getEdge2Dprofiles()
{
    kipl::base::TImage<float,2> img;
    EdgeFileListItem *item = nullptr;

    saveCurrent();
    ImageReader reader;
    std::vector<size_t> crop;

    m_Edges2D.clear();
    m_DEdges2D.clear();

    ImagingQAAlgorithms::ProfileExtractor pe;
    pe.setPrecision(1.0f);

    if (ui->widget_roiEdge2D->isChecked())
        ui->widget_roiEdge2D->getROI(crop);

    std::map<float,float> pvec;
    for (int i=0; i<ui->listEdgeFiles->count(); ++i)
    {
        item = dynamic_cast<EdgeFileListItem *>(ui->listEdgeFiles->item(i));
        logger.message(item->filename.toStdString());
        if (item->checkState()==Qt::CheckState::Unchecked)
            continue;

        try
        {
            img=reader.Read(item->filename.toStdString(),
                            kipl::base::ImageFlipNone,
                            kipl::base::ImageRotateNone,
                            1.0f,
                            crop);
        }
        catch (kipl::base::KiplException &e)
        {
            qDebug() << e.what();
            logger.error("Failed to load image");
            return ;
        }

        pvec=pe.getProfile(img);
        if (pvec.rbegin()->second<pvec.begin()->second)
        {
            for (auto & pval : pvec)
                pval.second = -pval.second;
        }


        m_Edges2D[item->distance]=pvec;
        std::string fname="edge.csv";
        kipl::io::serializeMap(pvec,fname);
        m_DEdges2D[item->distance]=diff(pvec);
        kipl::io::serializeMap(m_DEdges2D[item->distance],"dedge.csv");
    }
}

void NIQAMainWindow::estimateResolutions()
{

}

void NIQAMainWindow::fitEdgeProfiles()
{
    std::ostringstream msg;
    int item_idx=0;

    const double FWHMconst=2*sqrt(2*log(2));
    ui->listWidget_edgeInfo->clear();
    for (auto & edgeItem :m_DEdges2D)
    {
        ++item_idx;
        auto edge=edgeItem.second;

        int nEdge=static_cast<int>(edge.size());
        arma::vec x(nEdge);
        arma::vec y(nEdge);
        arma::vec sig(nEdge);
        int i=0;

        for (auto edgeVal : edge)
        {
            sig[i]=1.0;
            x[i]=static_cast<double>(edgeVal.first);
            y[i]=static_cast<double>(edgeVal.second);
            ++i;
        }


        EdgeInfoListItem *edgeInfoItem = new EdgeInfoListItem;

        Nonlinear::LevenbergMarquardt mrqfit(0.001,5000);
        try {
            double maxval = -std::numeric_limits<double>::max();
            double minval =  std::numeric_limits<double>::max();
            int maxpos=0;
            int minpos=0;
            int idx=0;

            for (const auto & eitem : edge)
            {
                if (maxval<eitem.second)
                {
                    maxval=eitem.second;
                    maxpos=idx;
                }
                if (eitem.second< minval)
                {
                    minval=eitem.second;
                    minpos=idx;
                }
                idx++;
            }

            double halfmax=(maxval-minval)/2+minval;
            int HWHM=0;

            for (const auto & eitem : edge)
            {
                if (halfmax < eitem.second)
                    break;
                HWHM ++;
            }

            auto & fitModel = edgeInfoItem->fitModel;

            fitModel[0] = maxval;
            fitModel[1] = x[maxpos];
            fitModel[2] = (x[maxpos] - x[HWHM]) * 2;
            fitModel[3] = 0.0;

            qDebug() << edge.size()<< maxpos<< maxval<< minval << halfmax << HWHM ;
            qDebug() << "Fitter init"
                     << "ampl "  << fitModel[0]
                     << "pos "   << fitModel[1]
                     << "width " << fitModel[2];
            if (fitModel[2]<2)
            {
                logger.warning("Could not find FWHM, using constant =10");
                fitModel[2]=10.0;
            }

            mrqfit.fit(x,y,sig,fitModel);
            qDebug() << "Fitter done"
                     << "ampl "  << fitModel[0]
                     << "pos "   << fitModel[1]
                     << "width " << fitModel[2];
        }
        catch (kipl::base::KiplException &e)
        {
            logger.error(e.what());
            return ;
        }
        catch (std::exception &e)
        {
            msg.str("");
            msg<<"Failed with an stl exception "<< e.what();
            logger.message(msg.str());
            return ;
        }
        catch (...)
        {
            logger.message("An unknown exeption occurred.");
            return ;
        }

        msg.str("");
        msg<<edgeInfoItem->fitModel[0]<<", "<<edgeInfoItem->fitModel[1]<<", "<<edgeInfoItem->fitModel[2];
        logger.message(msg.str());

        edgeInfoItem->distance=edgeItem.first;
        edgeInfoItem->FWHMpixels=FWHMconst*edgeInfoItem->fitModel[2];
        edgeInfoItem->FWHMmetric=FWHMconst*config.edgeAnalysis2D.pixelSize*(edgeInfoItem->fitModel[2]);
        msg.str("");
        msg<<"distance="<<(edgeItem.first)<<"mm, FWHM="<<edgeInfoItem->FWHMmetric<<"mm ("<<edgeInfoItem->FWHMpixels<<" pixels)";
        logger.message(msg.str());
        edgeInfoItem->setData(Qt::DisplayRole,QString::fromStdString(msg.str()));

        ui->listWidget_edgeInfo->addItem(edgeInfoItem);
    }

    qDebug() << " list count"<< ui->listWidget_edgeInfo->count();

}

void NIQAMainWindow::fitEdgeProfile(std::vector<float> &dataX, std::vector<float> &dataY, std::vector<float> &dataSig, Nonlinear::FitFunctionBase &fitFunction)
{
    arma::vec x(dataX.size());
    arma::vec y(dataY.size());
    arma::vec sig(dataY.size());

    for (size_t i=0; i<dataY.size(); ++i) {
        x[i]=dataX[i];
        y[i]=dataY[i];
        if (dataSig.empty()==true)
            sig[i]=1.0;
        else
            sig[i]=dataSig[i];
    }

    fitEdgeProfile(x,y,sig,fitFunction);

}

void NIQAMainWindow::fitEdgeProfile(arma::vec &dataX, arma::vec &dataY, arma::vec &dataSig, Nonlinear::FitFunctionBase &fitFunction)
{
    std::ostringstream msg;
    Nonlinear::LevenbergMarquardt mrqfit(0.001,5000);
    try
    {
        double maxval = -std::numeric_limits<double>::max();
        double minval = std::numeric_limits<double>::max();
        int maxpos=0;
        int minpos=0;
        int idx=0;
        for (auto i=0; i<dataY.n_elem ; ++i)
        {
            if (maxval<dataY[i])
            {
                maxval=dataY[i];
                maxpos=idx;
            }
            if (dataY[i]< minval)
            {
                minval=dataY[i];
                minpos=idx;
            }
            idx++;
        }

        double halfmax=(maxval-minval)/2+minval;
        int HWHM=maxpos;

        for (; HWHM<dataY.n_elem; ++HWHM)
        {
            if (dataY[HWHM]<halfmax)
                break;
        }
        fitFunction[0]=maxval;
        fitFunction[1]=dataX[maxpos];
        fitFunction[2]=(dataX[HWHM]-dataX[maxpos])*2;
        double d=dataX[1]-dataX[0];
        if (fitFunction[2]<d)
        {
            logger.warning("Could not find FWHM, using constant 3*dx");
            fitFunction[2]=3*d;
        }
        mrqfit.fit(dataX,dataY,dataSig,fitFunction);

    }
    catch (kipl::base::KiplException &e)
    {
        logger.error(e.what());
        return ;
    }
    catch (std::exception &e)
    {
        logger.message(msg.str());
        return ;
    }
    qDebug() << "post fit";
    msg.str("");
    msg<<"Fitted data to "<<fitFunction[0]<<", "<<fitFunction[1]<<", "<<fitFunction[2];

    logger.message(msg.str());
}

void NIQAMainWindow::estimateCollimation()
{
    size_t N=static_cast<size_t>(ui->listWidget_edgeInfo->count());

    if (N < 3)
    {
        QMessageBox::warning(this,"Too few data points","The collimation estimate needs three or more edges to work. Please add more edge images.", QMessageBox::Ok);
        return ;
    }

    arma::vec y(N);
    arma::mat H(N,2);
    arma::mat C(N,N,arma::fill::eye);
    arma::vec param;

    QLineSeries *series = new QLineSeries(); //Life time

    const double eps=0.1;
    for (size_t i=0; i<N; ++i)
    {
        auto item = dynamic_cast<EdgeInfoListItem *>(ui->listWidget_edgeInfo->item(i));

        double distance = item->distance;
        double width    = item->FWHMmetric;

        y(i)   = width*width  ;
        H(i,0) = 1.0 ;
        H(i,1) = distance*distance;
        C(i,i) = 1.0/(distance+eps);

        qDebug() <<"Distance: "<<distance<<", Width: "<<width;
        series->append(QPointF(distance,width));
    }

    series->setName("Measured");
    ui->chart_collimation->setCurveData(0,series);
    kipl::math::weightedLSFit(H,C,y,param);

    double res = sqrt(param(0));
    double LD  = 1.0/sqrt(param(1));
    QString text;
    QTextStream(&text) << "Intrinsic resolution:" << res << " L/D:" <<LD;
    qDebug() << "param[0]:" << res << "param[1]:" <<LD;
    ui->label_collimationFit->setText(text);

    arma::vec fit=H*param;
    QLineSeries *fit_series = new QLineSeries();
    for (size_t i=0 ; i<N ; ++i) {
        double distance = sqrt(H(i,1));
        double width    = sqrt(fit(i));
        fit_series->append(QPointF(distance,width));
    }

    fit_series->setName("Fitted");
    ui->chart_collimation->setCurveData(1,fit_series);
    ui->chart_collimation->setXLabel("Distance");
    ui->chart_collimation->setYLabel("FWHM");
}

void NIQAMainWindow::plotEdgeProfiles()
{
    std::ostringstream msg;

    ui->chart_2Dedges->clearAllCurves();
    int idx=0;
    switch (ui->comboBox_edgePlotType->currentIndex())
    {
    case 0:
    {
        for (auto it = m_Edges2D.begin(); it!=m_Edges2D.end(); ++it,++idx)
        {
            QLineSeries *series = new QLineSeries(); //Life time

            msg.str("");
            msg<<it->first<<"mm";
            series->setName(msg.str().c_str());

            auto edge=it->second;

            for (auto dit=edge.begin(); dit!=edge.end(); ++dit)
            {
                series->append(static_cast<qreal>(dit->first),static_cast<qreal>(dit->second));
            }
            ui->chart_2Dedges->setCurveData(idx,series);
        }
        break;
    }
    case 1:
    {
        for (auto it = m_DEdges2D.begin(); it!=m_DEdges2D.end(); ++it,++idx)
        {
            QLineSeries *series = new QLineSeries(); //Life time

            msg.str("");
            msg<<it->first<<"mm";
            series->setName(msg.str().c_str());

            auto edge=it->second;
            int i=0;

            for (auto dit=edge.begin(); dit!=edge.end(); ++dit, ++i)
            {
                series->append(static_cast<qreal>(dit->first), static_cast<qreal>(dit->second));
            }

            ui->chart_2Dedges->setCurveData(idx,series);
        }
    }
        break;
    case 2:
    {

    }
    }
}

void NIQAMainWindow::on_widget_roi3DBalls_getROIclicked()
{
   QRect rect=ui->viewer_Packing->get_marked_roi();
   ui->widget_roi3DBalls->setROI(rect);
   ui->viewer_Packing->set_rectangle(rect,QColor("red"),0);
}

void NIQAMainWindow::on_widget_roi3DBalls_valueChanged(int x0, int y0, int x1, int y1)
{
    (void) x0;
    (void) y0;
    (void) x1;
    (void) y1;
    QRect roi;
    ui->widget_roi3DBalls->getROI(roi);

    ui->viewer_Packing->set_rectangle(roi,QColor("red"),0);
}

void NIQAMainWindow::on_pushButton_contrast_pixelSize_clicked()
{
    PixelSizeDlg dlg(this);

    int res=0;
    try {
        res=dlg.exec();
    }
    catch (kipl::base::KiplException &e) {
        QMessageBox::warning(this,"Warning","Image could not be loaded",QMessageBox::Ok);

        logger.warning(e.what());
        return;
    }

    if (res==dlg.Accepted) {
        ui->spin_contrast_pixelsize->setValue(dlg.pixelSize());
    }


}



void NIQAMainWindow::on_pushButton_2dEdge_pixelSize_clicked()
{
    PixelSizeDlg dlg(this);

    int res=0;
    try {
        res=dlg.exec();
    }
    catch (kipl::base::KiplException &e) {
        QMessageBox::warning(this,"Warning","Image could not be loaded",QMessageBox::Ok);

        logger.warning(e.what());
        return;
    }

    if (res==dlg.Accepted) {
        ui->doubleSpinBox_2dEdge_pixelSize->setValue(dlg.pixelSize());
    }
}

void NIQAMainWindow::on_actionNew_triggered()
{
    NIQAConfig conf;
    config = conf;
    updateDialog();
}

void NIQAMainWindow::on_actionLoad_triggered()
{
    logger.message("Load config");
    QString fname=QFileDialog::getOpenFileName(this,"Load configuration file",QDir::homePath());

    bool fail=false;
    std::string failmessage;

    try {
        config.loadConfigFile(fname.toStdString(),"niqa");
    }
    catch (kipl::base::KiplException &e) {
        fail=true;
        failmessage=e.what();
    }
    catch (std::exception &e) {
        fail=true;
        failmessage=e.what();
    }

    if (fail==true) {
        std::ostringstream msg;
        msg<<"Could not load config file\n"<<failmessage;
        logger.warning(msg.str());
        QMessageBox::warning(this,"Warning","Could not open specified config file",QMessageBox::Ok);
        return;
    }

    logger.message(config.WriteXML());
    updateDialog();
}

void NIQAMainWindow::on_actionSave_triggered()
{
    if (configFileName=="niqaconfig.xml") {
        on_actionSave_as_triggered();
    }
    else {
        saveConfig(configFileName);
    }
}

void NIQAMainWindow::on_actionSave_as_triggered()
{
    QString fname=QFileDialog::getSaveFileName(this,"Save configuration file",QDir::homePath());

    saveConfig(fname.toStdString());
}

void NIQAMainWindow::on_actionQuit_triggered()
{
    saveCurrent();
    close();
}


void NIQAMainWindow::on_pushButton_createReport_clicked()
{
    QMessageBox::warning(this,"Not implemented","The report generation is not implemented in this version of NIQA.");

    return;

//    updateConfig();
//    saveCurrent();
//    ReportMaker report;

//    //report.addContrastInfo(ui->chart_contrast,m_ContrastSampleAnalyzer.getStatistics());
//    std::map<double,double> edges;

//    for (int i=0; i<ui->listWidget_edgeInfo->count(); ++i) {
//        EdgeInfoListItem *item=dynamic_cast<EdgeInfoListItem *>(ui->listWidget_edgeInfo->item(i));
//        edges.insert(std::make_pair(item->distance,item->FWHMmetric));
//    }
//   // report.addEdge2DInfo(ui->chart_2Dedges,ui->chart_collimation,edges);
//    report.makeReport(QString::fromStdString(config.userInformation.reportName),config);

}

void NIQAMainWindow::on_comboBox_edgeFitFunction_currentIndexChanged(int index)
{
    (void)index;
}

void NIQAMainWindow::on_comboBox_edgePlotType_currentIndexChanged(int index)
{
    (void)index;
    plotEdgeProfiles();
}

EdgeInfoListItem::EdgeInfoListItem()
{

}

EdgeInfoListItem::EdgeInfoListItem(const EdgeInfoListItem &item) :
    QListWidgetItem(item),
    fitModel(item.fitModel),
    edge(item.edge),
    dedge(item.dedge),
    distance(item.distance),
    FWHMpixels(item.FWHMpixels),
    FWHMmetric(item.FWHMmetric)
{

}

const EdgeInfoListItem &EdgeInfoListItem::operator=(const EdgeInfoListItem &item)
{
    fitModel   = item.fitModel;
    edge       = item.edge;
    dedge      = item.dedge;
    distance   = item.distance;
    FWHMpixels = item.FWHMpixels;
    FWHMmetric = item.FWHMmetric;

    return *this;
}

void NIQAMainWindow::on_radioButton_contrast_interval_toggled(bool checked)
{
    if (checked==true) {
        ui->label_contrast_intensity0->setText("Min");
        ui->label_contrast_intensity1->setText("Max");
        ui->spin_contrast_intensity0->setValue(config.contrastAnalysis.intensityMin);
        ui->spin_contrast_intensity1->setValue(config.contrastAnalysis.intensityMax);
    }
}

void NIQAMainWindow::on_radioButton_contrast_scaling_toggled(bool checked)
{
    if (checked==true) {
        ui->label_contrast_intensity0->setText("Slope");
        ui->label_contrast_intensity1->setText("Intercept");
        ui->spin_contrast_intensity0->setValue(config.contrastAnalysis.intensitySlope);
        ui->spin_contrast_intensity1->setValue(config.contrastAnalysis.intensityIntercept);
    }
}


void NIQAMainWindow::on_spin_contrast_intensity0_valueChanged(double arg1)
{
    if (ui->radioButton_contrast_interval->isChecked()==true) {
        config.contrastAnalysis.intensityMin=ui->spin_contrast_intensity0->value();
    }
    if (ui->radioButton_contrast_scaling->isChecked()==true) {
        config.contrastAnalysis.intensitySlope=ui->spin_contrast_intensity0->value();
    }
}

void NIQAMainWindow::on_spin_contrast_intensity1_valueChanged(double arg1)
{
    if (ui->radioButton_contrast_interval->isChecked()==true) {
        config.contrastAnalysis.intensityMax=ui->spin_contrast_intensity1->value();
    }
    if (ui->radioButton_contrast_scaling->isChecked()==true) {
        config.contrastAnalysis.intensityIntercept=ui->spin_contrast_intensity1->value();
    }
}

void NIQAMainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this,"About NIQA","Developer: Anders Kaestner");
}

void NIQAMainWindow::on_actionUser_manual_triggered()
{
    QUrl url=QUrl("https://neutronimaging.github.io/ImagingQuality/");
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::critical(this,"Could not open user manual","NIQA could not open your web browser with the link https://neutronimaging.github.io/ImagingQuality/",QMessageBox::Ok);
    }
}

void NIQAMainWindow::on_actionReport_a_bug_triggered()
{
    QUrl url=QUrl("https://github.com/neutronimaging/ImagingQuality/issues");
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::critical(this,"Could not open repository","NIQA could not open your web browser with the link https://github.com/neutronimaging/ImagingQuality/issues",QMessageBox::Ok);
    }
}

void NIQAMainWindow::on_pushButton_bigball_pixelsize_clicked()
{
    PixelSizeDlg dlg(this);

    int res=0;
    try {
        res=dlg.exec();
    }
    catch (kipl::base::KiplException &e) {
        QMessageBox::warning(this,"Warning","Image could not be loaded",QMessageBox::Ok);

        logger.warning(e.what());
        return;
    }

    if (res==dlg.Accepted) {
        ui->dspin_bigball_pixelsize->setValue(dlg.pixelSize());
    }
}



void NIQAMainWindow::on_actionLogging_triggered()
{
        if (logdlg->isHidden()) {

            logdlg->show();
        }
        else {
            logdlg->hide();
        }
}

void NIQAMainWindow::on_button_bigball_analyze_clicked()
{
    if (m_BigBall.Size()==0)
    {
        QMessageBox::warning(this,"Data missing","Please load the data before analysis.");
        return;
    }
    m_BallAnalyzer.setImage(m_BigBall);
    float R=m_BallAnalyzer.getRadius();

    kipl::base::coords3Df center=m_BallAnalyzer.getCenter();

    std::ostringstream msg;
    msg<<"["<<center.x<<", "<<center.y<<", "<<center.z<<"]";
    ui->label_bigball_center->setText(QString::fromStdString(msg.str()));

    msg.str("");
    msg<<R<<" pixels = "<<R*ui->dspin_bigball_pixelsize->value();
    ui->label_bigball_radius->setText(QString::fromStdString(msg.str()));

    float width2=ui->dspin_bigball_profileWidth->value()*0.5;
    m_BallAnalyzer.getEdgeProfile(R-width2,R+width2,m_edge3DDistance,m_edge3DProfile,m_edge3DStdDev);

    size_t profileWidth2=m_edge3DProfile.size()/2;
    float s0=std::accumulate(m_edge3DProfile.begin(),m_edge3DProfile.begin()+profileWidth2-1,0.0f);
    float s1=std::accumulate(m_edge3DProfile.begin()+profileWidth2,m_edge3DProfile.end(),0.0f);

    float sign= s0<s1 ? 1.0f : -1.0f;
    m_edge3DDprofile.clear();

    for (size_t i=1; i<m_edge3DProfile.size(); ++i)
        m_edge3DDprofile.push_back(sign*(m_edge3DProfile[i]-m_edge3DProfile[i-1]));

    m_edge3DDprofile.push_back(m_edge3DDprofile.back());

    Nonlinear::Gaussian gaussian;
    std::vector<float> sig;

    fitEdgeProfile(m_edge3DDistance,m_edge3DDprofile,sig,gaussian);
    msg.str("");
    msg<<gaussian[2]*2 <<"pixels, "<<gaussian[2]*2*ui->dspin_bigball_pixelsize->value()<<" mm";
    ui->label_bigball_FWHM->setText(QString::fromStdString(msg.str()));

    plot3DEdgeProfiles(ui->comboBox_bigball_plotinformation->currentIndex());

}

void NIQAMainWindow::on_button_clearAllEdgeFiles_clicked()
{
    while (ui->listEdgeFiles->count())
    {
        delete ui->listEdgeFiles->takeItem(0);
    }
}

