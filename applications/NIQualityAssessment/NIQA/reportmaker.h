#ifndef REPORTMAKER_H
#define REPORTMAKER_H

#include <string>
#include <map>
#include <vector>

#include <QString>
#include <QtCharts/QChartView>
#include <QTextDocument>
#include <QTextCursor>

#include <math/statistics.h>

#include "niqaconfig.h"

class ReportMaker
{
public:
    ReportMaker();
    void makeReport(QString reportName, NIQAConfig &config);
    void addContrastInfo(QChartView *c, std::vector<kipl::math::Statistics> insets);
    void addEdge2DInfo(QChartView *c, QChartView *d, std::map<double, double> edges);
    void addEdge3DInfo(QChartView *c, double FWHM, double radius);
    void addBallsInfo();
private:
    void makeInfoSection();
    void makeContrastSection(bool active);
    void makeEdge2DSection(bool active);
    void makeEdge3DSection(bool active);
    void makeBallsSection(bool active);
    void insertFigure(QChartView *cv, QString imgname, int width, bool nl);
    void insertPageBreak();



    NIQAConfig mConfig;
    QChartView *contrast_plot;
    std::vector<kipl::math::Statistics> contrast_insets;

    QChartView *edge2d_edgeplots;
    QChartView *edge2d_collimation;
    std::map<double,double> edge2d_edges;

    QChartView *edge3d_edgeplot;
    double edge3d_FWHM;
    double edge3d_radius;

    QTextDocument doc;
    QTextCursor cursor;

};

#endif // REPORTMAKER_H
